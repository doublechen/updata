# 🎨 应用程序图标配置说明

## ✅ 已完成的配置

### 1. 创建了 Windows 资源文件
**文件**: `app.rc`
- 定义了应用程序图标
- 包含版本信息（文件描述、版本号等）
- 会被编译到 exe 文件中

### 2. 修改了 CMakeLists.txt
**变更**:
- 添加了资源文件引用
- Windows 平台会自动包含 `app.rc`
- 编译时会将图标嵌入到 exe 中

### 3. 添加了 GitHub Actions 构建步骤
**新步骤**: "Convert PNG to ICO"
- 自动将 `icon-1024x1024.png` 转换为 `app_icon.ico`
- 使用 Python Pillow 库进行转换
- 生成多尺寸图标（256、128、64、48、32、16）

## 📋 文件说明

### app.rc（Windows 资源文件）
```rc
IDI_ICON1 ICON "app_icon.ico"  // 应用程序图标
VS_VERSION_INFO VERSIONINFO    // 版本信息
```

### 生成的图标尺寸
- 256x256 - 高清显示
- 128x128 - 标准显示
- 64x64 - 中等图标
- 48x48 - 标准图标
- 32x32 - 小图标
- 16x16 - 最小图标

## 🚀 使用方法

### 方式 1: 提交并构建（推荐）
```bash
git add .
git commit -m "添加应用程序图标"
git push
```

GitHub Actions 会自动：
1. 将 PNG 转换为 ICO
2. 编译资源文件
3. 将图标嵌入到 exe 中

### 方式 2: 本地测试

#### 步骤 1: 安装 Pillow
```bash
pip install Pillow
```

#### 步骤 2: 转换图标
```bash
python -c "from PIL import Image; img = Image.open('icon-1024x1024.png'); img.save('app_icon.ico', format='ICO', sizes=[(256,256),(128,128),(64,64),(48,48),(32,32),(16,16)])"
```

#### 步骤 3: 构建项目
```bash
cmake -B build -S .
cmake --build build --config Release
```

## 📊 效果预览

编译后的 `DataUploadTool.exe` 会显示：
- ✅ 文件资源管理器中显示自定义图标
- ✅ 任务栏显示自定义图标
- ✅ Alt+Tab 切换时显示自定义图标
- ✅ 文件属性中显示版本信息

## 🔧 更换图标

如果要更换其他图标：

1. **替换 PNG 文件**
   ```bash
   # 替换为新的图标（建议 1024x1024 像素）
   cp new_icon.png icon-1024x1024.png
   ```

2. **重新构建**
   ```bash
   git add icon-1024x1024.png
   git commit -m "更新应用图标"
   git push
   ```

## 📝 注意事项

### 图标要求
- **格式**: PNG（GitHub Actions 会自动转换为 ICO）
- **推荐尺寸**: 1024x1024 像素（会自动缩放）
- **最小尺寸**: 256x256 像素
- **颜色**: 支持透明背景（RGBA）

### 文件位置
```
项目根目录/
  ├── icon-1024x1024.png  ← 源图标（PNG）
  ├── app_icon.ico        ← 转换后的图标（构建时生成）
  └── app.rc              ← Windows 资源文件
```

### Windows 特有功能
- **版本信息**: 右键 exe → 属性 → 详细信息
  - 文件描述: "40+成绩发布系统"
  - 产品名称: "40+成绩发布系统"
  - 版本: 1.0.0.0
  - 版权: Copyright (C) 2025

## 🐛 故障排除

### 问题 1: 图标没有显示
**原因**: Windows 可能缓存了旧图标
**解决方案**:
```bash
# 清除图标缓存
ie4uinit.exe -show
```
或者删除并重新下载 exe 文件

### 问题 2: 编译错误 "cannot open file app_icon.ico"
**原因**: ICO 文件未生成
**解决方案**:
```bash
# 手动转换图标
pip install Pillow
python -c "from PIL import Image; img = Image.open('icon-1024x1024.png'); img.save('app_icon.ico', format='ICO', sizes=[(256,256),(128,128),(64,64),(48,48),(32,32),(16,16)])"
```

### 问题 3: GitHub Actions 构建失败
**检查**:
- `icon-1024x1024.png` 是否存在
- "Convert PNG to ICO" 步骤的日志
- Pillow 是否安装成功

## 🎯 验证图标是否成功

### 方法 1: 文件资源管理器
1. 下载构建的 exe
2. 在资源管理器中查看
3. 应该显示 "40+" 图标（青绿色背景）

### 方法 2: 文件属性
1. 右键 exe → 属性
2. 查看"详细信息"选项卡
3. 应该显示版本信息

### 方法 3: 运行程序
1. 运行 exe
2. 查看任务栏图标
3. 应该显示自定义图标

---

**✨ 现在你的应用程序有专业的图标了！**

