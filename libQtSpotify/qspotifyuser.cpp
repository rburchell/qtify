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


#include "qspotifyuser.h"
#include "qspotifyplaylistcontainer.h"
#include "qspotifysession.h"
#include "qspotifyplaylist.h"
#include "qspotifytrack.h"
#include "qspotifyalbumbrowse.h"
#include "qspotifyalbum.h"

#include <libspotify/api.h>

QSpotifyUser::QSpotifyUser(sp_user *user)
    : QSpotifyObject(true)
    , m_playlistContainer(0)
    , m_starredList(0)
    , m_inbox(0)
{
    sp_user_add_ref(user);
    m_sp_user = user;
    m_canonicalName = QString::fromUtf8(sp_user_canonical_name(m_sp_user));

    connect(this, SIGNAL(dataChanged()), this, SIGNAL(userDataChanged()));

    metadataUpdated();
}

QSpotifyUser::~QSpotifyUser()
{
    delete m_playlistContainer;
    sp_user_release(m_sp_user);
}

bool QSpotifyUser::isLoaded()
{
    return sp_user_is_loaded(m_sp_user);
}

bool QSpotifyUser::updateData()
{
    QString canonicalName = QString::fromUtf8(sp_user_canonical_name(m_sp_user));
    QString displayName = QString::fromUtf8(sp_user_display_name(m_sp_user));

    bool updated = false;
    if (m_canonicalName != canonicalName) {
        m_canonicalName = canonicalName;
        updated = true;
    }
    if (m_displayName != displayName) {
        m_displayName = displayName;
        updated = true;
    }

    return updated;
}

QSpotifyPlaylistContainer *QSpotifyUser::playlistContainer() const
{
    if (!m_playlistContainer) {
        sp_playlistcontainer *pc;
        if (QSpotifySession::instance()->user() == this) {
            pc = sp_session_playlistcontainer(QSpotifySession::instance()->m_sp_session);
            sp_playlistcontainer_add_ref(pc);
        } else {
            pc = sp_session_publishedcontainer_for_user_create(QSpotifySession::instance()->m_sp_session, m_canonicalName.toUtf8().constData());
        }
        m_playlistContainer = new QSpotifyPlaylistContainer(pc);
        connect(m_playlistContainer, SIGNAL(playlistContainerDataChanged()), this, SIGNAL(playlistsChanged()));
        connect(m_playlistContainer, SIGNAL(playlistsNameChanged()), this, SIGNAL(playlistsNameChanged()));
    }
    return m_playlistContainer;
}

QSpotifyPlaylist *QSpotifyUser::starredList() const
{
    if (!m_starredList) {
        sp_playlist *sl;
        if (QSpotifySession::instance()->user() == this) {
            sl = sp_session_starred_create(QSpotifySession::instance()->m_sp_session);
        } else {
            sl = sp_session_starred_for_user_create(QSpotifySession::instance()->m_sp_session, m_canonicalName.toUtf8().constData());
        }
        m_starredList = new QSpotifyPlaylist(QSpotifyPlaylist::Starred, sl, false);
    }
    return m_starredList;
}

QSpotifyPlaylist *QSpotifyUser::inbox() const
{
    if (QSpotifySession::instance()->user() != this)
        return 0;

    if (!m_inbox) {
        sp_playlist *in;
        in = sp_session_inbox_create(QSpotifySession::instance()->m_sp_session);
        m_inbox = new QSpotifyPlaylist(QSpotifyPlaylist::Inbox, in, false);
    }
    return m_inbox;
}

QList<QObject*> QSpotifyUser::playlistsAsQObject() const
{
    QList<QObject*> list;
    list.append((QObject*)inbox());
    list.append((QObject*)starredList());
    list.append(playlistContainer()->formattedPlaylists());
    return list;
}

QList<QSpotifyPlaylist *> QSpotifyUser::playlists() const
{
    return playlistContainer()->playlists();
}

QList<QObject*> QSpotifyUser::playlistsFlat() const
{
    return playlistContainer()->playlistsFlat();
}

bool QSpotifyUser::createPlaylist(const QString &name)
{
    if (name.trimmed().isEmpty())
        return false;

    QString n = name;
    if (n.size() > 255)
        n.resize(255);
    sp_playlist *pl = sp_playlistcontainer_add_new_playlist(m_playlistContainer->m_container, n.toUtf8().constData());
    return pl != 0;
}

