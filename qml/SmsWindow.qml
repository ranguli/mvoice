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
    title: qsTr("SMS Texting")
    modality: Qt.ApplicationModal
    flags: Qt.Dialog
    visible: false
    width: 600
    height: 260

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Label { text: qsTr("Destination Callsign:") }
            TextField {
                id: smsDstField
                Layout.fillWidth: true
                text: "@ALL"
            }

            Button {
                text: qsTr("Send")
                enabled: smsMessageField.text.length > 0
                onClicked: {
                    if (uiController) {
                        uiController.sendSms(smsDstField.text, smsMessageField.text)
                        smsMessageField.text = ""
                    }
                }
            }

            Button {
                text: qsTr("Clear")
                enabled: smsMessageField.text.length > 0
                onClicked: smsMessageField.text = ""
            }
        }

        TextArea {
            id: smsMessageField
            Layout.fillWidth: true
            Layout.fillHeight: true
            wrapMode: TextArea.Wrap
        }
    }
}

