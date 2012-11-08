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


#include "qspotifytrack.h"
#include "qspotifysession.h"
#include "qspotifyuser.h"
#include "qspotifyplaylist.h"
#include "qspotifytracklist.h"
#include "qspotifyartist.h"
#include "qspotifyalbum.h"
#include "qspotifyplayqueue.h"
#include <libspotify/api.h>

#include <QDebug>

QSpotifyTrack::QSpotifyTrack(sp_track *track, QSpotifyPlaylist *playlist)
    : QSpotifyObject(true)
    , m_playlist(playlist)
    , m_trackList(playlist->m_trackList)
    , m_album(0)
    , m_discNumber(0)
    , m_duration(0)
    , m_discIndex(0)
    , m_isAvailable(false)
    , m_numArtists(0)
    , m_popularity(0)
    , m_seen(true)
    , m_offlineStatus(No)
    , m_isCurrentPlayingTrack(false)
{
    sp_track_add_ref(track);
    m_sp_track = track;
    m_error = TrackError(sp_track_error(m_sp_track));

    connect(QSpotifySession::instance(), SIGNAL(currentTrackChanged()), this, SLOT(onSessionCurrentTrackChanged()));
    connect(this, SIGNAL(dataChanged()), this, SIGNAL(trackDataChanged()));
    connect(QSpotifySession::instance(), SIGNAL(offlineModeChanged()), this, SLOT(onSessionOfflineModeChanged()));

    metadataUpdated();
}

QSpotifyTrack::QSpotifyTrack(sp_track *track, QSpotifyTrackList *tracklist)
    : QSpotifyObject(true)
    , m_playlist(0)
    , m_trackList(tracklist)
    , m_album(0)
    , m_discNumber(0)
    , m_duration(0)
    , m_discIndex(0)
    , m_isAvailable(false)
    , m_numArtists(0)
    , m_popularity(0)
    , m_seen(true)
    , m_offlineStatus(No)
    , m_isCurrentPlayingTrack(false)
{
    sp_track_add_ref(track);
    m_sp_track = track;
    m_error = TrackError(sp_track_error(m_sp_track));

    connect(QSpotifySession::instance(), SIGNAL(currentTrackChanged()), this, SLOT(onSessionCurrentTrackChanged()));
    connect(this, SIGNAL(dataChanged()), this, SIGNAL(trackDataChanged()));

    metadataUpdated();
}

QSpotifyTrack::~QSpotifyTrack()
{
    stop();
    sp_track_release(m_sp_track);
    qDeleteAll(m_artists);
    delete m_album;
}

bool QSpotifyTrack::isLoaded()
{
    return sp_track_is_loaded(m_sp_track);
}

bool QSpotifyTrack::updateData()
{
    bool updated = false;

    TrackError error = TrackError(sp_track_error(m_sp_track));
    if (m_error != error) {
        m_error = error;
        updated = true;
    }

    if (m_error == Ok) {
        QString name = QString::fromUtf8(sp_track_name(m_sp_track));
        int discNumber = sp_track_disc(m_sp_track);
        int duration = sp_track_duration(m_sp_track);
        int discIndex = sp_track_index(m_sp_track);
        bool isAvailable = sp_track_get_availability(QSpotifySession::instance()->m_sp_session, m_sp_track) == SP_TRACK_AVAILABILITY_AVAILABLE;
        int numArtists = sp_track_num_artists(m_sp_track);
        int popularity = sp_track_popularity(m_sp_track);
        OfflineStatus offlineSt = OfflineStatus(sp_track_offline_get_status(m_sp_track));
        if (m_playlist && m_playlist->type() == QSpotifyPlaylist::Inbox) {
            int tindex = m_trackList->m_tracks.indexOf(this);

            if (tindex > -1) {
                bool seen = sp_playlist_track_seen(m_playlist->m_sp_playlist, tindex);
                if (m_seen != seen)
                    updateSeen(seen);

                QString crea = QString::fromUtf8(sp_user_canonical_name(sp_playlist_track_creator(m_playlist->m_sp_playlist, tindex)));
                if (m_creator != crea) {
                    m_creator = crea;
                    updated = true;
                }

                int cd = sp_playlist_track_create_time(m_playlist->m_sp_playlist, tindex);
                QDateTime dt = QDateTime::fromTime_t(cd);
                if (m_creationDate != dt) {
                    m_creationDate = dt;
                    updated = true;
                }
            }
        }


        if (m_name != name) {
            m_name = name;
            updated = true;
        }
        if (m_discNumber != discNumber) {
            m_discNumber = discNumber;
            updated = true;
        }
        if (m_duration != duration) {
            m_duration = duration;
            m_durationString = QSpotifySession::instance()->formatDuration(m_duration);
            updated = true;
        }
        if (m_discIndex != discIndex) {
            m_discIndex = discIndex;
            updated = true;
        }
        if (m_isAvailable != isAvailable) {
            m_isAvailable = isAvailable;
            emit isAvailableChanged();
            updated = true;
        }
        if (m_numArtists != numArtists) {
            m_numArtists = numArtists;
            updated = true;
        }
        if (m_popularity != popularity) {
            m_popularity = popularity;
            updated = true;
        }
        if (m_offlineStatus != offlineSt) {
            m_offlineStatus = offlineSt;
            emit offlineStatusChanged();
            updated = true;
        }

        if (m_artists.isEmpty()) {
            int count = sp_track_num_artists(m_sp_track);
            for (int i = 0; i < count; ++i) {
                sp_artist *artist = sp_track_artist(m_sp_track, i);
                QSpotifyArtist *a = new QSpotifyArtist(artist);
                m_artists.append(a);
                m_artistsString += a->name();
                if (i != count - 1)
                    m_artistsString += QLatin1String(", ");
            }
            updated = true;
        }
        if (!m_album) {
            m_album = new QSpotifyAlbum(sp_track_album(m_sp_track));
            updated = true;
            m_albumString = m_album->name();
        }
    }

    return updated;
}

