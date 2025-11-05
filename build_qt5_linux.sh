#!/bin/bash
# Qt5 Linux构建脚本
# 使用方法: chmod +x build_qt5_linux.sh && ./build_qt5_linux.sh

set -e  # 遇到错误立即退出

# ============================================
# 配置区域 - 请根据您的环境修改
# ============================================

# Qt5安装路径（如果使用系统包管理器安装，可能不需要设置）
# 如果Qt5安装在系统路径，可以留空或注释掉
# QT_DIR="/opt/Qt5.15.2/5.15.2/gcc_64"

# 构建类型: Release 或 Debug
BUILD_TYPE="Release"

# ============================================
# 检查Qt5安装
# ============================================
echo "检查Qt5安装..."

if command -v qmake &> /dev/null; then
    QMAKE_VERSION=$(qmake -version | head -n 1)
    echo "找到系统Qt: $QMAKE_VERSION"
    QT_DIR=""
elif [ ! -z "$QT_DIR" ] && [ -f "$QT_DIR/bin/qmake" ]; then
    echo "使用指定Qt路径: $QT_DIR"
    export PATH="$QT_DIR/bin:$PATH"
else
    echo "错误: 找不到Qt5安装"
    echo ""
    echo "请选择以下方式之一安装Qt5:"
    echo "1. 使用包管理器安装:"
    echo "   Ubuntu/Debian: sudo apt-get install qt5-default qtbase5-dev qtbase5-dev-tools"
    echo "   CentOS/RHEL: sudo yum install qt5-qtbase-devel"
    echo "   Fedora: sudo dnf install qt5-qtbase-devel"
    echo ""
    echo "2. 从Qt官网下载: https://www.qt.io/download"
    echo "   然后设置QT_DIR变量指向Qt5安装目录"
    exit 1
fi

# 验证Qt版本
QT_VERSION=$(qmake -query QT_VERSION)
echo "Qt版本: $QT_VERSION"

# 检查Qt版本是否为5.x
if [[ ! "$QT_VERSION" =~ ^5\. ]]; then
    echo "警告: 检测到的Qt版本不是5.x系列，当前版本: $QT_VERSION"
    read -p "是否继续? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# ============================================
# 安装依赖（如果需要）
# ============================================
echo ""
echo "检查系统依赖..."

if command -v apt-get &> /dev/null; then
    echo "检测到apt包管理器，检查依赖..."
    # 检查并安装必要的依赖
    MISSING_DEPS=""
    for dep in build-essential cmake; do
        if ! dpkg -l | grep -q "^ii  $dep"; then
            MISSING_DEPS="$MISSING_DEPS $dep"
        fi
    done
    
    if [ ! -z "$MISSING_DEPS" ]; then
        echo "需要安装以下依赖: $MISSING_DEPS"
        read -p "是否现在安装? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            sudo apt-get update
            sudo apt-get install -y $MISSING_DEPS
        fi
    fi
fi

# ============================================
# 创建构建目录
# ============================================
if [ -d "build" ]; then
    echo "清理旧的构建目录..."
    rm -rf build
fi

mkdir build
cd build

# ============================================
# 配置CMake
# ============================================
echo ""
echo "正在配置CMake (Qt5)..."

CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
if [ ! -z "$QT_DIR" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_PREFIX_PATH=$QT_DIR"
fi

cmake .. $CMAKE_ARGS

if [ $? -ne 0 ]; then
    echo ""
    echo "CMake配置失败！"
    echo ""
    echo "可能的原因:"
    echo "1. 未安装CMake (sudo apt-get install cmake)"
    echo "2. Qt5开发包未正确安装"
    echo "3. 编译器未安装"
    exit 1
fi

# ============================================
# 编译项目
# ============================================
echo ""
echo "正在编译项目..."
cmake --build . --parallel

if [ $? -ne 0 ]; then
    echo ""
    echo "编译失败！"
    exit 1
fi

# ============================================
# 创建发布目录
# ============================================
cd ..
if [ -d "release" ]; then
    rm -rf release
fi

mkdir release

# ============================================
# 复制可执行文件
# ============================================
cp build/DataUploadTool release/

if [ ! -f "release/DataUploadTool" ]; then
    echo "错误: 找不到编译生成的可执行文件"
    exit 1
fi

# ============================================
# 打包依赖（可选）
# ============================================
echo ""
echo "构建完成！"
echo ""
echo "可执行文件位置: release/DataUploadTool"
echo ""
echo "注意: Linux下的Qt应用依赖系统的Qt库，"
echo "如果要在其他机器上运行，需要确保目标机器安装了相同的Qt5库"
echo ""
echo "可以使用以下命令检查依赖:"
echo "  ldd release/DataUploadTool"
echo ""

