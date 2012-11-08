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


#include "qspotifytoplist.h"

#include "qspotifyplaylist.h"
#include "qspotifytracklist.h"
#include "qspotifysession.h"
#include "qspotifytrack.h"
#include "qspotifyuser.h"
#include "qspotifyalbum.h"
#include "qspotifyartist.h"

#include <QtCore/QMutex>
#include <QtCore/QCoreApplication>

#include <libspotify/api.h>

static QHash<sp_toplistbrowse *, QSpotifyToplist *> g_toplistObjects;
static QMutex g_mutex;

class QSpotifyToplistCompleteEvent : public QEvent
{
public:
    QSpotifyToplistCompleteEvent(sp_toplistbrowse *tlbrowse)
        : QEvent(Type(QEvent::User))
        , m_tlbrowse(tlbrowse)
    { }

    sp_toplistbrowse *toplistBrowse() const { return m_tlbrowse; }

private:
    sp_toplistbrowse *m_tlbrowse;
};

static void callback_toplistbrowse_complete(sp_toplistbrowse *result, void *)
{
    QMutexLocker lock(&g_mutex);
    QSpotifyToplist *tl = g_toplistObjects.value(result);
    if (tl)
        QCoreApplication::postEvent(tl, new QSpotifyToplistCompleteEvent(result));
}

QSpotifyToplist::QSpotifyToplist(QObject *parent)
    : QObject(parent)
    , m_sp_browsetracks(0)
    , m_sp_browseartists(0)
    , m_sp_browsealbums(0)
    , m_busy(false)
    , m_trackResults(0)
{
}

QSpotifyToplist::~QSpotifyToplist()
{
    clear();
}

QList<QObject *> QSpotifyToplist::tracks() const
{
    QList<QObject*> list;
    if (m_trackResults != 0) {
        int c = m_trackResults->m_tracks.count();
        for (int i = 0; i < c; ++i)
            list.append((QObject*)(m_trackResults->m_tracks[i]));
    }
    return list;
}

void QSpotifyToplist::updateResults()
{
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_busy || (m_lastUpdate.isValid() && m_lastUpdate.secsTo(currentTime) < 7200))
        return;

    m_lastUpdate = currentTime;

    clear();

    m_busy = true;
    emit busyChanged();

    QMutexLocker lock(&g_mutex);
    m_sp_browsetracks = sp_toplistbrowse_create(QSpotifySession::instance()->spsession(), SP_TOPLIST_TYPE_TRACKS, SP_TOPLIST_REGION_EVERYWHERE, NULL, callback_toplistbrowse_complete, 0);
    g_toplistObjects.insert(m_sp_browsetracks, this);
    m_sp_browseartists = sp_toplistbrowse_create(QSpotifySession::instance()->spsession(), SP_TOPLIST_TYPE_ARTISTS, SP_TOPLIST_REGION_EVERYWHERE, NULL, callback_toplistbrowse_complete, 0);
    g_toplistObjects.insert(m_sp_browseartists, this);
    m_sp_browsealbums = sp_toplistbrowse_create(QSpotifySession::instance()->spsession(), SP_TOPLIST_TYPE_ALBUMS, SP_TOPLIST_REGION_EVERYWHERE, NULL, callback_toplistbrowse_complete, 0);
    g_toplistObjects.insert(m_sp_browsealbums, this);

}

void QSpotifyToplist::clear()
{
    if (m_trackResults)
        m_trackResults->release();
    m_trackResults = 0;
    qDeleteAll(m_albumResults);
    m_albumResults.clear();
    qDeleteAll(m_artistResults);
    m_artistResults.clear();
    emit resultsChanged();

    QMutexLocker lock(&g_mutex);
    if (m_sp_browsetracks)
        sp_toplistbrowse_release(m_sp_browsetracks);
    g_toplistObjects.remove(m_sp_browsetracks);
    m_sp_browsetracks = 0;
    if (m_sp_browseartists)
        sp_toplistbrowse_release(m_sp_browseartists);
    g_toplistObjects.remove(m_sp_browseartists);
    m_sp_browseartists = 0;
    if (m_sp_browsealbums)
        sp_toplistbrowse_release(m_sp_browsealbums);
    g_toplistObjects.remove(m_sp_browsealbums);
    m_sp_browsealbums = 0;
}

bool QSpotifyToplist::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        QSpotifyToplistCompleteEvent *ev = static_cast<QSpotifyToplistCompleteEvent *>(e);
        populateResults(ev->toplistBrowse());
        e->accept();
        return true;
    }
    return QObject::event(e);
}

void QSpotifyToplist::populateResults(sp_toplistbrowse *tl)
{
    if (sp_toplistbrowse_error(tl) != SP_ERROR_OK)
        return;

    if (tl == m_sp_browsetracks) {
        m_trackResults = new QSpotifyTrackList;
        int c = sp_toplistbrowse_num_tracks(tl);
        for (int i = 0; i < c; ++i) {
            QSpotifyTrack *track = new QSpotifyTrack(sp_toplistbrowse_track(tl, i), m_trackResults);
            m_trackResults->m_tracks.append(track);
            connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksAdded(QVector<sp_track*>)), track, SLOT(onStarredListTracksAdded(QVector<sp_track*>)));
            connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksRemoved(QVector<sp_track*>)), track, SLOT(onStarredListTracksRemoved(QVector<sp_track*>)));
        }
    }

    if (tl == m_sp_browseartists) {
        int c = sp_toplistbrowse_num_artists(tl);
        for (int i = 0; i < c; ++i) {
            QSpotifyArtist *artist = new QSpotifyArtist(sp_toplistbrowse_artist(tl, i));
            m_artistResults.append((QObject *)artist);
        }
    }

    if (tl == m_sp_browsealbums) {
        int c = sp_toplistbrowse_num_albums(tl);
        for (int i = 0; i < c; ++i) {
            sp_album *a = sp_toplistbrowse_album(tl, i);
            QSpotifyAlbum *album = new QSpotifyAlbum(a);
            m_albumResults.append((QObject *)album);
        }
    }

    if (m_trackResults && m_artistResults.count() > 0 && m_albumResults.count() > 0) {
        m_busy = false;
        emit busyChanged();
    }

    emit resultsChanged();
}
