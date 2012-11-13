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
    property variant selectedPlaylist: tableView.model[tableView.currentIndex]

    color: "#474747"

    TableView {
        id: tableView
        anchors.fill: parent
        anchors.topMargin: 8

        frame: false
        headerVisible: false
        alternateRowColor: false
        backgroundColor: "transparent"
        Component.onCompleted:  {
            horizontalScrollBar.visible = false
        }

        verticalScrollBar.style: VerticalScrollBarDelegate {
            anchors.topMargin: -10
        }

        TableColumn{ role: "name" ; width: 200 }

        model: spotify.user.playlists

        section.property: "modelData.listSection"
        section.delegate: Text {
            anchors.left: parent.left
            anchors.leftMargin: 7
            height: section.length === 0 ? 11 : 23
            text: section
            verticalAlignment: Text.AlignVCenter
            color: "#868686"
            style: Text.Sunken
            font.capitalization: Font.AllUppercase
            font.pixelSize: 11
        }

        rowDelegate: BorderImage {
            id: rowBackground
            border.top: 1
            height: 20
            states: [
                State {
                    name: "normal"
                    when: !itemSelected
                    PropertyChanges { target: rowBackground; source: "" }
                },
                State {
                    name: "selectedActiveFocus"
                    when: itemSelected && tableView.activeFocus
                    PropertyChanges { target: rowBackground; source: "images/itemlistselectedfocus.png" }
                },
                State {
                    name: "selectedNoFocus"
                    when: itemSelected && !tableView.activeFocus
                    PropertyChanges { target: rowBackground; source: "images/itemlistselected.png" }
                }
            ]
        }

        itemDelegate: Item {
            width: tableView.verticalScrollBar.visible ? tableView.width - tableView.verticalScrollBar.width : tableView.width

            Image {
                id: playlistImage
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 15
                source: "images/" +
                        (modelData.type === SpotifyPlaylist.Inbox ? "inbox"
                                                                  : modelData.type === SpotifyPlaylist.Starred ? "starred"
                                                                                                               : "playlist")
                        + (modelData.collaborative ? "-collaborative" : "")
                        + (itemSelected && tableView.activeFocus ? "-activefocus" : "")
                        + ".png"
            }

            Text {
                id: playlistText
                anchors.top: parent.top
                anchors.topMargin: 1
                anchors.bottom: parent.bottom
                anchors.left: playlistImage.right
                anchors.leftMargin: 5
                anchors.right: playingIcon.left
                anchors.rightMargin: playingIcon.source.length ? 5 : 0
                verticalAlignment: Text.AlignVCenter
                text: modelData.type === SpotifyPlaylist.Starred ? "Starred" : modelData.type === SpotifyPlaylist.Inbox ? "Inbox" : modelData.name
                elide: Text.ElideRight
                style: Text.Raised
                font.pixelSize: 11
                opacity: 0.79
            }

            Image {
                id: playingIcon
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 5
                source: spotify.currentTrack && spotify.currentTrack.playlist === modelData ? ("images/" +
                                                ("currenttrack" + (spotify.isPlaying ? "-playing" : "") + (itemSelected && tableView.activeFocus ? "-activefocus" : ""))
                                                + ".png")
                                             : ""
            }
            states: [
                State {
                    name: "normal"
                    when: !itemSelected
                    PropertyChanges { target: playlistText; color: "#ffffff" }
                    PropertyChanges { target: playlistText; styleColor: "#111111" }
                    PropertyChanges { target: playlistText; font.bold: false }
                },
                State {
                    name: "selectedActiveFocus"
                    when: itemSelected && tableView.activeFocus
                    PropertyChanges { target: playlistText; color: "#111111" }
                    PropertyChanges { target: playlistText; styleColor: "#dddddd" }
                    PropertyChanges { target: playlistText; font.bold: true }
                },
                State {
                    name: "selectedNoFocus"
                    when: itemSelected && !tableView.activeFocus
                    PropertyChanges { target: playlistText; color: "#ffffff" }
                    PropertyChanges { target: playlistText; styleColor: "#222222" }
                    PropertyChanges { target: playlistText; font.bold: true }
                }
            ]
        }
    }
}
