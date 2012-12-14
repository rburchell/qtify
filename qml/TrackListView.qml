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
    property alias model: tableView.model

    TableView {
        id: tableView
        anchors.fill: parent

        backgroundColor: "#00000000"
        frame: false
        //    cacheBuffer: 5000

        Component.onCompleted: {
            verticalScrollBar.anchors.topMargin = 0
        }

        verticalScrollBar.style: CustomScrollBarStyle { }
        horizontalScrollBar.style: CustomScrollBarStyle { }

        TableColumn{ role: "isStarred" ; title: ""; width: 30 }
        TableColumn{ role: "name" ; title: "Track"; width: 220 }
        TableColumn{ role: "artists" ; title: "Artist"; width: 140 }
        TableColumn{ role: "duration" ; title: "Time"; width: 50 }
        TableColumn{ role: "album" ; title: "Album"; width: 150 }

        onActivated: model[currentIndex].play()

        headerDelegate: BorderImage {
            source: "images/table-header.png"
            border.left: 2
            border.right: 2
            border.top: 1
            border.bottom: 1

            Text {
                anchors.fill: parent
                anchors.leftMargin: 5
                verticalAlignment: Text.AlignVCenter
                text: itemValue
                font.pixelSize: 12
                color: "#111111"
                font.family: "Arial"
                font.bold: true
                style: Text.Raised
                styleColor: "#cccccc"
            }
        }

        rowDelegate: BorderImage {
            id: rowBackground
            border.top: 1
            height: 20

            states: [
                State {
                    name: "normal"
                    when: !itemSelected && !itemAlternateBackground && (!modelData || !modelData.isCurrentPlayingTrack)
                    PropertyChanges { target: rowBackground; source: "" }
                },
                State {
                    name: "normalalternate"
                    when: !itemSelected && itemAlternateBackground && (!modelData || !modelData.isCurrentPlayingTrack)
                    PropertyChanges { target: rowBackground; source: "images/mainlist-row-alternate.png" }
                },
                State {
                    name: "normalplaying"
                    when: !itemSelected && modelData.isCurrentPlayingTrack
                    PropertyChanges { target: rowBackground; source: "images/mainlist-row-playing-bg.png" }
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

        itemDelegate: Loader {
            anchors.fill: parent
            sourceComponent: role === "isStarred" ? imageComponent : textComponent

            Component {
                id: imageComponent
                Item {
                    CustomButton {
                        anchors.centerIn: parent
                        iconName: (modelData.isCurrentPlayingTrack && !containsMouse && !pressed ? ("currenttrack" + (spotify.isPlaying ? "-playing" : "") + (itemSelected && tableView.activeFocus ? "-activefocus" : ""))
                                                                                                 : modelData.isStarred ? "star-on" : "star-off")
                        hoverEnabled: true
                        onClicked: modelData.isStarred = !modelData.isStarred
                    }
                }
            }

            Component {
                id: textComponent
                Text {
                    id: text
                    anchors.fill: parent
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    anchors.topMargin: 1
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    text: itemValue
                    elide: Text.ElideRight
                    font.pixelSize: 11
                    opacity: 0.79

                    states: [
                        State {
                            name: "normal"
                            when: !itemSelected && !modelData.isCurrentPlayingTrack
                            PropertyChanges { target: text; color: "#ffffff" }
                        },
                        State {
                            name: "playing"
                            when: !itemSelected && modelData.isCurrentPlayingTrack
                            PropertyChanges { target: text; color: "#abf781" }
                        },
                        State {
                            name: "selectedActiveFocus"
                            when: itemSelected && tableView.activeFocus
                            PropertyChanges { target: text; color: "#111111" }
                        },
                        State {
                            name: "selectedNoFocus"
                            when: itemSelected && !tableView.activeFocus
                            PropertyChanges { target: text; color: "#ffffff" }
                        }
                    ]
                }
            }
        }
    }

    BorderImage {
        source: "images/scrollbar-corner.png"
        border.left: 1; border.top: 1
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        visible: tableView.verticalScrollBar.visible && tableView.horizontalScrollBar.visible
    }
}
