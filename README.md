# psmon

`psmon` 是一个同时包含 Linux 内核模块和用户空间程序的进程监控项目骨架。

## 目录结构

```text
kernel/                  内核模块源码和 Kbuild 文件
userspace/common/        用户程序共享代码
userspace/apps/          各个用户空间程序
include/uapi/            内核与用户空间共享的稳定 ABI
scripts/                 模块加载和卸载脚本
tests/                   内核态及用户态测试
docs/                    架构与协议文档
```

当前内核模块每隔一段时间统计一次进程数和线程数，并写入内核日志。用户空间的
`psmonctl` 和 `psmon-agent` 目前是可编译、可运行的程序骨架；内核通信接口将在
后续版本中实现。

## 构建

构建全部组件：

```sh
make
```

也可以分别构建：

```sh
make kernel
make userspace
```

生成文件为：

```text
kernel/psmon.ko
userspace/build/psmonctl
userspace/build/psmon-agent
```

构建内核模块需要安装当前运行内核对应的头文件和构建工具。

## 使用

加载模块时可以设置扫描间隔，单位为毫秒，最小值为 100：

```sh
sudo ./scripts/load.sh interval_ms=2000
sudo dmesg -w
sudo ./scripts/unload.sh
```

查看用户空间程序骨架的版本：

```sh
./userspace/build/psmonctl --version
./userspace/build/psmon-agent --version
```

## VS Code 内核头文件配置

项目通过 Kbuild 的实际编译参数生成 `compile_commands.json`，供 VS Code C/C++
扩展或 clangd 使用。生成命令需要安装 Bear（Debian/Ubuntu 软件包名为
`bear`）：

```sh
make compile-commands
```

生成后，在 VS Code 中执行一次 `Developer: Reload Window`。如果切换了运行内核或
内核头文件目录，需要重新执行该命令。使用非当前内核的构建目录时可以指定：

```sh
make compile-commands KDIR=/path/to/kernel/build
```

运行用户空间冒烟测试并清理构建产物：

```sh
make test
make clean
```

某些启用了 Secure Boot 的系统会拒绝加载未签名模块；这种情况下需要按照发行版
文档为 `kernel/psmon.ko` 签名。
