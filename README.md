## qFlipper 中文版

> 基于 [flipperdevices/qFlipper](https://github.com/flipperdevices/qFlipper) 的简体中文汉化版本
>
> **汉化作者：** 112114141（左晟宇）
> **许可证：** GPL-3.0（与原项目一致）

### 用于通过 PC 更新 [Flipper Zero](https://flipperzero.one/) 固件的图形桌面应用程序
qFlipper 完全开源，基于 [Qt](https://www.qt.io/) 框架。可在 Windows、macOS、Linux 上运行。

<img alt="qFlipper" width="450" src="https://cdn.flipperzero.one/qflipper_logo_with_connected_flipper.png" />

## 下载

在此下载官方 qFlipper 构建：[update.flipperzero.one](https://update.flipperzero.one/)

<a href="https://update.flipperzero.one"><img width="300" alt="download qFlipper Button" src="https://cdn.flipperzero.one/download_qFlipper_button.png" /></a>


## 功能特性
* 一键更新 Flipper 的固件和补充数据
* 修复损坏的固件安装
* 串流 Flipper 的显示屏并进行远程控制
* 从 `.dfu` 文件安装固件
* 备份和恢复设置、进度和配对数据
* 自动自更新功能
* 命令行界面

## 从源码构建 
### 克隆
确保连同子模块一起克隆项目： 
```sh
git clone https://github.com/flipperdevices/qFlipper.git --recursive
```
### Windows

构建要求：
- MS Visual Studio 2019 或更新版本
- Qt5（MSVC 构建）>= 5.15.0 或 Qt6 >= 6.3.0
- NSIS（用于生成安装程序）

编辑 `build_windows.bat` 以适配您的构建环境，然后运行：
```cmd
build_windows.bat
```

注意：本仓库不提供 STM32 Bootloader 驱动程序。

### Linux
#### Docker 构建（AppImage，官方）

通过运行以下命令设置开发容器：
```sh 
docker compose up -d
```
通过运行以下命令编译 qFlipper：
```sh
docker compose exec dev ./build_linux.sh
```

#### 独立构建
构建要求：
- Qt5 >= 5.15.0 或 Qt6 >= 6.3.0
- libusb >= 1.0.16
- zlib >= 1.2.0

确保安装以下 Qt 模块（确切的包名可能因您的 Linux 发行版而略有不同）： 
```
base, tools, serialport, declarative,  wayland, [quickcontrols2, graphicaleffects] (Qt5 only), qt5-compat (Qt6 only)
```
然后运行：
```sh
mkdir build && cd build
qmake ../qFlipper.pro PREFIX=/path/to/install/dir -spec linux-g++ CONFIG+=qtquickcompiler && 
make qmake_all && make && make install
```
**注意：**不建议 `make install` 到系统前缀。相反，请使用此方法构建特定发行版的软件包。 
在这种情况下，可以通过在 `qmake` 调用中传入 `DEFINES+=DISABLE_APPLICATION_UPDATES` 来禁用内置的应用程序更新功能。

### MacOS

构建要求：

- Xcode 或命令行工具
- 来自 [Flipper brew tap](https://github.com/flipperdevices/homebrew-flipper) 的 Qt6 6.3.1 静态通用版本
- 来自 [Flipper brew tap](https://github.com/flipperdevices/homebrew-flipper) 的 libusb 1.0.24 通用版本
- [dmgbuild](https://pypi.org/project/dmgbuild/) >= 1.5.2

如果您想对二进制文件进行签名，请设置 `SIGNING_KEY` 环境变量：

	export SIGNING_KEY="Your Developer Key ID"

构建、签名并创建软件包：

	./build_mac.sh

生成的镜像文件位于：`build_mac/qFlipper.dmg`

## 运行

### Linux
```sh
./build/qFlipper-x86_64.AppImage
```

或者直接从您喜欢的文件管理器中启动上述文件。
您可能需要设置 udev 规则才能以普通用户身份使用 qFlipper：
```sh
./qFlipper-x86_64.AppImage rules install [/optional/path/to/rules/dir]
```

#### 包管理器支持
请参阅 [contrib](./contrib) 了解可用选项。

## 项目结构
- `application` - 主图形应用程序，主要使用 QML 编写。
- `cli` - 命令行界面，提供几乎所有主应用程序的功能。
- `backend` - 后端库，使用 C++ 编写。负责大部分逻辑处理。
- `dfu` - 用于访问 USB 和 DFU 设备的底层库。
- `plugins` - 基于 Protobuf 的通信协议支持。
- `3rdparty` - 第三方库。
- `contrib` - 贡献的软件包和脚本。
- `driver-tool` - 用于 Windows 的 DFU 驱动安装工具（基于 `libwdi`）。
- `docker` - Docker 配置文件。
- `installer-assets` - 用于部署的补充数据。

## 报告 Bug
qFlipper 是一个正在积极开发中的项目。请报告遇到的任何 Bug 以帮助改进！

（基本）完整的指南位于[此处](./.github/ISSUE_TEMPLATE/bug_report.md)。

## 已知问题

* 有时 Flipper 的串口无法被操作系统识别，这会导致固件更新错误。这是一个固件问题。
* 在某些系统上，打开、关闭或调整日志区域大小时会出现明显的闪烁。
* 发布源码归档由 Github 自动生成，不适合用于构建，因为它们不包含子模块。
