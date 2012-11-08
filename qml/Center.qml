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
import QtDesktop 0.2

SplitterRow {

    function showSearchResults(search) {
        mainView.setSource("SearchResults.qml", { "search": search })
    }

    handleDelegate: Rectangle {
        width: 1
        color: "#292929"

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            anchors.leftMargin: (parent.width <= 1) ? -2 : 0
            anchors.rightMargin: (parent.width <= 1) ? -2 : 0
            drag.axis: Qt.YAxis
            drag.target: handle
            cursorShape: Qt.SplitHCursor
        }
    }

    Item {
        Splitter.maximumWidth: 300
        Splitter.minimumWidth: 120
        width: 200

        Playlists {
            id: playlists
            anchors.top: parent.top
            anchors.bottom: nowplaying.top
            width: parent.width

            onSelectedPlaylistChanged: {
                mainView.setSource("PlaylistView.qml", { "playlist": playlists.selectedPlaylist })
            }
        }

        NowPlaying {
            id: nowplaying
            anchors.bottom: parent.bottom
            width: parent.width
        }
    }

    Loader {
        id: mainView
        Splitter.expanding: true
    }
}
