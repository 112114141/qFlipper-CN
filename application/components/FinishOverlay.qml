import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import QFlipper 1.0
import Theme 1.0
import Misc 1.0

AbstractOverlay {
    id: overlay

    TextLabel {
        id: successLabel

        y: 19

        capitalized: false
        visible: Backend.backendState === ApplicationBackend.Finished

        font.family: "Born2bSportyV2"
        font.pixelSize: 48

        anchors.horizontalCenter: parent.horizontalCenter

        text: qsTr("成功！")
    }

    RowLayout {
        visible: Backend.backendState === ApplicationBackend.ErrorOccured

        anchors.fill: parent
        anchors.margins: 36
        anchors.leftMargin: 0

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextLabel {
                id: errorLabel

                Layout.alignment: Qt.AlignHCenter

                capitalized: false
                font.family: "Born2bSportyV2"
                font.pixelSize: 48

                text: {
                    switch(Backend.errorType) {
                    case BackendError.InvalidDevice:
                        return qsTr("无效设备")
                    case BackendError.InternetError:
                        return qsTr("网络错误")
                    case BackendError.DiskError:
                        return qsTr("磁盘访问错误")
                    case BackendError.SerialAccessError:
                        return qsTr("权限被拒绝")
                    case BackendError.RecoveryAccessError:
                        return qsTr("无法找到 DFU 设备")
                    case BackendError.BackupError:
                        return qsTr("备份失败")
                    case BackendError.FileError:
                        return qsTr("数据已损坏")
                    case BackendError.OperationError:
                        return qsTr("操作错误")
                    case BackendError.UpdaterError:
                        return qsTr("更新器错误")
                    default:
                        return qsTr("未知错误")
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Image {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: -15

                    sourceSize: Qt.size(246, 187)
                    source: Backend.errorType === BackendError.SerialAccessError ||
                            Backend.errorType === BackendError.RecoveryAccessError ? "qrc:/assets/gfx/images/error-access.svg" :
                            Backend.errorType === BackendError.InternetError ? "qrc:/assets/gfx/images/error-internet.svg" :
                                                                               "qrc:/assets/gfx/images/error-client.svg"

                    visible: !flipperError.visible
                }

                Image {
                    id: flipperError
                    visible: Backend.errorType === BackendError.InvalidDevice  ||
                             Backend.errorType === BackendError.OperationError ||
                             Backend.errorType === BackendError.BackupError ||
                             Backend.errorType === BackendError.UpdaterError ||
                             Backend.errorType === BackendError.UnknownError

                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: -30
                    anchors.horizontalCenterOffset: -10

                    sourceSize: Qt.size(360, 156)
                    source: "qrc:/assets/gfx/images/flipper.svg"

                    Image {
                        x: 93
                        y: 26

                        source: Backend.errorType === BackendError.OperationError ? "qrc:/assets/gfx/images/error-cross-eyes.svg" :
                                                                                    "qrc:/assets/gfx/images/error-exclamation.svg"
                        sourceSize: Qt.size(128, 64)
                    }
                }
            }
        }

        TextBox {
            Layout.preferredWidth: 335
            Layout.alignment: Qt.AlignVCenter

            style: ErrorStrings.errorStyle
            text: {
                switch(Backend.errorType) {
                case BackendError.InternetError:
                    return ErrorStrings.errorInternet
                case BackendError.InvalidDevice:
                    return Qt.platform.os === "linux" ? ErrorStrings.errorInvalidDeviceLinux :
                                                        ErrorStrings.errorInvalidDevice
                case BackendError.DiskError:
                    return ErrorStrings.errorDisk
                case BackendError.SerialAccessError:
                    return Qt.platform.os === "linux" ? ErrorStrings.errorSerialLinux :
                                                        ErrorStrings.errorSerial
                case BackendError.RecoveryAccessError:
                    return Qt.platform.os === "linux" ? ErrorStrings.errorRecoveryLinux :
                           Qt.platform.os === "windows" ? ErrorStrings.errorRecoveryWindows :
                                                          ErrorStrings.errorRecovery
                case BackendError.BackupError:
                    return ErrorStrings.errorBackup
                case BackendError.DataError:
                    return ErrorStrings.errorData
                case BackendError.OperationError:
                    return ErrorStrings.errorOperation
                case BackendError.UpdaterError:
                    return ErrorStrings.errorUpdater
                default:
                    return ErrorStrings.errorUnknown
                }
            }
        }
    }

    Button {
        id: backButton
        action: backAction

        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 25

        icon.width: 24
        icon.height: 24
        icon.source: "qrc:/assets/gfx/symbolic/arrow-back.svg"

        visible: Backend.backendState === ApplicationBackend.ErrorOccured
    }

    MainButton {
        id: continueButton
        action: continueAction
        anchors.horizontalCenter: parent.horizontalCenter
        visible: Backend.backendState === ApplicationBackend.Finished
        focus: visible
        y: 270

        Keys.onPressed: continueAction.trigger()
    }

    Action {
        id: backAction
        text: qsTr("返回")
        onTriggered: Backend.finalizeOperation()
    }

    Action {
        id: continueAction
        text: qsTr("继续")
        onTriggered: Backend.finalizeOperation()
    }
}
