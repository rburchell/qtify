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


#ifndef QSPOTIFYUSER_H
#define QSPOTIFYUSER_H

#include "qspotifyobject.h"

struct sp_user;
class QSpotifyPlaylistContainer;
class QSpotifyPlaylist;
class QSpotifyTrack;
class QSpotifyAlbumBrowse;

class QSpotifyUser : public QSpotifyObject
{
    Q_OBJECT
    Q_PROPERTY(QString canonicalName READ canonicalName NOTIFY userDataChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY userDataChanged)
    Q_PROPERTY(QList<QObject *> playlists READ playlistsAsQObject NOTIFY playlistsChanged)
    Q_PROPERTY(QList<QObject *> playlistsFlat READ playlistsFlat NOTIFY playlistsChanged)
public:
    ~QSpotifyUser();

    bool isLoaded();

    QString canonicalName() const { return m_canonicalName; }
    QString displayName() const { return m_displayName; }

    QSpotifyPlaylistContainer *playlistContainer() const;
    QSpotifyPlaylist *starredList() const;
    QSpotifyPlaylist *inbox() const;

    QList<QSpotifyPlaylist *> playlists() const;
    QList<QObject *> playlistsAsQObject() const;
    QList<QObject *> playlistsFlat() const;

    Q_INVOKABLE bool createPlaylist(const QString &name);
    Q_INVOKABLE bool createPlaylistInFolder(const QString &name, QSpotifyPlaylist *folder);
    Q_INVOKABLE bool createPlaylistFromTrack(QSpotifyTrack *track);
    Q_INVOKABLE bool createPlaylistFromAlbum(QSpotifyAlbumBrowse *album);
    Q_INVOKABLE void removePlaylist(QSpotifyPlaylist *playlist);
    Q_INVOKABLE bool ownsPlaylist(QSpotifyPlaylist *playlist);
    Q_INVOKABLE bool canModifyPlaylist(QSpotifyPlaylist *playlist);

    void deleteFolderAndContent(QSpotifyPlaylist *playlist);

Q_SIGNALS:
    void userDataChanged();
    void playlistsChanged();
    void playlistsNameChanged();

protected:
    bool updateData();

private:
    QSpotifyUser(sp_user *user);

    sp_user *m_sp_user;

    QString m_canonicalName;
    QString m_displayName;

    mutable QSpotifyPlaylistContainer *m_playlistContainer;
    mutable QSpotifyPlaylist *m_starredList;
    mutable QSpotifyPlaylist *m_inbox;

    friend class QSpotifySession;
    friend class QSpotifyPlaylist;
};

#endif // QSPOTIFYUSER_H
