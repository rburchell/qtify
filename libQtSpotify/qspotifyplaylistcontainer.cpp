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


#include "qspotifyplaylistcontainer.h"
#include "qspotifyplaylist.h"
#include "qspotifysession.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QStack>
#include <libspotify/api.h>

static QHash<sp_playlistcontainer*, QSpotifyPlaylistContainer*> g_containerObjects;

class QSpotifyPlaylistAddedEvent : public QEvent
{
public:
    QSpotifyPlaylistAddedEvent(sp_playlist *playlist, int pos)
        : QEvent(Type(User + 1))
        , m_playlist(playlist)
        , m_position(pos)
    { }

    sp_playlist *playlist() const { return m_playlist; }
    int position() const { return m_position; }

private:
    sp_playlist *m_playlist;
    int m_position;
};

class QSpotifyPlaylistRemovedEvent : public QEvent
{
public:
    QSpotifyPlaylistRemovedEvent(int position)
        : QEvent(Type(User + 2))
        , m_position(position)
    { }

    int position() const { return m_position; }

private:
    int m_position;
};

class QSpotifyPlaylistMovedEvent : public QEvent
{
public:
    QSpotifyPlaylistMovedEvent(int oldpos, int newpos)
        : QEvent(Type(User + 3))
        , m_position(oldpos)
        , m_newposition(newpos)
    { }

    int position() const { return m_position; }
    int newPosition() const { return m_newposition; }

private:
    int m_position;
    int m_newposition;
};

static void SP_CALLCONV callback_container_loaded(sp_playlistcontainer *pc, void *)
{
    QCoreApplication::postEvent(g_containerObjects.value(pc), new QEvent(QEvent::User));
}

static void SP_CALLCONV callback_playlist_added(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *)
{
    QCoreApplication::postEvent(g_containerObjects.value(pc), new QSpotifyPlaylistAddedEvent(playlist, position));
}

static void SP_CALLCONV callback_playlist_removed(sp_playlistcontainer *pc, sp_playlist *, int position, void *)
{
    QCoreApplication::postEvent(g_containerObjects.value(pc), new QSpotifyPlaylistRemovedEvent(position));
}

static void SP_CALLCONV callback_playlist_moved(sp_playlistcontainer *pc, sp_playlist *, int position, int new_position, void *)
{
    QCoreApplication::postEvent(g_containerObjects.value(pc), new QSpotifyPlaylistMovedEvent(position, new_position));
}

QSpotifyPlaylistContainer::QSpotifyPlaylistContainer(sp_playlistcontainer *container)
    : QSpotifyObject(true)
    , m_updateEventPosted(false)
{
    m_container = container;
    g_containerObjects.insert(container, this);
    m_callbacks = new sp_playlistcontainer_callbacks;
    m_callbacks->container_loaded = callback_container_loaded;
    m_callbacks->playlist_added = callback_playlist_added;
    m_callbacks->playlist_moved = callback_playlist_moved;
    m_callbacks->playlist_removed = callback_playlist_removed;
    sp_playlistcontainer_add_callbacks(m_container, m_callbacks, 0);
    connect(QSpotifySession::instance(), SIGNAL(offlineModeChanged()), this, SLOT(updatePlaylists()));

    metadataUpdated();
}

QSpotifyPlaylistContainer::~QSpotifyPlaylistContainer()
{
    g_containerObjects.remove(m_container);
    sp_playlistcontainer_remove_callbacks(m_container, m_callbacks, 0);
    qDeleteAll(m_playlists);
    sp_playlistcontainer_release(m_container);
    delete m_callbacks;
}

bool QSpotifyPlaylistContainer::isLoaded()
{
    return sp_playlistcontainer_is_loaded(m_container);
}

bool QSpotifyPlaylistContainer::updateData()
{
    bool updated = false;

    if (m_playlists.isEmpty()) {
        int count = sp_playlistcontainer_num_playlists(m_container);
        for (int i = 0; i < count; ++i) {
            addPlaylist(sp_playlistcontainer_playlist(m_container, i), i);
            if(sp_playlistcontainer_playlist_type(m_container, i) == SP_PLAYLIST_TYPE_PLACEHOLDER)
                sp_playlistcontainer_remove_playlist(m_container, i);
        }
        updated = true;
        updatePlaylists();
    }

    return updated;
}

