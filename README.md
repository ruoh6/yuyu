# yuyu
高性能C++服务框架

## 开发环境
- Ubuntu 16.04
- gcc 9.1
- cmake 3.14.5
## 依赖
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) 
## 项目路径
```
./
|
bin -- 二进制文件 
|
build -- 中间文件目录
|
build.sh -- 构建脚本
|
CMakeLists.txt -- cmake文件
|
lib -- 库输出目录
|
README.md -- README文件
|
tests -- 测试代码
|
yuyu -- 框架代码
```
## 日志系统
制作一个类似log4j的日志系统
```
Logger （定义日志类别）
|
|------- Formatter（日志格式） 
|
Appender （日志输出地）
```
## 配置系统
规则[约定优于配置](https://zh.wikipedia.org/wiki/%E7%BA%A6%E5%AE%9A%E4%BC%98%E4%BA%8E%E9%85%8D%E7%BD%AE)
