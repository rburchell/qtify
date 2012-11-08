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


#include "qspotifyplayqueue.h"

#include "qspotifytrack.h"
#include "qspotifytracklist.h"
#include "qspotifysession.h"

QSpotifyPlayQueue::QSpotifyPlayQueue()
    : QObject()
    , m_implicitTracks(0)
    , m_currentExplicitTrack(0)
    , m_currentTrackIndex(0)
    , m_shuffle(false)
    , m_repeat(false)
{
}

QSpotifyPlayQueue::~QSpotifyPlayQueue()
{
    clear();
}

void QSpotifyPlayQueue::playTrack(QSpotifyTrack *track)
{
    if (m_currentExplicitTrack) {
        m_currentExplicitTrack->release();
        m_currentExplicitTrack = 0;
    }

    if (m_implicitTracks != track->m_trackList) {
        if (m_implicitTracks)
            m_implicitTracks->release();
        m_implicitTracks = track->m_trackList;
        m_implicitTracks->addRef();
    }
    m_implicitTracks->playTrackAtIndex(m_implicitTracks->m_tracks.indexOf(track));
    m_implicitTracks->setShuffle(m_shuffle);

    emit tracksChanged();
}

void QSpotifyPlayQueue::enqueueTrack(QSpotifyTrack *track)
{
    track->addRef();
    m_explicitTracks.enqueue(track);

    emit tracksChanged();
}

void QSpotifyPlayQueue::enqueueTracks(QList<QSpotifyTrack *> tracks)
{
    for (int i = 0; i < tracks.count(); ++i) {
        QSpotifyTrack *t = tracks.at(i);
        t->addRef();
        m_explicitTracks.enqueue(t);
    }
    emit tracksChanged();
}

void QSpotifyPlayQueue::selectTrack(QSpotifyTrack *track)
{
    if (m_currentExplicitTrack == track || m_implicitTracks->m_currentTrack == track)
        return;

    if (m_currentExplicitTrack) {
        m_currentExplicitTrack->release();
        m_currentExplicitTrack = 0;
    }

    int explicitPos = m_explicitTracks.indexOf(track);
    if (explicitPos != -1) {
        m_explicitTracks.removeAt(explicitPos);
        m_currentExplicitTrack = track;
        if (m_currentExplicitTrack->isLoaded())
            onTrackReady();
        else
            connect(m_currentExplicitTrack, SIGNAL(isLoadedChanged()), this, SLOT(onTrackReady()));
    } else {
        m_implicitTracks->playTrackAtIndex(m_implicitTracks->m_tracks.indexOf(track));
    }

    emit tracksChanged();
}

bool QSpotifyPlayQueue::isExplicitTrack(int index)
{
    return index > m_currentTrackIndex && index <= m_currentTrackIndex + m_explicitTracks.count();
}

void QSpotifyPlayQueue::playNext(bool repeat)
{
    if (repeat) {
        QSpotifySession::instance()->stop(true);
        QSpotifySession::instance()->play(m_currentExplicitTrack ? m_currentExplicitTrack : m_implicitTracks->m_currentTrack);
    } else {
        if (m_currentExplicitTrack) {
            m_currentExplicitTrack->release();
            m_currentExplicitTrack = 0;
        }

        if (m_explicitTracks.isEmpty()) {
            if (m_implicitTracks) {
                if (!m_implicitTracks->next()) {
                    if (m_repeat) {
                        m_implicitTracks->play();
                    } else {
                        QSpotifySession::instance()->stop();
                        m_implicitTracks->release();
                        m_implicitTracks = 0;
                    }
                }
            } else {
                QSpotifySession::instance()->stop();
            }
        } else {
            m_currentExplicitTrack = m_explicitTracks.dequeue();
            if (m_currentExplicitTrack->isLoaded())
                onTrackReady();
            else
                connect(m_currentExplicitTrack, SIGNAL(isLoadedChanged()), this, SLOT(onTrackReady()));
        }
    }

    emit tracksChanged();
}

