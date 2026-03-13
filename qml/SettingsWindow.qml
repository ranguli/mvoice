/*
 *   Copyright (C) 2026 by Joshua Murphy VO1RFX
 *
 *   Based on the mvoice project by Thomas A. Early N7TAE
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Window {
    id: root
    title: qsTr("Settings")
    modality: Qt.ApplicationModal
    flags: Qt.Dialog
    visible: false
    width: 480
    height: 420

    onVisibleChanged: if (visible && settingsController) settingsController.refreshSettings()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        TabBar {
            id: settingsTabBar
            Layout.fillWidth: true
            TabButton { text: qsTr("Station") }
            TabButton { text: qsTr("Network") }
            TabButton { text: qsTr("DHT") }
            TabButton { text: qsTr("Audio") }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: settingsTabBar.currentIndex

            // Station tab
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth
                clip: true

                ColumnLayout {
                    width: root.width - 32
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Label { text: qsTr("My Callsign:") }
                        TextField {
                            id: settingsCallsignField
                            Layout.preferredWidth: 120
                            maximumLength: 8
                            text: settingsController ? settingsController.settingsCallsign : ""
                            onTextChanged: if (settingsController) settingsController.setSettingsCallsign(text)
                            background: Rectangle {
                                color: settingsController && settingsController.settingsCallsignValid ? "#ccffcc" : "#ffcccc"
                            }
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Input your callsign, up to 8 characters")
                        }
                        Label { text: qsTr("Module:") }
                        ComboBox {
                            id: settingsModuleCombo
                            Layout.preferredWidth: 56
                            model: ["A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"]
                            currentIndex: settingsController ? settingsController.settingsModuleIndex : 0
                            onCurrentIndexChanged: if (settingsController && settingsController.settingsModuleIndex !== currentIndex) settingsController.setSettingsModuleIndex(currentIndex)
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Assign the transceiver module")
                        }
                    }

                    GroupBox {
                        title: qsTr("Codec")
                        Layout.fillWidth: true
                        RowLayout {
                            RadioButton {
                                id: voiceOnlyRadio
                                text: qsTr("Voice-only")
                                checked: settingsController ? settingsController.settingsVoiceOnly : true
                                onToggled: if (settingsController && checked) settingsController.setSettingsVoiceOnly(true)
                                ToolTip.visible: hovered
                                ToolTip.text: qsTr("Higher quality, 3200 bits/s codec")
                            }
                            RadioButton {
                                text: qsTr("Voice+Data")
                                checked: settingsController ? !settingsController.settingsVoiceOnly : false
                                onToggled: if (settingsController && checked) settingsController.setSettingsVoiceOnly(false)
                                ToolTip.visible: hovered
                                ToolTip.text: qsTr("1600 bits/s codec")
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Label { text: qsTr("Latitude:") }
                        TextField {
                            id: settingsLatField
                            Layout.preferredWidth: 100
                            text: settingsController ? settingsController.settingsLatitude : "0.0"
                            onTextChanged: if (settingsController) settingsController.setSettingsLatitude(text)
                            background: Rectangle {
                                color: settingsController && settingsController.settingsLatitudeValid ? "#ccffcc" : "#ffcccc"
                            }
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("North is +, South is -")
                        }
                        Label { text: qsTr("Longitude:") }
                        TextField {
                            id: settingsLongField
                            Layout.preferredWidth: 100
                            text: settingsController ? settingsController.settingsLongitude : "0.0"
                            onTextChanged: if (settingsController) settingsController.setSettingsLongitude(text)
                            background: Rectangle {
                                color: settingsController && settingsController.settingsLongitudeValid ? "#ccffcc" : "#ffcccc"
                            }
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("East is +, West is -")
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Label { text: qsTr("Message:") }
                        TextField {
                            id: settingsMessageField
                            Layout.fillWidth: true
                            maximumLength: 52
                            text: settingsController ? settingsController.settingsMessage : ""
                            onTextChanged: if (settingsController) settingsController.setSettingsMessage(text)
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Up to 52 character text message")
                        }
                    }
                }
            }

            // Network tab
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                Item { height: 20 }
                RadioButton {
                    text: qsTr("IPv4 Only")
                    checked: settingsController ? settingsController.settingsNetTypeIndex === 0 : true
                    onToggled: if (settingsController && checked) settingsController.setSettingsNetTypeIndex(0)
                }
                RadioButton {
                    text: qsTr("IPv6 Only")
                    checked: settingsController ? settingsController.settingsNetTypeIndex === 1 : false
                    onToggled: if (settingsController && checked) settingsController.setSettingsNetTypeIndex(1)
                }
                RadioButton {
                    text: qsTr("IPv4 && IPv6")
                    checked: settingsController ? settingsController.settingsNetTypeIndex === 2 : false
                    onToggled: if (settingsController && checked) settingsController.setSettingsNetTypeIndex(2)
                }
            }

            // DHT tab
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                Label {
                    text: qsTr("DHT support not compiled in.")
                    visible: settingsController ? !settingsController.hasDht : true
                    wrapMode: Text.Wrap
                }
                RowLayout {
                    Layout.fillWidth: true
                    visible: settingsController ? settingsController.hasDht : false
                    spacing: 6
                    Label { text: qsTr("DHT Bootstrap:") }
                    TextField {
                        id: settingsBootstrapField
                        Layout.fillWidth: true
                        text: (settingsController && settingsController.hasDht) ? settingsController.settingsBootstrap : ""
                        onTextChanged: if (settingsController && settingsController.hasDht) settingsController.setSettingsBootstrap(text)
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("An existing node on the DHT Network")
                    }
                }
                Label {
                    text: qsTr("Restart the application to use a new bootstrap.")
                    visible: settingsController ? settingsController.hasDht : false
                    wrapMode: Text.Wrap
                }
            }

            // Audio tab
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth
                clip: true

                ColumnLayout {
                    width: root.width - 32
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Label { text: qsTr("Input:") }
                        ComboBox {
                            id: audioInputCombo
                            Layout.fillWidth: true
                            model: settingsController ? settingsController.audioInputNames : []
                            currentIndex: settingsController ? settingsController.audioInputIndex : 0
                            onCurrentIndexChanged: if (settingsController && settingsController.audioInputIndex !== currentIndex) settingsController.setAudioInputIndex(currentIndex)
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Select your audio input device, usually \"default\"")
                        }
                    }
                    Label {
                        text: settingsController ? settingsController.audioInputDescription : ""
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Label { text: qsTr("Output:") }
                        ComboBox {
                            id: audioOutputCombo
                            Layout.fillWidth: true
                            model: settingsController ? settingsController.audioOutputNames : []
                            currentIndex: settingsController ? settingsController.audioOutputIndex : 0
                            onCurrentIndexChanged: if (settingsController && settingsController.audioOutputIndex !== currentIndex) settingsController.setAudioOutputIndex(currentIndex)
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Select the audio output device, usually \"default\"")
                        }
                    }
                    Label {
                        text: settingsController ? settingsController.audioOutputDescription : ""
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Button {
                        text: qsTr("Rescan")
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: if (settingsController) settingsController.rescanAudio()
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Rescan for new audio devices")
                    }
                }
            }
        }

        Button {
            text: qsTr("Update")
            Layout.alignment: Qt.AlignRight
            enabled: settingsController ? settingsController.settingsCanApply : false
            onClicked: {
                if (settingsController && settingsController.applySettings())
                    root.visible = false
            }
        }
    }
}

