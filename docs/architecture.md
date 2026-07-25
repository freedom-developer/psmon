# psmon architecture

项目分为内核态和用户态两个部分：

- `kernel/` 构建 `psmon.ko`，负责在内核中采集进程信息。
- `userspace/` 构建命令行工具和常驻程序，并通过 `common/` 共享客户端代码。
- `include/uapi/` 保存内核态与用户态共同遵守的稳定 ABI 定义。

当前内核模块只将统计信息写入内核日志，尚未实现用户态通信接口。未来可根据
数据量、消息方向和兼容性需求，在字符设备、Generic Netlink 或其他机制中选择
一种，并在 `protocol.md` 中记录其语义。

内核内部实现细节应保留在 `kernel/psmon_internal.h`，不得加入 UAPI 头文件。
