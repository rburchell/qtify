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


#include "qspotifyplaylist.h"
#include "qspotifytrack.h"
#include "qspotifysession.h"
#include "qspotifyplayqueue.h"
#include "qspotifyuser.h"
#include "qspotifyalbumbrowse.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QDebug>

static QHash<sp_playlist*, QSpotifyPlaylist*> g_playlistObjects;

class QSpotifyTracksAddedEvent : public QEvent
{
public:
    QSpotifyTracksAddedEvent(QVector<sp_track*> tracks, int pos)
        : QEvent(Type(User + 3))
        , m_tracks(tracks)
        , m_position(pos)
    { }

    QVector<sp_track*> tracks() const { return m_tracks; }
    int position() const { return m_position; }

private:
    QVector<sp_track*> m_tracks;
    int m_position;
};

class QSpotifyTracksRemovedEvent : public QEvent
{
public:
    QSpotifyTracksRemovedEvent(QVector<int> positions)
        : QEvent(Type(User + 4))
        , m_positions(positions)
    { }

    QVector<int> positions() const { return m_positions; }

private:
    QVector<int> m_positions;
};

class QSpotifyTracksMovedEvent : public QEvent
{
public:
    QSpotifyTracksMovedEvent(QVector<int> positions, int newpos)
        : QEvent(Type(User + 5))
        , m_positions(positions)
        , m_newposition(newpos)
    { }

    QVector<int> positions() const { return m_positions; }
    int newPosition() const { return m_newposition; }

private:
    QVector<int> m_positions;
    int m_newposition;
};

class QSpotifyTrackSeenEvent : public QEvent
{
public:
    QSpotifyTrackSeenEvent(int pos, bool seen)
        : QEvent(Type(User + 6))
        , m_position(pos)
        , m_seen(seen)
    { }

    int position() const { return m_position; }
    bool seen() const { return m_seen; }

private:
    int m_position;
    bool m_seen;
};

static void SP_CALLCONV callback_playlist_state_changed(sp_playlist *playlist, void *)
{
    QCoreApplication::postEvent(g_playlistObjects.value(playlist), new QEvent(QEvent::User));
}

static void SP_CALLCONV callback_playlist_metadata_updated(sp_playlist *playlist, void *)
{
    QCoreApplication::postEvent(g_playlistObjects.value(playlist), new QEvent(QEvent::Type(QEvent::User + 1)));
}

static void SP_CALLCONV callback_playlist_renamed(sp_playlist *playlist, void *)
{
    QCoreApplication::postEvent(g_playlistObjects.value(playlist), new QEvent(QEvent::Type(QEvent::User + 2)));
}

static void SP_CALLCONV callback_tracks_added(sp_playlist *pl, sp_track *const *tracks, int num_tracks, int position, void *)
{
    QVector<sp_track*> vec;
    for (int i = 0; i < num_tracks; ++i)
        vec.append(tracks[i]);
    QCoreApplication::postEvent(g_playlistObjects.value(pl), new QSpotifyTracksAddedEvent(vec, position));
}

static void SP_CALLCONV callback_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *)
{
    QVector<int> vec;
    for (int i = 0; i < num_tracks; ++i)
        vec.append(tracks[i]);
    QCoreApplication::postEvent(g_playlistObjects.value(pl), new QSpotifyTracksRemovedEvent(vec));
}

static void SP_CALLCONV callback_tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *)
{
    QVector<int> vec;
    for (int i = 0; i < num_tracks; ++i)
        vec.append(tracks[i]);
    QCoreApplication::postEvent(g_playlistObjects.value(pl), new QSpotifyTracksMovedEvent(vec, new_position));
}

static void SP_CALLCONV callback_track_seen_changed(sp_playlist *pl, int position, bool seen, void *)
{
    QCoreApplication::postEvent(g_playlistObjects.value(pl), new QSpotifyTrackSeenEvent(position, seen));
}


