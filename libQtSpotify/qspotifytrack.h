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


#ifndef QSPOTIFYTRACK_H
#define QSPOTIFYTRACK_H

#include <libspotify/api.h>
#include "qspotifyobject.h"
#include <QtCore/QVector>
#include <QtCore/QDateTime>

class QSpotifyPlaylist;
class QSpotifyTrackList;
class QSpotifyArtist;
class QSpotifyAlbum;

class QSpotifyTrack : public QSpotifyObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY trackDataChanged)
    Q_PROPERTY(QString artists READ artists NOTIFY trackDataChanged)
    Q_PROPERTY(QString album READ album NOTIFY trackDataChanged)
    Q_PROPERTY(QString albumCoverId READ albumCoverId NOTIFY trackDataChanged)
    Q_PROPERTY(int discNumber READ discNumber NOTIFY trackDataChanged)
    Q_PROPERTY(QString duration READ durationString NOTIFY trackDataChanged)
    Q_PROPERTY(int durationMs READ duration NOTIFY trackDataChanged)
    Q_PROPERTY(TrackError error READ error NOTIFY trackDataChanged)
    Q_PROPERTY(int discIndex READ discIndex NOTIFY trackDataChanged)
    Q_PROPERTY(bool isAvailable READ isAvailable NOTIFY isAvailableChanged)
    Q_PROPERTY(bool isStarred READ isStarred WRITE setIsStarred NOTIFY isStarredChanged)
    Q_PROPERTY(int popularity READ popularity NOTIFY trackDataChanged)
    Q_PROPERTY(bool isCurrentPlayingTrack READ isCurrentPlayingTrack NOTIFY isCurrentPlayingTrackChanged)
    Q_PROPERTY(bool seen READ seen WRITE setSeen NOTIFY seenChanged)
    Q_PROPERTY(QString creator READ creator NOTIFY trackDataChanged)
    Q_PROPERTY(QDateTime creationDate READ creationDate NOTIFY trackDataChanged)
    Q_PROPERTY(QSpotifyAlbum *albumObject READ albumObject NOTIFY trackDataChanged)
    Q_PROPERTY(QSpotifyArtist *artistObject READ artistObject NOTIFY trackDataChanged)
    Q_PROPERTY(OfflineStatus offlineStatus READ offlineStatus NOTIFY offlineStatusChanged)
    Q_PROPERTY(QSpotifyPlaylist *playlist READ playlist NOTIFY trackDataChanged)
    Q_ENUMS(TrackError)
    Q_ENUMS(OfflineStatus)
public:
    enum TrackError {
        Ok = SP_ERROR_OK,
        IsLoading = SP_ERROR_IS_LOADING,
        OtherPermanent = SP_ERROR_OTHER_PERMANENT
    };

    enum OfflineStatus {
        No = SP_TRACK_OFFLINE_NO,
        Waiting = SP_TRACK_OFFLINE_WAITING,
        Downloading = SP_TRACK_OFFLINE_DOWNLOADING,
        Yes = SP_TRACK_OFFLINE_DONE,
        Error = SP_TRACK_OFFLINE_ERROR,
        DoneExpired = SP_TRACK_OFFLINE_DONE_EXPIRED,
        LimitExceeded = SP_TRACK_OFFLINE_LIMIT_EXCEEDED,
        DoneResync = SP_TRACK_OFFLINE_DONE_RESYNC
    };

    ~QSpotifyTrack();

    bool isLoaded();

    QString artists() const;
    QString album() const;
    QString albumCoverId() const;
    int numArtists() const { return m_numArtists; }
    int discNumber() const { return m_discNumber; }
    int duration() const { return m_duration; }
    QString durationString() const { return m_durationString; }
    TrackError error() const { return m_error; }
    int discIndex() const { return m_discIndex; }
    bool isAvailable() const;
    bool isStarred() const;
    void setIsStarred(bool v);
    QString name() const { return m_name; }
    int popularity() const { return m_popularity; }
    QSpotifyAlbum *albumObject() const { return m_album; }
    QSpotifyArtist *artistObject() const { return m_artists.at(0); }
    bool seen() const { return m_seen; }
    void setSeen(bool s);
    QString creator() const { return m_creator; }
    QDateTime creationDate() const { return m_creationDate; }
    OfflineStatus offlineStatus() const { return m_offlineStatus; }
    QSpotifyPlaylist *playlist() const { return m_playlist; }

    bool isCurrentPlayingTrack() const { return m_isCurrentPlayingTrack; }

    bool isAvailableOffline() const;

    sp_track *sptrack() const { return m_sp_track; }

    void updateSeen(bool s);

public Q_SLOTS:
    void play();
    void pause();
    void resume();
    void stop();
    void seek(int offset);
    void enqueue();
    void removeFromPlaylist();

Q_SIGNALS:
    void isCurrentPlayingTrackChanged();
    void trackDataChanged();
    void isStarredChanged();
    void seenChanged();
    void isAvailableChanged();
    void offlineStatusChanged();

protected:
    bool updateData();

private Q_SLOTS:
    void onSessionCurrentTrackChanged();
    void onStarredListTracksAdded(QVector<sp_track *>);
    void onStarredListTracksRemoved(QVector<sp_track *>);
    void onSessionOfflineModeChanged();

private:
    QSpotifyTrack(sp_track *track, QSpotifyPlaylist *playlist);
    QSpotifyTrack(sp_track *track, QSpotifyTrackList *tracklist);

    sp_track *m_sp_track;
    QSpotifyPlaylist *m_playlist;
    QSpotifyTrackList *m_trackList;

    QSpotifyAlbum *m_album;
    QList<QSpotifyArtist *> m_artists;
    QString m_albumString;
    QString m_artistsString;
    int m_discNumber;
    int m_duration;
    QString m_durationString;
    TrackError m_error;
    int m_discIndex;
    bool m_isAvailable;
    QString m_name;
    int m_numArtists;
    int m_popularity;
    bool m_seen;
    QString m_creator;
    QDateTime m_creationDate;
    OfflineStatus m_offlineStatus;

    bool m_isCurrentPlayingTrack;

    friend class QSpotifyPlaylist;
    friend class QSpotifySession;
    friend class QSpotifySearch;
    friend class QSpotifyPlayQueue;
    friend class QSpotifyUser;
    friend class QSpotifyAlbumBrowse;
    friend class QSpotifyArtistBrowse;
    friend class QSpotifyToplist;
};

#endif // QSPOTIFYTRACK_H
