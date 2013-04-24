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
import QtSpotify 1.0

BorderImage {
    id: headerRoot
    border.bottom: 1
    source: "images/header-bg.png"

    property SpotifySearch search: search
    signal searchRequested()

    Row {
        id: buttonRow
        anchors.left: parent.left
        anchors.leftMargin: 7
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6

        CustomButton {
            iconName: "back"
            enabled: false
        }

        CustomButton {
            iconName: "forward"
            enabled: false
        }
    }

    TextField {
        id: searchField
        anchors.bottom: buttonRow.bottom
        anchors.left: buttonRow.right
        anchors.leftMargin: 8
        placeholderText: "Search"
        font.family: "Arial"
        font.pixelSize: 12
        width: 225
        Keys.onReturnPressed: doSearch()
        Keys.onEnterPressed: doSearch()

        function doSearch() {
            if (text.length) {
                headerRoot.searchRequested()
                search.query = text;
                search.search();
            }
        }

        style: TextFieldStyle {
            leftMargin: 30
            rightMargin: 25

            background: BorderImage {
                border.left: 14
                border.right: 14
                border.top: 14
                border.bottom: 14
                source: searchField.activeFocus ? "images/toolbar-search-bg-focus.png" : "images/toolbar-search-bg.png"
            }
        }

        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 8
            source: "images/toolbar-search_glass.png"
        }

        CustomButton {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            iconName: "toolbar-search-clear"
            anchors.rightMargin: 6
            visible: searchField.text.length
            onClicked: searchField.text = ""
        }
    }

    SpotifySearch {
        id: search
    }

    CustomButton {
        id: accountButton
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 4
        anchors.right: parent.right
        anchors.rightMargin: 5
        width: accountName.paintedWidth + 47
        iconName: "account-bg"
        hoverEnabled: true
        borderTop: 7
        borderBottom: 7
        borderLeft: 7
        borderRight: 7
        onClicked: accountMenu.popup()

        Text {
            id: accountName
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: -13
            text: spotify.user.displayName
            opacity: 0.79
            font.pixelSize: 12
            color: accountButton.pressed ? "lightgray" : "black"
            style: Text.Raised
            styleColor: accountButton.pressed ? "black" : "#DDDDDD"
        }

        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            source: "images/user.png"
        }
    }


    Menu {
        id: accountMenu
        MenuItem {
            text: "Log Out"
            onTriggered: spotify.logout(false)
        }
    }
}
