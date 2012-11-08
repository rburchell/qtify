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


#include <QtCore/QMutex>
#include <QtCore/QCoreApplication>

#include "qspotifysearch.h"

#include "qspotifyplaylist.h"
#include "qspotifytracklist.h"
#include "qspotifysession.h"
#include "qspotifytrack.h"
#include "qspotifyuser.h"
#include "qspotifyalbum.h"
#include "qspotifyartist.h"

#include <libspotify/api.h>

static QHash<sp_search *, QSpotifySearch *> g_searchObjects;
static QMutex g_mutex;

static void SP_CALLCONV callback_search_complete(sp_search *result, void *)
{
    QMutexLocker lock(&g_mutex);
    QSpotifySearch *s = g_searchObjects.value(result);
    if (s)
        QCoreApplication::postEvent(s, new QEvent(QEvent::User));
}

QSpotifySearch::QSpotifySearch(QObject *parent)
    : QObject(parent)
    , m_sp_search(0)
    , m_trackResults(0)
    , m_busy(false)
    , m_tracksLimit(100)
    , m_albumsLimit(50)
    , m_artistsLimit(50)
{
}

QSpotifySearch::~QSpotifySearch()
{
    if (m_trackResults)
        m_trackResults->release();
    qDeleteAll(m_albumResults);
    m_albumResults.clear();
    qDeleteAll(m_artistResults);
    m_artistResults.clear();
//    clearSearch();
}

void QSpotifySearch::setQuery(const QString &q)
{
    if (q == m_query)
        return;

    m_query = q;
    emit queryChanged();
}

QList<QObject *> QSpotifySearch::tracks() const
{
    QList<QObject*> list;
    if (m_trackResults != 0) {
        int c = m_trackResults->m_tracks.count();
        for (int i = 0; i < c; ++i)
            list.append((QObject*)(m_trackResults->m_tracks[i]));
    }
    return list;
}

void QSpotifySearch::search()
{
    clearSearch();

    m_busy = true;
    emit busyChanged();

    if (!m_query.isEmpty()) {
        QMutexLocker lock(&g_mutex);
        m_sp_search = sp_search_create(QSpotifySession::instance()->m_sp_session, m_query.toUtf8().constData(),
                                       0, m_tracksLimit,
                                       0, m_albumsLimit,
                                       0, m_artistsLimit,
                                       0, 0,
                                       SP_SEARCH_STANDARD,
                                       callback_search_complete, 0);
        g_searchObjects.insert(m_sp_search, this);
    } else {
        populateResults();
    }
}

void QSpotifySearch::clearSearch()
{
    QMutexLocker lock(&g_mutex);
    if (m_sp_search)
        sp_search_release(m_sp_search);
    g_searchObjects.remove(m_sp_search);
    m_sp_search = 0;
}

bool QSpotifySearch::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        populateResults();
        e->accept();
        return true;
    }
    return QObject::event(e);
}

void QSpotifySearch::populateResults()
{
    if (m_trackResults) {
        m_trackResults->release();
        m_trackResults = 0;
    }
    qDeleteAll(m_albumResults);
    m_albumResults.clear();
    qDeleteAll(m_artistResults);
    m_artistResults.clear();

    if (m_sp_search) {
        if (sp_search_error(m_sp_search) != SP_ERROR_OK)
            return;

        // Populate tracks
        m_trackResults = new QSpotifyTrackList;
        int c = sp_search_num_tracks(m_sp_search);
        for (int i = 0; i < c; ++i) {
            QSpotifyTrack *track = new QSpotifyTrack(sp_search_track(m_sp_search, i), m_trackResults);
            m_trackResults->m_tracks.append(track);
            connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksAdded(QVector<sp_track*>)), track, SLOT(onStarredListTracksAdded(QVector<sp_track*>)));
            connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksRemoved(QVector<sp_track*>)), track, SLOT(onStarredListTracksRemoved(QVector<sp_track*>)));
        }

        // Populate albums
        c = sp_search_num_albums(m_sp_search);
        for (int i = 0; i < c; ++i) {
            sp_album *a = sp_search_album(m_sp_search, i);
            if (!sp_album_is_available(a))
                continue;
            QSpotifyAlbum *album = new QSpotifyAlbum(a);
            m_albumResults.append((QObject *)album);
        }

        // Populate artists
        c = sp_search_num_artists(m_sp_search);
        for (int i = 0; i < c; ++i) {
            QSpotifyArtist *artist = new QSpotifyArtist(sp_search_artist(m_sp_search, i));
            m_artistResults.append((QObject *)artist);
        }

        m_didYouMean = QString::fromUtf8(sp_search_did_you_mean(m_sp_search));
    }

    m_busy = false;
    emit busyChanged();

    emit resultsChanged();
}