QSpotifyPlaylist::QSpotifyPlaylist(Type type, sp_playlist *playlist, bool incrRefCount)
    : QSpotifyObject(true)
    , m_type(type)
    , m_offlineStatus(No)
    , m_collaborative(false)
    , m_offlineDownloadProgress(0)
    , m_availableOffline(false)
    , m_skipUpdateTracks(false)
    , m_updateEventPosted(false)
{
    m_trackList = 0;
    if (type != Folder && type != None)
        m_trackList = new QSpotifyTrackList(type == Starred || type == Inbox);

    if (incrRefCount)
        sp_playlist_add_ref(playlist);
    m_sp_playlist = playlist;
    g_playlistObjects.insert(playlist, this);
    m_callbacks = new sp_playlist_callbacks;
    m_callbacks->playlist_state_changed = callback_playlist_state_changed;
    m_callbacks->description_changed = 0;
    m_callbacks->image_changed = 0;
    m_callbacks->playlist_metadata_updated = callback_playlist_metadata_updated;
    m_callbacks->playlist_renamed = callback_playlist_renamed;
    m_callbacks->playlist_update_in_progress = 0;
    m_callbacks->subscribers_changed = 0;
    m_callbacks->tracks_added = callback_tracks_added;
    m_callbacks->tracks_moved = callback_tracks_moved;
    m_callbacks->tracks_removed = callback_tracks_removed;
    m_callbacks->track_created_changed = 0;
    m_callbacks->track_message_changed = 0;
    m_callbacks->track_seen_changed = callback_track_seen_changed;
    sp_playlist_add_callbacks(m_sp_playlist, m_callbacks, 0);
    connect(this, SIGNAL(dataChanged()), this, SIGNAL(playlistDataChanged()));
    connect(this, SIGNAL(isLoadedChanged()), this, SIGNAL(thisIsLoadedChanged()));
    connect(this, SIGNAL(playlistDataChanged()), this , SIGNAL(seenCountChanged()));
    connect(this, SIGNAL(playlistDataChanged()), this, SIGNAL(tracksChanged()));

    metadataUpdated();
}

QSpotifyPlaylist::~QSpotifyPlaylist()
{
    emit playlistDestroyed();
    g_playlistObjects.remove(m_sp_playlist);
    sp_playlist_remove_callbacks(m_sp_playlist, m_callbacks, 0);
    if (m_trackList)
        m_trackList->release();
    sp_playlist_release(m_sp_playlist);
    delete m_callbacks;
}

bool QSpotifyPlaylist::isLoaded()
{
    return sp_playlist_is_loaded(m_sp_playlist);
}

bool QSpotifyPlaylist::updateData()
{
    bool updated = false;

    if (m_type != Folder) {
        QString name = QString::fromUtf8(sp_playlist_name(m_sp_playlist));
        if (m_name != name) {
            m_name = name;
            updated = true;
        }
    }

    QString owner = QString::fromUtf8(sp_user_canonical_name(sp_playlist_owner(m_sp_playlist)));
    if (m_owner != owner) {
        m_owner = owner;
        updated = true;
    }

    bool collab = sp_playlist_is_collaborative(m_sp_playlist);
    if (m_collaborative != collab) {
        m_collaborative = collab;
        updated = true;
    }

    if (m_trackList && m_trackList->m_tracks.isEmpty() && !m_skipUpdateTracks) {
        int count = sp_playlist_num_tracks(m_sp_playlist);
        for (int i = 0; i < count; ++i)
            addTrack(sp_playlist_track(m_sp_playlist, i));
        updated = true;
    }

    OfflineStatus os = OfflineStatus(sp_playlist_get_offline_status(QSpotifySession::instance()->spsession(), m_sp_playlist));
    if (m_offlineStatus != os) {
        if (os == Waiting && m_offlineTracks.count() == m_availableTracks.count())
            m_offlineStatus = Yes;
        else if (os == Yes && m_offlineTracks.count() < m_availableTracks.count())
            m_offlineStatus = Waiting;
        else
            m_offlineStatus = os;

        if (m_offlineStatus != No) {
            m_availableOffline = true;
            emit availableOfflineChanged();
        }

        updated = true;
    }

    if (m_offlineStatus == Downloading) {
        int dp = sp_playlist_get_offline_download_completed(QSpotifySession::instance()->spsession(), m_sp_playlist);
        if (m_offlineDownloadProgress != dp) {
            m_offlineDownloadProgress = dp;
            updated = true;
        }
    }

    return updated;
}

