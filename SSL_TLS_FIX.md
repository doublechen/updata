# SSL/TLS 初始化失败修复指南

如果遇到 "TLS initialization failed" 错误，这是因为 Qt 应用程序缺少 OpenSSL DLL 文件。

## 问题原因

Qt 在 Windows 上进行 HTTPS 请求需要 OpenSSL 库的支持。Qt 5.15.2 需要 OpenSSL 1.1.1 版本的 DLL。

## 解决方法

### 方法 1: 自动修复（推荐）

GitHub Actions 工作流已自动包含 OpenSSL DLL 的复制步骤。重新构建后，DLL 文件会自动包含在发布包中。

### 方法 2: 手动复制 OpenSSL DLL

如果是在本地构建，需要手动复制 OpenSSL DLL 文件：

1. **从 Qt 安装目录复制**（如果 Qt 包含 OpenSSL）：
   ```
   从: C:\Qt\5.15.2\msvc2019_64\bin\
   复制以下文件到 release 目录:
   - libcrypto-1_1.dll
   - libssl-1_1.dll
   ```

2. **下载 OpenSSL DLL**：
   - 访问: https://slproweb.com/products/Win32OpenSSL.html
   - 下载 OpenSSL 1.1.1 版本（64位）
   - 安装后，从安装目录的 `bin` 文件夹复制 DLL 文件

3. **使用预编译的 DLL**：
   - 从以下地址下载: https://indy.fulgan.com/SSL/
   - 下载 `openssl-1.1.1u-x64_86-win64.zip`
   - 解压后找到 `libcrypto-1_1.dll` 和 `libssl-1_1.dll`
   - 复制到应用程序所在目录

### 方法 3: 使用 windeployqt 时包含 OpenSSL

```powershell
# 在 release 目录中运行
windeployqt.exe --ssl DataUploadTool.exe
```

## 验证修复

修复后，运行应用程序并尝试上传数据。如果仍然失败，检查：

1. DLL 文件是否在 exe 同目录
2. DLL 版本是否匹配（Qt 5.15.2 需要 OpenSSL 1.1.1）
3. 是否为 64 位版本（与应用程序架构匹配）

## 注意事项

- OpenSSL DLL 必须与应用程序在同一目录
- 确保 DLL 版本与 Qt 版本兼容
- 如果是 64 位应用程序，必须使用 64 位 OpenSSL DLL

