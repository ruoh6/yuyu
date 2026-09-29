# yuyu

**English** | [中文](README_CN.md)

> *If I should see you, after long year,*
> *How should I greet thee? With silence and tears.*
> *— Lord Byron, "When We Two Parted"*

`yuyu` is a high-performance C++ async server framework built on **userspace fibers + epoll-based
IO scheduling + syscall hooks**. The goal is simple: **write synchronous code, get asynchronous
performance.**

Every network IO call is intercepted by the hook layer. Whenever a call would block the
thread, the scheduler switches to another fiber instead — the thread never sits idle.
So you can write plain blocking-style code (one `recv()` line, then one `send()` line)
and still get event-driven, high-concurrency throughput.

---

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Directory Layout](#directory-layout)
- [Requirements & Dependencies](#requirements--dependencies)
- [Building](#building)
- [Quick Start](#quick-start)
- [Core Modules](#core-modules)
- [Configuration](#configuration)
- [Tests](#tests)

---

## Features

| Area | Capability |
| --- | --- |
| Concurrency | Userspace fibers (`ucontext`), N:M scheduling (1 Scheduler → N Threads → M Fibers) |
| IO engine | epoll event loop, timers (`TimerManager`), tickle-pipe wakeup |
| Hook | Function-pointer replacement for `sleep` / `read` / `write` / `send` / `recv` / `connect` / `accept` / `close` / `fcntl` / `ioctl` / socket calls, with per-fd timeout and blocking-state tracking |
| Logging | log4j style: `Logger` → `LogFormatter` → `LogAppender`, YAML-configured, pattern-formatted |
| Config | `ConfigVar<T>` + bidirectional `LexicalCast`, STL container support, change listeners, convention over configuration |
| Networking | `Address` (IPv4/IPv6/Unix), `Socket`/`SSLSocket`, `ByteArray`, `Stream`, `SocketStream`, `TcpServer` |
| HTTP | Server (Ragel parsers + Servlet routing + glob matching), client (keep-alive connection pool) |
| Compression | `ZlibStream` (gzip / zlib / deflate, encode and decode) |

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│ Application  HttpServer / Servlet / HttpConnection / …  │
├─────────────────────────────────────────────────────────┤
│ HTTP         HttpRequest · HttpResponse · Ragel parsers │
│              HttpSession · ServletDispatch              │
├─────────────────────────────────────────────────────────┤
│ Networking   Address · Socket · ByteArray · Stream      │
│              SocketStream · TcpServer · ZlibStream      │
├─────────────────────────────────────────────────────────┤
│ Hook         function-pointer replacement + FdManager   │
├─────────────────────────────────────────────────────────┤
│ Scheduling   IOManager = Scheduler + TimerManager       │
│              idle() → epoll_wait                        │
├─────────────────────────────────────────────────────────┤
│ Coroutines   Fiber (ucontext) + Thread pool             │
├─────────────────────────────────────────────────────────┤
│ Foundation   Log · Config · Singleton · Util · Thread   │
└─────────────────────────────────────────────────────────┘
```

**Lifecycle of a typical request** (HTTP path):

```
accept()  ── hooked; register read event, fiber suspends
   │
   ▼
epoll_wait returns readable ── wake the fiber
   │
   ▼
HttpSession::recvRequest()  ── Ragel parses the request
   │
   ▼
ServletDispatch::handle()   ── exact match → glob match → NotFound
   │
   ▼
sendResponse() ── async write; keep-alive loops back to recvRequest
```

---

## Directory Layout

```
./
├── bin/                 # Executable output directory
├── lib/                 # Library output directory (libyuyu.so)
├── build/               # CMake build tree
├── build.sh             # Build script (prefers ccache)
├── Makefile             # make wrapper (equivalent to running make inside build/)
├── CMakeLists.txt       # Top-level build definition
├── cmake/utils.cmake    # Custom functions: ragelmaker / yuyu_add_executable
├── conf/                # Sample configuration
│   ├── log.yml          #   Logging config
│   └── test.yml         #   Config-system demo (STL containers + custom structs)
├── example/             # Sample programs
│   ├── echo_server.cc
│   └── example_http_parser.cc
├── tests/               # Test cases (one .cc per module)
├── thirdpart/           # Third-party submodules (openssl-1.0.1 / hiredis-vip)
├── yuyu/                # Framework source
│   ├── http/            #   HTTP module
│   └── streams/         #   SocketStream / ZlibStream
├── .github/workflows/   # GitHub Actions CI
└── CLAUDE.md            # Project notes for AI assistants
```

---

## Requirements & Dependencies

- **OS**: Linux (needs `epoll` and `ucontext`); verified on Ubuntu 16.04 and WSL2
- **Compiler**: g++ 9.1+ (`-std=c++11` is set explicitly in `CMAKE_CXX_FLAGS`; `CMAKE_CXX_STANDARD` is 17)
- **CMake**: >= 3.14
- **Libraries**:
  - [yaml-cpp](https://github.com/jbeder/yaml-cpp) (required, located via `find_package`)
  - OpenSSL and zlib (linked as `ssl` / `crypto` / `z`)
- **Code generation**: [ragel](https://www.colm.net/open-source/ragel/) (needed to regenerate `.rl.cc` after editing a `.rl` file)
- **Tests**: GoogleTest (downloaded automatically by CMake `FetchContent` on first configure)
- **Optional**: ccache (used by `build.sh`)

The third-party submodules (`thirdpart/openssl-1.0.1`, `thirdpart/hiredis-vip`) are empty by
default. Initialize them if you need them:

```bash
git submodule update --init --recursive
```

> `CMakeLists.txt` hardcodes `thirdpart/openssl-1.0.1/build/install/{include,lib}` as a header
> and library search path.

---

## Building

### Option 1: build script

```bash
./build.sh
```

### Option 2: Makefile wrapper

```bash
make            # first run does mkdir build && cmake .., later runs just make -j4
make test_log   # build a single target
```

### Option 3: plain CMake

```bash
mkdir -p build && cd build
cmake ..
make -j4
```

Artifacts:

- Shared library: `lib/libyuyu.so`
- Executables: `bin/`

---

## Quick Start

### 1. Echo server

Full source: [`example/echo_server.cc`](example/echo_server.cc).

```cpp
#include "yuyu/tcp_server.h"
#include "yuyu/iomanager.h"
#include "yuyu/bytearray.h"

static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

class EchoServer : public yuyu::TcpServer {
public:
    EchoServer(int type) : m_type(type) {}
    void handleClient(yuyu::Socket::ptr client) override {
        yuyu::ByteArray::ptr ba(new yuyu::ByteArray);
        while (true) {
            ba->clear();
            std::vector<iovec> iovs;
            ba->getWriteBuffers(iovs, 1024);

            int rt = client->recv(&iovs[0], iovs.size());   // sync style, async execution
            if (rt <= 0) break;

            ba->setPosition(rt);
            ba->setPosition(0);

            std::vector<iovec> read_iovs;
            ba->getReadBuffers(read_iovs, rt);
            if (client->send(&read_iovs[0], read_iovs.size()) <= 0) break;
        }
    }
private:
    int m_type = 1;
};

void run() {
    EchoServer::ptr es(new EchoServer(1));
    auto addr = yuyu::Address::LookupAny("0.0.0.0:8020");
    while (!es->bind(addr)) sleep(2);
    es->start();
}

int main(int argc, char** argv) {
    yuyu::IOManager iom(2);      // 2 scheduler threads
    iom.schedule(run);           // run() executes inside a fiber
    return 0;
}
```

```bash
./bin/example_echo_server -t   # -t text mode; -b hex mode
nc 127.0.0.1 8020
```

Note there is no `while(true)` and no manual `epoll_wait` in `main`: `IOManager` keeps the
event loop running until it is destroyed.

### 2. HTTP server

Full source: [`tests/test_http_server.cc`](tests/test_http_server.cc).

```cpp
#include "yuyu/http/http_server.h"

void run() {
    auto server = std::make_shared<yuyu::http::HttpServer>(true /* keepalive */);
    auto addr = yuyu::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr)) sleep(2);

    auto sd = server->getServletDispatch();

    // exact match
    sd->addServlet("/yuyu/xx", [](yuyu::http::HttpRequest::ptr req,
                                  yuyu::http::HttpResponse::ptr rsp,
                                  yuyu::http::HttpSession::ptr session) {
        rsp->setBody(req->toString());
        return 0;
    });

    // glob match
    sd->addGlobServlet("/yuyu/*", [](yuyu::http::HttpRequest::ptr req,
                                     yuyu::http::HttpResponse::ptr rsp,
                                     yuyu::http::HttpSession::ptr session) {
        rsp->setBody("Glob:\r\n" + req->toString());
        return 0;
    });

    server->start();
}

int main(int argc, char** argv) {
    yuyu::IOManager iom(1, true, "main");
    iom.schedule(run);
    return 0;
}
```

Routing priority: **exact match → glob match → default Servlet (`NotFoundServlet`)**.
Glob matching uses `fnmatch(3)` and walks the list in registration order, so **the first
matching pattern wins** — mind the order in which you register `/yuyu/*`-style patterns.

### 3. HTTP client

```cpp
#include "yuyu/http/http_connection.h"
#include "yuyu/iomanager.h"

void run() {
    // one-off request
    auto r = yuyu::http::HttpConnection::DoGet("http://www.example.com/", 300 /* ms */, {
        {"User-Agent", "yuyu"}
    });
    if (r->result == 0) {
        std::cout << r->response->getBody() << std::endl;
    } else {
        std::cout << "error: " << r->error << std::endl;
    }

    // connection pool (keep-alive reuse)
    auto pool = yuyu::http::HttpConnectionPool::Create(
        "http://www.example.com", "", 10 /* max_size */,
        1000 * 30 /* max_alive_time */, 5 /* max_request */);

    auto r2 = pool->doGet("/", 3000);
}
```

The `HttpResult::Error` enum covers `INVALID_URL` / `INVALID_HOST` / `CONNECT_FAIL` /
`SEND_CLOSE_BY_PEER` / `SEND_SOCKET_ERROR` / `TIMEOUT` / `CREATE_SOCKET_ERROR` /
`POOL_GET_CONNECTION` / `POOL_INVALID_CONNECTION`, among others.

---

## Core Modules

### Fiber — userspace coroutines

`yuyu/fiber.h`, built on `ucontext_t`.

- State machine: `INIT → EXEC → HOLD / READY / TERM`
- Each fiber owns its stack; `swapIn()` / `swapOut()` switch context
- With `use_caller = true`, the scheduling thread itself participates as a fiber
  (the main fiber is special-cased)
- `Fiber::GetThis()` returns the current fiber; `YieldToReady()` / `YieldToHold()` yield
- The current fiber is tracked in a `thread_local` pointer — `%F` in a log pattern is the fiber id

### Scheduler — fiber scheduler

`yuyu/scheduler.h`, modeled as **1 Scheduler → N Threads → M Fibers**.

```cpp
Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
void schedule(FiberOrCb fc, int thread = -1);   // pin to a thread id; -1 means any
void start();
void stop();
```

- Keeps a task queue (`std::list<FiberAndThread>`); `schedule()` enqueues and calls `tickle()`
  when needed
- `idle()` is virtual: subclasses define what happens when there is nothing to run
  (`IOManager` calls `epoll_wait` there)
- With `use_caller = true`, a stack-allocated `IOManager iom(2)` keeps the event loop running
  until destruction

### IOManager — IO event scheduling

`yuyu/iomanager.h`, inherits both `Scheduler` and `TimerManager`.

```cpp
enum Event { NONE = 0x0, READ = 0x1, WRITE = 0x4 };

int  addEvent(int fd, Event event, std::function<void()> cb = nullptr);
bool delEvent(int fd, Event event);
bool cancelEvent(int fd, Event event);   // delete after one trigger
bool cancelAll(int fd);
static IOManager* GetThis();
```

- One `FdContext` per fd; the read and write slots each hold a scheduler + fiber + callback
- `idle()` calls `epoll_wait`, with the timeout derived from `TimerManager::getNextTimer()`
- `m_tickleFds` is a pipe pair: writing one byte when a task is enqueued wakes `epoll_wait`
- Both `Scheduler` and `IOManager` expose `GetThis()` for the current thread's instance

### TimerManager — timers

`yuyu/timer.h`; `Timer` objects live in a `std::set` ordered by expiry time.

```cpp
Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);
Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb,
                             std::weak_ptr<void> weak_cond, bool recurring = false);
```

- `addConditionTimer` binds a weak condition: once the condition expires the callback stops
  firing (the usual "only call back if the object is still alive" pattern)
- Inserting a timer at the front of the set triggers `onTimerInsertedAtFront()`, which lets
  `IOManager` recompute the `epoll_wait` timeout
- Clock rollback is handled internally (`detectClockRollover`)

### Hook — turning blocking calls into fiber switches

`yuyu/hook.h` / `yuyu/hook.cc` — the heart of the framework.

- Original function pointers are captured with `dlsym(RTLD_NEXT, ...)` and stored as
  `sleep_f` / `read_f` / `connect_f` and friends
- Same-named symbols are overridden, and the custom implementation decides:

  ```
  Hook enabled? ── no ──▶ call the original function directly
        │
       yes
        ▼
  fd is a socket? ── no ──▶ call the original function directly
        │
       yes
        ▼
  set non-blocking, attempt the call
        │
   success ──▶ return
        │
   EAGAIN ──▶ register event + timeout with IOManager, fiber yields
        │
   event ready ──▶ fiber wakes up and retries
  ```

- `set_hook_enable(bool)` is the global switch; pair it with `is_hook_enable()`
- `connect_with_timeout()` handles the `EINPROGRESS` case of non-blocking connect separately
- `FdManager` / `FdCtx` (`yuyu/fd_manager.h`) record, per fd: whether it is a socket, the
  system/user non-blocking flags, and the read/write timeouts
  - The **system** non-blocking flag is controlled by the hook itself; the **user** flag is
    whatever the application set via `fcntl`, and the hook respects it

### Networking

| Class | File | Notes |
| --- | --- | --- |
| `Address` | `address.h` | IPv4 / IPv6 / Unix address abstraction; `LookupAny`, `LookupAnyIPAddress` for DNS resolution |
| `Socket` | `socket.h` | RAII lifetime management; `accept`/`bind`/`connect`/`listen`/`send`/`recv`, iovec batching, `cancel*` |
| `SSLSocket` | `socket.h` | OpenSSL support via `loadCertificates(cert, key)` |
| `ByteArray` | `bytearray.h` | Linked-list buffer; typed read/write (`readInt32`, …), `getReadBuffers`/`getWriteBuffers` export iovecs, `toHexString` |
| `Stream` | `stream.h` | Abstract read/write interface, plus `readFixSize` / `writeFixSize` |
| `SocketStream` | `streams/socket_stream.h` | Socket-backed `Stream` |
| `ZlibStream` | `streams/zlib_stream.h` | gzip / zlib / deflate codec stream; factories like `CreateGzip(encode)` |
| `TcpServer` | `tcp_server.h` | `bind(addr, ssl)` → `start()`; three `IOManager`s drive accept / io / process |

`TcpServer` is described by `TcpServerConf` (address list, keepalive, timeout, ssl, cert files,
accept/io/process worker pools, custom args). A `LexicalCast` specialization lets it round-trip
through YAML, but it is **not yet registered as a `ConfigVar`** — callers must `setConf()`
explicitly. Subclasses only need to override `handleClient(Socket::ptr)`.

### HTTP

**Models** (`yuyu/http/http.h`)

- `HttpMethod` / `HttpStatus` enums
- `HttpRequest` / `HttpResponse`: header / param / cookie maps are all **case-insensitive**
- `getHeaderAs<T>()` / `getParamAs<T>()` for typed lookups

**Parsing** (Ragel-generated)

- `http11_parser.rl` → server-side request parsing (`HttpRequestParser`)
- `httpclient_parser.rl` → client-side response parsing (`HttpResponseParser`)
- Incremental: `execute(data, len)` returns how many bytes were consumed, so split and
  coalesced packets are handled correctly

**Server**

- `HttpSession` (extends `SocketStream`): `recvRequest()` / `sendResponse()`
- `HttpServer` (extends `TcpServer`): overrides `handleClient()`, creates an `HttpSession`
  and hands it to `ServletDispatch`
- Servlet family:
  - `Servlet` — the `handle(req, rsp, session)` interface
  - `FunctionServlet` — wraps a lambda
  - `ServletDispatch` — URI routing via `addServlet` / `addGlobServlet` / `addServletCreator<T>()`
  - `NotFoundServlet` — the fallback

**Client**

- `HttpConnection` (extends `SocketStream`): `sendRequest()` / `recvResponse()`, plus the
  static helpers `DoGet` / `DoPost` / `DoRequest`
- `HttpConnectionPool`: keep-alive connection reuse, parameterized by `max_size` /
  `max_alive_time` / `max_request`

### Logging

Three-layer log4j-style design:

```
Logger (the log category: level + appender list)
  │
  ├── LogFormatter (pattern formatting)
  │
  └── LogAppender (output sink: StdoutLogAppender / FileLogAppender)
```

```cpp
static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();     // root logger
YUYU_LOG_INFO(g_logger) << "hello " << 42;               // streaming style
YUYU_LOG_FMT_INFO(g_logger, "hello %s %d", "world", 42); // printf style
```

Common macros: `YUYU_LOG_DEBUG/INFO/WARN/ERROR/FATAL`, `YUYU_LOG_FMT_*`,
`YUYU_LOG_ROOT()`, `YUYU_LOG_NAME("system")`.

Levels: `DEBUG` / `INFO` / `WARN` / `ERROR` / `FATAL` (`LogLevel`).

Pattern symbols:

| Symbol | Meaning | Symbol | Meaning |
| --- | --- | --- | --- |
| `%m` | message | `%f` | filename |
| `%p` | level | `%l` | line number |
| `%r` | elapsed ms | `%T` | tab |
| `%c` | logger name | `%F` | fiber id |
| `%t` | thread id | `%N` | thread name |
| `%d` | datetime | `%n` | newline |

### Configuration

`yuyu/config.h`: `ConfigVar<T>` is a typed configuration entry with change listeners.

```cpp
// register / look up (same name and type only creates once)
static yuyu::ConfigVar<int>::ptr g_port =
    yuyu::Config::Lookup("system.port", 8080, "server port");

// change listener
g_port->addListener([](const int& old_v, const int& new_v) {
    // hot-reload logic
});

// load from YAML (matched by the dotted path "a.b.c")
yuyu::Config::LoadFromYaml(YAML::LoadFile("conf/test.yml"));
```

- `LexicalCast<F, T>` provides bidirectional string ↔ type conversion, specialized for
  `vector` / `list` / `set` / `unordered_set` / `map` / `unordered_map` and nested combinations
- Config keys use lowercase dotted names (convention over configuration)
- The log configuration is itself a `ConfigVar<std::set<LogDefine>>` named `log`, watched by
  `LogIniter` — so **as long as the YAML has a `log:` section and you call `LoadFromYaml`,
  logging gets configured**

### Foundation primitives

- `thread.h`: `Thread`, `Mutex` / `RWMutex` / `Spinlock` / `CASLock` / `Semaphore`, `ScopedLockImpl`
- `singleton.h`: `Singleton<T>` (static local initialization), e.g. `LoggerMgr`, `FdMgr`
- `util.h`: `GetThreadId` / `GetFiberId`, `Backtrace` (crash stacks), time conversion,
  `FSUtil`, `StringUtil`, `TypeToName<T>()`
- `macro.h`: `YUYU_ASSERT(x)` / `YUYU_ASSERT2(x, w)` — print a backtrace on assertion failure
- Convention: every class defines `typedef std::shared_ptr<X> ptr`

---

## Configuration

### Logging config `conf/log.yml`

```yaml
log:
  - name: root            # logger name
    level: info           # level
    formatter: "%d%T%m%n" # pattern
    appenders:            # note: the key is "appenders" (plural)
      - type: FileLogAppender
        file: log.txt
      - type: StdoutLogAppender
```

Supported appenders: `FileLogAppender` (requires `file`) and `StdoutLogAppender`.
Each appender may set its own `level` and `formatter`.

> ⚠️ The checked-in `conf/log.yml` currently spells the key `appender:` (singular), while
> `LexicalCast<std::string, LogDefine>` reads `appenders`. Rewrite it as `appenders` (as above)
> for it to take effect.

### Config-system demo `conf/test.yml`

Shows how each type is loaded:

```yaml
system:
    port: 8080
    value: 15
    int_vec: [10, 30]
    int_set: [30, 20, 10]
    int_map:
      ke: 10086

class:
    person:
      name: yuyu
      age: 18
      sex: false
```

After `Config::LoadFromYaml(YAML::LoadFile("conf/test.yml"))`, `Config::Lookup<int>("system.port", 0)`
returns 8080.

---

## Tests

Test sources live in `tests/`, one per module: `test_log`, `test_config`, `test_thread`,
`test_util`, `test_fiber`, `test_scheduler`, `test_iomanager`, `test_hook`, `test_address`,
`test_socket`, `test_bytearray`, `test_tcp_server`, `test_http`, `test_http_parser`,
`test_http_server`, `test_http_connection`.

### Running a single test

```bash
cd build
make test_fiber && ./bin/test_fiber
```

Executables are written to `bin/`.

### Running through CTest

```bash
cd build && ctest
```

### The GoogleTest suite

```bash
cd build && make run_tests && ./run_tests
```

GoogleTest is downloaded by `FetchContent` on the first CMake configure (network required).

### CI

`.github/workflows/cmake-single-platform.yml` installs `cmake g++ libgtest-dev ragel` on
`ubuntu-latest` and runs configure → build → `ctest`.
