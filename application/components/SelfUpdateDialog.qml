import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Theme 1.0
import QFlipper 1.0

CustomDialog {
    id: control

    closable: App.updater.state === ApplicationUpdater.ErrorOccured
    closePolicy: Popup.NoAutoClose

    title: App.updater.state === ApplicationUpdater.Idle ? qsTr("更新 qFlipper？") : qsTr("正在更新 qFlipper")

    contentWidget: Item {
        implicitWidth: 430
        implicitHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            width: parent.implicitWidth

            TextLabel {
                id: messageLabel
                visible: App.updater.state === ApplicationUpdater.Idle
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: qsTr("将安装更新版本的 qFlipper<br/>")

                lineHeight: 1.4
                wrapMode: Text.Wrap

                Layout.topMargin: 24
                Layout.bottomMargin: -8
                Layout.fillWidth: true
            }

            // TODO: Find a way to use a DialogButtonBox properly

            RowLayout {
                id: buttonBox
                visible: App.updater.state === ApplicationUpdater.Idle
                spacing: 30
                Layout.margins: 20
                Layout.fillWidth: true
                Layout.preferredHeight: 42
                layoutDirection: Qt.platform.os === "osx" ? Qt.RightToLeft : Qt.LeftToRight

                SmallButton {
                    radius: 7
                    text: qsTr("更新")
                    highlighted: true
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onClicked: App.selfUpdate();
                }

                SmallButton {
                    radius: 7
                    text: qsTr("取消")
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onClicked: control.rejected()
                }
            }

            ProgressBar {
                id: progressBar

                visible: App.updater.state !== ApplicationUpdater.Idle

                implicitWidth: 286
                implicitHeight: 56

                from: 0
                to: 100

                value: App.updater.progress
                indeterminate: value < 0

                Layout.topMargin: 40
                Layout.bottomMargin: 20
                Layout.alignment: Qt.AlignHCenter
            }

            TextLabel {
                id: progressLabel
                padding: 0

                visible: App.updater.state !== ApplicationUpdater.Idle

                text: {
                    switch(App.updater.state) {
                    case ApplicationUpdater.Downloading:
                        return qsTr("正在下载最新版本...");
                    case ApplicationUpdater.Updating:
                        return qsTr("正在启动更新进程...");
                    case ApplicationUpdater.ErrorOccured:
                        return qsTr("更新失败（请查看日志）。");
                    default:
                        return qsTr("准备中...");
                    }
                }

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                Layout.fillWidth: true
                Layout.bottomMargin: 14

                color: App.updater.state === ApplicationUpdater.ErrorOccured ? Theme.color.lightred3 : Theme.color.lightorange2
            }
        }
    }
}
