// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
// curl's arguments as Windows' own requests read them (launcher/src/winhttp_curl.h): every shape the ports use, and
// the ones left to the system's curl.
#include "../launcher/src/winhttp_curl.h"

#include <cstdio>

static int failures = 0;
#define CHECK(x)                                                     \
    do {                                                             \
        if (!(x)) {                                                  \
            std::printf("%s:%d: failed: %s\n", __FILE__, __LINE__, #x); \
            failures++;                                              \
        }                                                            \
    } while (0)

int main() {
    using launcher::curl_request;
    // (update.cpp: the release's text, and a file)
    auto text = curl_request({"-fsSL", "--connect-timeout", "15", "--max-time", "60", "-H", "Accept: application/vnd.github+json", "-A",
                              "recomp-launcher", "https://api.github.com/repos/a/b/releases/latest"});
    CHECK(text.ok && text.fail && text.follow && text.show_error && text.connect_timeout == 15 && text.max_time == 60);
    CHECK(text.method == "GET" && text.headers.size() == 1 && text.user_agent == "recomp-launcher" && text.output_file.empty());
    auto file = curl_request({"-fsSL", "--connect-timeout", "15", "--speed-limit", "1024", "--speed-time", "60", "--retry", "2", "-A",
                              "recomp-launcher", "-o", "D:\\Jogos\\Zoë\\x.zip", "https://github.com/a/b/releases/download/v1/x.zip"});
    CHECK(file.ok && file.retry == 2 && file.speed_time == 60 && file.output_file == "D:\\Jogos\\Zoë\\x.zip");
    // (online.cpp and a game's online play: a status after the body)
    auto post = curl_request({"-sS", "--max-time", "10", "-X", "POST", "-H", "content-type: application/json", "-w", "\\n%{http_code}",
                              "--data-binary", "{\"a\":1}", "https://x.workers.dev/match"});
    CHECK(post.ok && post.method == "POST" && post.has_body && post.body == "{\"a\":1}" && !post.fail && !post.follow);
    CHECK(launcher::curl_write_out(post.write_out, 200) == "\n200");
    CHECK(launcher::curl_write_out(post.write_out, 0) == "\n000");
    auto bodyless = curl_request({"-sS", "--max-time", "8", "-w", "\n%{http_code}", "https://x/list"});
    CHECK(bodyless.ok && bodyless.method == "GET" && launcher::curl_write_out(bodyless.write_out, 404) == "\n404");
    CHECK(curl_request({"--data-binary", "{}", "http://x/"}).method == "POST");
    // (left to curl: file://, an argument not known, a body from a file, other -w fields, two URLs, a missing value)
    CHECK(!curl_request({"-fsSL", "file:///tmp/releases.json"}).ok);
    CHECK(!curl_request({"-sS", "--compressed", "https://x/"}).ok);
    CHECK(!curl_request({"-k", "https://x/"}).ok);
    CHECK(!curl_request({"--data-binary", "@body.json", "https://x/"}).ok);
    CHECK(!curl_request({"-w", "%{time_total}", "https://x/"}).ok);
    CHECK(!curl_request({"https://x/", "https://y/"}).ok);
    CHECK(!curl_request({"https://x/", "-o"}).ok);
    CHECK(!curl_request({"--max-time", "soon", "https://x/"}).ok);
    CHECK(!curl_request({"-sS"}).ok);
    if (failures) {
        return 1;
    }
    std::printf("curl_args_test: all as it should be\n");
}
