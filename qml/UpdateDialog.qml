import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Omafiles.Runtime

Dialog {
    id: root

    anchors.centerIn: Overlay.overlay
    width: 420
    modal: true
    closePolicy: Popup.CloseOnEscape
    title: qsTr("Frissítés elérhető")

    property var updateChecker: null

    Connections {
        target: updateChecker
        function onDownloadProgressChanged() {
            if (updateChecker.downloadProgress >= 100) {
                progressLabel.text = qsTr("Telepítés...");
            }
        }
        function onUpdateInstalled() {
            root.close();
        }
        function onErrorOccurred() {
            progressLabel.text = updateChecker.errorMessage;
            progressBar.visible = false;
            installButton.enabled = true;
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Image {
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                source: Colors.tint("image://fileicon/folder", Colors.accent)
                sourceSize: Qt.size(48, 48)
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    textFormat: Text.PlainText
                    text: qsTr("Új verzió elérhető")
                    color: Colors.text
                    font.pixelSize: 16
                    font.bold: true
                }

                Text {
                    textFormat: Text.PlainText
                    text: updateChecker ? updateChecker.currentVersion + " → " + updateChecker.latestVersion : ""
                    color: Colors.textDim
                    font.pixelSize: 13
                }
            }
        }

        // Description
        Text {
            textFormat: Text.PlainText
            Layout.fillWidth: true
            text: qsTr("Szeretnéd telepíteni a legújabb verziót? A telepítés után az alkalmazás újraindul.")
            color: Colors.text
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }

        // Progress
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: progressBar.visible || progressLabel.text !== ""

            ProgressBar {
                id: progressBar
                Layout.fillWidth: true
                from: 0
                to: 100
                value: updateChecker ? updateChecker.downloadProgress : 0
                visible: updateChecker && updateChecker.downloadProgress > 0
            }

            Text {
                id: progressLabel
                textFormat: Text.PlainText
                Layout.fillWidth: true
                text: ""
                color: Colors.textDim
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }

        Item { Layout.fillHeight: true }

        // Buttons
        DialogButtonBox {
            Layout.fillWidth: true

            Button {
                id: laterButton
                text: qsTr("Később")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }

            Button {
                id: installButton
                text: qsTr("Frissítés most")
                highlighted: true
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                onClicked: {
                    installButton.enabled = false;
                    progressBar.visible = true;
                    progressLabel.text = qsTr("Letöltés...");
                    if (updateChecker) {
                        updateChecker.downloadAndInstall();
                    }
                }
            }
        }
    }
}