void QSpotifyPlaylistContainer::updatePlaylists()
{
    m_formattedAvailablePlaylists.clear();
    m_formattedUnavailablePlaylists.clear();
    m_playlistsFlat.clear();
    QStack<QSpotifyPlaylist *> folders;
    for (int i = 0; i < m_playlists.count(); ++i) {
        QSpotifyPlaylist *playlist = m_playlists.at(i);
        if (playlist->type() == QSpotifyPlaylist::Playlist) {
            m_playlistsFlat.append((QObject*)playlist);
            if (!QSpotifySession::instance()->offlineMode() || playlist->availableOffline()) {
                if (folders.isEmpty())
                    m_formattedAvailablePlaylists.append((QObject*)(playlist));
                else
                    folders.top()->m_availablePlaylists.append((QObject*)(playlist));
            } else {
                if (folders.isEmpty())
                    m_formattedUnavailablePlaylists.append((QObject*)(playlist));
                else
                    folders.top()->m_unavailablePlaylists.append((QObject*)(playlist));
            }
        } else if (playlist->type() == QSpotifyPlaylist::Folder) {
            playlist->clearPlaylists();
            folders.push(playlist);
        } else if (playlist->type() == QSpotifyPlaylist::FolderEnd) {
            if (folders.isEmpty())
                continue;
            QSpotifyPlaylist *folder = folders.pop();
            if (folders.isEmpty())
                m_formattedAvailablePlaylists.append((QObject *)(folder));
            else
                folders.top()->m_availablePlaylists.append(folder);
            emit folder->playlistsChanged();
        }
    }

    emit playlistContainerDataChanged();
}

void QSpotifyPlaylistContainer::addPlaylist(sp_playlist *playlist, int pos)
{
    if (playlist != sp_playlistcontainer_playlist(m_container, pos)) {
        int count = sp_playlistcontainer_num_playlists(m_container);
        for (int i = 0; i < count; ++i) {
            if (playlist == sp_playlistcontainer_playlist(m_container, i)) {
                pos = i;
                break;
            }
        }
    }

    sp_playlist_type type = sp_playlistcontainer_playlist_type(m_container, pos);
    QSpotifyPlaylist *pl = new QSpotifyPlaylist(QSpotifyPlaylist::Type(type), playlist);
    if (pos == -1)
        m_playlists.append(pl);
    else
        m_playlists.insert(pos, pl);
    if (type == SP_PLAYLIST_TYPE_START_FOLDER) {
        char buffer[200];
        sp_playlistcontainer_playlist_folder_name(m_container, pos, buffer, 200);
        pl->m_name = QString::fromUtf8(buffer);
    }
    connect(pl, SIGNAL(nameChanged()), this, SIGNAL(playlistsNameChanged()));
}

bool QSpotifyPlaylistContainer::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        metadataUpdated();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 1) {
        // PlaylistAdded event
        QSpotifyPlaylistAddedEvent *ev = static_cast<QSpotifyPlaylistAddedEvent *>(e);
        addPlaylist(ev->playlist(), ev->position());
        emit dataChanged();
        postUpdateEvent();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 2) {
        // PlaylistRemoved event
        QSpotifyPlaylistRemovedEvent *ev = static_cast<QSpotifyPlaylistRemovedEvent *>(e);
        int i = ev->position();
        if (i >= 0 && i < m_playlists.count()) {
            QSpotifyPlaylist *pl = m_playlists.takeAt(i);
            delete pl;
            emit dataChanged();
        }
        postUpdateEvent();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 3) {
        // PlaylistMoved event
        QSpotifyPlaylistMovedEvent *ev = static_cast<QSpotifyPlaylistMovedEvent *>(e);
        int i = ev->position();
        int newpos = ev->newPosition();
        if (i >= 0 && i < m_playlists.count()) {
            QSpotifyPlaylist *pl = m_playlists.takeAt(i);
            m_playlists.insert(newpos > i ? newpos - 1 : newpos, pl);
            emit dataChanged();
        }
        postUpdateEvent();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 4) {
        updatePlaylists();
        m_updateEventPosted = false;
        e->accept();
        return true;
    }
    return QSpotifyObject::event(e);
}

void QSpotifyPlaylistContainer::postUpdateEvent()
{
    if (!m_updateEventPosted) {
        QCoreApplication::postEvent(this, new QEvent(QEvent::Type(QEvent::User + 4)));
        m_updateEventPosted = true;
    }
}
