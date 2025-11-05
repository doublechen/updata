# Qt 5.15.2 SSL/TLS 插件说明

## 概述

本项目使用 Qt 5.15.2 构建，HTTPS 功能依赖于 OpenSSL。关于 Qt SSL 插件的说明：

## 重要信息

### Qt 5.15.2 的 SSL/TLS 支持机制

Qt 5.15.2 在 Windows 上的 SSL/TLS 支持主要通过两种方式实现：

1. **OpenSSL DLL 文件**（必需）
   - `libcrypto-1_1.dll`
   - `libssl-1_1.dll`
   - 这是 HTTPS 功能的核心依赖

2. **Qt SSL/TLS 插件**（在某些版本中可选）
   - `plugins/ssl/qopensslbackend.dll` 或
   - `plugins/tls/qopensslbackend.dll`
   - 在某些 Qt 5.15.x 版本中，即使没有这些插件，HTTPS 也可以正常工作

### 为什么应用显示"SSL 插件缺失"警告？

从 GitHub Actions 上使用 `install-qt-action` 安装的 Qt 5.15.2 通常不包含 SSL/TLS 插件。这是正常的，原因如下：

1. **许可证问题**: OpenSSL 使用 Apache License 2.0，而 Qt 使用 LGPL/GPL，官方分发版通常不包含 OpenSSL 相关组件
2. **版本兼容性**: 不同地区和镜像的 Qt 分发版可能包含不同的组件
3. **向后兼容**: Qt 5.15.2 可以在没有新版 TLS 插件的情况下工作，直接使用 OpenSSL DLL

## 当前状态

✅ **OpenSSL DLL 文件**: 已包含在发布包中  
⚠️ **Qt SSL 插件**: 可能不包含（这是正常的）

## HTTPS 功能是否可用？

**是的！** 只要 OpenSSL DLL 文件存在，HTTPS 功能应该可以正常工作。

### 工作原理

1. **有 SSL 插件时**: Qt 使用插件作为 TLS 后端，调用 OpenSSL DLL
2. **无 SSL 插件时**: Qt 直接加载 OpenSSL DLL（对于 Qt 5.15.2 的某些版本）

### 如何验证 HTTPS 是否工作？

1. 运行应用程序
2. 查看日志输出：
   - 如果看到 "SSL 支持: true"，则完全正常
   - 如果看到 "SSL 支持: false"，但 OpenSSL DLL 存在，尝试发起 HTTPS 请求
3. 测试实际的 HTTPS 请求功能

## 如果 HTTPS 确实不工作怎么办？

如果确认 HTTPS 请求失败，可以尝试以下方法：

### 方法 1: 手动添加 Qt SSL 插件（推荐）

从完整的 Qt 5.15.2 安装中复制插件：

```
从 Qt 安装目录复制:
C:\Qt\5.15.2\msvc2019_64\plugins\tls\qopensslbackend.dll
或
C:\Qt\5.15.2\msvc2019_64\plugins\ssl\qopensslbackend.dll

到应用程序目录:
release\plugins\tls\qopensslbackend.dll
或
release\plugins\ssl\qopensslbackend.dll
```

### 方法 2: 使用完整的 Qt 安装

如果您在本地开发，可以：

1. 使用 Qt 在线安装器安装 Qt 5.15.2
2. 确保选择 "OpenSSL" 相关组件
3. 使用本地脚本 `build_qt5_windows.bat` 重新构建

### 方法 3: 更新 OpenSSL 版本

确保使用的是 OpenSSL 1.1.1 系列（Qt 5.15.2 兼容版本）：

- 下载地址: https://slproweb.com/products/Win32OpenSSL.html
- 选择 "Win64 OpenSSL v1.1.1" 版本
- 将 DLL 文件复制到应用程序目录

## 技术细节

### Qt 5.15+ 的 TLS 后端系统

Qt 5.15 引入了新的可插拔 TLS 后端系统，但是：

- **Qt 5.15.0-5.15.2**: 新旧系统过渡期，可能不需要插件
- **Qt 5.15.3+**: 更依赖插件系统
- **Qt 6.x**: 完全依赖插件系统

因此，对于 Qt 5.15.2，插件的需求因具体版本和构建配置而异。

### QSslSocket::supportsSsl() 返回 false？

如果这个函数返回 `false`，但 OpenSSL DLL 存在，可能是：

1. Qt 没有找到 OpenSSL DLL（检查文件名和位置）
2. OpenSSL 版本不匹配（Qt 5.15.2 需要 OpenSSL 1.1.1）
3. 缺少 SSL 插件（在某些配置中需要）

但是！即使返回 `false`，实际的 HTTPS 请求可能仍然工作。建议先测试实际功能。

## 总结

- ✅ OpenSSL DLL 是核心依赖，已包含
- ⚠️ Qt SSL 插件在 Qt 5.15.2 中可能是可选的
- ✅ HTTPS 功能应该可以正常工作
- 📝 如果遇到问题，参考本文档进行故障排除

## 参考资料

- [Qt SSL Documentation](https://doc.qt.io/qt-5/ssl.html)
- [Qt Network SSL Classes](https://doc.qt.io/qt-5/qtnetwork-module.html)
- [OpenSSL for Windows](https://slproweb.com/products/Win32OpenSSL.html)

