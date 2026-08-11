import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Theme 1.0

CustomDialog {
    id: control

    title: qsTr("未找到 SD 卡")

    property bool isAltPressed: false

    contentWidget: RowLayout {
        spacing: -8

        Image {
            Layout.margins: 20
            source: "qrc:/assets/gfx/images/warning-no-sdcard.svg"
            sourceSize: Qt.size(118, 148)
        }

        ColumnLayout {
            spacing: 20
            Layout.margins: 20

            TextLabel {
                text: qsTr("更新固件前，请安装 <font color=\"%1\">SD 卡</font><br>")
                    .arg(Theme.color.lightred4)
                lineHeight: 1.35
                horizontalAlignment: Text.AlignRight
            }

            Button {
                id: okButton
                text: control.isAltPressed ? qsTr("强制安装") : qsTr("确定")

                Keys.onPressed: control.isAltPressed = event.modifiers & Qt.AltModifier;
                Keys.onReleased: control.isAltPressed = false;

                onClicked: control.isAltPressed ? control.accepted() : control.rejected()

                Layout.preferredWidth: 200
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }

    onOpened: {
        okButton.forceActiveFocus();
    }

    onClosed: {
        isAltPressed = false;
    }
}