void QSpotifyPlaylist::addTrack(sp_track *track, int pos)
{
    QSpotifyTrack *qtrack = new QSpotifyTrack(track, this);

    registerTrackType(qtrack);

    if (pos == -1)
        m_trackList->m_tracks.append(qtrack);
    else
        m_trackList->m_tracks.insert(pos, qtrack);
    m_tracksSet.insert(track);
    connect(qtrack, SIGNAL(trackDataChanged()), this, SIGNAL(playlistDataChanged()));
    connect(qtrack, SIGNAL(offlineStatusChanged()), this, SLOT(onTrackChanged()));
    connect(qtrack, SIGNAL(isAvailableChanged()), this, SLOT(onTrackChanged()));
    if (m_type != Starred) {
        connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksAdded(QVector<sp_track*>)), qtrack, SLOT(onStarredListTracksAdded(QVector<sp_track*>)));
        connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksRemoved(QVector<sp_track*>)), qtrack, SLOT(onStarredListTracksRemoved(QVector<sp_track*>)));
    }
    if (m_type == Inbox) {
        connect(qtrack, SIGNAL(seenChanged()), this, SIGNAL(seenCountChanged()));
    }
    qtrack->metadataUpdated();
}

bool QSpotifyPlaylist::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        m_skipUpdateTracks = true;
        metadataUpdated();
        m_skipUpdateTracks = false;
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 1) {
        // TracksMetadata updated
        if (m_trackList) {
            for (int i = 0; i < m_trackList->m_tracks.count(); ++i) {
                m_trackList->m_tracks.at(i)->metadataUpdated();
            }
        }
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 2) {
        // Playlist renamed
        m_name = QString::fromUtf8(sp_playlist_name(m_sp_playlist));
        postUpdateEvent();
        emit nameChanged();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 3) {
        // TracksAdded event
        QSpotifyTracksAddedEvent *ev = static_cast<QSpotifyTracksAddedEvent *>(e);
        QVector<sp_track*> tracks = ev->tracks();
        int pos = ev->position();
        for (int i = 0; i < tracks.count(); ++i)
            addTrack(tracks.at(i), pos++);
        postUpdateEvent();
        if (m_type == Starred || m_type == Inbox)
            emit tracksAdded(tracks);
        m_trackList->setShuffle(m_trackList->isShuffle());
        if (QSpotifySession::instance()->playQueue()->isCurrentTrackList(m_trackList))
            QSpotifySession::instance()->playQueue()->tracksUpdated();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 4) {
        // TracksRemoved event
        QSpotifyTracksRemovedEvent *ev = static_cast<QSpotifyTracksRemovedEvent *>(e);
        QVector<int> tracks = ev->positions();
        QVector<sp_track *> tracksSignal;
        for (int i = 0; i < tracks.count(); ++i) {
            int pos = tracks.at(i);
            if (pos < 0 || pos >= m_trackList->m_tracks.count())
                continue;
            QSpotifyTrack *tr = m_trackList->m_tracks[pos];
            unregisterTrackType(tr);
            disconnect(tr, SIGNAL(offlineStatusChanged()), this, SLOT(onTrackChanged()));
            disconnect(tr, SIGNAL(isAvailableChanged()), this, SLOT(onTrackChanged()));
            tracksSignal.append(tr->m_sp_track);
            m_tracksSet.remove(tr->m_sp_track);
            tr->release();
            m_trackList->m_tracks.replace(pos, 0);
        }
        m_trackList->m_tracks.removeAll(0);
        postUpdateEvent();
        if (m_type == Starred)
            emit tracksRemoved(tracksSignal);
        if (QSpotifySession::instance()->playQueue()->isCurrentTrackList(m_trackList))
            QSpotifySession::instance()->playQueue()->tracksUpdated();
        e->accept();
        m_trackList->setShuffle(m_trackList->isShuffle());
        return true;
    } else if (e->type() == QEvent::User + 5) {
        // TracksMoved event
        QSpotifyTracksMovedEvent *ev = static_cast<QSpotifyTracksMovedEvent *>(e);
        QVector<int> positions = ev->positions();
        int newpos = ev->newPosition();
        QVector<QSpotifyTrack*> tracks;
        for (int i = 0; i < positions.count(); ++i) {
            int pos = positions.at(i);
            if (pos < 0 || pos >= m_trackList->m_tracks.count())
                continue;
            tracks.append(m_trackList->m_tracks[pos]);
            m_trackList->m_tracks.replace(pos, 0);
        }
        for (int i = 0; i < tracks.count(); ++i)
            m_trackList->m_tracks.insert(newpos++, tracks.at(i));
        m_trackList->m_tracks.removeAll(0);
        postUpdateEvent();
        if (QSpotifySession::instance()->playQueue()->isCurrentTrackList(m_trackList))
            QSpotifySession::instance()->playQueue()->tracksUpdated();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 6) {
        // TrackSeen event
        if (m_type == Inbox) {
            QSpotifyTrackSeenEvent *ev = static_cast<QSpotifyTrackSeenEvent*>(e);
            m_trackList->m_tracks.at(ev->position())->updateSeen(ev->seen());
        }
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 7) {
        emit dataChanged();
        m_updateEventPosted = false;
        e->accept();
        return true;
    }
    return QSpotifyObject::event(e);
}

