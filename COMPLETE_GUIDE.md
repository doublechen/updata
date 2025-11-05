# 🎯 完整操作指南

## 📦 当前任务清单

### ✅ 已完成
1. ✅ 切换到 Qt 5.12.12（解决 SSL 插件问题）
2. ✅ 增强 SSL/TLS 诊断功能
3. ✅ 添加 OpenSSL DLL 预加载
4. ✅ 启用控制台输出（可选）
5. ✅ **新增：应用程序图标配置**

### 🎨 最新添加：应用图标

**使用图标**: `icon-1024x1024.png`（40+ 青绿色图标）

**自动功能**:
- ✅ GitHub Actions 自动将 PNG 转换为 ICO
- ✅ 编译时自动嵌入到 exe 中
- ✅ Windows 资源信息（版本、描述等）

---

## 🚀 一键操作：提交并构建

```bash
git add .
git commit -m "添加应用图标和增强 SSL 诊断"
git push
```

**GitHub Actions 会自动**:
1. 转换图标（PNG → ICO）
2. 编译资源文件（嵌入图标和版本信息）
3. 使用 Qt 5.12.12 构建
4. 配置 OpenSSL DLL
5. 打包成 zip

---

## 📥 下载和测试

### 1. 下载构建产物
1. 访问 GitHub Actions 页面
2. 等待构建完成（约 5-10 分钟）
3. 下载 `DataUploadTool-windows.zip`

### 2. 解压文件
```
DataUploadTool/
  ├── DataUploadTool.exe       ← 带图标的主程序
  ├── run_with_log.bat          ← 调试脚本
  ├── libcrypto-1_1.dll         ← OpenSSL DLL
  ├── libssl-1_1.dll            ← OpenSSL DLL
  ├── libcrypto-1_1-x64.dll     ← OpenSSL DLL (x64)
  ├── libssl-1_1-x64.dll        ← OpenSSL DLL (x64)
  └── ... 其他 Qt DLL
```

### 3. 运行应用

#### 选项 A: 正常运行（无控制台）
```
双击 DataUploadTool.exe
```
- 只显示 GUI 窗口
- 适合最终用户

#### 选项 B: 调试运行（有控制台）
如果需要查看日志，修改 `CMakeLists.txt`:
```cmake
WIN32_EXECUTABLE FALSE  # 启用控制台
```
然后重新构建。

---

## 🎨 图标效果

运行后你会看到：
- ✅ 文件资源管理器中的图标：**40+**（青绿色）
- ✅ 任务栏图标：**40+**（青绿色）
- ✅ Alt+Tab 切换图标：**40+**（青绿色）
- ✅ 右键属性 → 详细信息：
  - 文件描述: "40+成绩发布系统"
  - 产品名称: "40+成绩发布系统"
  - 版本: 1.0.0.0

---

## 🔍 验证 SSL/TLS 配置

### 如果需要查看诊断日志：

1. **临时启用控制台**（修改 `CMakeLists.txt`）:
   ```cmake
   WIN32_EXECUTABLE FALSE
   ```

2. **重新构建并运行**

3. **查看日志**:
   ```
   ========================================
   预加载 OpenSSL DLL
   ========================================
   应用程序目录: C:\...\DataUploadTool
   ✓ 成功加载: libcrypto-1_1.dll
   ✓ 成功加载: libssl-1_1.dll
   预加载结果: libcrypto = 成功, libssl = 成功

   ========================================
   SSL/TLS 配置检查
   ========================================
   Qt 版本: 5.12.12
   SSL 支持: ✓ 是
   SSL 运行时版本: OpenSSL 1.1.1...
   ```

4. **测试 HTTPS 上传**

---

## 📝 如果仍然有 TLS 错误

### 诊断步骤：

1. **启用控制台**（如上）
2. **查看启动日志**
3. **尝试上传数据**
4. **记录错误信息**

### 提供以下信息：

```
【启动日志】
（粘贴控制台输出）

【SSL 支持状态】
- SSL 支持: ✓ 是 / ✗ 否
- SSL 运行时版本: ...
- DLL 预加载: 成功 / 失败

【上传错误】
（粘贴具体错误信息）
```

---

## 🔧 自定义配置

### 更换图标
替换 `icon-1024x1024.png`，重新构建即可。

### 修改版本信息
编辑 `app.rc` 文件:
```rc
VALUE "FileVersion", "1.0.0.0"      ← 修改版本号
VALUE "ProductName", "40+成绩发布系统"  ← 修改产品名称
```

### 更改 Qt 版本
编辑 `.github/workflows/build-windows.yml`:
```yaml
aqt install-qt windows desktop 5.12.12 win64_msvc2017_64
                                # ↑ 修改版本号
```

---

## 📞 需要帮助？

### 图标问题
查看 `ICON_SETUP.md`

### SSL/TLS 问题
查看 `DEBUG_SSL_ISSUE.md` 和 `QUICK_TEST.md`

### 控制台日志
查看 `CONSOLE_FIX.md`

---

## 🎯 总结

**当前配置**:
- ✅ Qt 5.12.12（兼容 OpenSSL，无需插件）
- ✅ OpenSSL 1.1.1 DLL 预加载
- ✅ 自定义应用图标（40+）
- ✅ Windows 版本信息
- ✅ 详细的 SSL 诊断

**下一步**:
1. 提交代码（`git push`）
2. 等待构建完成
3. 下载并测试 exe
4. 查看图标是否显示
5. 测试 HTTPS 上传功能

---

**🎉 一切准备就绪，开始构建吧！**

