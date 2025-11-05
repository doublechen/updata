# SSL/TLS 支持指南

## 快速说明

### ✅ 正常情况

如果应用启动时显示：
```
⚠ 警告: Qt SSL 插件缺失，HTTPS 功能可能受限。
但 OpenSSL DLL 文件已存在，某些功能可能仍然可用。
```

**这通常是正常的！** 对于 Qt 5.15.2，只要 OpenSSL DLL 存在，HTTPS 功能应该可以工作。

**操作**: 先测试 HTTPS 功能，如果正常则无需修复。

### ❌ 需要修复

如果应用显示：
```
✗ OpenSSL DLL 文件缺失！
```

**这需要立即修复！** 参考下面的修复方法。

## 为什么会这样？

Qt 5.15.2 的 HTTPS 支持主要依赖两部分：

1. **OpenSSL DLL 文件**（必需）✅
   - `libcrypto-1_1.dll`
   - `libssl-1_1.dll`
   - **已包含在应用中**

2. **Qt SSL 插件**（可选）⚠️
   - `qopensslbackend.dll`
   - 某些 Qt 5.15.2 版本不需要此插件也能工作
   - **可能不包含（这是正常的）**

## 修复方法（仅在 HTTPS 确实不工作时）

### 方法 1: 确认 OpenSSL DLL 存在

检查应用程序目录下是否有：
- `libcrypto-1_1.dll`
- `libssl-1_1.dll`

如果没有，从以下地址下载 OpenSSL 1.1.1:
https://slproweb.com/products/Win32OpenSSL.html

### 方法 2: 添加 Qt SSL 插件（可选）

仅在确认 HTTPS 不工作时才需要：

1. 从完整的 Qt 5.15.2 安装获取插件：
   ```
   源文件: C:\Qt\5.15.2\msvc2019_64\plugins\tls\qopensslbackend.dll
   目标: release\plugins\tls\qopensslbackend.dll
   ```

2. 或者使用 windeployqt（本地构建时）：
   ```powershell
   cd release
   windeployqt.exe DataUploadTool.exe
   ```

## 常见问题

**Q: 为什么应用说"SSL 插件缺失"但 HTTPS 可以工作？**

A: Qt 5.15.2 可以在没有插件的情况下直接使用 OpenSSL DLL。这是正常的。

**Q: QSslSocket::supportsSsl() 返回 false？**

A: 这可能是因为缺少插件，但不一定影响实际功能。先测试 HTTPS 请求。

**Q: 我应该使用哪个 OpenSSL 版本？**

A: Qt 5.15.2 需要 **OpenSSL 1.1.1** 系列（如 1.1.1u）。不要使用 OpenSSL 3.x。

## 技术细节

详细的技术说明请参考: `QT_SSL_PLUGIN_INFO.md`

## 总结

- ✅ OpenSSL DLL 是核心依赖，已包含
- ⚠️ Qt SSL 插件在 Qt 5.15.2 中可能是可选的
- ✅ 先测试 HTTPS 功能，大多数情况下可以正常工作
- 🔧 仅在确认不工作时才需要添加插件

