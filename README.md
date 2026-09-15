# Flip（看图）

面向办公场景的轻量 Linux 图片查看器：打开一张图，即可在**同一文件夹**里前后翻页，类似经典 Windows 照片查看器。

- 品牌名：**Flip**（中文名：**看图**）
- 许可：**MIT**
- 开发：[喜相逢科技 / Xixiangfeng Tech](https://www.ak129.cn/flip/)
- 目标系统：Ubuntu，以及统信 UOS、银河麒麟等国产 Linux（amd64）

## 功能

- 打开图片或文件夹（菜单 / 文件对话框 / 拖放 / 命令行参数）
- 同一目录上一张 / 下一张（方向键、工具栏、左右边缘点击、滚轮）
- 默认缩放：小于窗口的图片按 **100%** 显示；大于窗口的缩小以适应窗口；居中；默认不放大
- 常见格式（由 Qt 图像插件提供）：JPEG、PNG、BMP、GIF、WebP、TIFF 等
- 窗口标题显示文件名
- 界面语言：简体中文 / English（可跟随系统，也可在「查看 → 语言」里切换）
- 每次启动显示使用提示（可选择「不再提示」）
- 每次启动在线检查更新（读取官网 `version.json` 的 `linux` 字段）。有新版本时弹出说明；「前往更新」会在 **Gitee 与 GitHub** 之间短超时竞速（优先探测 amd64 安装包地址，否则发布页），打开先响应的源；失败则换另一个，再官网。**不会自动覆盖本机程序**。「不更新」之后不再询问；「稍后再说」下次启动仍会检查
- 关于对话框含版本号、MIT 说明与 https://www.ak129.cn/flip/

## 依赖

编译需要：

- CMake ≥ 3.16
- C++17 编译器（GCC / Clang）
- **Qt 6** Widgets + Gui（推荐；本仓库在 Ubuntu 24.04 上用 Qt 6.4 验证）
- 若系统只有 **Qt 5.15**，CMake 会自动回退

运行时建议安装图像格式插件，以便打开 WebP、TIFF 等：

```bash
# Debian / Ubuntu / 多数衍生版
sudo apt install qt6-image-formats-plugins
```

统信 UOS、银河麒麟请使用各自的包管理器（`apt` / `yum` / 应用商店），包名可能是 `qt6-base-dev`、`libqt6-dev` 或 `qt5-default` 一类，以发行版仓库为准。

## 在 Linux amd64 上编译

```bash
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-base-dev-tools qt6-image-formats-plugins
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

可执行文件：`build/Flip`

无显示器（例如服务器冒烟）时：

```bash
QT_QPA_PLATFORM=offscreen ./build/Flip --self-test
```

有桌面时：

```bash
./build/Flip examples/img1.png
```

### 统信 UOS / 银河麒麟

1. 安装「开发者工具」或对应的 `gcc`、`cmake`、Qt 开发包（优先 Qt 6，没有再用 Qt 5.15）。
2. 用上面同一套 `cmake -S . -B build` 命令即可；不要依赖本仓库以外的私有服务器。
3. 国产系统桌面（DDE / UKUI）一般能直接运行 Qt Widgets 程序。若图标未出现在启动器，把 `packaging/flip.desktop` 复制到 `~/.local/share/applications/`，并把 `resources/icons/flip.svg` 安装为 `flip` 图标。
4. 中文界面依赖系统中文字体（如文泉驿、Noto CJK）。UOS / 麒麟默认已包含。

## 打包

### 源码旁的二进制 tarball（最简单）

目标机器需已安装 Qt 6 运行库（Ubuntu 上为 `libqt6widgets6t64` 等，名称因发行版而异）以及 `qt6-image-formats-plugins`。

```bash
./scripts/make-tarball.sh
```

产物在 `dist/flip-linux-<version>-amd64.tar.gz`。

### AppImage（可选）

可用 [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) 与 `linuxdeploy-plugin-qt` 把 `build/Flip` 打成 AppImage，便于拷到没有开发包的电脑。本仓库不绑定特定打包主机。在 amd64 Linux 上大致步骤：

```bash
# 下载 linuxdeploy 与 linuxdeploy-plugin-qt 到 PATH 后：
export QMAKE=$(command -v qmake6 || command -v qmake)
linuxdeploy --appdir AppDir -e build/Flip -d packaging/flip.desktop -i resources/icons/flip.svg --plugin qt --output appimage
```

无 FUSE 的环境可用 `APPIMAGE_EXTRACT_AND_RUN=1` 运行 linuxdeploy 的 AppImage 发行包。

## 键盘

| 按键 | 作用 |
| --- | --- |
| ← → ↑ ↓、空格、PageUp/PageDown | 上一张 / 下一张（目录内循环） |
| Home / End | 第一张 / 最后一张 |
| Ctrl+滚轮、+ / − | 放大 / 缩小 |
| 0 或 F | 恢复默认适应窗口 |
| 1 | 实际大小 100% |
| 双击图片 | 在适应窗口与 100% 之间切换 |
| F11 | 全屏 |
| Ctrl+O / Ctrl+Shift+O | 打开图片 / 打开文件夹 |

---

# Flip (看图) — English

A small, free image viewer for Linux: open one file, then page through the **same folder**, like classic Windows Photo Viewer.

- License: MIT
- Developer: [喜相逢科技 / Xixiangfeng Tech](https://www.ak129.cn/flip/)
- Targets: Ubuntu and Chinese domestic Linux desktops (UnionTech UOS, Kylin) on **amd64**
- Each launch shows a short tips dialog unless you chose **Don't show again**
- Each launch checks [version.json](https://www.ak129.cn/flip/version.json) (`linux`). A newer build shows notes; **Go to update** races **Gitee vs GitHub** (HEAD/GET of the amd64 asset when listed, otherwise the release page) and opens whichever answers first. If that source fails, the other forge is tried, then the site. The app never overwrites its own binary. **Don't update** stops asking; **Later** asks again next time

Build:

```bash
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-base-dev-tools qt6-image-formats-plugins
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/Flip examples/img1.png
```

CMake prefers Qt 6 and falls back to Qt 5.15 if Qt 6 development packages are missing. Install `qt6-image-formats-plugins` (or your distro’s equivalent) for WebP and TIFF.

UOS / Kylin: install GCC, CMake, and Qt 6 (or 5.15) from the distro repositories, then use the same CMake commands. Widgets UI is intended to stay compatible with DDE and UKUI.

See the Chinese section above for packaging notes (tarball / AppImage) and keyboard shortcuts.
