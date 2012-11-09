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

BorderImage {
    source: "images/footer-bg.png"
    border.top: 2
    border.bottom: 2
    border.left: 0
    border.right: 0

    // Playback controls
    Row {
        id: playbackControls
        height: parent.height
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 2
        anchors.left: parent.left
        anchors.leftMargin: 13
        spacing: 3

        enabled: spotify.hasCurrentTrack

        CustomButton {
            anchors.verticalCenter: parent.verticalCenter
            iconName: "prev"
            onClicked: spotify.playPrevious()
        }
        CustomButton {
            anchors.verticalCenter: parent.verticalCenter
            iconName: spotify.isPlaying ? "pause" : "play"
            onClicked: spotify.isPlaying ? spotify.pause() : spotify.resume()
        }
        CustomButton {
            anchors.verticalCenter: parent.verticalCenter
            iconName: "next"
            onClicked: spotify.playNext()
        }
    }

    // Volume control
    Slider {
        id: volumeSlider
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: playbackControls.right
        anchors.leftMargin: 19
        width: 85
        height: parent.height
        minimumValue: 0
        maximumValue: 100
        value: spotify.volume

        onValueChanged: spotify.setVolume(value)

        delegate: CustomSliderDelegate {
            slider: volumeSlider
        }
    }
    Image {
        id: volumeIcon1
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: volumeSlider.right
        anchors.leftMargin: 3
        anchors.verticalCenterOffset: -1
        source: "images/volume-1.png"
        opacity: mix(0.3, 1.0, smoothstep(0.0, 0.5, spotify.volume / 100))
    }
    Image {
        id: volumeIcon2
        anchors.verticalCenter: volumeIcon1.verticalCenter
        anchors.left: volumeIcon1.right
        anchors.leftMargin: -1
        source: "images/volume-2.png"
        opacity: mix(0.3, 1.0, smoothstep(0.5, 1.0, spotify.volume / 100))
    }
    function smoothstep(x, y, a) {
        var p;
        if (a < x) return 0;
        if (a >= y) return 1;
        if (x === y) return -1;
        p = (a - x) / (y - x);
        return (p * p * (3 - 2 * p));
    }
    function mix(x, y, a) {
        return x * (1.0 - a) + y * a;
    }

    // Track position slider
    Slider {
        id: positionSlider
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: volumeSlider.right
        anchors.leftMargin: 80
        anchors.right: parent.right
        anchors.rightMargin: 137
        height: parent.height
        minimumValue: 0
        maximumValue: spotify.currentTrack ? spotify.currentTrack.durationMs : 0
        stepSize: 1000

        enabled: spotify.hasCurrentTrack

        onPressedChanged:  {
            if (!positionSlider.pressed) {
                spotify.seek(positionSlider.value)
            }
        }

        Connections {
            target: spotify
            onCurrentTrackPositionChanged: {
                if (!positionSlider.pressed)
                    positionSlider.value = spotify.currentTrackPosition;
            }
        }

        delegate: CustomSliderDelegate {
            slider: positionSlider
        }
    }

    // Current track time
    Text {
        anchors.right: positionSlider.left
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        color: "white"
        opacity: 0.79
        text: spotify.formatDuration(positionSlider.value)
        font.pixelSize: 10
        font.family: "Arial"
        style: Text.Sunken
        visible: positionSlider.enabled
    }

    // Total track time
    Text {
        anchors.left: positionSlider.right
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        color: "white"
        opacity: 0.79
        text: spotify.currentTrack ? spotify.currentTrack.duration : ""
        font.pixelSize: 10
        font.family: "Arial"
        style: Text.Sunken
        visible: positionSlider.enabled
    }

    // shuffle, repeat
    CustomButton {
        id: repeatButton
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 20
        iconName: spotify.repeat ? "repeat-on" : "repeat-off"
        onClicked: spotify.repeat = !spotify.repeat
    }
    CustomButton {
        anchors.right: repeatButton.left
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        iconName: spotify.shuffle ? "shuffle-on" : "shuffle-off"
        onClicked: spotify.shuffle = !spotify.shuffle
    }
}
