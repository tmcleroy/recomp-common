// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// Windows' own requests (launcher/src/winhttp_curl.h) against GitHub, as the launcher's updates and the online
// requests make them, with curl's exit codes and output. Windows only, with the internet (CI's Windows job).
#include "../launcher/src/winhttp_curl.h"

#include <cstdio>
#include <filesystem>

static int failures = 0;
#define CHECK(x)                                                        \
    do {                                                                \
        if (!(x)) {                                                     \
            std::printf("%s:%d: failed: %s\n", __FILE__, __LINE__, #x); \
            failures++;                                                 \
        }                                                               \
    } while (0)

int main() {
    using launcher::winhttp_curl;
    std::string out;
    // (update.cpp's fetch_text)
    int code = winhttp_curl({"-fsSL", "--connect-timeout", "15", "--max-time", "60", "-H", "Accept: application/vnd.github+json", "-A",
                             "recomp-launcher", "https://api.github.com/repos/tmcleroy/recomp-common"},
                            &out);
    std::printf("text: %d, %zu bytes\n", code, out.size());
    CHECK(code == 0 && out.find("\"full_name\"") != std::string::npos);
    // (update.cpp's fetch_file: a redirect to another host, into a file)
    const std::filesystem::path file = std::filesystem::temp_directory_path() / "winhttp_test.zip";
    code = winhttp_curl({"-fsSL", "--connect-timeout", "15", "--speed-limit", "1024", "--speed-time", "60", "--retry", "2", "-A", "recomp-launcher",
                         "-o", file.u8string(), "https://github.com/tmcleroy/recomp-common/archive/refs/heads/main.zip"},
                        nullptr);
    std::error_code error;
    const auto size = std::filesystem::file_size(file, error);
    std::printf("file: %d, %llu bytes\n", code, static_cast<unsigned long long>(size));
    CHECK(code == 0 && !error && size > 1000);
    std::filesystem::remove(file, error);
    // (-f and an error status: 22)
    out.clear();
    code = winhttp_curl({"-fsSL", "-A", "recomp-launcher", "https://api.github.com/repos/tmcleroy/no-such-repository-here"}, &out);
    std::printf("missing, -f: %d\n", code);
    CHECK(code == 22);
    // (online.cpp: the status after the body, a POST with a JSON body, no -f: 0 and the status)
    out.clear();
    code = winhttp_curl({"-sS", "--max-time", "10", "-X", "POST", "-H", "content-type: application/json", "-w", "\\n%{http_code}", "--data-binary",
                         "{\"a\":1}", "https://api.github.com/repos/tmcleroy/recomp-common/issues"},
                        &out);
    const size_t line = out.rfind('\n');
    std::printf("post: %d, status %s\n", code, line == std::string::npos ? "none" : out.c_str() + line + 1);
    CHECK(code == 0 && line != std::string::npos && std::atoi(out.c_str() + line + 1) >= 400);
    // (no such host: 6)
    code = winhttp_curl({"-sS", "--max-time", "10", "https://no-such-host.invalid/"}, &out);
    std::printf("no host: %d\n", code);
    CHECK(code == 6);
    // (left to curl.exe)
    CHECK(winhttp_curl({"-fsSL", "file:///C:/x.json"}, &out) == -1);
    if (failures) {
        return 1;
    }
    std::printf("winhttp_test: all as it should be\n");
}
