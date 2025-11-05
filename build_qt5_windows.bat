@echo off
REM Qt5 Windows构建和打包脚本
REM 使用方法: 修改下面的Qt路径和编译器配置后运行此脚本

REM ============================================
REM 配置区域 - 请根据您的环境修改
REM ============================================

REM Qt5安装路径（根据您的Qt5版本修改）
REM 常见路径示例:
REM   C:\Qt\5.15.2\msvc2019_64
REM   C:\Qt\5.12.12\msvc2017_64
REM   C:\Qt\5.15.2\mingw81_64
set QT_DIR=C:\Qt\5.15.2\msvc2019_64

REM 构建类型: Release 或 Debug
set BUILD_TYPE=Release

REM CMake生成器（根据您的编译器选择）
REM Visual Studio 2019 (64位)
set CMAKE_GENERATOR=Visual Studio 16 2019
set CMAKE_ARCH=x64

REM Visual Studio 2017 (64位)
REM set CMAKE_GENERATOR=Visual Studio 15 2017 Win64
REM set CMAKE_ARCH=x64

REM MinGW (64位)
REM set CMAKE_GENERATOR=MinGW Makefiles
REM set CMAKE_ARCH=

REM ============================================
REM 检查Qt5路径
REM ============================================
if not exist "%QT_DIR%\bin\qmake.exe" (
    echo 错误: 找不到Qt5安装目录
    echo 当前配置路径: %QT_DIR%
    echo.
    echo 请修改脚本中的QT_DIR变量指向正确的Qt5安装路径
    echo 常见Qt5安装路径:
    echo   C:\Qt\5.15.2\msvc2019_64
    echo   C:\Qt\5.12.12\msvc2017_64
    echo   C:\Qt\5.15.2\mingw81_64
    pause
    exit /b 1
)

REM 检查Qt版本
"%QT_DIR%\bin\qmake.exe" -version
echo.

REM ============================================
REM 创建构建目录
REM ============================================
if exist build rmdir /s /q build
mkdir build
cd build

REM ============================================
REM 配置CMake
REM ============================================
echo 正在配置CMake (Qt5)...
if "%CMAKE_ARCH%"=="" (
    cmake .. -G "%CMAKE_GENERATOR%" -DCMAKE_PREFIX_PATH=%QT_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
) else (
    cmake .. -G "%CMAKE_GENERATOR%" -A %CMAKE_ARCH% -DCMAKE_PREFIX_PATH=%QT_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
)

if errorlevel 1 (
    echo.
    echo CMake配置失败！
    echo.
    echo 可能的原因:
    echo 1. Qt5路径不正确
    echo 2. 未安装CMake (下载: https://cmake.org/download/)
    echo 3. 未安装对应的Visual Studio或MinGW
    echo.
    pause
    exit /b 1
)

REM ============================================
REM 编译项目
REM ============================================
echo.
echo 正在编译项目...
if "%CMAKE_GENERATOR%"=="MinGW Makefiles" (
    mingw32-make -j%NUMBER_OF_PROCESSORS%
) else (
    cmake --build . --config %BUILD_TYPE% --parallel
)

if errorlevel 1 (
    echo.
    echo 编译失败！
    echo.
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
if "%CMAKE_GENERATOR%"=="MinGW Makefiles" (
    copy build\DataUploadTool.exe release\ >nul 2>&1
) else (
    copy build\%BUILD_TYPE%\DataUploadTool.exe release\ >nul 2>&1
)

if errorlevel 1 (
    echo 错误: 找不到编译生成的exe文件
    if "%CMAKE_GENERATOR%"=="MinGW Makefiles" (
        echo 请检查构建目录: build\
    ) else (
        echo 请检查构建目录: build\%BUILD_TYPE%\
    )
    pause
    exit /b 1
)

REM ============================================
REM 使用windeployqt打包依赖
REM ============================================
echo.
echo 正在打包Qt5依赖...
cd release
"%QT_DIR%\bin\windeployqt.exe" DataUploadTool.exe

if errorlevel 1 (
    echo 警告: windeployqt执行失败，可能需要手动复制依赖文件
    echo.
    echo 手动打包步骤:
    echo 1. 复制以下Qt5 DLL到release目录:
    echo    - Qt5Core.dll
    echo    - Qt5Network.dll
    echo    - Qt5Widgets.dll
    echo    - Qt5Gui.dll
    echo.
    echo 2. 复制Qt5插件目录 (plugins/platforms/qwindows.dll)
    echo.
) else (
    echo Qt5依赖打包完成！
)

cd ..

echo.
echo ============================================
echo Qt5构建完成！
echo ============================================
echo 可执行文件位置: release\DataUploadTool.exe
echo.
echo 您可以将release目录中的所有文件打包分发
echo ============================================
pause

