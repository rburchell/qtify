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
import QtSpotify 1.0

Rectangle {

    property variant playlist: null

    color: "#373737"

    Item {
        id: header
        anchors.top: parent.top
        width: parent.width
        height: 150

        Component {
            id: singleArt
            SpotifyImage {
                width: 128; height: width
                defaultImage: "images/128-playlist.png"
                spotifyId: playlist ? playlist.albumArtsForPlaylistImage()[0] : ""
            }
        }

        Component {
            id: multiArt
            Grid {
                columns: 2
                Repeater {
                    model: playlist ? playlist.albumArtsForPlaylistImage() : 0

                    SpotifyImage {
                        width: 64; height: 64
                        spotifyId: modelData
                        defaultImage: "images/now_playing-art_placeholder-album.png"
                    }
                }
            }
        }

        BorderImage {
            width: imageGrid.width + 8; height: imageGrid.height + 8
            anchors.centerIn: imageGrid
            anchors.verticalCenterOffset: 1
            border.bottom: 7
            border.top: 7
            border.left: 7
            border.right: 7
            source: "images/gridview-shadow-flex.png"
        }

        ShaderEffectSource {
            id: shaderSource
            hideSource: true
            sourceItem: imageGrid
        }
        ShaderEffectSource {
            id: maskSource
            hideSource: true
            sourceItem: mask
        }

        ShaderEffect {
            anchors.fill: imageGrid
            property variant source: shaderSource
            property variant mask: maskSource

            fragmentShader:
                "uniform lowp float qt_Opacity;" +
                "uniform sampler2D source;" +
                "uniform sampler2D mask;" +
                "varying highp vec2 qt_TexCoord0;" +
                "void main() {" +
                "    gl_FragColor = texture2D(source, qt_TexCoord0) * texture2D(mask, qt_TexCoord0);" +
                "}"
        }

        Rectangle {
            id: mask
            anchors.fill: imageGrid
            radius: 4
            antialiasing: true
        }

        Loader {
            id: imageGrid
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20
            sourceComponent: playlist && playlist.albumArtsForPlaylistImage().length < 4 ? singleArt : multiArt
        }

        Row {
            anchors.left: imageGrid.right
            anchors.leftMargin: 12
            anchors.top: imageGrid.top
            spacing: 3

            Image {
                source: "images/" +
                        (playlist.type === SpotifyPlaylist.Inbox ? "inbox"
                                                                 : playlist.type === SpotifyPlaylist.Starred ? "starred"
                                                                                                             : "playlist")
                        + (playlist.collaborative ? "-collaborative" : "")
                        + ".png"
            }

            Text {
                y: 1
                font.pixelSize: 14
                color: "white"
                style: Text.Raised
                font.bold: true
                text: playlist.type === SpotifyPlaylist.Starred ? "Starred" : playlist.type === SpotifyPlaylist.Inbox ? "Inbox" : playlist.name
            }
        }

        Text {
            anchors.top: imageGrid.top
            anchors.right: parent.right
            anchors.rightMargin: 20
            font.pixelSize: 14
            color: "#9a9a9a"
            style: Text.Sunken
            text: playlist.trackCount + " track" + (playlist.trackCount > 1 ? "s" : "") + ", " + spotify.formatDuration(playlist.totalDuration)
        }
    }

    TrackListView {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        model: playlist ? playlist.tracks : 0
    }
}
