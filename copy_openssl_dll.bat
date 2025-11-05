@echo off
REM 复制 OpenSSL DLL 文件到 release 目录
REM 用于支持 HTTPS/TLS 功能

echo ========================================
echo 复制 OpenSSL DLL 文件
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
    goto :copy_ssl_plugin
)

REM 方法1b: 从项目 dll 目录复制（备用）
if exist "dll\libcrypto-1_1.dll" if exist "dll\libssl-1_1.dll" (
    copy "dll\libcrypto-1_1.dll" "%RELEASE_DIR%\" /Y >nul
    copy "dll\libssl-1_1.dll" "%RELEASE_DIR%\" /Y >nul
    echo 已从项目 dll 目录复制 OpenSSL DLL
    echo   - libcrypto-1_1.dll
    echo   - libssl-1_1.dll
    goto :copy_ssl_plugin
)

REM 方法2: 尝试从 Qt 目录复制
if defined Qt5_DIR (
    set QT_BIN=%Qt5_DIR%\bin
    echo 查找 Qt 目录: %QT_BIN%
    
    if exist "%QT_BIN%\libcrypto-1_1.dll" (
        copy "%QT_BIN%\libcrypto-1_1.dll" "%RELEASE_DIR%\" /Y >nul
        echo 已从 Qt 目录复制: libcrypto-1_1.dll
    )
    
    if exist "%QT_BIN%\libssl-1_1.dll" (
        copy "%QT_BIN%\libssl-1_1.dll" "%RELEASE_DIR%\" /Y >nul
        echo 已从 Qt 目录复制: libssl-1_1.dll
    )
)

REM 复制 Qt SSL 插件（关键！）
:copy_ssl_plugin
if defined Qt5_DIR (
    set QT_PLUGINS=%Qt5_DIR%\plugins\ssl
    if exist "%QT_PLUGINS%" (
        echo.
        echo 复制 Qt SSL 插件...
        if not exist "%RELEASE_DIR%\plugins\ssl" mkdir "%RELEASE_DIR%\plugins\ssl"
        copy "%QT_PLUGINS%\*.dll" "%RELEASE_DIR%\plugins\ssl\" /Y >nul
        echo 已复制 Qt SSL 插件到 plugins\ssl 目录
    ) else (
        echo 警告: 未找到 Qt SSL 插件目录: %QT_PLUGINS%
    )
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
        echo SSL 插件已复制到 plugins\ssl 目录
    ) else (
        echo 警告: SSL 插件可能未复制
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

