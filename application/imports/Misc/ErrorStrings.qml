pragma Singleton

import QtQuick 2.15
import QFlipper 1.0

QtObject {
    readonly property string errorStyle:
"<style type='text/css'>p { margin-top: 0px; margin-bottom: 5px; } a { color: #fe8a2c; } </style>"

    readonly property string errorInvalidDevice:
"<p>无法识别设备。</p>
<p>=========== 如何修复 ============</p>
<p>1. 断开本电脑上所有其他类似设备的连接。</p>
<p>2. 重新连接您的 Flipper。</p>
<p>3. 如果问题仍然存在，请将 Flipper 重启至恢复模式，点击修复执行全新安装。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorInvalidDeviceLinux:
"<p>无法识别设备。</p>
<p>=========== 如何修复 ============</p>
<p>1. 断开本电脑上所有其他类似设备的连接。</p>
<p>2. 授予用户访问串口设备的权限。</p>
<p>运行 \"./qFlipper-x86_64-%1.AppImage rules install\" 可自动完成此操作。</p>
<p>3. 如果问题仍然存在，请将 Flipper 重启至恢复模式，点击修复执行全新安装。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>".arg(App.version)

    readonly property string errorSerial:
"<p>无法连接到 Flipper。设备正忙。</p>
<p>=========== 如何修复 ============</p>
<p>1. 确保 Flipper 未通过蓝牙或终端会话连接。</p>
<p>2. 重新连接您的 Flipper。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorSerialLinux:
"<p>无法连接到 Flipper。设备正忙。</p>
<p>=========== 如何修复 ============</p>
<p>1. 确保 Flipper 未通过蓝牙或终端会话连接。</p>
<p>2. 授予用户访问串口设备的权限。</p>
<p>运行 \"./qFlipper-x86_64-%1.AppImage rules install\" 可自动完成此操作。</p>
<p>3. 重新连接您的 Flipper。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>".arg(App.version)

    readonly property string errorRecovery:
"<p>无法在更新和恢复模式下连接 Flipper。未找到设备。</p>
<p>=========== 如何修复 ============</p>
<p>1. 检查 Flipper 是否处于更新和恢复模式。</p>
<p>2. 重新连接您的 Flipper。</p>
<p>3. 将 Flipper 重启至 Flipper OS 后重试。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorRecoveryWindows:
"<p>无法在更新和恢复模式下连接 Flipper。未找到设备。</p>
<p>=========== 如何修复 ============</p>
<p>1. 检查 Flipper 是否处于更新和恢复模式。</p>
<p>2. 重新连接您的 Flipper。</p>
<p>3. 重新安装 qFlipper 以更新 DFU 设备驱动。</p>
<p>4. 将 Flipper 重启至 Flipper OS 后重试。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorRecoveryLinux:
"<p>无法在更新和恢复模式下连接 Flipper。未找到设备。</p>
<p>=========== 如何修复 ============</p>
<p>1. 检查 Flipper 是否处于更新和恢复模式。</p>
<p>2. 重新连接您的 Flipper。</p>
<p>3. 授予用户访问 DFU 设备的权限。</p>
<p>运行 \"./qFlipper-x86_64-%1.AppImage rules install\" 可自动完成此操作。</p>
<p>4. 将 Flipper 重启至 Flipper OS 后重试。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>".arg(App.version)

    readonly property string errorInternet:
"<p>无法连接到更新服务器。</p>
<p>=========== 如何修复 ============</p>
<p>1. 检查您的网络连接。</p>
<p>2. 确保更新服务器未宕机。</p>
<p>3. 请重试更新。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorDisk:
"<p>无法在本地文件系统上保存或读取文件。</p>
<p>=========== 如何修复 ============</p>
<p>1. 确保本地驱动器有可用空间。</p>
<p>2. 检查 qFlipper 是否有磁盘写入权限。</p>
<p>3. 如适用，请确保将 qFlipper 指向正确的文件/目录。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorBackup:
"<p>无法从 Flipper 获取数据，可能由内部错误引起。</p>
<p>=========== 如何修复 ============</p>
<p>1. 关闭所有可能使用 Flipper 的其他应用程序。</p>
<p>2. 重启 Flipper 并通过 USB 重新连接。</p>
<p>3. 请重试该操作。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorData:
"<p>必要的数据似乎已损坏。</p>
<p>=========== 如何修复 ============</p>
<p>1. 如适用，请确保将 qFlipper 指向正确的输入文件。</p>
<p>2. 重启 Flipper 并通过 USB 重新连接。</p>
<p>3. 请重试该操作。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorOperation:
"<p>当前操作被中断。与设备的连接已断开。</p>
<p>=========== 如何修复 ============</p>
<p>1. 检查 USB 连接。</p>
<p>2. 确保 Flipper 未被 PIN 码锁定。</p>
<p>3. 将 Flipper 置于更新和恢复模式并开始修复。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorUpdater:
"<p>无法启动固件更新。</p>
<p>=========== 如何修复 ============</p>
<p>1. 查看下方日志中的详细信息。</p>
<p>2. 请尝试解决问题。</p>
<p>3. 重试。</p>
<p>4. 如果错误仍然存在，请提交错误报告。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"

    readonly property string errorUnknown:
"<p>发生未知错误。</p>
<p>=========== 如何修复 ============</p>
<p>1. 查看下方日志中的详细信息。</p>
<p>2. 请尝试解决问题。</p>
<p>3. 重试。</p>
<p>4. 如果错误仍然存在，请提交错误报告。</p>
<p>-----------------------------------</p>
<center><a href='https://docs.flipper.net/'>了解更多</a></center>"
}
