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
    title: qsTr("About MVoice")
    modality: Qt.ApplicationModal
    flags: Qt.Dialog
    visible: false
    width: 400
    height: 200

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Image {
            Layout.alignment: Qt.AlignHCenter
            source: "image://appicon/main"
            width: 64
            height: 64
            fillMode: Image.PreserveAspectFit
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("MVoice QML UI")
            font.pixelSize: 18
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("MVoice version # %1").arg("1.4.1")
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Copyright (c) 2025 by Thomas A. Early N7TAE")
        }
    }
}

