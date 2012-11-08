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


#ifndef QSPOTIFYPLAYLISTCONTAINER_H
#define QSPOTIFYPLAYLISTCONTAINER_H

#include "qspotifyobject.h"
#include <QMetaType>

struct sp_playlistcontainer;
struct sp_playlistcontainer_callbacks;
struct sp_playlist;
class QSpotifyPlaylist;

class QSpotifyPlaylistContainer : public QSpotifyObject
{
    Q_OBJECT
public:
    ~QSpotifyPlaylistContainer();

    bool isLoaded();

    QList<QSpotifyPlaylist *> playlists() const { return m_playlists; }
    QList<QObject *> formattedPlaylists() const { return m_formattedAvailablePlaylists + m_formattedUnavailablePlaylists; }
    QList<QObject *> playlistsFlat() const { return m_playlistsFlat; }

    sp_playlistcontainer *spcontainer() { return m_container; }

Q_SIGNALS:
    void playlistContainerDataChanged();
    void playlistsNameChanged();

protected:
    bool updateData();

    bool event(QEvent *);

private Q_SLOTS:
    void updatePlaylists();

private:
    QSpotifyPlaylistContainer(sp_playlistcontainer *container);
    void addPlaylist(sp_playlist *, int pos = -1);

    void postUpdateEvent();

    sp_playlistcontainer *m_container;
    sp_playlistcontainer_callbacks *m_callbacks;

    QList<QSpotifyPlaylist *> m_playlists;
    QList<QObject *> m_formattedAvailablePlaylists;
    QList<QObject *> m_formattedUnavailablePlaylists;
    QList<QObject *> m_playlistsFlat;

    bool m_updateEventPosted;

    friend class QSpotifyUser;
    friend class QSpotifyPlaylist;
};

#endif // QSPOTIFYPLAYLISTCONTAINER_H
