# Security

The library takes its input from the program that includes it (rows, names, button bits, a pixel
buffer and its size), not from files or the network, so there is little to attack. A bug that makes
it read or write outside the buffer it was given, for any picture size or any configuration, is
still a real bug and worth a private report.

Please report such a thing privately through GitHub ("Report a vulnerability" on the repository's
Security tab) or to tmcleroy@gmail.com,
and not in a public issue.
