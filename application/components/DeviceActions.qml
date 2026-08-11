import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Theme 1.0
import QFlipper 1.0


Item {
    id: container

    property alias backupAction: backupAction
    property alias restoreAction: restoreAction
    property alias eraseAction: eraseAction
    property alias reinstallAction: reinstallAction
    property alias selfUpdateAction: selfUpdateAction

    implicitWidth: 318
    implicitHeight: control.implicitHeight + verticalPadding * 2

    readonly property int horizontalPadding: Math.floor((container.implicitWidth - control.implicitWidth) / 2)
    readonly property int verticalPadding: 10

    ColumnLayout {
        id: control
        spacing: 10

        x: horizontalPadding
        y: verticalPadding

        TransparentLabel {
            color: Theme.color.lightorange2
            text: qsTr("固件更新通道")
        }

        ComboBox {
            id: channelComboBox

            enabled: Backend.firmwareUpdateState !== Backend.Unknown &&
                     Backend.firmwareUpdateState !== Backend.Checking &&
                     Backend.firmwareUpdateState !== Backend.ErrorOccured

            delegate: ChannelDelegate {}

            model: Backend.firmwareUpdateModel
            textRole: "name"

            Layout.fillWidth: true

            currentIndex: Backend.firmwareUpdateState !== Backend.Unknown ? find(Preferences.updateChannel) : -1
            onActivated: function(index) {Preferences.updateChannel = textAt(index);}

            ToolTip {
                visible: parent.hovered
                text: qsTr("更改固件更新通道")
                implicitWidth: 250
            }
        }

        TransparentLabel {
            color: Theme.color.lightorange2
            text: qsTr("备份与恢复")
        }

        GridLayout {
            columns: 2
            rowSpacing: control.spacing
            columnSpacing: control.spacing

            Layout.fillWidth: true

            SmallButton {
                action: backupAction
                Layout.fillWidth: true

                font.family: "Microsoft YaHei UI"
                font.pixelSize: 22
                font.capitalization: Font.MixedCase

                icon.source: "qrc:/assets/gfx/symbolic/backup-symbolic.svg"
                icon.width: 18
                icon.height: 20

                ToolTip {
                    visible: parent.hovered
                    text: qsTr("将 Flipper 内部存储的内容保存到本机磁盘。")
                    implicitWidth: 250
                }
            }

            SmallButton {
                action: restoreAction
                Layout.fillWidth: true

                font.family: "Microsoft YaHei UI"
                font.pixelSize: 22
                font.capitalization: Font.MixedCase

                icon.source: "qrc:/assets/gfx/symbolic/restore-symbolic.svg"
                icon.width: 18
                icon.height: 20

                ToolTip {
                    visible: parent.hovered
                    text: qsTr("将备份目录的内容下载到 Flipper 的内部存储。")
                    implicitWidth: 250
                }
            }

            SmallButtonRed {
                action: eraseAction
                Layout.fillWidth: true

                font.family: "Microsoft YaHei UI"
                font.pixelSize: 22
                font.capitalization: Font.MixedCase

                icon.source: "qrc:/assets/gfx/symbolic/trashcan.svg"
                icon.width: 18
                icon.height: 20

                ToolTip {
                    visible: parent.hovered
                    text: qsTr("擦除 Flipper 所有数据并恢复出厂状态。警告！此操作不可撤销！")
                    implicitWidth: 250
                }
            }

            SmallButton {
                action: reinstallAction
                Layout.fillWidth: true

                font.family: "Microsoft YaHei UI"
                font.pixelSize: 22
                font.capitalization: Font.MixedCase

                icon.source: "qrc:/assets/gfx/symbolic/update-symbolic.svg"
                icon.width: 16
                icon.height: 16

                ToolTip {
                    visible: parent.hovered
                    text: qsTr("重新安装当前固件版本。不建议日常使用。")
                    implicitWidth: 250
                }
            }
        }

        TransparentLabel {
            color: Theme.color.lightorange2
            text: qsTr("应用程序更新")
            visible: Preferences.checkAppUpdates
        }

        Button {
            action: selfUpdateAction
            Layout.fillWidth: true
            visible: Preferences.checkAppUpdates

            font.family: "Microsoft YaHei UI"
            font.pixelSize: 22
            font.capitalization: Font.MixedCase

            icon.source: "qrc:/assets/gfx/symbolic/update-symbolic.svg"
            icon.width: 16
            icon.height: 16
        }

        Action {
            id: backupAction
            text: qsTr("备份")
            enabled: Backend.deviceState && !Backend.deviceState.isRecoveryMode
        }

        Action {
            id: restoreAction
            text: qsTr("恢复")
            enabled: Backend.deviceState && !Backend.deviceState.isRecoveryMode
        }

        Action {
            id: eraseAction
            text: qsTr("格式化")
            enabled: Backend.deviceState && !Backend.deviceState.isRecoveryMode
        }

        Action {
            id: reinstallAction
            text: qsTr("重新安装")
            enabled: Backend.firmwareUpdateState === Backend.NoUpdates
        }

        Action {
            id: selfUpdateAction
            text: App.updateStatus === App.Checking ? qsTr("检查中...") :
                  App.updateStatus === App.NoUpdates && checkTimer.running ? qsTr("暂无更新") : qsTr("检查应用更新")

            enabled: Preferences.checkAppUpdates && App.updateStatus !== App.Checking && !checkTimer.running
            onTriggered: App.checkForUpdates()
        }

        Timer {
            id: checkTimer
            interval: 1000

            Component.onCompleted: {
                App.updateStatusChanged.connect(function() {
                    if(App.updateStatus === App.NoUpdates) {
                        start();
                    }
                });
            }
        }
    }
}
