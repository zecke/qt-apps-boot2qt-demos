/****************************************************************************
**
** Copyright (C) 2014 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.digia.com
**
** This file is part of the QtQuick Enterprise Controls Add-on.
**
** $QT_BEGIN_LICENSE$
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://qt.digia.com
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0

Rectangle {
    property bool pressed: false

    gradient: Gradient {
        GradientStop {
            color: pressed ? "#222" : "#333"
            position: 0
        }
        GradientStop {
            color: "#222"
            position: 1
        }
    }
    Rectangle {
        height: 1
        width: parent.width
        anchors.top: parent.top
        color: "#444"
        visible: !pressed
    }
    Rectangle {
        height: 1
        width: parent.width
        anchors.bottom: parent.bottom
        color: "#000"
    }
}
