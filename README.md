# qFlipper 中文版

> 基于 [flipperdevices/qFlipper](https://github.com/flipperdevices/qFlipper) 的简体中文汉化版本

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

## 简介

qFlipper 是用于通过 PC 更新 [Flipper Zero](https://flipperzero.one/) 固件的图形桌面应用程序，基于 [Qt](https://www.qt.io/) 框架，可在 Windows、macOS、Linux 上运行。

本仓库为 qFlipper 的**简体中文汉化版**，对所有用户可见的界面文本、错误提示、命令行输出、安装程序等进行了完整的中文本地化，同时保持了原版的全部功能。

<img alt="qFlipper" width="450" src="https://cdn.flipperzero.one/qflipper_logo_with_connected_flipper.png" />

## 汉化说明

| 项目 | 内容 |
|------|------|
| 汉化作者 | 112114141（左晟宇） |
| 许可证 | GPL-3.0（与原项目一致） |
| 版本标识 | 版本号后缀 `CN`（如 `1.3.3CN`） |
| 汉化范围 | QML 界面、C++ 后端、CLI 命令行、NSIS 安装程序、错误提示、文档 |
| 中文字体 | Microsoft YaHei UI（微软雅黑） |

## 下载

### 中文版安装包

前往 [Releases 页面](https://github.com/112114141/qFlipper-CN/releases) 下载最新中文版安装包。

### 官方原版

如需官方原版 qFlipper，请前往 [update.flipperzero.one](https://update.flipperzero.one/) 下载。

## 功能特性

- 一键更新 Flipper 的固件和补充数据
- 修复损坏的固件安装
- 串流 Flipper 的显示屏并进行远程控制
- 从 `.dfu` 文件安装固件
- 备份和恢复设置、进度和配对数据
- 自动自更新功能
- 命令行界面

## 从源码构建

### 克隆

```sh
git clone https://github.com/112114141/qFlipper-CN.git --recursive
```

### Windows 构建

**构建要求：**
- MS Visual Studio 2019 或更新版本（含 Build Tools）
- Qt 6.4.2（MSVC 2019 64-bit）
- NSIS（用于生成安装程序）

**构建步骤：**

1. 配置好上述构建环境
2. 编辑 `build_windows.bat` 以适配您的 Qt 和 MSVC 安装路径
3. 运行：
```cmd
build_windows.bat
```

构建产物位于 `build/` 目录：
- `qFlipper.exe` — 主程序
- `qFlipper-cli.exe` — 命令行工具
- `qFlipperSetup-64bit.exe` — Windows 安装包

> **注意：** 本仓库不提供 STM32 Bootloader 驱动程序。

### Linux / macOS 构建

请参考[原项目文档](https://github.com/flipperdevices/qFlipper#readme)中的构建说明。

## 项目结构

| 目录 | 说明 |
|------|------|
| `application` | 主图形应用程序（QML） |
| `cli` | 命令行界面 |
| `backend` | 后端库（C++），负责大部分逻辑处理 |
| `dfu` | USB 和 DFU 设备访问的底层库 |
| `plugins` | Protobuf 通信协议支持 |
| `3rdparty` | 第三方库 |
| `installer-assets` | 部署用的补充数据 |

## 报告 Bug

如发现汉化版的问题，请在 [Issues](https://github.com/112114141/qFlipper-CN/issues) 页面提交。

## 致谢

- [flipperdevices/qFlipper](https://github.com/flipperdevices/qFlipper) — 原项目
- [Flipper Zero](https://flipperzero.one/) — Flipper Zero 官方

## 许可证

本项目基于 [GPL-3.0](LICENSE) 许可证开源，与原项目一致。
