# 手动修复 OpenSSL/TLS 问题

如果遇到 "TLS initialization failed" 错误，说明缺少 OpenSSL DLL 文件。以下是手动修复步骤：

## 方法 1: 使用提供的脚本（推荐）

运行项目根目录下的 `copy_openssl_dll.bat` 脚本：

```batch
copy_openssl_dll.bat
```

脚本会自动尝试从 Qt 目录复制 OpenSSL DLL，如果找不到会提示您手动操作。

## 方法 2: 手动下载和复制

### 步骤 1: 下载 OpenSSL DLL

**选项 A: 从官方下载（推荐）**
1. 访问: https://slproweb.com/products/Win32OpenSSL.html
2. 下载 "Win64 OpenSSL v1.1.1" 版本（Light 版本即可）
3. 安装后，从安装目录的 `bin` 文件夹复制 DLL

**选项 B: 从预编译版本下载**
1. 访问: https://indy.fulgan.com/SSL/
2. 下载 `openssl-1.1.1u-x64_86-win64.zip`
3. 解压后找到 DLL 文件

### 步骤 2: 复制 DLL 文件

将以下两个文件复制到 `release` 目录（与 `DataUploadTool.exe` 同一目录）：

- `libcrypto-1_1.dll`
- `libssl-1_1.dll`

### 步骤 3: 验证

运行程序，如果不再出现 "TLS initialization failed" 错误，说明修复成功。

## 方法 3: 使用 windeployqt（如果 Qt 包含 OpenSSL）

如果您的 Qt 安装包含 OpenSSL，可以尝试：

```powershell
cd release
windeployqt.exe --ssl DataUploadTool.exe
```

## 检查 DLL 是否存在

运行程序前，检查 `release` 目录是否包含以下文件：

```
release/
  ├── DataUploadTool.exe
  ├── libcrypto-1_1.dll  ← 必须存在
  ├── libssl-1_1.dll     ← 必须存在
  └── ... (其他 Qt DLL)
```

## 注意事项

- **版本匹配**: 必须使用 OpenSSL 1.1.1 版本（Qt 5.15.2 需要）
- **架构匹配**: 必须使用 64 位版本（如果您的应用是 64 位）
- **文件位置**: DLL 文件必须与 exe 文件在同一目录
- **文件名**: 确保文件名是 `libcrypto-1_1.dll` 和 `libssl-1_1.dll`

## 验证修复

修复后，运行程序并尝试上传数据。如果不再出现 TLS 错误，说明修复成功。

