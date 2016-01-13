/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of Qt for Device Creation.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.6

Item {
    //property alias color: rect.color
    property alias text: buttonText
    signal triggered()
    signal alternateTrigger()
    signal slideTrigger()
    property bool enableAlternate: false
    property bool enableSlide: false
    property bool longPressed: false

    property color buttonColor: "lightgray"
    Rectangle {
        id: rect
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        radius: 10
        color: (enableSlide && mouser.drag.active) ? "#df3f1f" : mouser.containsPress ? (longPressed ? "#3faf1f" : "steelblue") : buttonColor

        Text {
            anchors.margins: 10
            id: buttonText
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            text: "[Uninitialized]"
        }
    }

    MouseArea {
        id: mouser
        anchors.fill: parent

        onPressed: {
            if (enableSlide) {
                rect.anchors.left = undefined
                rect.anchors.right = undefined
            }
        }

        onPressAndHold: if (enableAlternate) parent.longPressed = true
        onReleased: {
            if (enableSlide && drag.active) {
                //console.log("drag end: " + rect.x + " w: " + rect.width)
                if (rect.x > rect.width || rect.x < -rect.width*3/4)
                    parent.slideTrigger()
            } else if (containsMouse) {
                //console.log("clicked " + mouse.wasHeld + " alt " + enableAlternate)
                if (mouse.wasHeld && enableAlternate)
                    parent.alternateTrigger()
                else
                    parent.triggered()
            }

            rect.anchors.left = parent.left
            rect.anchors.right = parent.right
            parent.longPressed = false
        }

        drag.target: rect
        drag.threshold: 5
        drag.axis: Drag.XAxis
        //drag.maximumX: 0
        drag.minimumX: 0
    }
}