void QSpotifyPlaylist::postUpdateEvent()
{
    if (!m_updateEventPosted) {
        QCoreApplication::postEvent(this, new QEvent(QEvent::Type(QEvent::User + 7)));
        m_updateEventPosted = true;
    }
}

void QSpotifyPlaylist::add(QSpotifyTrack *track)
{
    if (!track)
        return;

    sp_playlist_add_tracks(m_sp_playlist, const_cast<sp_track* const*>(&track->m_sp_track), 1, m_trackList->m_tracks.count(), QSpotifySession::instance()->spsession());
}

void QSpotifyPlaylist::remove(QSpotifyTrack *track)
{
    if (!track)
        return;

    int i = m_trackList->m_tracks.indexOf(track);
    if (i > -1)
        sp_playlist_remove_tracks(m_sp_playlist, &i, 1);
}

void QSpotifyPlaylist::addAlbum(QSpotifyAlbumBrowse *album)
{
    if (!album || !album->m_albumTracks)
        return;

    int c = album->m_albumTracks->m_tracks.count();
    if (c < 1)
        return;

	QVector<sp_track*> tracks;
    for (int i = 0; i < c; ++i)
        tracks[i] = album->m_albumTracks->m_tracks.at(i)->sptrack();
    sp_playlist_add_tracks(m_sp_playlist, const_cast<sp_track* const*>(tracks.data()), c, m_trackList->m_tracks.count(), QSpotifySession::instance()->spsession());
}

void QSpotifyPlaylist::rename(const QString &name)
{
    if (name.trimmed().isEmpty())
        return;

    QString n = name;
    if (n.size() > 255)
        n.resize(255);

    sp_playlist_rename(m_sp_playlist, n.toUtf8().constData());
}

int QSpotifyPlaylist::trackCount() const
{
    int c = 0;
    if (m_trackList) {
        for (int i = 0; i < m_trackList->m_tracks.count(); ++i) {
            if (m_trackList->m_tracks.at(i)->error() == QSpotifyTrack::Ok)
                ++c;
        }
    }
    return c;
}

static bool stringContainsWord(const QString &string, const QString &word)
{
    if (word.isEmpty())
        return true;

    int index = string.indexOf(word, 0, Qt::CaseInsensitive);

    if (index == -1)
        return false;

    if (index == 0 || string.at(index - 1) == QLatin1Char(' '))
        return true;

    return false;
}

QList<QObject*> QSpotifyPlaylist::tracksAsQObject() const
{
    QList<QObject*> list;
    if (m_type == Starred || m_type == Inbox) {
        // Reverse order for StarredList to get the most recents first
        for (int i = m_trackList->m_tracks.count() - 1; i >= 0 ; --i) {
            QSpotifyTrack *t = m_trackList->m_tracks[i];
            if (t->error() == QSpotifyTrack::Ok && (m_trackFilter.isEmpty()
                                                    || stringContainsWord(t->name(), m_trackFilter)
                                                    || stringContainsWord(t->artists(), m_trackFilter)
                                                    || stringContainsWord(t->album(), m_trackFilter)
                                                    || stringContainsWord(t->creator(), m_trackFilter))) {
                list.append((QObject*)(t));
            }
        }
    } else if (m_type == Playlist) {
        for (int i = 0; i < m_trackList->m_tracks.count(); ++i) {
            QSpotifyTrack *t = m_trackList->m_tracks[i];
            if (t->error() == QSpotifyTrack::Ok && (m_trackFilter.isEmpty()
                                                    || stringContainsWord(t->name(), m_trackFilter)
                                                    || stringContainsWord(t->artists(), m_trackFilter)
                                                    || stringContainsWord(t->album(), m_trackFilter))) {
                list.append((QObject*)(t));
            }
        }
    }
    return list;
}

int QSpotifyPlaylist::totalDuration() const
{
    if (!m_trackList)
        return 0;

    return m_trackList->totalDuration();
}

QString QSpotifyPlaylist::listSection() const
{
    if (m_type == Inbox)
        return QLatin1String("Main");
    else if (m_type == Starred)
        return QLatin1String("Collection");
    else
        return QLatin1String("");
}

