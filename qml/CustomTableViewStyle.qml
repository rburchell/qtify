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
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0

TableViewStyle {
    minimumHandleLength: 20
    handleOverlap: 4
    scrollToClickedPosition: true

    padding.top: 0
    padding.bottom: 0
    padding.left: 0
    padding.right: 0

    backgroundColor: "transparent"

    corner: BorderImage {
        source: "images/scrollbar-corner.png"
        border.left: 1
        border.top: 1
        anchors.bottom: parent.bottom
        anchors.right: parent.right
    }

    scrollBarBackground: BorderImage {
        source: styleData.horizontal ? "images/scrollbar-horizontal-bg.png" : "images/scrollbar-vertical-bg.png"
        border.left: 1
    }

    incrementControl: Image {
        source: "images/scrollbar" + (styleData.horizontal ? "-horizontal" : "-vertical") + "-down" + (styleData.pressed ? "-pressed" : "") + ".png"
    }

    decrementControl: Image {
        source: "images/scrollbar" + (styleData.horizontal ? "-horizontal" : "-vertical") + "-up" + (styleData.pressed ? "-pressed" : "") + ".png"
    }

    handle: BorderImage {
        source: styleData.horizontal ? "images/scrollbar-horizontal-handle.png" : "images/scrollbar-vertical-handle.png"
        border.left: 7; border.top: 7
        border.right: 7; border.bottom: 7
    }
}
