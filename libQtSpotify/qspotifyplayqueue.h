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


#ifndef QSPOTIFYPLAYQUEUE_H
#define QSPOTIFYPLAYQUEUE_H

#include <QtCore/QObject>
#include <QtCore/QQueue>

class QSpotifyTrackList;
class QSpotifyTrack;

class QSpotifyPlayQueue : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY tracksChanged)
public:
    QSpotifyPlayQueue();
    ~QSpotifyPlayQueue();

    void playTrack(QSpotifyTrack *track);
    void enqueueTrack(QSpotifyTrack *track);
    void enqueueTracks(QList<QSpotifyTrack *> tracks);
    Q_INVOKABLE void selectTrack(QSpotifyTrack *track);

    Q_INVOKABLE bool isExplicitTrack(int index);

    void playNext(bool repeat);
    void playPrevious();

    void clear();

    void setShuffle(bool s);
    void setRepeat(bool r);

    int currentIndex() const { return m_currentTrackIndex; }

    QList<QObject *> tracks() const;

    bool isCurrentTrackList(QSpotifyTrackList *tl);
    void tracksUpdated();

Q_SIGNALS:
    void tracksChanged();

private Q_SLOTS:
    void onTrackReady();
    void onOfflineModeChanged();

private:

    QSpotifyTrackList *m_implicitTracks;
    QQueue<QSpotifyTrack *> m_explicitTracks;
    QSpotifyTrack *m_currentExplicitTrack;

    mutable int m_currentTrackIndex;

    bool m_shuffle;
    bool m_repeat;

    friend class QSpotifyPlaylist;
};

#endif // QSPOTIFYPLAYQUEUE_H
