# PowerShell 脚本：将 Qt 应用打包成单个 exe 文件
# 使用方法：.\package_single_exe.ps1

param(
    [string]$ExePath = "release\DataUploadTool.exe",
    [string]$OutputPath = "release\DataUploadTool_Portable.exe"
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Qt 应用单文件打包工具" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 检查可执行文件是否存在
if (-not (Test-Path $ExePath)) {
    Write-Host "错误: 找不到可执行文件: $ExePath" -ForegroundColor Red
    Write-Host "请确保已运行 windeployqt 收集依赖项" -ForegroundColor Yellow
    exit 1
}

# 获取 release 目录
$ReleaseDir = Split-Path -Parent $ExePath
if ([string]::IsNullOrEmpty($ReleaseDir)) {
    $ReleaseDir = "."
}

Write-Host "源文件: $ExePath" -ForegroundColor Green
Write-Host "输出文件: $OutputPath" -ForegroundColor Green
Write-Host "依赖目录: $ReleaseDir" -ForegroundColor Green
Write-Host ""

# 检查 Enigma Virtual Box 是否已安装
$EnigmaPath = "${env:ProgramFiles}\Enigma Virtual Box\enigmavb.exe"
$EnigmaPathX86 = "${env:ProgramFiles(x86)}\Enigma Virtual Box\enigmavb.exe"

if (Test-Path $EnigmaPath) {
    $EnigmaExe = $EnigmaPath
    Write-Host "找到 Enigma Virtual Box: $EnigmaExe" -ForegroundColor Green
} elseif (Test-Path $EnigmaPathX86) {
    $EnigmaExe = $EnigmaPathX86
    Write-Host "找到 Enigma Virtual Box: $EnigmaExe" -ForegroundColor Green
} else {
    Write-Host "未找到 Enigma Virtual Box" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "请下载并安装 Enigma Virtual Box:" -ForegroundColor Cyan
    Write-Host "  https://enigmaprotector.com/en/downloads.html" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "安装后，此脚本可以自动打包您的应用程序为单个 exe 文件。" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "或者手动使用 Enigma Virtual Box GUI:" -ForegroundColor Yellow
    Write-Host "  1. 打开 Enigma Virtual Box" -ForegroundColor White
    Write-Host "  2. 选择主程序: $ExePath" -ForegroundColor White
    Write-Host "  3. 添加所有文件: $ReleaseDir\*" -ForegroundColor White
    Write-Host "  4. 勾选 '压缩文件' 和 '文件虚拟化'" -ForegroundColor White
    Write-Host "  5. 点击 '打包' 生成单个 exe" -ForegroundColor White
    exit 1
}

# 创建 Enigma Virtual Box 项目文件
$ProjectFile = "$ReleaseDir\package.enigmavb"
$ProjectContent = @"
[Project]
MainFile=$ExePath
OutputFile=$OutputPath
[Options]
FileVirtualization=1
FileCompression=1
[Files]
"$ReleaseDir\*.*"="$ReleaseDir"
"@

Set-Content -Path $ProjectFile -Value $ProjectContent

Write-Host "正在使用 Enigma Virtual Box 打包..." -ForegroundColor Green
Write-Host "这可能需要几分钟时间..." -ForegroundColor Yellow
Write-Host ""

# 使用 Enigma Virtual Box 命令行打包
# 注意：Enigma Virtual Box 的命令行用法可能需要调整
# 如果命令行不可用，脚本会提示手动操作
$Process = Start-Process -FilePath $EnigmaExe -ArgumentList $ProjectFile -Wait -PassThru -NoNewWindow

if ($Process.ExitCode -eq 0 -and (Test-Path $OutputPath)) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "打包成功！" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "输出文件: $OutputPath" -ForegroundColor Green
    $FileSize = (Get-Item $OutputPath).Length / 1MB
    Write-Host "文件大小: $([math]::Round($FileSize, 2)) MB" -ForegroundColor Green
    Write-Host ""
    Remove-Item $ProjectFile -ErrorAction SilentlyContinue
} else {
    Write-Host ""
    Write-Host "命令行打包可能失败，请使用 GUI 方式:" -ForegroundColor Yellow
    Write-Host "  1. 打开 Enigma Virtual Box" -ForegroundColor White
    Write-Host "  2. 文件 -> 打开项目: $ProjectFile" -ForegroundColor White
    Write-Host "  3. 点击 '执行封包' 按钮" -ForegroundColor White
    Write-Host ""
}

