# macOS 上编译 Windows EXE 文件指南

在 macOS 上编译 Windows 的 exe 文件需要交叉编译。以下是几种可行的方案：

## 方案对比

| 方案 | 难度 | 推荐度 | 说明 |
|------|------|--------|------|
| Windows 虚拟机 | ⭐⭐ | ⭐⭐⭐⭐⭐ | 最简单可靠 |
| GitHub Actions | ⭐⭐ | ⭐⭐⭐⭐⭐ | 自动化，免费 |
| Wine + MXE | ⭐⭐⭐⭐ | ⭐⭐⭐ | 复杂但可行 |
| Docker | ⭐⭐⭐ | ⭐⭐⭐⭐ | 需要 Docker 环境 |
| 远程 Windows 机器 | ⭐ | ⭐⭐⭐⭐ | 如果有 Windows 机器 |

## 方案 1: Windows 虚拟机（最推荐）

### 优点
- ✅ 最简单可靠
- ✅ 完全兼容 Windows 环境
- ✅ 可以使用 Windows 工具链

### 步骤

1. **安装虚拟机软件**
   - Parallels Desktop（商业软件，性能最好）
   - VMware Fusion（商业软件）
   - VirtualBox（免费）

2. **安装 Windows 10/11**
   - 下载 Windows ISO
   - 在虚拟机中安装

3. **在 Windows 虚拟机中编译**
   ```cmd
   # 安装 Qt5 和 Visual Studio
   # 然后运行构建脚本
   build_qt5_windows.bat
   ```

## 方案 2: GitHub Actions（自动化编译）

### 优点
- ✅ 完全自动化
- ✅ 免费使用
- ✅ 支持多平台编译

### 创建 GitHub Actions 工作流

创建 `.github/workflows/build-windows.yml`:

```yaml
name: Build Windows EXE

on:
  push:
    branches: [ main, master ]
  pull_request:
    branches: [ main, master ]
  workflow_dispatch:

jobs:
  build-windows:
    runs-on: windows-latest
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Setup Qt
      uses: jurplel/install-qt-action@v3
      with:
        version: '5.15.2'
        modules: 'qtbase qtnetwork qtdeclarative'
        arch: 'win64_msvc2019_64'
    
    - name: Configure CMake
      run: |
        cmake -B build -S qt-app `
          -DCMAKE_BUILD_TYPE=Release `
          -DCMAKE_PREFIX_PATH="$env:Qt5_DIR"
    
    - name: Build
      run: cmake --build build --config Release
    
    - name: Package
      run: |
        mkdir release
        copy build\Release\DataUploadTool.exe release\
        $env:Qt5_DIR\bin\windeployqt.exe release\DataUploadTool.exe
    
    - name: Upload artifacts
      uses: actions/upload-artifact@v3
      with:
        name: DataUploadTool-Windows
        path: release/*
```

### 使用方法

1. 将代码推送到 GitHub
2. GitHub Actions 会自动编译
3. 在 Actions 页面下载编译好的 exe

## 方案 3: Wine + MXE（交叉编译）

### 优点
- ✅ 在 macOS 上直接编译
- ✅ 不需要虚拟机

### 缺点
- ❌ 配置复杂
- ❌ 需要较长时间设置

### 安装步骤

1. **安装 Homebrew（如果还没有）**
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. **安装 Wine**
```bash
brew install --cask wine-stable
```

3. **安装 MXE（Minimal Qt Build Environment）**
```bash
# 克隆 MXE
git clone https://github.com/mxe/mxe.git
cd mxe

# 编译工具链（需要很长时间，可能需要几小时）
make qtbase MXE_TARGETS=x86_64-w64-mingw32.shared

# 编译 Qt 网络模块
make qtnetwork MXE_TARGETS=x86_64-w64-mingw32.shared
```

4. **设置环境变量**
```bash
export PATH="$HOME/mxe/usr/bin:$PATH"
export CMAKE_PREFIX_PATH="$HOME/mxe/usr/x86_64-w64-mingw32.shared/qt5"
```

5. **交叉编译**
```bash
cd qt-app
mkdir build-cross && cd build-cross
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=~/mxe/usr/x86_64-w64-mingw32.shared/share/cmake/mxe-conf.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## 方案 4: Docker（Windows 容器）

### 优点
- ✅ 隔离环境
- ✅ 可重复构建

### 步骤

1. **创建 Dockerfile**
```dockerfile
FROM mcr.microsoft.com/windows/servercore:ltsc2019

# 安装 Chocolatey
RUN powershell -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; \
    iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# 安装 Qt 和构建工具
RUN choco install cmake visualstudio2019buildtools -y

# 设置工作目录
WORKDIR /app
```

2. **构建 Docker 镜像**
```bash
docker build -t qt-windows-builder .
```

3. **在容器中编译**
```bash
docker run -v $(pwd):/app qt-windows-builder
```

## 方案 5: 使用在线编译服务

### AppVeyor
- 免费提供 Windows 编译环境
- 支持 GitHub 集成
- 网址: https://www.appveyor.com/

### 配置示例 (appveyor.yml):
```yaml
image: Visual Studio 2019

build_script:
  - cmake -B build -S qt-app -DCMAKE_BUILD_TYPE=Release
  - cmake --build build --config Release

artifacts:
  - path: build\Release\DataUploadTool.exe
```

## 方案 6: 使用远程 Windows 机器

如果有 Windows 机器（实体机或云服务器）：

1. **使用 SSH 连接**
```bash
ssh user@windows-machine
```

2. **在 Windows 上编译**
```cmd
git clone <your-repo>
cd qt-app
build_qt5_windows.bat
```

3. **下载编译结果**
```bash
scp user@windows-machine:/path/to/exe ./DataUploadTool.exe
```

## 推荐方案

### 对于个人开发者
**推荐：GitHub Actions** 或 **Windows 虚拟机**

- GitHub Actions: 自动化，无需本地配置
- Windows 虚拟机: 如果需要调试和测试

### 对于团队
**推荐：GitHub Actions** 或 **CI/CD 服务**

- 自动化构建
- 统一构建环境
- 版本管理

## 快速开始（GitHub Actions）

1. **在项目根目录创建 `.github/workflows/build-windows.yml`**
2. **将代码推送到 GitHub**
3. **在 Actions 页面查看编译进度**
4. **下载编译好的 exe 文件**

## 注意事项

1. **代码签名**: Windows exe 可能需要代码签名以避免安全警告
2. **依赖库**: 确保所有依赖都包含在 exe 中
3. **测试**: 在真实的 Windows 环境中测试 exe
4. **版本号**: 确保版本号正确设置

## 相关资源

- [MXE 官方文档](https://mxe.cc/)
- [GitHub Actions 文档](https://docs.github.com/en/actions)
- [Qt 交叉编译指南](https://doc.qt.io/qt-5/configure-options.html)

