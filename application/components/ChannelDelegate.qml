import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import Theme 1.0
import Primitives 1.0

ItemDelegate {
    id: control

    text: name
    readonly property bool last: index === ListView.view.model.count - 1

    highlighted: control.highlightedIndex === index
    hoverEnabled: control.hoverEnabled

    width: ListView.view.width

    contentItem: RowLayout {
        spacing: 10

        Rectangle {
            Layout.preferredWidth: 4
            Layout.fillHeight: true
            Layout.topMargin: 2
            Layout.bottomMargin: 2

            color: text === "development" ? "orangered" :
                   text === "release-candidate" ? "blueviolet" : "limegreen"
        }

        TextLabel {
            text: {
                if (control.text === "development") return qsTr("开发版");
                if (control.text === "release-candidate") return qsTr("候选版");
                if (control.text === "release") return qsTr("正式版");
                return control.text;
            }
            color: control.down ? Theme.color.darkorange1 : control.hovered ? Theme.color.lightorange1 : Theme.color.lightorange2
            font.family: "Microsoft YaHei UI"
            font.pixelSize: 16
            font.capitalization: Font.MixedCase
            Layout.fillWidth: true
        }
    }

    background: AdvancedRectangle {
        x: 2
        width: parent.width - 4
        color: control.down ? Theme.color.lightorange2 : control.hovered ? Theme.color.mediumorange2 : Theme.color.darkorange1
        bottomRadius: control.last ? 5 : 0

        Behavior on color {
            ColorAnimation {
                duration: 150
                easing.type: Easing.OutQuad
            }
        }
    }

    ToolTip {
        delay: 300
        visible: parent.hovered
        text: {
            if (control.text === "development") return qsTr("最新但最不稳定的开发版本，包含最新功能但可能有较多错误");
            if (control.text === "release-candidate") return qsTr("候选版本，即将发布的稳定版本，已修复大部分已知问题");
            if (control.text === "release") return qsTr("正式稳定版本，经过充分测试，推荐大多数用户使用");
            return description;
        }
        implicitWidth: 250
    }
}
