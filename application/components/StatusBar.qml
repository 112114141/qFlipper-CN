import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl 2.15

import Theme 1.0
import QFlipper 1.0

Rectangle {
    id: control

    readonly property bool errorOccured: Backend.backendState === Backend.ErrorOccured ||
                                         Backend.firmwareUpdateState === Backend.ErrorOccured

    color: errorOccured ? Theme.color.darkred2 : Theme.color.darkorange1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        spacing: 10

        IconImage {
            color: message.color
            sourceSize: Qt.size(22, 22)
            source: "qrc:/assets/gfx/symbolic/info-small.svg"
            rotation: errorOccured ? 180 : 0
        }

        TextLabel {
            id: message

            text: Backend.backendState === ApplicationBackend.ErrorOccured ?
                  Backend.errorType === BackendError.InvalidDevice ? qsTr("发现不支持的设备") :
                  Backend.errorType === BackendError.InternetError ? qsTr("无法连接到更新服务器") :
                  Backend.errorType === BackendError.SerialAccessError ? qsTr("无法访问串口"):
                  Backend.errorType === BackendError.RecoveryAccessError ? qsTr("无法访问恢复模式下的设备"):
                  Backend.errorType === BackendError.DiskError ? qsTr("磁盘访问错误"):
                  Backend.errorType === BackendError.BackupError ? qsTr("无法保存 Flipper 数据"):
                  Backend.errorType === BackendError.OperationError ? qsTr("操作已中断"):
                  Backend.errorType === BackendError.DataError ? qsTr("数据已损坏"): qsTr("出现错误。请查看日志以获取详细信息。") :

                  Backend.backendState === ApplicationBackend.WaitingForDevices ? qsTr("等待设备...") :
                  Backend.backendState > ApplicationBackend.ScreenStreaming && Backend.backendState < ApplicationBackend.Finished ? qsTr("请勿拔出设备...") :
                  Backend.backendState === ApplicationBackend.Finished ? qsTr("操作已成功完成。") :
                  Backend.firmwareUpdateState === ApplicationBackend.ErrorOccured ? qsTr("无法连接到更新服务器") :
                  Backend.firmwareUpdateState === ApplicationBackend.Checking ? qsTr("正在检查固件更新...") : qsTr("就绪。")

            color: control.errorOccured ? Theme.color.lightred4 : Theme.color.lightorange2

            verticalAlignment: Text.AlignVCenter
            Layout.fillWidth: true
        }
    }

    Behavior on color {
        ColorAnimation {
            duration: 150
            easing.type: Easing.OutQuad
        }
    }
}
