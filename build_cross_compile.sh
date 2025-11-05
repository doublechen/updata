#!/bin/bash
# macOS 交叉编译 Windows EXE 脚本（使用 MXE）
# 注意：此脚本需要先安装 MXE，可能需要几小时

set -e

# ============================================
# 配置区域
# ============================================

# MXE 安装路径（默认在用户目录）
MXE_DIR="${HOME}/mxe"
MXE_TARGET="x86_64-w64-mingw32.shared"

# ============================================
# 检查 MXE 是否已安装
# ============================================
if [ ! -d "$MXE_DIR" ]; then
    echo "错误: MXE 未安装"
    echo ""
    echo "请先安装 MXE:"
    echo "  git clone https://github.com/mxe/mxe.git ~/mxe"
    echo "  cd ~/mxe"
    echo "  make qtbase qtnetwork MXE_TARGETS=x86_64-w64-mingw32.shared"
    echo ""
    echo "注意: 编译过程可能需要数小时"
    exit 1
fi

# 检查工具链是否已编译
if [ ! -f "$MXE_DIR/usr/$MXE_TARGET/bin/g++.exe" ]; then
    echo "错误: MXE 工具链未完全编译"
    echo ""
    echo "请先编译 MXE 工具链:"
    echo "  cd ~/mxe"
    echo "  make qtbase qtnetwork MXE_TARGETS=$MXE_TARGET"
    exit 1
fi

# ============================================
# 设置环境变量
# ============================================
export PATH="$MXE_DIR/usr/bin:$PATH"
export CMAKE_PREFIX_PATH="$MXE_DIR/usr/$MXE_TARGET/qt5"

# ============================================
# 清理旧的构建
# ============================================
if [ -d "build-cross" ]; then
    echo "清理旧的构建目录..."
    rm -rf build-cross
fi

# ============================================
# 创建构建目录
# ============================================
mkdir -p build-cross
cd build-cross

# ============================================
# 配置 CMake
# ============================================
echo "配置 CMake (交叉编译)..."

# 查找 MXE 的 CMake 工具链文件
TOOLCHAIN_FILE="$MXE_DIR/usr/$MXE_TARGET/share/cmake/mxe-conf.cmake"

if [ ! -f "$TOOLCHAIN_FILE" ]; then
    # 尝试其他可能的位置
    TOOLCHAIN_FILE="$MXE_DIR/usr/$MXE_TARGET/share/cmake/mxe-conf.cmake"
    
    if [ ! -f "$TOOLCHAIN_FILE" ]; then
        echo "错误: 找不到 MXE CMake 工具链文件"
        echo "请检查 MXE 安装是否完整"
        exit 1
    fi
fi

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"

# ============================================
# 编译
# ============================================
echo "开始编译..."
cmake --build . --parallel

# ============================================
# 创建发布目录
# ============================================
cd ..
if [ -d "release-cross" ]; then
    rm -rf release-cross
fi
mkdir -p release-cross

# 复制可执行文件
cp build-cross/DataUploadTool.exe release-cross/ 2>/dev/null || true

if [ -f "release-cross/DataUploadTool.exe" ]; then
    echo ""
    echo "============================================"
    echo "交叉编译完成！"
    echo "============================================"
    echo "可执行文件位置: release-cross/DataUploadTool.exe"
    echo ""
    echo "注意: 这是 Windows 可执行文件，无法在 macOS 上直接运行"
    echo "请将其传输到 Windows 系统进行测试"
    echo "============================================"
else
    echo "警告: 未找到编译生成的可执行文件"
    echo "请检查编译日志以查找错误"
    exit 1
fi

