@echo off
setlocal enabledelayedexpansion
REM 复制 OpenSSL DLL 文件到 release 目录
REM 用于支持 HTTPS/TLS 功能

echo ========================================
echo 复制 OpenSSL DLL 文件和 Qt SSL 插件
echo ========================================
echo.

set RELEASE_DIR=release

REM 检查 release 目录是否存在
if not exist "%RELEASE_DIR%" (
    echo 错误: release 目录不存在
    echo 请先运行构建脚本
    pause
    exit /b 1
)

REM 检查 exe 文件是否存在
if not exist "%RELEASE_DIR%\DataUploadTool.exe" (
    echo 错误: 找不到 DataUploadTool.exe
    pause
    exit /b 1
)

REM 方法1: 优先从项目 dll/64 目录复制（64位版本，最可靠）
if exist "dll\64\libcrypto-1_1.dll" if exist "dll\64\libssl-1_1.dll" (
    copy "dll\64\libcrypto-1_1.dll" "%RELEASE_DIR%\" /Y >nul
    copy "dll\64\libssl-1_1.dll" "%RELEASE_DIR%\" /Y >nul
    echo 已从项目 dll/64 目录复制 OpenSSL DLL (64位版本)
    echo   - libcrypto-1_1.dll
    echo   - libssl-1_1.dll
    REM 继续查找Qt目录以复制SSL插件（不goto，继续执行）
)

REM 方法1b: 从项目 dll 目录复制（备用）
if exist "dll\libcrypto-1_1.dll" if exist "dll\libssl-1_1.dll" (
    copy "dll\libcrypto-1_1.dll" "%RELEASE_DIR%\" /Y >nul
    copy "dll\libssl-1_1.dll" "%RELEASE_DIR%\" /Y >nul
    echo 已从项目 dll 目录复制 OpenSSL DLL
    echo   - libcrypto-1_1.dll
    echo   - libssl-1_1.dll
    REM 继续查找Qt目录以复制SSL插件
)

REM 方法2: 尝试从 Qt 目录复制
set QT_FOUND=0

REM 优先使用环境变量
if defined Qt5_DIR (
    set QT_BIN=%Qt5_DIR%\bin
    echo 尝试从环境变量 Qt5_DIR 查找: %QT_BIN%
    if exist "%QT_BIN%\libcrypto-1_1.dll" if exist "%QT_BIN%\libssl-1_1.dll" (
        copy "%QT_BIN%\libcrypto-1_1.dll" "%RELEASE_DIR%\" /Y >nul
        copy "%QT_BIN%\libssl-1_1.dll" "%RELEASE_DIR%\" /Y >nul
        echo 已从 Qt 目录复制 OpenSSL DLL
        echo   - libcrypto-1_1.dll
        echo   - libssl-1_1.dll
        set QT_FOUND=1
        set QT_DIR=%Qt5_DIR%
    )
)

REM 如果环境变量未设置，尝试从常见路径查找
if %QT_FOUND%==0 (
    echo 环境变量 Qt5_DIR 未设置，尝试从常见路径查找...
    for %%Q in (
        "C:\Qt\5.15.2\msvc2019_64"
        "C:\Qt\5.15.2\msvc2017_64"
        "C:\Qt\5.12.12\msvc2019_64"
        "C:\Qt\5.12.12\msvc2017_64"
        "C:\Qt\5.15.2\mingw81_64"
    ) do (
        if not exist "%RELEASE_DIR%\libcrypto-1_1.dll" (
            set QT_BIN=%%~Q\bin
            if exist "!QT_BIN!\libcrypto-1_1.dll" if exist "!QT_BIN!\libssl-1_1.dll" (
                copy "!QT_BIN!\libcrypto-1_1.dll" "%RELEASE_DIR%\" /Y >nul
                copy "!QT_BIN!\libssl-1_1.dll" "%RELEASE_DIR%\" /Y >nul
                echo 已从 Qt 目录复制 OpenSSL DLL: %%~Q
                echo   - libcrypto-1_1.dll
                echo   - libssl-1_1.dll
                set QT_DIR=%%~Q
                set QT_FOUND=1
                goto :qt_found
            )
        )
    )
    :qt_found
)

REM 复制 Qt SSL 插件（关键！）
:copy_ssl_plugin
if defined QT_DIR (
    set QT_PLUGINS=%QT_DIR%\plugins\ssl
    if exist "%QT_PLUGINS%" (
        echo.
        echo 复制 Qt SSL 插件...
        if not exist "%RELEASE_DIR%\plugins\ssl" mkdir "%RELEASE_DIR%\plugins\ssl"
        copy "%QT_PLUGINS%\*.dll" "%RELEASE_DIR%\plugins\ssl\" /Y >nul
        if errorlevel 1 (
            echo 警告: Qt SSL 插件复制失败
        ) else (
            echo 已复制 Qt SSL 插件到 plugins\ssl 目录
            dir "%RELEASE_DIR%\plugins\ssl\*.dll" /b
        )
    ) else (
        echo 警告: 未找到 Qt SSL 插件目录: %QT_PLUGINS%
        echo 这可能导致 SSL/TLS 功能无法正常工作
    )
) else (
    echo 警告: 未找到 Qt 安装目录，无法复制 SSL 插件
    echo 请手动复制 Qt 安装目录下的 plugins\ssl 文件夹到 %RELEASE_DIR%\plugins\ssl
)

REM 检查是否已复制成功
:success
if exist "%RELEASE_DIR%\libcrypto-1_1.dll" if exist "%RELEASE_DIR%\libssl-1_1.dll" (
    echo.
    echo ========================================
    echo OpenSSL DLL 文件已成功复制！
    echo ========================================
    echo.
    echo 文件列表:
    dir "%RELEASE_DIR%\libcrypto*.dll" /b
    dir "%RELEASE_DIR%\libssl*.dll" /b
    echo.
    if exist "%RELEASE_DIR%\plugins\ssl\*.dll" (
        echo SSL 插件已复制到 plugins\ssl 目录:
        dir "%RELEASE_DIR%\plugins\ssl\*.dll" /b
    ) else (
        echo 警告: SSL 插件可能未复制
        echo 这可能导致 SSL/TLS 功能无法正常工作
        echo 请手动复制 Qt 安装目录下的 plugins\ssl 文件夹
    )
    echo.
    pause
    exit /b 0
)

REM 方法2: 提示用户手动下载
echo.
echo ========================================
echo 未找到 OpenSSL DLL 文件
echo ========================================
echo.
echo 请手动下载 OpenSSL 1.1.1 DLL 文件:
echo 1. 访问: https://slproweb.com/products/Win32OpenSSL.html
echo 2. 下载 OpenSSL 1.1.1 64位版本
echo 3. 安装后，从安装目录的 bin 文件夹复制以下文件到 %RELEASE_DIR% 目录:
echo    - libcrypto-1_1.dll
echo    - libssl-1_1.dll
echo.
echo 或者从以下地址下载预编译版本:
echo https://indy.fulgan.com/SSL/
echo.
pause

