#!/bin/bash
# 将 Qt 应用打包成单个 exe 文件的说明脚本
# 注意：此脚本主要用于说明，实际打包需要在 Windows 上使用 PowerShell 脚本

echo "=========================================="
echo "Qt 应用单文件打包说明"
echo "=========================================="
echo ""
echo "Windows 平台打包方法："
echo ""
echo "方法 1: 使用 Enigma Virtual Box (推荐)"
echo "1. 下载安装 Enigma Virtual Box:"
echo "   https://enigmaprotector.com/en/downloads.html"
echo ""
echo "2. 运行打包脚本:"
echo "   .\package_single_exe.ps1"
echo ""
echo "方法 2: 使用静态链接 Qt"
echo "需要静态编译的 Qt 库，修改 CMakeLists.txt 启用静态链接"
echo ""
echo "方法 3: 使用其他打包工具"
echo "- BoxedApp Packer"
echo "- UPX (压缩，但不打包 DLL)"
echo ""

