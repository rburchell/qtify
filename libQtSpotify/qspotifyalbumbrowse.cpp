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


#include "qspotifyalbumbrowse.h"

#include "qspotifytracklist.h"
#include "qspotifyalbum.h"
#include "qspotifysession.h"
#include "qspotifyuser.h"
#include "qspotifyplaylist.h"
#include "qspotifytrack.h"
#include "qspotifyplayqueue.h"

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QCoreApplication>
#include <libspotify/api.h>

static QHash<sp_albumbrowse*, QSpotifyAlbumBrowse*> g_albumBrowseObjects;
static QMutex g_mutex;

static void callback_albumbrowse_complete(sp_albumbrowse *result, void *)
{
    QMutexLocker lock(&g_mutex);
    QSpotifyAlbumBrowse *s = g_albumBrowseObjects.value(result);
    if (s)
        QCoreApplication::postEvent(s, new QEvent(QEvent::User));
}

QSpotifyAlbumBrowse::QSpotifyAlbumBrowse(QObject *parent)
    : QObject(parent)
    , m_sp_albumbrowse(0)
    , m_album(0)
    , m_albumTracks(0)
    , m_hasMultipleArtists(false)
    , m_busy(false)
{
}

QSpotifyAlbumBrowse::~QSpotifyAlbumBrowse()
{
    clearData();
}

bool QSpotifyAlbumBrowse::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        processData();
        e->accept();
        return true;
    }
    return QObject::event(e);
}

void QSpotifyAlbumBrowse::setAlbum(QSpotifyAlbum *album)
{
    if (m_album == album)
        return;
    m_album = album;
    emit albumChanged();
    clearData();

    if (!m_album)
        return;

    m_busy = true;
    emit busyChanged();

    QMutexLocker lock(&g_mutex);
    m_sp_albumbrowse = sp_albumbrowse_create(QSpotifySession::instance()->spsession(), m_album->spalbum(), callback_albumbrowse_complete, 0);
    g_albumBrowseObjects.insert(m_sp_albumbrowse, this);
}

QList<QObject *> QSpotifyAlbumBrowse::tracks() const
{
    QList<QObject*> list;
    if (m_albumTracks != 0) {
        int c = m_albumTracks->m_tracks.count();
        for (int i = 0; i < c; ++i)
            list.append((QObject*)(m_albumTracks->m_tracks[i]));
    }
    return list;
}

void QSpotifyAlbumBrowse::clearData()
{
    if (m_sp_albumbrowse) {
        g_albumBrowseObjects.remove(m_sp_albumbrowse);
        sp_albumbrowse_release(m_sp_albumbrowse);
        m_sp_albumbrowse = 0;
    }
    if (m_albumTracks) {
        m_albumTracks->release();
        m_albumTracks = 0;
    }
    m_hasMultipleArtists = false;
    m_review.clear();
}

void QSpotifyAlbumBrowse::processData()
{
    if (m_sp_albumbrowse) {
        if (sp_albumbrowse_error(m_sp_albumbrowse) != SP_ERROR_OK)
            return;

        m_albumTracks = new QSpotifyTrackList;
        int c = sp_albumbrowse_num_tracks(m_sp_albumbrowse);
        for (int i = 0; i < c; ++i) {
            sp_track *track = sp_albumbrowse_track(m_sp_albumbrowse, i);
            QSpotifyTrack *qtrack = new QSpotifyTrack(track, m_albumTracks);
            m_albumTracks->m_tracks.append(qtrack);
            connect(qtrack, SIGNAL(isStarredChanged()), this, SIGNAL(isStarredChanged()));
            connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksAdded(QVector<sp_track*>)), qtrack, SLOT(onStarredListTracksAdded(QVector<sp_track*>)));
            connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksRemoved(QVector<sp_track*>)), qtrack, SLOT(onStarredListTracksRemoved(QVector<sp_track*>)));
            if (qtrack->artists() != m_album->artist())
                m_hasMultipleArtists = true;
        }

        m_review = QString::fromUtf8(sp_albumbrowse_review(m_sp_albumbrowse)).split(QLatin1Char('\n'), QString::SkipEmptyParts);
        if (m_review.isEmpty())
            m_review << QLatin1String("No review available");

        m_busy = false;
        emit busyChanged();

        emit tracksChanged();
    }
}

int QSpotifyAlbumBrowse::totalDuration() const
{
    if (m_albumTracks)
        return m_albumTracks->totalDuration();
    else
        return 0;
}

void QSpotifyAlbumBrowse::play()
{
    if (!m_albumTracks || m_albumTracks->m_tracks.isEmpty())
        return;

    QSpotifyTrack *track = m_albumTracks->m_tracks.at(0);
    QSpotifySession::instance()->playQueue()->playTrack(track);
}

void QSpotifyAlbumBrowse::enqueue()
{
    if (!m_albumTracks)
        return;

    QSpotifySession::instance()->playQueue()->enqueueTracks(m_albumTracks->m_tracks);
}

bool QSpotifyAlbumBrowse::isStarred() const
{
    if (!m_albumTracks)
        return false;

    int c = m_albumTracks->m_tracks.count();
    for (int i = 0; i < c; ++i) {
        if (!m_albumTracks->m_tracks.at(i)->isStarred())
            return false;
    }
    return true;
}

void QSpotifyAlbumBrowse::setStarred(bool s)
{
    if (!m_albumTracks)
        return;

    int c = m_albumTracks->m_tracks.count();
    const sp_track *tracks[c];
    for (int i = 0; i < c; ++i)
        tracks[i] = m_albumTracks->m_tracks.at(i)->sptrack();
    sp_track_set_starred(QSpotifySession::instance()->spsession(), const_cast<sp_track* const*>(tracks), c, s);
}
