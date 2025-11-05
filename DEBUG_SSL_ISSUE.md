# SSL/TLS 问题调试指南

## 🔍 当前状态

运行时仍然报 "TLS initialization failed" 错误。

## 📋 已完成的尝试

1. ✅ 切换到 Qt 5.15.2 + aqtinstall
2. ✅ 切换到 Qt 5.12.12（应该不需要插件）
3. ✅ 添加 OpenSSL DLL 预加载
4. ✅ 添加详细诊断日志

## 🔬 下一步诊断

### 步骤 1: 查看完整日志

重新构建并运行应用，**查看启动时的完整日志输出**。关键信息：

```
========================================
预加载 OpenSSL DLL
========================================
应用程序目录: ...
✓ 成功加载: libcrypto-1_1.dll （或失败信息）
✓ 成功加载: libssl-1_1.dll （或失败信息）
预加载结果: libcrypto = 成功/失败, libssl = 成功/失败

========================================
SSL/TLS 配置检查
========================================
Qt 版本: ...
SSL 支持: ✓ 是 / ✗ 否
SSL 运行时版本: ...
OpenSSL DLL 文件:
  libcrypto-1_1.dll: ✓ 存在 / ✗ 不存在
    路径: ...
    大小: ...
  libssl-1_1.dll: ✓ 存在 / ✗ 不存在
    路径: ...
    大小: ...
```

### 步骤 2: 根据日志诊断

#### 情况 A: DLL 文件不存在

```
✗ 文件不存在: libcrypto-1_1.dll
✗ 文件不存在: libssl-1_1.dll
```

**解决方案**: OpenSSL DLL 未被打包到应用中
- 检查 GitHub Actions 构建日志
- 确认"Copy OpenSSL DLL files"步骤成功
- 检查下载的 zip 包中是否包含这些 DLL

#### 情况 B: DLL 存在但加载失败

```
✓ 成功加载: libcrypto-1_1.dll
✓ 成功加载: libssl-1_1.dll
但 SSL 支持: ✗ 否
```

**可能原因**:
1. OpenSSL 版本不匹配 Qt 5.12.12
2. DLL 文件损坏或架构不对
3. 缺少 MSVC 运行时依赖

**解决方案**:
- 尝试下载不同版本的 OpenSSL 1.1.1
- 确认是 64 位版本
- 检查是否需要安装 MSVC 2017 Redistributable

#### 情况 C: DLL 加载失败

```
✗ 加载失败: libcrypto-1_1.dll 错误: Cannot load library ...
```

**可能原因**:
1. 缺少依赖的其他 DLL
2. 文件损坏
3. 架构不匹配

**解决方案**:
- 使用 Dependency Walker 检查 DLL 依赖
- 重新下载 OpenSSL DLL
- 确认是 64 位版本

## 🔧 可能的解决方案

### 方案 1: 更换 OpenSSL 版本

当前使用的是项目中的 OpenSSL 1.1.1。尝试：

1. 从官方下载最新的 OpenSSL 1.1.1u：
   https://slproweb.com/products/Win32OpenSSL.html

2. 替换 `dll/` 目录中的文件

3. 重新构建

### 方案 2: 使用 Qt 自带的 OpenSSL

某些 Qt 分发版自带 OpenSSL DLL：

1. 检查 Qt 安装目录：`Qt5_DIR/bin/`
2. 如果有 `libcrypto-1_1-x64.dll` 和 `libssl-1_1-x64.dll`
3. 复制到应用目录并测试

### 方案 3: 使用 Schannel 后端（Windows 原生）

修改代码使用 Windows 原生的 Schannel 而不是 OpenSSL：

```cpp
// 在 QNetworkAccessManager 创建后
QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
// 配置使用 Schannel...
```

### 方案 4: 回退到 Qt 5.9.x

Qt 5.9 LTS 版本对 OpenSSL 的要求更宽松：

```yaml
aqt install-qt windows desktop 5.9.9 win64_msvc2017_64
```

## 📝 提供日志

**请运行应用并提供完整的日志输出**，包括：

1. 应用启动时的所有诊断信息
2. 尝试 HTTPS 请求时的错误信息
3. 文件列表（release 目录中的所有文件）

这将帮助我们准确诊断问题。

## 🚨 紧急备选方案

如果无法解决 OpenSSL 问题，可以考虑：

1. **使用 HTTP 而不是 HTTPS**（如果服务器支持）
2. **使用代理工具**（如 nginx）处理 HTTPS
3. **在服务器端修改** API 支持 HTTP

## 📞 需要的信息

请提供以下信息以便进一步诊断：

1. 应用启动时的完整日志
2. 上传数据时的错误日志
3. release 目录中的文件列表（`dir release /s`）
4. Qt 版本信息（从日志中获取）
5. OpenSSL DLL 文件大小（从日志中获取）

---

**重要**: 这是一个系统性的调试过程。请按步骤进行，提供详细的日志输出。

