# 单文件打包说明

将 Qt 应用程序打包成单个 exe 文件的方法。

## 方法 1: 使用 Enigma Virtual Box (推荐)

### 步骤：

1. **下载并安装 Enigma Virtual Box**
   - 下载地址: https://enigmaprotector.com/en/downloads.html
   - 安装后，命令行工具位于: `C:\Program Files\Enigma Virtual Box\enigmavb.exe`

2. **使用提供的 PowerShell 脚本**
   ```powershell
   .\package_single_exe.ps1
   ```

3. **手动使用 Enigma Virtual Box GUI**
   - 打开 Enigma Virtual Box
   - 选择主程序: `release\DataUploadTool.exe`
   - 添加所有依赖文件（windeployqt 收集的 DLL 和插件）
   - 勾选"压缩文件"和"文件虚拟化"
   - 点击"打包"生成单个 exe

## 方法 2: 使用静态链接 Qt

需要静态编译的 Qt 库。修改 CMakeLists.txt 启用静态链接：

```cmake
# 在 CMakeLists.txt 中添加
if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    set(QT_USE_STATIC ON)
endif()
```

## 方法 3: 使用 BoxedApp Packer

类似的打包工具，可以将 DLL 打包进 exe。

## 当前工作流

GitHub Actions 工作流会自动：
1. 编译应用程序
2. 使用 windeployqt 收集所有依赖
3. 生成包含所有文件的 release 文件夹

下载 artifacts 后，可以使用上述方法之一打包成单个 exe。

