/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: For any questions to Digia, please use the contact form at
** http://qt.digia.com/
**
** This file is part of the examples of the Qt Enterprise Embedded.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 2.0

Column {
    width: 400 * root.contentScale
    spacing: 20 * root.contentScale
    visible: maximumZoom > 1

    property alias maximumZoom: zoomSlider.maximum
    property alias requestedZoom: zoomSlider.value
    property real actualZoom: 1

    Rectangle {
        anchors.horizontalCenter: zoomSlider.horizontalCenter
        width: zoomText.width + 10 * root.contentScale
        height: zoomText.height + 10 * root.contentScale
        color: "#77333333"
        radius: 5 * root.contentScale
        rotation: root.contentRotation
        Behavior on rotation { NumberAnimation { } }

        Text {
            id: zoomText
            anchors.centerIn: parent
            font.pixelSize: Math.round(24 * root.contentScale)
            color: "white"
            font.bold: true
            text: (Math.round(actualZoom * 100) / 100) + "x"
        }
    }

    Slider {
        id: zoomSlider
        width: parent.width
        rotation: root.contentRotation === -90 ? 180 : (root.contentRotation === 90 ? 0 : root.contentRotation)

        minimum: 1
        maximum: 1
        value: 1
    }
}
