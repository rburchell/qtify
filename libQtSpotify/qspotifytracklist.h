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


#ifndef QSPOTIFYTRACKLIST_H
#define QSPOTIFYTRACKLIST_H

#include <QtCore/QObject>
#include <QtCore/QList>

class QSpotifyTrack;

class QSpotifyTrackList : public QObject
{
    Q_OBJECT
public:
    QSpotifyTrackList(bool reverse = false);

    QList<QSpotifyTrack *> tracks() const { return m_tracks; }

    void play();
    bool playTrackAtIndex(int i);
    bool next();
    bool previous();
    void playLast();

    int totalDuration() const;

    bool isShuffle() const { return m_shuffle; }
    void setShuffle(bool s);

    void addRef() { ++m_refCount; }
    void release();

private Q_SLOTS:
    void onTrackReady();

private:
    ~QSpotifyTrackList();
    void playCurrentTrack();

    int nextAvailable(int i);
    int previousAvailable(int i);

    bool m_reverse;

    QList<QSpotifyTrack *> m_tracks;

    int m_currentIndex;
    QSpotifyTrack *m_currentTrack;

    bool m_shuffle;
    QList<int> m_shuffleList;
    int m_shuffleIndex;

    int m_refCount;

    friend class QSpotifyTrack;
    friend class QSpotifyPlaylist;
    friend class QSpotifySearch;
    friend class QSpotifyPlayQueue;
    friend class QSpotifyAlbumBrowse;
    friend class QSpotifyArtistBrowse;
    friend class QSpotifyUser;
    friend class QSpotifyToplist;
};

#endif // QSPOTIFYTRACKLIST_H