bool QSpotifyUser::createPlaylistInFolder(const QString &name, QSpotifyPlaylist *folder)
{
    if (!folder || folder->type() != QSpotifyPlaylist::Folder)
        return createPlaylist(name);

    if (name.trimmed().isEmpty())
        return false;

    QString n = name;
    if (n.size() > 255)
        n.resize(255);
    sp_playlist *pl = sp_playlistcontainer_add_new_playlist(m_playlistContainer->m_container, n.toUtf8().constData());
    if (!pl)
        return false;

    int i = m_playlistContainer->m_playlists.indexOf(folder);
    if (i == -1)
        return true;

    sp_uint64 folderId = sp_playlistcontainer_playlist_folder_id(m_playlistContainer->m_container, i);
    int count = sp_playlistcontainer_num_playlists(m_playlistContainer->m_container);
    for (int j = i + 1; j < count; ++j) {
        if (folderId == sp_playlistcontainer_playlist_folder_id(m_playlistContainer->m_container, j))
            i = j;
    }
    sp_playlistcontainer_move_playlist(m_playlistContainer->m_container, count - 1, i, false);

    return true;
}

bool QSpotifyUser::createPlaylistFromTrack(QSpotifyTrack *track)
{
    if (!track)
        return false;

    sp_playlist *pl = sp_playlistcontainer_add_new_playlist(m_playlistContainer->m_container, track->name().toUtf8().constData());
    if (pl == 0)
        return false;
    sp_playlist_add_tracks(pl, const_cast<sp_track* const*>(&track->m_sp_track), 1, 0, QSpotifySession::instance()->spsession());
    return true;
}

bool QSpotifyUser::createPlaylistFromAlbum(QSpotifyAlbumBrowse *album)
{
    if (!album || !album->m_albumTracks || album->m_albumTracks->m_tracks.count() < 1)
        return false;

    QString playlistName = album->album()->artist() + QLatin1String(" - ") + album->album()->name();
    sp_playlist *pl = sp_playlistcontainer_add_new_playlist(m_playlistContainer->m_container, playlistName.toUtf8().constData());
    if (pl == 0)
        return false;

    int c = album->m_albumTracks->m_tracks.count();
	QVector<sp_track*> tracks;
    for (int i = 0; i < c; ++i)
        tracks[i] = album->m_albumTracks->m_tracks.at(i)->sptrack();
    sp_playlist_add_tracks(pl, const_cast<sp_track* const*>(tracks.data()), c, 0, QSpotifySession::instance()->spsession());
    return true;
}

void QSpotifyUser::removePlaylist(QSpotifyPlaylist *playlist)
{
    if (!playlist)
        return;

    int i = m_playlistContainer->m_playlists.indexOf(playlist);
    if (i == -1)
        return;

    if (playlist->type() == QSpotifyPlaylist::Folder) {
        sp_uint64 folderId = sp_playlistcontainer_playlist_folder_id(m_playlistContainer->m_container, i);
        int count = sp_playlistcontainer_num_playlists(m_playlistContainer->m_container);
        for (int j = i + 1; j < count; ++j) {
            if (folderId == sp_playlistcontainer_playlist_folder_id(m_playlistContainer->m_container, j)) {
                sp_playlistcontainer_remove_playlist(m_playlistContainer->m_container, j);
                break;
            }
        }
    }
    sp_playlistcontainer_remove_playlist(m_playlistContainer->m_container, i);
}

void QSpotifyUser::deleteFolderAndContent(QSpotifyPlaylist *playlist)
{
    if (!playlist || playlist->type() != QSpotifyPlaylist::Folder)
        return;

    int i = m_playlistContainer->m_playlists.indexOf(playlist);
    if (i == -1)
        return;

    sp_uint64 folderId = sp_playlistcontainer_playlist_folder_id(m_playlistContainer->m_container, i);
    int count;
    sp_uint64 currId;
    do {
        sp_playlistcontainer_remove_playlist(m_playlistContainer->m_container, i);
        count = sp_playlistcontainer_num_playlists(m_playlistContainer->m_container);
        currId = sp_playlistcontainer_playlist_folder_id(m_playlistContainer->m_container, i);
    } while (i < count && currId != folderId);
    if (currId == folderId)
        sp_playlistcontainer_remove_playlist(m_playlistContainer->m_container, i);
}

bool QSpotifyUser::ownsPlaylist(QSpotifyPlaylist *playlist)
{
    if (!playlist)
        return false;
    return playlist->owner() == m_canonicalName;
}

bool QSpotifyUser::canModifyPlaylist(QSpotifyPlaylist *playlist)
{
    if (!playlist)
        return false;
    return ownsPlaylist(playlist) || playlist->collaborative();
}
