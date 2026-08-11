import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Theme 1.0
import QFlipper 1.0

Item {
    id: container

    implicitWidth: 318
    implicitHeight: control.implicitHeight + verticalPadding * 2

    readonly property int horizontalPadding: Math.floor((container.implicitWidth - control.implicitWidth) / 2)
    readonly property int verticalPadding: 10

    property alias installRadioAction: installRadioAction
    property alias installFusAction: installFusAction

    ColumnLayout {
        id: control
        spacing: 10

        x: horizontalPadding
        y: verticalPadding

        TransparentLabel {
            color: Theme.color.lightorange2
            text: qsTr("开发者模式")
        }

        SmallButtonRed {
            action: installRadioAction
            Layout.fillWidth: true

            font.family: "Microsoft YaHei UI"
            font.pixelSize: 22
            font.capitalization: Font.MixedCase

            icon.source: "qrc:/assets/gfx/symbolic/restore-symbolic.svg"
            icon.width: 18
            icon.height: 20

            ToolTip {
                visible: parent.hovered
                text: qsTr("安装自定义无线协议栈。警告！这可能导致 Flipper 无法使用！")
                implicitWidth: 250
            }
        }

        SmallButtonRed {
            action: installFusAction
            Layout.fillWidth: true

            font.family: "Microsoft YaHei UI"
            font.pixelSize: 22
            font.capitalization: Font.MixedCase

            icon.source: "qrc:/assets/gfx/symbolic/restore-symbolic.svg"
            icon.width: 18
            icon.height: 20

            ToolTip {
                visible: parent.hovered
                text: qsTr("安装自定义 FUS 固件。！！严重警告！！这将导致加密密钥丢失！")
                implicitWidth: 250
            }
        }

        Action {
            id: installRadioAction
            text: qsTr("安装无线固件")
        }

        Action {
            id: installFusAction
            text: qsTr("安装 FUS 固件")
            enabled: Backend.deviceState && Backend.deviceState.isRecoveryMode
        }
    }
}
