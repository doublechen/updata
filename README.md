# 数据上传工具 - Qt版本

这是使用Qt框架重写的HTA应用程序，提供相同的界面和功能，可以打包成独立的exe文件。

## 功能特性

- ✅ 从HTTP服务器获取数据（rawinfo、allplay、InquiryPage）
- ✅ 自动编码检测和转换（UTF-8、GBK、GB2312）
- ✅ 使用multipart/form-data格式上传数据
- ✅ 定时循环执行任务
- ✅ 彩色日志输出（成功、错误、警告、信息、时间）
- ✅ 现代化的UI界面

## 系统要求

- Qt 5.12+ 或 Qt 6.0+
- CMake 3.16+
- C++17 编译器
- Windows 10/11 (用于打包exe)

## 编译步骤

### Qt5 编译指南

**详细指南请查看: [QT5_BUILD_GUIDE.md](QT5_BUILD_GUIDE.md)**

#### Windows (Qt5) - 快速开始

1. **修改构建脚本中的Qt路径**
   
   编辑 `build_qt5_windows.bat`，设置：
```batch
set QT_DIR=C:\Qt\5.15.2\msvc2019_64
```

2. **运行脚本**
```cmd
build_qt5_windows.bat
```

#### Linux (Qt5) - 快速开始

1. **安装Qt5开发包**
```bash
sudo apt-get install qt5-default qtbase5-dev qtbase5-dev-tools cmake
```

2. **运行构建脚本**
```bash
chmod +x build_qt5_linux.sh
./build_qt5_linux.sh
```

#### 手动编译 (Qt5)

1. **配置项目**
```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt5
```

Windows示例（Qt5安装在 `C:\Qt\5.15.2\msvc2019_64`）:
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64
```

2. **编译**
```bash
cmake --build . --config Release
```

Windows示例:
```cmd
cmake --build . --config Release
```

### Qt6 编译指南

#### Windows (Qt6) - 快速开始

1. **修改构建脚本中的Qt路径**
   
   编辑 `build_and_package.bat`，设置：
```batch
set QT_DIR=C:\Qt\6.5.0\msvc2019_64
```

2. **运行脚本**
```cmd
build_and_package.bat
```

#### 手动编译 (Qt6)

1. **配置项目**
```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
```

Windows示例（假设Qt安装在 `C:\Qt\6.5.0\msvc2019_64`）:
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
```

2. **编译**
```bash
cmake --build . --config Release
```

Windows示例:
```cmd
cmake --build . --config Release
```

### 使用Qt Creator

1. 打开Qt Creator
2. 选择 `File -> Open File or Project`
3. 选择 `CMakeLists.txt` 文件
4. 配置Kit（选择Qt版本和编译器）
5. 点击 `Build -> Build Project`

## 打包成独立exe文件

### Windows平台

#### 方法1: 使用windeployqt（推荐）

1. **编译Release版本**
```cmd
cmake --build . --config Release
```

2. **复制exe到单独目录**
```cmd
mkdir release
copy build\Release\DataUploadTool.exe release\
```

3. **运行windeployqt**
```cmd
cd release
C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe DataUploadTool.exe
```

这将自动复制所有必需的Qt DLL文件到exe所在目录。

#### 方法2: 使用NSIS或Inno Setup制作安装包

1. 使用windeployqt准备所有文件
2. 使用NSIS或Inno Setup创建安装程序

#### 方法3: 使用Qt Installer Framework

Qt官方提供的打包工具，可以创建专业的安装程序。

### Linux平台

```bash
# 安装依赖
sudo apt-get install qtbase5-dev qtbase5-dev-tools

# 编译
mkdir build && cd build
cmake ..
make

# 打包（使用linuxdeployqt或手动打包）
```

### macOS平台

```bash
# 使用macdeployqt
macdeployqt DataUploadTool.app
```

## 项目结构

```
qt-app/
├── CMakeLists.txt      # CMake构建配置
├── main.cpp            # 程序入口
├── MainWindow.h        # 主窗口头文件
├── MainWindow.cpp      # 主窗口实现
└── README.md          # 本文件
```

## 使用说明

1. **启动程序**
   - 双击 `DataUploadTool.exe` 启动程序

2. **配置参数**
   - HTTP地址: 输入数据源服务器的地址（如：http://127.0.0.1）
   - 间隔时间: 设置任务循环执行的间隔（秒）
   - 调用key: 输入API调用密钥

3. **开始执行**
   - 点击"开始执行"按钮开始任务
   - 程序会自动从3个接口获取数据并上传
   - 日志区域会显示详细的执行信息

4. **停止任务**
   - 点击"取消执行"按钮停止任务

## 技术栈

- **框架**: Qt Widgets
- **网络**: QNetworkAccessManager
- **编码处理**: QTextCodec
- **构建系统**: CMake
- **语言**: C++17

## 与原HTA版本的对应关系

| HTA功能 | Qt实现 |
|---------|--------|
| WinHttp.WinHttpRequest | QNetworkAccessManager |
| ADODB.Stream编码转换 | QTextCodec |
| setTimeout定时器 | QTimer |
| HTML日志显示 | QTextEdit + HTML格式 |
| 样式表 | QStyleSheet |

## 故障排除

### 问题1: 找不到Qt库
**解决方案**: 确保设置了正确的 `CMAKE_PREFIX_PATH`，指向Qt安装目录。

### 问题2: 打包后缺少DLL
**解决方案**: 使用 `windeployqt` 工具自动收集所有依赖。

### 问题3: 中文显示乱码
**解决方案**: 确保源代码文件使用UTF-8编码保存。

### 问题4: 网络请求失败
**解决方案**: 检查防火墙设置和网络连接，确保目标服务器可访问。

## 许可证

本项目基于原HTA应用程序重写，保留相同的功能和界面。

## 更新日志

### v1.0.0
- 初始版本
- 实现所有HTA版本的功能
- 支持Windows平台打包

