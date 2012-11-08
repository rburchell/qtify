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


#include "qspotifytracklist.h"

#include "qspotifytrack.h"
#include "qspotifysession.h"

QSpotifyTrackList::QSpotifyTrackList(bool reverse)
    : QObject()
    , m_reverse(reverse)
    , m_currentIndex(0)
    , m_currentTrack(0)
    , m_shuffle(false)
    , m_shuffleIndex(0)
    , m_refCount(1)
{
}

QSpotifyTrackList::~QSpotifyTrackList()
{
    int c = m_tracks.count();
    for (int i = 0; i < c; ++i)
        m_tracks[i]->release();
}

void QSpotifyTrackList::play()
{
    if (m_tracks.count() == 0)
        return;

    if (m_shuffle)
        playTrackAtIndex(m_shuffleList.first());
    else
        playTrackAtIndex(m_reverse ? previousAvailable(m_tracks.count()) : nextAvailable(-1));
}

bool QSpotifyTrackList::playTrackAtIndex(int i)
{
    if (i < 0 || i >= m_tracks.count()) {
        m_currentTrack->release();
        m_currentTrack = 0;
        m_currentIndex = 0;
        return false;
    }

    if (m_shuffle)
        m_shuffleIndex = m_shuffleList.indexOf(i);
    m_currentTrack = m_tracks.at(i);
    m_currentTrack->addRef();
    m_currentIndex = i;
    playCurrentTrack();
    return true;
}

bool QSpotifyTrackList::next()
{
    if (m_shuffle) {
        if (m_shuffleIndex + 1 >= m_shuffleList.count()) {
            m_currentTrack->release();
            m_currentTrack = 0;
            return false;
        }
        return playTrackAtIndex(m_shuffleList.at(m_shuffleIndex + 1));
    } else {
        int index = m_tracks.indexOf(m_currentTrack);
        if (index == -1) {
            int newi = qMin(m_currentIndex, m_tracks.count() - 1);
            return playTrackAtIndex(m_reverse ? previousAvailable(newi) : nextAvailable(newi - 1));
        }
        return playTrackAtIndex(m_reverse ? previousAvailable(index) : nextAvailable(index));
    }
}

bool QSpotifyTrackList::previous()
{
    if (m_shuffle) {
        if (m_shuffleIndex - 1 < 0) {
            m_currentTrack->release();
            m_currentTrack = 0;
            return false;
        }
        return playTrackAtIndex(m_shuffleList.at(m_shuffleIndex - 1));
    } else {
        int index = m_tracks.indexOf(m_currentTrack);
        if (index == -1) {
            int newi = qMin(m_currentIndex, m_tracks.count() - 1);
            return playTrackAtIndex(m_reverse ? nextAvailable(newi - 1) : previousAvailable(newi));
        }
        return playTrackAtIndex(m_reverse ? nextAvailable(index) : previousAvailable(index));
    }
}

void QSpotifyTrackList::playLast()
{
    if (m_tracks.count() == 0)
        return;

    if (m_shuffle)
        playTrackAtIndex(m_shuffleList.last());
    else
        playTrackAtIndex(m_reverse ? nextAvailable(-1) : previousAvailable(m_tracks.count()));
}

void QSpotifyTrackList::playCurrentTrack()
{
    if (!m_currentTrack)
        return;

    if (m_currentTrack->isLoaded())
        onTrackReady();
    else
        connect(m_currentTrack, SIGNAL(isLoadedChanged()), this, SLOT(onTrackReady()));
}

void QSpotifyTrackList::onTrackReady()
{
    disconnect(this, SLOT(onTrackReady()));
    QSpotifySession::instance()->play(m_currentTrack);
}

void QSpotifyTrackList::setShuffle(bool s)
{
    m_shuffle = s;

    m_shuffleList.clear();
    m_shuffleIndex = 0;
    bool currentTrackStillExists = m_currentTrack && m_tracks.contains(m_currentTrack);

    if (m_shuffle) {
        qsrand(QTime::currentTime().msec());
        int currentTrackIndex = 0;
        if (currentTrackStillExists) {
            currentTrackIndex = m_tracks.indexOf(m_currentTrack);
            m_shuffleList.append(currentTrackIndex);
        }
        QList<int> indexes;
        for (int i = 0; i < m_tracks.count(); ++i) {
            if ((currentTrackStillExists && i == currentTrackIndex) || !m_tracks.at(i)->isAvailable())
                continue;
            indexes.append(i);
        }
        while (!indexes.isEmpty()) {
            int i = indexes.takeAt(indexes.count() == 1 ? 0 : (qrand() % (indexes.count() - 1)));
            m_shuffleList.append(i);
        }
    }
}

void QSpotifyTrackList::release()
{
    --m_refCount;
    if (m_refCount == 0)
        deleteLater();
}

int QSpotifyTrackList::totalDuration() const
{
    qint64 total = 0;
    for (int i = 0; i < m_tracks.count(); ++i)
        total += m_tracks.at(i)->duration();

    return total;
}

int QSpotifyTrackList::nextAvailable(int i)
{
    do {
        ++i;
    } while (i < m_tracks.count() && !m_tracks.at(i)->isAvailable());
    return i;
}

int QSpotifyTrackList::previousAvailable(int i)
{
    do {
        --i;
    } while (i > -1 && !m_tracks.at(i)->isAvailable());
    return i;
}