QString QSpotifyTrack::artists() const
{
    return m_artistsString;
}

QString QSpotifyTrack::album() const
{
    return m_albumString;
}

QString QSpotifyTrack::albumCoverId() const
{
    if (!m_album)
        return QString();

    return m_album->coverId();
}

bool QSpotifyTrack::isStarred() const
{
    return QSpotifySession::instance()->user()->starredList()->contains(m_sp_track);
}

void QSpotifyTrack::setIsStarred(bool v)
{
    sp_track_set_starred(QSpotifySession::instance()->m_sp_session, const_cast<sp_track* const*>(&m_sp_track), 1, v);
}

void QSpotifyTrack::play()
{
    QSpotifySession::instance()->m_playQueue->playTrack(this);
}

void QSpotifyTrack::pause()
{
    if (isCurrentPlayingTrack())
        QSpotifySession::instance()->pause();
}

void QSpotifyTrack::resume()
{
    if (isCurrentPlayingTrack())
        QSpotifySession::instance()->resume();
}

void QSpotifyTrack::stop()
{
    if (isCurrentPlayingTrack())
        QSpotifySession::instance()->stop();
}

void QSpotifyTrack::seek(int offset)
{
    if (!isCurrentPlayingTrack())
        play();
    QSpotifySession::instance()->seek(offset);
}

void QSpotifyTrack::enqueue()
{
    QSpotifySession::instance()->enqueue(this);
}

void QSpotifyTrack::removeFromPlaylist()
{
    if (m_playlist)
        m_playlist->remove(this);
}

void QSpotifyTrack::onSessionCurrentTrackChanged()
{
    bool newValue = QSpotifySession::instance()->currentTrack() == this;
    if (m_isCurrentPlayingTrack != newValue) {
        m_isCurrentPlayingTrack = newValue;
        emit isCurrentPlayingTrackChanged();
    }
}

void QSpotifyTrack::onStarredListTracksAdded(QVector<sp_track *> v)
{
    if (v.contains(m_sp_track))
        emit isStarredChanged();
}

void QSpotifyTrack::onStarredListTracksRemoved(QVector<sp_track *> v)
{
    if (v.contains(m_sp_track))
        emit isStarredChanged();
}

void QSpotifyTrack::updateSeen(bool s)
{
    m_seen = s;
    emit seenChanged();
}

void QSpotifyTrack::setSeen(bool s)
{
    if (!m_playlist)
        return;

    sp_playlist_track_set_seen(m_playlist->m_sp_playlist, m_trackList->m_tracks.indexOf(this), s);
}

bool QSpotifyTrack::isAvailableOffline() const
{
    return m_offlineStatus == Yes || m_offlineStatus == DoneResync;
}

bool QSpotifyTrack::isAvailable() const
{
    return m_isAvailable && (!QSpotifySession::instance()->offlineMode() || isAvailableOffline());
}

void QSpotifyTrack::onSessionOfflineModeChanged()
{
    if (!isAvailableOffline())
        emit isAvailableChanged();
}
