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

Item {
    property bool maximized: true

    function toggleMinimizeMaximize()
    {
        heightBehavior.enabled = true;
        maximized = !maximized;
    }

    height: maximized ? albumart.height + info.height : info.height
    Behavior on height {
        id: heightBehavior
        enabled: false
        SequentialAnimation {
            NumberAnimation { duration: 200 }
            ScriptAction { script: heightBehavior.enabled = false }
        }
    }

    SpotifyImage {
        id: albumart
        width: parent.width
        height: width
        smooth: true
        spotifyId: spotify.currentTrack ? spotify.currentTrack.albumCoverId : ""
        defaultImage: "images/now_playing-art_placeholder-album.png"
        fillMode: Image.PreserveAspectCrop

        Image {
            width: parent.width
            source: "images/now_playing-hrule.png"
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            CustomButton {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 6
                anchors.rightMargin: 6
                iconName: "now-playing-collapse"
                opacity: parent.containsMouse ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 300 } }

                onClicked: toggleMinimizeMaximize()
            }
        }
    }

    Image {
        id: info
        anchors.bottom: parent.bottom
        width: parent.width
        height: 51
        source: "images/now_playing-info-bg.png"

        MouseArea {
            id: mouseHovered
            anchors.fill: parent
            hoverEnabled: true
        }

        SpotifyImage {
            id: miniart
            x: maximized ? -width : 0
            Behavior on x { NumberAnimation { duration: 200 } }
            width: height
            height: parent.height - 2
            anchors.bottom: parent.bottom
            spotifyId: spotify.currentTrack ? spotify.currentTrack.albumCoverId : ""
            defaultImage: "images/now_playing-art_placeholder-album.png"
            fillMode: Image.PreserveAspectCrop

            CustomButton {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 6
                anchors.rightMargin: 6
                iconName: "now-playing-expand"
                opacity: mouseHovered.containsMouse || containsMouse ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 300 } }

                onClicked: toggleMinimizeMaximize()
            }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: miniart.right
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8
            spacing: 5

            Text {
                width: parent.width
                elide: Text.ElideRight
                color: "white"
                style: Text.Sunken
                font.bold: true
                font.pixelSize: 11
                text: spotify.currentTrack ? spotify.currentTrack.name : ""
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                color: "white"
                style: Text.Sunken
                font.pixelSize: 9
                text: spotify.currentTrack ? spotify.currentTrack.artists : ""
            }
        }
    }
}
