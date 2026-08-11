import QtQuick 2.15
import QtQuick.Layouts 1.15

import QFlipper 1.0
import Theme 1.0

Item {
    id: container

    implicitWidth: 318
    implicitHeight: control.implicitHeight + verticalPadding * 2

    readonly property int horizontalPadding: Math.floor((container.implicitWidth - control.implicitWidth) / 2)
    readonly property int verticalPadding: 10

    readonly property var deviceState: Backend.deviceState
    readonly property var deviceInfo: deviceState ? deviceState.info : undefined
    readonly property bool extraFields: deviceState ? !deviceState.isRecoveryMode : false

    RowLayout {
        id: control
        spacing: 30

        x: horizontalPadding + 5
        y: verticalPadding

        ColumnLayout {
            id: keys

            TextLabel {
                text: qsTr("固件")
                visible: extraFields
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                text: qsTr("构建日期")
                visible: extraFields
                color: Theme.color.mediumorange4
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                text: qsTr("SD 卡")
                visible: extraFields
                horizontalAlignment: Text.AlignRight
                color: Theme.color.mediumorange4
                Layout.fillWidth: true
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                text: qsTr("数据库")
                visible: extraFields
                horizontalAlignment: Text.AlignRight
                color: Theme.color.mediumorange4
                Layout.fillWidth: true
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                color: extraFields ? Theme.color.mediumorange4 : Theme.color.lightorange2
                text: qsTr("硬件")
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                text: qsTr("无线电固件")
                visible: extraFields
                horizontalAlignment: Text.AlignRight
                color: Theme.color.mediumorange4
                Layout.fillWidth: true
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }
        }

        ColumnLayout {
            id: values

            TextLabel {
                text: !deviceInfo ? text : deviceInfo.firmware.branch === "dev" ?
                       deviceInfo.firmware.commit : deviceInfo.firmware.version

                visible: extraFields
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                text: deviceInfo ? Qt.formatDateTime(deviceInfo.firmware.date, "yyyy.MM.dd") : text
                color: Theme.color.lightorange3
                visible: extraFields
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                text: deviceInfo && deviceInfo.storage.isExternalPresent ? deviceInfo.storage.externalFree + qsTr("% 可用") : qsTr("不存在")
                color: deviceInfo && deviceInfo.storage.isExternalPresent ? Theme.color.lightorange3 : Theme.color.lightred3
                visible: extraFields
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                text: deviceInfo && deviceInfo.storage.isAssetsInstalled ? qsTr("已安装") : qsTr("缺失")
                color: deviceInfo && deviceInfo.storage.isAssetsInstalled ? Theme.color.lightorange3 : Theme.color.lightred3
                visible: extraFields
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                color: extraFields ? Theme.color.lightorange3 : Theme.color.lightorange2

                text: {
                    if(!deviceInfo) {
                        text
                    } else {
                        deviceInfo.hardware.version + "." +
                        deviceInfo.hardware.target +
                        deviceInfo.hardware.body +
                        deviceInfo.hardware.connect
                    }
                }

                Layout.fillWidth: true
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }

            TextLabel {
                text: deviceInfo && deviceInfo.radioVersion.length ? "%1 %2".arg(deviceInfo.radioVersion).arg(stackTypeString(deviceInfo.stackType)) : qsTr("已损坏")
                color: deviceInfo && deviceInfo.radioVersion.length ? Theme.color.lightorange3 : Theme.color.lightred3
                visible: extraFields
                font.family: "Microsoft YaHei UI"
                font.pixelSize: 16
                font.capitalization: Font.MixedCase
            }
        }
    }

    function stackTypeString(num) {
        switch(num) {
        case 1: return qsTr("完整版");
        case 2: return "HCI";
        case 3: return qsTr("精简版");
        default: return num;
        }
    }
}
