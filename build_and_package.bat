@echo off
REM Windows构建和打包脚本
REM 使用方法: 修改下面的Qt路径后运行此脚本

REM ============================================
REM 配置区域 - 请根据您的环境修改
REM ============================================
set QT_DIR=C:\Qt\6.5.0\msvc2019_64
set BUILD_TYPE=Release
set CMAKE_GENERATOR=Visual Studio 16 2019
set CMAKE_ARCH=x64

REM ============================================
REM 检查Qt路径
REM ============================================
if not exist "%QT_DIR%\bin\qmake.exe" (
    echo 错误: 找不到Qt安装目录
    echo 请修改脚本中的QT_DIR变量指向正确的Qt安装路径
    pause
    exit /b 1
)

REM ============================================
REM 创建构建目录
REM ============================================
if exist build rmdir /s /q build
mkdir build
cd build

REM ============================================
REM 配置CMake
REM ============================================
echo 正在配置CMake...
cmake .. -G "%CMAKE_GENERATOR%" -A %CMAKE_ARCH% -DCMAKE_PREFIX_PATH=%QT_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 (
    echo CMake配置失败
    pause
    exit /b 1
)

REM ============================================
REM 编译项目
REM ============================================
echo 正在编译项目...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo 编译失败
    pause
    exit /b 1
)

REM ============================================
REM 创建发布目录
REM ============================================
cd ..
if exist release rmdir /s /q release
mkdir release

REM ============================================
REM 复制exe文件
REM ============================================
copy build\%BUILD_TYPE%\DataUploadTool.exe release\ >nul
if errorlevel 1 (
    echo 错误: 找不到编译生成的exe文件
    echo 请检查构建目录: build\%BUILD_TYPE%\
    pause
    exit /b 1
)

REM ============================================
REM 使用windeployqt打包依赖
REM ============================================
echo 正在打包Qt依赖...
cd release
"%QT_DIR%\bin\windeployqt.exe" DataUploadTool.exe
if errorlevel 1 (
    echo 警告: windeployqt执行失败，可能需要手动复制依赖文件
)

cd ..

echo.
echo ============================================
echo 构建完成！
echo ============================================
echo 可执行文件位置: release\DataUploadTool.exe
echo.
echo 您可以将release目录中的所有文件打包分发
echo ============================================
pause

