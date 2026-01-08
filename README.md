# Redis implementation in C (Codecrafters)

A small Redis-compatible server written in C, built as part of the [Codecrafters Redis](https://app.codecrafters.io/courses/redis) challenge.

This project focuses on:
- Implementing the Redis Serialization Protocol (**RESP**) parsing/encoding
- Handling multiple clients over TCP
- Implementing a subset of Redis commands (e.g. `PING`, `ECHO`, `SET`, `GET`, and related options depending on current progress)
- Basic key expiry handling (where implemented)
- (WIP) Parsing Redis RDB files

> Note
> 
> This is a learning project, not a production Redis replacement. Compatibility is “best effort” and limited to the subset implemented.

---

## Table of contents

- [Features](#features)
- [Project structure](#project-structure)
- [Build](#build)
- [Run](#run)
- [Test](#test)
- [Usage](#usage)
  - [Using `redis-cli`](#using-redis-cli)
  - [RESP examples](#resp-examples)
- [Command support](#command-support)
- [Design notes](#design-notes)
  - [SET command parser](#set-command-parser)
  - [Key expiry](#key-expiry)
  - [RDB file parsing (WIP)](#rdb-file-parsing-wip)
- [Debugging](#debugging)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)
- [References](#references)

---

## Features

- TCP server that speaks RESP (RESP2-style arrays/bulk strings)
- Basic protocol parsing and response encoding
- In-memory key/value store (simple hash map implementation)
- (Depending on your current milestone) TTL / expiry support
- Integration/unit tests via `make test` (see `tests/`)

---

## Project structure

High-level overview:

- `src/` — server implementation
  - `server.c` — socket server loop and connection handling
  - `protocol.c` — RESP parsing/encoding
  - `cmd_handler.c` — command parsing + execution
  - `simple_map.c` — in-memory map implementation
  - `rdb_file_parser.c` — Redis RDB parser (work in progress)
- `include/` — public headers
- `tests/`
  - `unit/` — unit tests for core modules
  - `integration/` — shell-based integration tests
- `client/` — a small client (if you want to test without `redis-cli`)
- `bin/` — build outputs
- `Makefile` — build orchestration

---

## Build

Requirements:
- Linux (or macOS with minor adjustments)
- `gcc`/`clang`
- `make`

Build the server:

```bash
make
```

Artifacts are typically placed under `bin/`.

---

## Run

Use the provided script (recommended):

```bash
./run.sh
```

Or run the server binary directly (path may vary depending on your Makefile):

```bash
./bin/server
```

If your server accepts flags (port, dir, dbfilename, etc.), run:

```bash
./bin/server --help
```

---

## Test

Build and run tests:

```bash
make test
```

Integration tests live under `tests/integration/` and are typically invoked by the test target.

---

## Usage

### Using `redis-cli`

If your server listens on the default Redis port (6379):

```bash
redis-cli -p 6379 ping
```

Example:

```bash
redis-cli -p 6379 set mykey hello
redis-cli -p 6379 get mykey
```

### RESP examples

Redis clients speak RESP. For example, this is a RESP-encoded request for:

```text
PING
```

It is serialized as:

```text
*1\r\n$4\r\nPING\r\n
```

And this is a request for:

```text
SET anotherkey "will expire in a minute" EX 60
```

Serialized as an array of bulk strings:

```text
*5\r\n$3\r\nSET\r\n$10\r\nanotherkey\r\n$22\r\nwill expire in a minute\r\n$2\r\nEX\r\n$2\r\n60\r\n
```

> Your implementation may currently accept only a subset of RESP types (commonly arrays of bulk strings).

---

## Command support

Implemented commands evolve with the challenge milestones.

Typical commands supported in this repository:
- `PING`
- `ECHO <message>`
- `SET key value [options...]`
- `GET key`

To see what’s implemented right now, check `src/cmd_handler.c` and the tests in `tests/`.

---

## Design notes

### SET command parser

The `SET` command validation accepts requests that respect the syntax:

```
SET key value [NX | XX] [GET] [EX seconds | PX milliseconds | EXAT unix-time-seconds | PXAT unix-time-milliseconds | KEEPTTL]
```

Important notes:
- Option order must be preserved; otherwise the parser treats it as invalid.
- `KEEPTTL` may not be supported yet (depending on current milestone).
- At the moment, the client must send the request as an array of bulk strings.

Example:

```
SET anotherkey "will expire in a minute" EX 60
```

Serialized as:

```
*5\r\n$3\r\nSET\r\n$10\r\nanotherkey\r\n$22\r\nwill expire in a minute\r\n$2\r\nEX\r\n$2\r\n60\r\n
```

### Key expiry

If expiry is implemented in your current version, it typically works by:
- storing metadata (absolute expiry timestamp) alongside the value
- checking expiry on read/write or via periodic cleanup

See the relevant logic in `src/cmd_handler.c` and utilities in `src/util.c`.

### RDB file parsing (WIP)

The repository contains an RDB parser under `src/rdb_file_parser.c`. This is a work-in-progress feature intended to support reading persisted Redis datasets (e.g. `dump.rdb`).

If you’re working on persistence support, common Redis CLI flags are:
- `--dir <path>`
- `--dbfilename <file>`

(Your server may or may not support these yet.)

---

## Debugging

If you use VS Code, you can set up a simple `launch.json` to debug the server process.

A helpful guide for multi-file C projects:
- https://dev.to/talhabalaj/setup-visual-studio-code-for-multi-file-c-projects-1jpi

General tip: build with debug symbols.
If your Makefile supports it, add `-g` and disable optimizations (`-O0`).

---

## Troubleshooting

- **`bind: Address already in use`**
  - Another process is using the port. Stop it or change your server port.

- **Client hangs / no response**
  - Verify requests are RESP-encoded as arrays of bulk strings.
  - Confirm the server is reading the full request and flushing responses.

- **Tests failing**
  - Run `make test` and inspect which suite fails (unit vs integration)
  - Review `tests/integration/*.sh` expected responses

---

## Contributing

Contributions are welcome.

Suggested workflow:
1. Fork the repository
2. Create a feature branch
3. Make changes with tests
4. Run `make` and `make test`
5. Open a PR with a clear description of the change

If you add new commands:
- update `src/cmd_handler.c`
- add/extend tests under `tests/`
- document the command behavior in this README

---

## License

Add a license file (e.g. `LICENSE`) if you plan to make this repository public.

If you are unsure, consider MIT or Apache-2.0 for a permissive open-source license.

---

## References

- Redis design/build guide: https://build-your-own.org/redis/
- Codecrafters Redis challenge: https://app.codecrafters.io/courses/redis/

Helpful links used during development:
- Makefiles: https://opensource.com/article/18/8/what-how-makefile
- Socket error handling:
  - https://thelinuxcode.com/catch-socket-errors-c/
  - https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-send-send-data-socket
- `errno` is thread-safe (discussion): https://stackoverflow.com/questions/1694164/is-errno-thread-safe
- Multithreading (C): https://tutorial.codeswithpankaj.com/c-programming/thread
- Function pointers in C: https://www.geeksforgeeks.org/function-pointer-in-c/
- `recv()` with timeout discussion: https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
- CLI parsing (`getopt`): https://thelinuxcode.com/getopt-3-c-function/
