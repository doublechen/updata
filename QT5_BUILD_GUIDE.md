# Qt5 配置和编译指南

本文档详细说明如何使用Qt5配置和编译数据上传工具。

## 目录

- [系统要求](#系统要求)
- [安装Qt5](#安装qt5)
- [Windows平台编译](#windows平台编译)
- [Linux平台编译](#linux平台编译)
- [macOS平台编译](#macos平台编译)
- [常见问题](#常见问题)

## 系统要求

### Windows
- Windows 7 或更高版本
- Qt 5.12.0 或更高版本（推荐 5.15.2）
- CMake 3.16 或更高版本
- Visual Studio 2017/2019/2022 或 MinGW 8.1+

### Linux
- Ubuntu 18.04+ / Debian 10+ / CentOS 7+ / Fedora 30+
- Qt 5.12.0 或更高版本（推荐 5.15.2）
- CMake 3.16 或更高版本
- GCC 7+ 或 Clang 6+

### macOS
- macOS 10.13 或更高版本
- Qt 5.12.0 或更高版本（推荐 5.15.2）
- CMake 3.16 或更高版本
- Xcode Command Line Tools

## 安装Qt5

### 方法1: 从Qt官网下载（推荐）

1. 访问 [Qt下载页面](https://www.qt.io/download)
2. 注册Qt账号（免费）
3. 下载Qt 5.15.2 或更新版本
4. 运行安装程序，选择以下组件：
   - Qt 5.15.2 (或更新版本)
   - 编译器（根据您的系统选择）：
     - Windows: MSVC 2019 64-bit 或 MinGW 8.1.0 64-bit
     - Linux: GCC 64-bit
     - macOS: Clang 64-bit
   - Qt Creator (可选，但推荐)

### 方法2: 使用包管理器（Linux）

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install qt5-default qtbase5-dev qtbase5-dev-tools
sudo apt-get install cmake build-essential
```

#### CentOS/RHEL
```bash
sudo yum install qt5-qtbase-devel cmake gcc-c++
```

#### Fedora
```bash
sudo dnf install qt5-qtbase-devel cmake gcc-c++
```

### 方法3: 使用Homebrew (macOS)
```bash
brew install qt@5 cmake
```

## Windows平台编译

### 方法1: 使用自动构建脚本（推荐）

1. **修改构建脚本**
   
   打开 `build_qt5_windows.bat`，修改以下配置：

```batch
REM Qt5安装路径
set QT_DIR=C:\Qt\5.15.2\msvc2019_64

REM 构建类型
set BUILD_TYPE=Release

REM CMake生成器（根据您的编译器选择）
set CMAKE_GENERATOR=Visual Studio 16 2019
set CMAKE_ARCH=x64
```

2. **运行脚本**
   
   双击运行 `build_qt5_windows.bat`，或在命令行中执行：
```cmd
build_qt5_windows.bat
```

3. **获取结果**
   
   编译完成后，可执行文件在 `release\DataUploadTool.exe`

### 方法2: 手动编译

#### 使用Visual Studio

1. **打开Developer Command Prompt**
   
   在开始菜单搜索 "Developer Command Prompt for VS 2019"

2. **配置CMake**
```cmd
cd qt-app
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64 ^
    -DCMAKE_BUILD_TYPE=Release
```

3. **编译**
```cmd
cmake --build . --config Release
```

4. **打包**
```cmd
cd ..
mkdir release
copy build\Release\DataUploadTool.exe release\
cd release
C:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe DataUploadTool.exe
```

#### 使用MinGW

1. **打开MinGW命令行**
   
   在开始菜单搜索 "Qt 5.15.2 for Desktop (MinGW 8.1.0 64-bit)"

2. **配置和编译**
```cmd
cd qt-app
mkdir build
cd build
cmake .. -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\mingw81_64 ^
    -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

3. **打包**
```cmd
cd ..
mkdir release
copy build\DataUploadTool.exe release\
cd release
C:\Qt\5.15.2\mingw81_64\bin\windeployqt.exe DataUploadTool.exe
```

### 验证Qt5安装

在命令行中运行：
```cmd
qmake -version
```

应该看到类似输出：
```
QMake version 3.1
Using Qt version 5.15.2 in C:/Qt/5.15.2/msvc2019_64/lib
```

## Linux平台编译

### 方法1: 使用自动构建脚本（推荐）

1. **赋予执行权限**
```bash
chmod +x build_qt5_linux.sh
```

2. **运行脚本**
```bash
./build_qt5_linux.sh
```

3. **如果使用自定义Qt路径**
   
   编辑 `build_qt5_linux.sh`，设置：
```bash
QT_DIR="/opt/Qt5.15.2/5.15.2/gcc_64"
```

### 方法2: 手动编译

1. **安装依赖**
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install qt5-default qtbase5-dev qtbase5-dev-tools cmake build-essential

# CentOS/RHEL
sudo yum install qt5-qtbase-devel cmake gcc-c++
```

2. **配置和编译**
```bash
cd qt-app
mkdir build && cd build

# 如果使用系统Qt5
cmake .. -DCMAKE_BUILD_TYPE=Release

# 如果使用自定义Qt5路径
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/opt/Qt5.15.2/5.15.2/gcc_64

# 编译
cmake --build . --parallel
```

3. **运行**
```bash
./DataUploadTool
```

### 验证Qt5安装

```bash
qmake -version
```

## macOS平台编译

1. **安装Qt5和CMake**
```bash
brew install qt@5 cmake
```

2. **配置环境变量**
```bash
export PATH="/usr/local/opt/qt@5/bin:$PATH"
```

3. **配置和编译**
```bash
cd qt-app
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@5
cmake --build . --parallel
```

4. **打包**
```bash
cd ..
mkdir release
cp -r build/DataUploadTool.app release/
cd release
/usr/local/opt/qt@5/bin/macdeployqt DataUploadTool.app
```

## 常见问题

### Q1: CMake找不到Qt5

**错误信息:**
```
CMake Error at CMakeLists.txt:13 (find_package):
  Could not find a package configuration file provided by "Qt5"
```

**解决方法:**

1. **检查Qt5安装路径**
```bash
# Windows
where qmake

# Linux/macOS
which qmake
```

2. **设置CMAKE_PREFIX_PATH**
```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt5
```

3. **Windows示例**
```cmd
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64
```

### Q2: 编译错误 "undefined reference to Qt5"

**原因:** Qt库链接失败

**解决方法:**

1. 确保CMake正确找到Qt5
2. 检查CMakeLists.txt中的Qt组件是否都安装了
3. 重新运行CMake配置

### Q3: windeployqt找不到DLL

**解决方法:**

1. 确保使用正确的windeployqt路径
2. 检查Qt5安装是否完整
3. 手动复制必要的DLL：
   - Qt5Core.dll
   - Qt5Network.dll
   - Qt5Widgets.dll
   - Qt5Gui.dll

### Q4: Linux下运行提示缺少Qt库

**解决方法:**

1. **检查依赖**
```bash
ldd DataUploadTool
```

2. **安装缺失的库**
```bash
# Ubuntu/Debian
sudo apt-get install libqt5core5a libqt5network5 libqt5widgets5 libqt5gui5

# CentOS/RHEL
sudo yum install qt5-qtbase
```

3. **或使用静态链接**（需要重新编译Qt）

### Q5: 中文显示乱码

**解决方法:**

1. 确保源代码文件使用UTF-8编码
2. Qt5默认支持UTF-8，无需特殊配置

### Q6: 如何选择Qt5版本？

**推荐版本:**
- **Qt 5.15.2** (LTS，最稳定)
- **Qt 5.12.12** (LTS，旧系统兼容性好)
- **Qt 5.9.9** (如果必须支持Windows 7)

**不推荐:**
- Qt 5.0-5.11 (太旧，可能有兼容性问题)
- Qt 5.13-5.14 (非LTS版本)

## 编译选项说明

### CMAKE_PREFIX_PATH

指定Qt5的安装路径，CMake会在此路径下查找Qt5的配置文件。

**Windows示例:**
```
-DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64
```

**Linux示例:**
```
-DCMAKE_PREFIX_PATH=/opt/Qt5.15.2/5.15.2/gcc_64
```

### CMAKE_BUILD_TYPE

指定构建类型：
- `Release`: 发布版本（优化，无调试信息）
- `Debug`: 调试版本（包含调试信息，未优化）

### 并行编译

加快编译速度：
```bash
cmake --build . --parallel
# 或指定线程数
cmake --build . --parallel 4
```

## 打包说明

### Windows

使用 `windeployqt` 自动收集依赖：
```cmd
windeployqt.exe DataUploadTool.exe
```

### Linux

通常需要目标系统安装相同的Qt5库，或者：
1. 使用静态链接（需要重新编译Qt）
2. 使用AppImage或Flatpak打包
3. 使用Snap打包

### macOS

使用 `macdeployqt`：
```bash
macdeployqt DataUploadTool.app
```

## 更多资源

- [Qt5官方文档](https://doc.qt.io/qt-5/)
- [CMake官方文档](https://cmake.org/documentation/)
- [Qt5安装指南](https://doc.qt.io/qt-5/gettingstarted.html)

