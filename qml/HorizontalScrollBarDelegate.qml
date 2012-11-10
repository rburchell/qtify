/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: Yoann Lopes (yoann.lopes@digia.com)
**
** This file is part of the Qtify project.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtDesktop 1.0

Item {
    id: hsbroot
    anchors.fill: parent
    anchors.leftMargin: -1
    anchors.topMargin: dummystyle.style === "mac" ? -1 : 0

    property string activeControl: ""
    property real value: 0
    property int maximumValue: 0

    StyleItem {
        id: dummystyle
    }

    function pixelMetric(arg) {
        if (arg === "scrollbarExtent")
            return bg.height;

        return 0;
    }

    function styleHint(arg) {
        return false;
    }

    function hitTest(argX, argY) {
        if (itemIsHit(handle, argX, argY))
            return "handle"
        else if (itemIsHit(upControl, argX, argY))
            return "up";
        else if (itemIsHit(downControl, argX, argY))
            return "down";
        else if (itemIsHit(bg, argX, argY)) {
            if (argX < handle.x)
                return "upPage"
            else
                return "downPage"
        }

        return "none";
    }

    function subControlRect(arg) {
        if (arg === "handle")
            return Qt.rect(handle.x, handle.y, handle.width, handle.height);
        else if (arg === "groove")
            return Qt.rect(16, 0, hsbroot.width - 32, hsbroot.height);
        return Qt.rect(0,0,0,0);
    }

    function itemIsHit(argItem, argX, argY) {
        var pos = argItem.mapFromItem(hsbroot, argX, argY);
        return (pos.x >= 0 && pos.x <= argItem.width && pos.y >= 0 && pos.y <= argItem.height);
    }



    Image {
        id: upControl
        anchors.left: parent.left
        source: activeControl === "up" ? "images/scrollbar-horizontal-up-pressed.png" : "images/scrollbar-horizontal-up.png"
    }

    BorderImage {
        id: bg
        anchors.left: upControl.right
        anchors.right: downControl.left
        source: "images/scrollbar-horizontal-bg.png"
        border.top: 1;
    }

    Image {
        id: downControl
        anchors.right: parent.right
        source: activeControl === "down" ? "images/scrollbar-horizontal-down-pressed.png" : "images/scrollbar-horizontal-down.png"
    }

    BorderImage {
        id: handle
        source: "images/scrollbar-horizontal-handle.png"
        border.left: 7; border.top: 7
        border.right: 7; border.bottom: 7
        width: Math.max(20, bg.width - maximumValue + 8)
        anchors.left: bg.left
        anchors.leftMargin: -4 + (hsbroot.value / maximumValue) * (bg.width + 8 - width)
    }
}