void QSpotifyPlayQueue::playPrevious()
{
    if (m_currentExplicitTrack) {
        m_currentExplicitTrack->release();
        m_currentExplicitTrack = 0;
    }

    if (m_implicitTracks) {
        if (!m_implicitTracks->previous()) {
            if (m_repeat) {
                m_implicitTracks->playLast();
            } else {
                QSpotifySession::instance()->stop();
                m_implicitTracks->release();
                m_implicitTracks = 0;
            }
        }
    } else {
        QSpotifySession::instance()->stop();
    }

    emit tracksChanged();
}

void QSpotifyPlayQueue::clear()
{
    if (m_currentExplicitTrack) {
        m_currentExplicitTrack->release();
        m_currentExplicitTrack = 0;
    }

    if (m_implicitTracks) {
        m_implicitTracks->release();
        m_implicitTracks = 0;
    }

    int c = m_explicitTracks.count();
    for (int i = 0; i < c; ++i)
        m_explicitTracks[i]->release();
    m_explicitTracks.clear();
}

void QSpotifyPlayQueue::setShuffle(bool s)
{
    if (m_shuffle == s)
        return;
    m_shuffle = s;
    if (m_implicitTracks)
        m_implicitTracks->setShuffle(s);

    emit tracksChanged();
}

void QSpotifyPlayQueue::setRepeat(bool r)
{
    if (m_repeat == r)
        return;

    m_repeat = r;

    emit tracksChanged();
}

void QSpotifyPlayQueue::onTrackReady()
{
    disconnect(this, SLOT(onTrackReady()));
    if (m_currentExplicitTrack)
        QSpotifySession::instance()->play(m_currentExplicitTrack);
}

QList<QObject *> QSpotifyPlayQueue::tracks() const
{
    QList<QObject *> list;

    if (!m_implicitTracks)
        return list;

    int currIndex = 0;

    if (m_shuffle) {
        for (int i = 0; i < m_implicitTracks->m_shuffleList.count(); ++i) {
            QSpotifyTrack * t = m_implicitTracks->m_tracks.at(m_implicitTracks->m_shuffleList.at(i));
            list.append((QObject*)t);
            if (t == m_implicitTracks->m_currentTrack)
                currIndex = i;
        }
    } else {
        if (m_implicitTracks->m_reverse) {
            int i = m_implicitTracks->previousAvailable(m_implicitTracks->m_tracks.count());
            while (i >= 0) {
                QSpotifyTrack * t = m_implicitTracks->m_tracks.at(i);
                list.append((QObject*)t);
                if (t == m_implicitTracks->m_currentTrack)
                    currIndex = m_implicitTracks->m_tracks.count() - 1 - i;
                i = m_implicitTracks->previousAvailable(i);
            }
        } else {
            int i = m_implicitTracks->nextAvailable(-1);
            while (i < m_implicitTracks->m_tracks.count()) {
                QSpotifyTrack * t = m_implicitTracks->m_tracks.at(i);
                list.append((QObject*)t);
                if (t == m_implicitTracks->m_currentTrack)
                    currIndex = i;
                i = m_implicitTracks->nextAvailable(i);
            }
        }
    }

    if (m_currentExplicitTrack)
        list.insert(++currIndex, (QObject*)m_currentExplicitTrack);
    for (int i = 0; i < m_explicitTracks.count(); ++i)
        list.insert(++currIndex, (QObject*)m_explicitTracks.at(i));

    if (m_currentExplicitTrack)
        m_currentTrackIndex = list.indexOf((QObject *)m_currentExplicitTrack);
    else if (m_implicitTracks->m_currentTrack)
        m_currentTrackIndex = list.indexOf((QObject *)m_implicitTracks->m_currentTrack);

    return list;
}

bool QSpotifyPlayQueue::isCurrentTrackList(QSpotifyTrackList *tl)
{
    return m_implicitTracks == tl;
}

void QSpotifyPlayQueue::tracksUpdated()
{
    emit tracksChanged();
}

void QSpotifyPlayQueue::onOfflineModeChanged()
{
    if (m_shuffle && m_implicitTracks)
        m_implicitTracks->setShuffle(true);
    emit tracksChanged();
}
