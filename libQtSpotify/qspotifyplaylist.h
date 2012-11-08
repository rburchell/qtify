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


#ifndef QSPOTIFYPLAYLIST_H
#define QSPOTIFYPLAYLIST_H

#include <QtCore/QSet>
#include <QtCore/QVector>
#include <QtCore/QMetaType>
#include <QStringList>

#include "qspotifyobject.h"
#include "qspotifytracklist.h"

#include <libspotify/api.h>

struct sp_playlist;
struct sp_playlist_callbacks;
struct sp_track;
class QSpotifyTrack;
class QSpotifyAlbumBrowse;

class QSpotifyPlaylist : public QSpotifyObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY playlistDataChanged)
    Q_PROPERTY(int trackCount READ trackCount NOTIFY playlistDataChanged)
    Q_PROPERTY(int totalDuration READ totalDuration NOTIFY playlistDataChanged)
    Q_PROPERTY(QList<QObject *> tracks READ tracksAsQObject NOTIFY tracksChanged)
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY thisIsLoadedChanged)
    Q_PROPERTY(Type type READ type NOTIFY playlistDataChanged)
    Q_PROPERTY(OfflineStatus offlineStatus READ offlineStatus NOTIFY playlistDataChanged)
    Q_PROPERTY(QString listSection READ listSection NOTIFY thisIsLoadedChanged)
    Q_PROPERTY(QString owner READ owner NOTIFY playlistDataChanged)
    Q_PROPERTY(bool collaborative READ collaborative WRITE setCollaborative NOTIFY playlistDataChanged)
    Q_PROPERTY(int offlineDownloadProgress READ offlineDownloadProgress NOTIFY playlistDataChanged)
    Q_PROPERTY(bool availableOffline READ availableOffline WRITE setAvailableOffline NOTIFY availableOfflineChanged)
    Q_PROPERTY(int unseenCount READ unseenCount NOTIFY seenCountChanged)
    Q_PROPERTY(bool hasOfflineTracks READ hasOfflineTracks NOTIFY hasOfflineTracksChanged)
    Q_PROPERTY(QString trackFilter READ trackFilter WRITE setTrackFilter NOTIFY trackFilterChanged)
    Q_PROPERTY(QList<QObject *> playlists READ playlists NOTIFY playlistsChanged)
    Q_PROPERTY(int playlistCount READ playlistCount NOTIFY playlistsChanged)
    Q_ENUMS(Type)
    Q_ENUMS(OfflineStatus)
public:
    enum Type {
        Playlist = SP_PLAYLIST_TYPE_PLAYLIST,
        Folder = SP_PLAYLIST_TYPE_START_FOLDER,
        FolderEnd = SP_PLAYLIST_TYPE_END_FOLDER,
        None = SP_PLAYLIST_TYPE_PLACEHOLDER,
        Starred,
        Inbox
    };

    enum OfflineStatus {
        No = SP_PLAYLIST_OFFLINE_STATUS_NO,
        Yes = SP_PLAYLIST_OFFLINE_STATUS_YES,
        Downloading = SP_PLAYLIST_OFFLINE_STATUS_DOWNLOADING,
        Waiting = SP_PLAYLIST_OFFLINE_STATUS_WAITING
    };

    virtual ~QSpotifyPlaylist();

    bool isLoaded();

    QString name() const { return m_name; }
    int trackCount() const;
    int totalDuration() const;
    Type type() const { return m_type; }
    OfflineStatus offlineStatus() const { return m_offlineStatus; }
    QString owner() const { return m_owner; }
    bool collaborative() const { return m_collaborative; }
    void setCollaborative(bool c);
    int offlineDownloadProgress() const { return m_offlineDownloadProgress; }
    bool availableOffline() const { return m_availableOffline; }
    void setAvailableOffline(bool offline);
    QString listSection() const;
    QList<QSpotifyTrack *> tracks() const { return m_trackList->tracks(); }
    QList<QObject *> tracksAsQObject() const;
    int unseenCount() const;
    bool hasOfflineTracks() const { return m_offlineTracks.count() > 0; }
    QString trackFilter() const { return m_trackFilter; }
    void setTrackFilter(const QString &filter);
    QList<QObject *> playlists() const { return m_availablePlaylists + m_unavailablePlaylists; }
    int playlistCount() const { return m_availablePlaylists.count() + m_unavailablePlaylists.count(); }

    void clearPlaylists() {
        m_availablePlaylists.clear();
        m_unavailablePlaylists.clear();
    }

    bool contains(sp_track *t) const { return m_tracksSet.contains(t); }

    Q_INVOKABLE void add(QSpotifyTrack *track);
    Q_INVOKABLE void remove(QSpotifyTrack *track);

    Q_INVOKABLE void addAlbum(QSpotifyAlbumBrowse *);

    Q_INVOKABLE void rename(const QString &name);

    Q_INVOKABLE void removeFromContainer();
    Q_INVOKABLE void deleteFolderContent();

    Q_INVOKABLE bool isCurrentPlaylist() const;

    Q_INVOKABLE QStringList albumArtsForPlaylistImage() const;

public Q_SLOTS:
    void play();
    void enqueue();

Q_SIGNALS:
    void playlistDestroyed();
    void playlistDataChanged();
    void thisIsLoadedChanged();
    void tracksAdded(QVector<sp_track *>);
    void tracksRemoved(QVector<sp_track *>);
    void availableOfflineChanged();
    void seenCountChanged();
    void hasOfflineTracksChanged();
    void trackFilterChanged();
    void tracksChanged();
    void nameChanged();
    void playlistsChanged();

protected:
    bool updateData();
    bool event(QEvent *);

private Q_SLOTS:
    void onTrackChanged();

private:
    QSpotifyPlaylist(Type type, sp_playlist *playlist, bool incrRefCount = true);
    void addTrack(sp_track *track, int pos = -1);
    void registerTrackType(QSpotifyTrack *t);
    void unregisterTrackType(QSpotifyTrack *t);

    void postUpdateEvent();

    sp_playlist *m_sp_playlist;
    sp_playlist_callbacks *m_callbacks;

    QSpotifyTrackList *m_trackList;
    QSet<sp_track *> m_tracksSet;

    QString m_name;
    Type m_type;
    OfflineStatus m_offlineStatus;
    QString m_owner;
    bool m_collaborative;
    int m_offlineDownloadProgress;
    bool m_availableOffline;

    QSet<QSpotifyTrack *> m_offlineTracks;
    QSet<QSpotifyTrack *> m_availableTracks;

    QList<QObject *> m_availablePlaylists;
    QList<QObject *> m_unavailablePlaylists;

    QString m_uri;

    bool m_skipUpdateTracks;

    QString m_trackFilter;

    bool m_updateEventPosted;

    friend class QSpotifyPlaylistContainer;
    friend class QSpotifyUser;
    friend class QSpotifyTrack;
};

#endif // QSPOTIFYPLAYLIST_H
