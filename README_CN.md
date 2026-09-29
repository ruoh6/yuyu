# yuyu

[English](README.md) | **中文**

> 「此去經年，應是良辰好景虛設。便縱有千種風情，更與何人說？」
> ——柳永《雨霖鈴》

`yuyu` 是一个高性能 C++ 异步服务框架，由**用户态协程（Fiber）+ epoll IO 调度 + 系统调用 Hook** 构成，
核心目标是：**让同步写法的代码跑出异步的性能**。

网络 IO 全部由 Hook 层拦截，一旦某个调用会让线程阻塞，就切换到别的协程去跑，线程绝不空等。
因此你可以用「一行 `recv()` 接一行 `send()`」这样最朴素的同步写法，却得到事件驱动的高并发吞吐。

---

## 目录

- [特性](#特性)
- [架构总览](#架构总览)
- [目录结构](#目录结构)
- [环境与依赖](#环境与依赖)
- [构建](#构建)
- [快速开始](#快速开始)
- [核心模块](#核心模块)
- [配置说明](#配置说明)
- [测试](#测试)

---

## 特性

| 分类 | 能力 |
| --- | --- |
| 并发模型 | 用户态协程（`ucontext`）、N:M 调度（1 Scheduler → N Thread → M Fiber） |
| IO 引擎 | epoll 事件循环、定时器（`TimerManager`）、tickle 管道唤醒 |
| Hook | `sleep` / `read` / `write` / `send` / `recv` / `connect` / `accept` / `close` / `fcntl` / `ioctl` / `socket` 系列函数指针替换，per-fd 超时与阻塞状态管理 |
| 日志 | log4j 风格：`Logger` → `LogFormatter` → `LogAppender`，YAML 配置、pattern 格式化 |
| 配置 | `ConfigVar<T>` + `LexicalCast` 双向转换，支持 STL 容器、变更回调，约定优于配置 |
| 网络 | `Address`(IPv4/IPv6/Unix)、`Socket`/`SSLSocket`、`ByteArray`、`Stream`、`SocketStream`、`TcpServer` |
| HTTP | 服务端（Ragel 解析 + Servlet 路由 + 通配匹配）、客户端（keep-alive 连接池） |
| 压缩 | `ZlibStream`（gzip / zlib / deflate 编解码） |

---

## 架构总览

```
┌───────────────────────────────────────────────────────┐
│ 应用层   HttpServer / Servlet / HttpConnection / ...  │
├───────────────────────────────────────────────────────┤
│ HTTP     HttpRequest · HttpResponse · Ragel 解析器     │
│          HttpSession · ServletDispatch                │
├───────────────────────────────────────────────────────┤
│ 网络     Address · Socket · ByteArray · Stream        │
│          SocketStream · TcpServer · ZlibStream        │
├───────────────────────────────────────────────────────┤
│ Hook     函数指针替换 + FdManager（fd 状态 / 超时）    │
├───────────────────────────────────────────────────────┤
│ 调度     IOManager = Scheduler + TimerManager         │
│          idle() → epoll_wait                          │
├───────────────────────────────────────────────────────┤
│ 协程     Fiber（ucontext）+ Thread 线程池              │
├───────────────────────────────────────────────────────┤
│ 基础     Log · Config · Singleton · Util · Thread 原语 │
└───────────────────────────────────────────────────────┘
```

**一次典型请求的流转**（HTTP 场景）：

```
accept()  ── Hook 拦截，注册读事件，协程挂起
   │
   ▼
epoll_wait 返回可读 ── 唤醒对应协程
   │
   ▼
HttpSession::recvRequest()  ── Ragel 解析请求
   │
   ▼
ServletDispatch::handle()   ── 精确匹配 → 通配匹配 → NotFound
   │
   ▼
sendResponse() ── 异步写回，keep-alive 则回到 recvRequest 循环
```

---

## 目录结构

```
./
├── bin/                 # 可执行文件输出目录
├── lib/                 # 库输出目录（libyuyu.so）
├── build/               # CMake 中间目录
├── build.sh             # 构建脚本（优先使用 ccache）
├── Makefile             # make 封装（等价于进入 build/ 执行 make）
├── CMakeLists.txt       # 顶层构建定义
├── cmake/utils.cmake    # ragelmaker / yuyu_add_executable 等自定义函数
├── conf/                # 示例配置
│   ├── log.yml          #   日志配置
│   └── test.yml         #   配置系统演示（含各类 STL 容器与自定义结构）
├── example/             # 示例程序
│   ├── echo_server.cc
│   └── example_http_parser.cc
├── tests/               # 测试用例（每个模块一个 .cc）
├── thirdpart/           # 第三方子模块（openssl-1.0.1 / hiredis-vip）
├── yuyu/                # 框架源码
│   ├── http/            #   HTTP 模块
│   └── streams/         #   SocketStream / ZlibStream
├── .github/workflows/   # GitHub Actions CI
└── CLAUDE.md            # 面向 AI 助手的项目说明
```

---

## 环境与依赖

- **系统**：Linux（依赖 `epoll` 与 `ucontext`），已在 Ubuntu 16.04 / WSL2 上验证
- **编译器**：g++ 9.1+（`CMAKE_CXX_FLAGS` 中显式指定 `-std=c++11`，`CMAKE_CXX_STANDARD` 为 17）
- **CMake**：>= 3.14
- **依赖库**：
  - [yaml-cpp](https://github.com/jbeder/yaml-cpp)（必需，`find_package` 查找）
  - OpenSSL、zlib（链接 `ssl` / `crypto` / `z`）
- **代码生成**：[ragel](https://www.colm.net/open-source/ragel/)（修改 `.rl` 文件后需要重新生成 `.rl.cc`）
- **测试**：GoogleTest（CMake `FetchContent` 首次配置时自动下载）
- **可选**：ccache（`build.sh` 会使用）

第三方子模块（`thirdpart/openssl-1.0.1`、`thirdpart/hiredis-vip`）默认为空，如需要请初始化：

```bash
git submodule update --init --recursive
```

> `CMakeLists.txt` 中硬编码了 `thirdpart/openssl-1.0.1/build/install/{include,lib}` 作为头文件与库搜索路径。

---

## 构建

### 方式一：构建脚本

```bash
./build.sh
```

### 方式二：Makefile 封装

```bash
make            # 首次会自动 mkdir build && cmake ..，之后直接 make -j4
make test_log   # 构建单个目标
```

### 方式三：手动 CMake

```bash
mkdir -p build && cd build
cmake ..
make -j4
```

产物：

- 动态库：`lib/libyuyu.so`
- 可执行文件：`bin/`

---

## 快速开始

### 1. Echo 服务器

完整代码见 [`example/echo_server.cc`](example/echo_server.cc)。

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

            int rt = client->recv(&iovs[0], iovs.size());   // 同步写法，异步执行
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
    yuyu::IOManager iom(2);      // 2 个调度线程
    iom.schedule(run);           // run() 在协程中执行
    return 0;
}
```

```bash
./bin/example_echo_server -t   # -t 文本模式；-b 十六进制模式
nc 127.0.0.1 8020
```

注意 `main` 里没有 `while(true)`、没有手动 `epoll_wait`：`IOManager` 析构前会一直跑事件循环。

### 2. HTTP 服务器

完整代码见 [`tests/test_http_server.cc`](tests/test_http_server.cc)。

```cpp
#include "yuyu/http/http_server.h"

void run() {
    auto server = std::make_shared<yuyu::http::HttpServer>(true /* keepalive */);
    auto addr = yuyu::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr)) sleep(2);

    auto sd = server->getServletDispatch();

    // 精确匹配
    sd->addServlet("/yuyu/xx", [](yuyu::http::HttpRequest::ptr req,
                                  yuyu::http::HttpResponse::ptr rsp,
                                  yuyu::http::HttpSession::ptr session) {
        rsp->setBody(req->toString());
        return 0;
    });

    // 通配匹配
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

路由优先级：**精确匹配 → 通配匹配 → 默认 Servlet（`NotFoundServlet`）**。
通配匹配使用 `fnmatch(3)`，按注册顺序遍历，**首个命中的模式生效**（`/yuyu/*` 这类模式请留意注册顺序）。

### 3. HTTP 客户端

```cpp
#include "yuyu/http/http_connection.h"
#include "yuyu/iomanager.h"

void run() {
    // 一次性请求
    auto r = yuyu::http::HttpConnection::DoGet("http://www.example.com/", 300 /* ms */, {
        {"User-Agent", "yuyu"}
    });
    if (r->result == 0) {
        std::cout << r->response->getBody() << std::endl;
    } else {
        std::cout << "error: " << r->error << std::endl;
    }

    // 连接池（keep-alive 复用）
    auto pool = yuyu::http::HttpConnectionPool::Create(
        "http://www.example.com", "", 10 /* max_size */,
        1000 * 30 /* max_alive_time */, 5 /* max_request */);

    auto r2 = pool->doGet("/", 3000);
}
```

`HttpResult::Error` 枚举覆盖了 `INVALID_URL` / `INVALID_HOST` / `CONNECT_FAIL` /
`SEND_CLOSE_BY_PEER` / `SEND_SOCKET_ERROR` / `TIMEOUT` / `CREATE_SOCKET_ERROR` /
`POOL_GET_CONNECTION` / `POOL_INVALID_CONNECTION` 等失败原因。

---

## 核心模块

### Fiber —— 用户态协程

`yuyu/fiber.h`，基于 `ucontext_t` 实现。

- 状态机：`INIT → EXEC → HOLD / READY / TERM`
- 每个协程独立栈，`swapIn()` / `swapOut()` 切换上下文
- `use_caller = true` 时，调度线程自身也作为一个协程参与调度（主协程特化）
- `Fiber::GetThis()` 获取当前协程，`YieldToReady()` / `YieldToHold()` 让出
- 通过 `thread_local` 指针记录当前协程，日志中 `%F` 即协程 ID

### Scheduler —— 协程调度器

`yuyu/scheduler.h`，模型为 **1 Scheduler → N Thread → M Fiber**。

```cpp
Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
void schedule(FiberOrCb fc, int thread = -1);   // 指定线程 id，-1 表示任意
void start();
void stop();
```

- 内部维护任务队列 `std::list<FiberAndThread>`，`schedule()` 入队并在必要时 `tickle()`
- `idle()` 是虚函数：无线程空闲/无任务时的行为由子类定义（`IOManager` 在此 `epoll_wait`）
- `use_caller = true` 时局部变量 `IOManager iom(2)` 能保证析构前跑完事件循环

### IOManager —— IO 事件调度

`yuyu/iomanager.h`，同时继承 `Scheduler` 与 `TimerManager`。

```cpp
enum Event { NONE = 0x0, READ = 0x1, WRITE = 0x4 };

int  addEvent(int fd, Event event, std::function<void()> cb = nullptr);
bool delEvent(int fd, Event event);
bool cancelEvent(int fd, Event event);   // 触发一次后删除
bool cancelAll(int fd);
static IOManager* GetThis();
```

- 每个 fd 一个 `FdContext`，读/写各持有一个「调度器 + 协程 + 回调」
- `idle()` 调用 `epoll_wait`，超时时间由 `TimerManager::getNextTimer()` 决定
- `m_tickleFds` 是一对管道：有新任务入队时写一个字节唤醒 `epoll_wait`
- `Scheduler` / `IOManager` 都通过 `GetThis()` 拿到当前线程所属实例

### TimerManager —— 定时器

`yuyu/timer.h`，`Timer` 存放在按到期时间排序的 `std::set` 中。

```cpp
Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);
Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb,
                             std::weak_ptr<void> weak_cond, bool recurring = false);
```

- `addConditionTimer` 绑定一个弱引用条件，条件失效后回调不再执行（常用于「对象还活着才回调」）
- 新定时器插到队首时会回调 `onTimerInsertedAtFront()`，`IOManager` 借此重新计算 `epoll_wait` 超时
- 内部处理了时钟回拨（`detectClockRollover`）

### Hook —— 让阻塞调用变成协程切换

`yuyu/hook.h` / `yuyu/hook.cc`，这是整个框架的灵魂。

- 通过 `dlsym(RTLD_NEXT, ...)` 拿到原始函数指针，保存为 `sleep_f` / `read_f` / `connect_f` 等
- 覆盖同名符号，在自定义实现里判断：

  ```
  Hook 是否开启？ ── 否 ──▶ 直接调用原始函数
        │
       是
        ▼
  fd 是否为 socket？ ── 否 ──▶ 直接调用原始函数
        │
       是
        ▼
  设为非阻塞，尝试执行
        │
   成功 ──▶ 返回
        │
   EAGAIN ──▶ 向 IOManager 注册事件 + 超时，协程 yield
        │
   事件就绪 ──▶ 协程被唤醒，重试
  ```

- `set_hook_enable(bool)` 是全局开关，配合 `is_hook_enable()` 使用
- `connect_with_timeout()` 单独处理非阻塞 connect 的 `EINPROGRESS` 场景
- `FdManager` / `FdCtx`（`yuyu/fd_manager.h`）记录每个 fd 的：是否 socket、系统/用户非阻塞位、读写超时
  - **系统非阻塞**由 Hook 自己控制；**用户非阻塞**是用户通过 `fcntl` 显式设置的，Hook 会尊重

### 网络层

| 类 | 文件 | 说明 |
| --- | --- | --- |
| `Address` | `address.h` | IPv4 / IPv6 / Unix 地址抽象；`LookupAny`、`LookupAnyIPAddress` 做 DNS 解析 |
| `Socket` | `socket.h` | 生命周期封装（RAII），`accept`/`bind`/`connect`/`listen`/`send`/`recv`，支持 iovec 批量与 `cancel*` |
| `SSLSocket` | `socket.h` | OpenSSL 支持，`loadCertificates(cert, key)` |
| `ByteArray` | `bytearray.h` | 链表式缓冲区，定长读写（`readInt32` 等）、`getReadBuffers`/`getWriteBuffers` 导出 iovec、`toHexString` |
| `Stream` | `stream.h` | 抽象读写接口，附带 `readFixSize` / `writeFixSize` |
| `SocketStream` | `streams/socket_stream.h` | `Stream` 的 socket 实现 |
| `ZlibStream` | `streams/zlib_stream.h` | gzip / zlib / deflate 编解码流，`CreateGzip(encode)` 等工厂方法 |
| `TcpServer` | `tcp_server.h` | `bind(addr, ssl)` → `start()`；三个 `IOManager` 分别负责 accept / io / process |

`TcpServer` 由 `TcpServerConf` 描述（地址列表、keepalive、超时、ssl、证书、accept/io/process
线程池、自定义 args），并已特化 `LexicalCast` 可与 YAML 互转；当前尚未注册为 `ConfigVar`，
需由使用方显式 `setConf()`。子类只需覆写 `handleClient(Socket::ptr)`。

### HTTP 模块

**模型**（`yuyu/http/http.h`）

- `HttpMethod` / `HttpStatus` 枚举
- `HttpRequest` / `HttpResponse`：header / param / cookie 均为**大小写不敏感**的 map
- 提供 `getHeaderAs<T>()` / `getParamAs<T>()` 按类型取值

**解析**（Ragel 生成）

- `http11_parser.rl` → 服务端请求解析（`HttpRequestParser`）
- `httpclient_parser.rl` → 客户端响应解析（`HttpResponseParser`）
- 增量式：`execute(data, len)` 返回本次消费的字节数，可处理粘包/半包

**服务端**

- `HttpSession`（继承 `SocketStream`）：`recvRequest()` / `sendResponse()`
- `HttpServer`（继承 `TcpServer`）：覆写 `handleClient()`，创建 `HttpSession` 并交给 `ServletDispatch`
- Servlet 体系：
  - `Servlet` —— 抽象接口 `handle(req, rsp, session)`
  - `FunctionServlet` —— 包装一个 lambda
  - `ServletDispatch` —— URI 路由，支持 `addServlet` / `addGlobServlet` / `addServletCreator<T>()`
  - `NotFoundServlet` —— 默认兜底

**客户端**

- `HttpConnection`（继承 `SocketStream`）：`sendRequest()` / `recvResponse()`，静态方法 `DoGet` / `DoPost` / `DoRequest`
- `HttpConnectionPool`：keep-alive 连接复用，参数为 `max_size` / `max_alive_time` / `max_request`

### 日志

log4j 风格三层结构：

```
Logger（日志类别，含级别与 Appender 列表）
  │
  ├── LogFormatter（pattern 格式化）
  │
  └── LogAppender（输出地：StdoutLogAppender / FileLogAppender）
```

```cpp
static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();   // 根 Logger
YUYU_LOG_INFO(g_logger) << "hello " << 42;             // 流式
YUYU_LOG_FMT_INFO(g_logger, "hello %s %d", "world", 42); // printf 风格
```

常用宏：`YUYU_LOG_DEBUG/INFO/WARN/ERROR/FATAL`、`YUYU_LOG_FMT_*`、
`YUYU_LOG_ROOT()`、`YUYU_LOG_NAME("system")`。

级别：`DEBUG` / `INFO` / `WARN` / `ERROR` / `FATAL`（`LogLevel`）。

格式化 pattern 可用符号：

| 符号 | 含义 | 符号 | 含义 |
| --- | --- | --- | --- |
| `%m` | 消息体 | `%f` | 文件名 |
| `%p` | 日志级别 | `%l` | 行号 |
| `%r` | 累计毫秒数 | `%T` | Tab |
| `%c` | 日志名称 | `%F` | 协程 ID |
| `%t` | 线程 ID | `%N` | 线程名称 |
| `%d` | 时间 | `%n` | 换行 |

### 配置

`yuyu/config.h`：`ConfigVar<T>` 是带类型、带变更回调的配置项。

```cpp
// 注册 / 查找（同名同类型只创建一次）
static yuyu::ConfigVar<int>::ptr g_port =
    yuyu::Config::Lookup("system.port", 8080, "server port");

// 变更回调
g_port->addListener([](const int& old_v, const int& new_v) {
    // 热更新逻辑
});

// 从 YAML 加载（会按 "a.b.c" 的层级路径匹配）
yuyu::Config::LoadFromYaml(YAML::LoadFile("conf/test.yml"));
```

- 通过 `LexicalCast<F, T>` 实现字符串 ↔ 类型双向转换，已特化 `vector` / `list` / `set` /
  `unordered_set` / `map` / `unordered_map` 及嵌套组合
- 配置名采用小写点分命名（约定优于配置）
- 日志配置本身也是一个 `ConfigVar<std::set<LogDefine>>`（名为 `log`），
  `LogIniter` 监听它，因此**只要 YAML 里带 `log:` 段并调用 `LoadFromYaml`，日志就会被配置**

### 基础原语

- `thread.h`：`Thread`、`Mutex` / `RWMutex` / `Spinlock` / `CASLock` / `Semaphore`、`ScopedLockImpl`
- `singleton.h`：`Singleton<T>` 模板（静态局部变量初始化），如 `LoggerMgr`、`FdMgr`
- `util.h`：`GetThreadId` / `GetFiberId`、`Backtrace`（崩溃栈）、时间转换、`FSUtil`、`StringUtil`、`TypeToName<T>()`
- `macro.h`：`YUYU_ASSERT(x)` / `YUYU_ASSERT2(x, w)`，断言失败打印栈回溯
- 命名约定：每个类都定义 `typedef std::shared_ptr<X> ptr`

---

## 配置说明

### 日志配置 `conf/log.yml`

```yaml
log:
  - name: root            # Logger 名称
    level: info           # 级别
    formatter: "%d%T%m%n" # pattern
    appenders:            # 注意：键名是 appenders（复数）
      - type: FileLogAppender
        file: log.txt
      - type: StdoutLogAppender
```

支持的 Appender：`FileLogAppender`（需 `file`）、`StdoutLogAppender`。
每个 Appender 可单独设置 `level` 与 `formatter`。

> ⚠️ 仓库中 `conf/log.yml` 当前写的是 `appender:`（单数），而 `LexicalCast<std::string, LogDefine>`
> 读取的是 `appenders`。按上面写法改为 `appenders` 才会生效。

### 配置系统演示 `conf/test.yml`

演示了各类类型的加载方式：

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

对应 `Config::LoadFromYaml(YAML::LoadFile("conf/test.yml"))`，
即可用 `Config::Lookup<int>("system.port", 0)` 取到 8080。

---

## 测试

测试代码在 `tests/` 下，按模块划分：`test_log`、`test_config`、`test_thread`、`test_util`、
`test_fiber`、`test_scheduler`、`test_iomanager`、`test_hook`、`test_address`、`test_socket`、
`test_bytearray`、`test_tcp_server`、`test_http`、`test_http_parser`、`test_http_server`、
`test_http_connection`。

### 运行单个测试

```bash
cd build
make test_fiber && ./bin/test_fiber
```

可执行文件统一输出到 `bin/`。

### 通过 CTest 运行

```bash
cd build && ctest
```

### GoogleTest 套件

```bash
cd build && make run_tests && ./run_tests
```

首次配置 CMake 时会通过 `FetchContent` 下载 GoogleTest（需要网络）。

### CI

`.github/workflows/cmake-single-platform.yml`：在 `ubuntu-latest` 上安装
`cmake g++ libgtest-dev ragel`，执行 configure → build → `ctest`。
