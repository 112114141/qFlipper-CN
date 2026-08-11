# qFlipper-cli
### qFlipper 的非交互式文本模式界面
该程序主要用于测试目的，尽管它也可以在终端模拟器中提供 qFlipper 的所有功能。

## 运行：
### Windows：
`<Program_files_directory>\qFlipper\qFlipper-cli.exe [args] [parameters]`
### MacOS：
`<Applications_directory>/qFlipper.app/Contents/MacOS/qFlipper-cli [args] [parameters]`
### Linux：
`<AppImage_directory>/qFlipper-x86_64-x.y.z.AppImage cli [args] [parameters]`

## 命令语法：
不带任何参数运行可执行快速更新/修复。
### 命令：
* `backup <target_dir>` - 备份内部存储内容。
* `restore <source_dir>` - 恢复内部存储内容。
* `erase` - 擦除内部存储内容（恢复出厂设置）。
* `wipe` - 擦除整个 MCU 闪存（尚未实现）。
* `firmware <firmware_file.dfu>` - 刷写 Core1 固件。
* `core2radio <firmware_file.bin>` - 刷写 Core2 无线电堆栈。
* `core2fus <firmware_file.bin> <0xaddress>` - 刷写 Core2 固件更新服务 **（警告！这会使您的安全区失效！）**

### 选项：
* `-d <n>, --debug-level <n>` - 设置调试输出级别，0 - 仅错误，1 - 简洁，2 - 全部。默认为 1。
* `-n <n>, --repeat-number <n>` - 重复操作 *n* 次，0 - 无限次，默认 - 一次。
* `-c <channel>, --update-channel <channel>` - 设置更新通道（可以是以下之一：`release`、`release-candidate`、`development`）。该选择会保存在配置文件中，默认为 `release`。
* `-v, --version` - 显示程序版本。
* `-h, --help` - 显示帮助。