void QSpotifyPlaylist::removeFromContainer()
{
    QSpotifySession::instance()->user()->removePlaylist(this);
}

void QSpotifyPlaylist::deleteFolderContent()
{
    QSpotifySession::instance()->user()->deleteFolderAndContent(this);
}

bool QSpotifyPlaylist::isCurrentPlaylist() const
{
    return QSpotifySession::instance()->m_playQueue->m_implicitTracks == m_trackList;
}

void QSpotifyPlaylist::setCollaborative(bool c)
{
    sp_playlist_set_collaborative(m_sp_playlist, c);
}

void QSpotifyPlaylist::setAvailableOffline(bool offline)
{
    if (m_type == Folder) {
        for (int i = 0; i < m_availablePlaylists.count(); ++i)
            dynamic_cast<QSpotifyPlaylist *>(m_availablePlaylists.at(i))->setAvailableOffline(offline);
    } else {
        if (m_availableOffline == offline)
            return;

        m_availableOffline = offline;
        sp_playlist_set_offline_mode(QSpotifySession::instance()->spsession(), m_sp_playlist, offline);
    }
    emit availableOfflineChanged();
}

void QSpotifyPlaylist::play()
{
    if (!m_trackList || m_trackList->m_tracks.isEmpty())
        return;

    int i = (m_type == Starred || m_type == Inbox) ? m_trackList->previousAvailable(m_trackList->m_tracks.count())
                                             : m_trackList->nextAvailable(-1);
    QSpotifySession::instance()->m_playQueue->playTrack(m_trackList->m_tracks.at(i));
}

void QSpotifyPlaylist::enqueue()
{
    if (m_type == Folder) {
        for (int i = 0; i < m_availablePlaylists.count(); ++i)
            dynamic_cast<QSpotifyPlaylist *>(m_availablePlaylists.at(i))->enqueue();
    } else {
        int c = m_trackList->m_tracks.count();
        if (m_type == Starred || m_type == Inbox) {
            // Reverse order for StarredList to get the most recents first
            QList<QSpotifyTrack *> tracks;
            for (int i = c - 1; i >= 0 ; --i)
                tracks.append(m_trackList->m_tracks.at(i));
            QSpotifySession::instance()->playQueue()->enqueueTracks(tracks);
        } else {
            QSpotifySession::instance()->playQueue()->enqueueTracks(m_trackList->m_tracks);
        }
    }
}

int QSpotifyPlaylist::unseenCount() const
{
    if (m_type != Inbox)
        return 0;

    int c = 0;
    for (int i = 0; i < m_trackList->m_tracks.count(); ++i) {
        QSpotifyTrack *t = m_trackList->m_tracks.at(i);
        if (t->error() == QSpotifyTrack::Ok && !t->seen())
            ++c;
    }
    return c;
}

void QSpotifyPlaylist::onTrackChanged()
{
    if (!sender())
        return;

    QSpotifyTrack *tr = dynamic_cast<QSpotifyTrack *>(sender());
    if (!tr)
        return;

    registerTrackType(tr);
}

void QSpotifyPlaylist::registerTrackType(QSpotifyTrack *t)
{
    int oldCount = m_offlineTracks.count();
    if (t->offlineStatus() == QSpotifyTrack::Yes)
        m_offlineTracks.insert(t);
    else
        m_offlineTracks.remove(t);
    if ((oldCount == 0 && m_offlineTracks.count() > 0) || (oldCount == 1 && m_offlineTracks.count() == 0))
        emit hasOfflineTracksChanged();

    if (t->m_isAvailable) {
        m_availableTracks.insert(t);
    }
}

void QSpotifyPlaylist::unregisterTrackType(QSpotifyTrack *t)
{
    m_offlineTracks.remove(t);
    m_availableTracks.remove(t);
}

void QSpotifyPlaylist::setTrackFilter(const QString &filter)
{
    if (m_trackFilter == filter)
        return;

    m_trackFilter = filter;
    emit trackFilterChanged();
    emit tracksChanged();
}

QStringList QSpotifyPlaylist::albumArtsForPlaylistImage() const
{
    QStringList list;

    for (int i = 0; i < m_trackList->m_tracks.count() && list.count() < 4; ++i) {
        QSpotifyTrack *tr = m_trackList->m_tracks.at(i);
        if (!tr->albumCoverId().isEmpty() && !list.contains(tr->albumCoverId()))
            list.append(tr->albumCoverId());
    }

    return list;
}
