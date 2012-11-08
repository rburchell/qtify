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


#ifndef QSPOTIFYALBUMBROWSE_H
#define QSPOTIFYALBUMBROWSE_H

#include <QtCore/QObject>
#include <QStringList>

struct sp_albumbrowse;
class QSpotifyAlbum;
class QSpotifyTrackList;

class QSpotifyAlbumBrowse : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSpotifyAlbum *album READ album WRITE setAlbum NOTIFY albumChanged)
    Q_PROPERTY(QList<QObject *> tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(int totalDuration READ totalDuration NOTIFY tracksChanged)
    Q_PROPERTY(bool isStarred READ isStarred WRITE setStarred NOTIFY isStarredChanged)
    Q_PROPERTY(bool hasMultipleArtists READ hasMultipleArtists NOTIFY albumChanged)
    Q_PROPERTY(QStringList review READ review NOTIFY tracksChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    QSpotifyAlbumBrowse(QObject *parent = 0);
    ~QSpotifyAlbumBrowse();

    QSpotifyAlbum *album() const { return m_album; }
    void setAlbum(QSpotifyAlbum *album);

    QList<QObject *> tracks() const;
    int totalDuration() const;
    bool hasMultipleArtists() const { return m_hasMultipleArtists; }

    QStringList review() const { return m_review; }

    bool busy() const { return m_busy; }

    bool event(QEvent *);

    Q_INVOKABLE void play();
    Q_INVOKABLE void enqueue();

    bool isStarred() const;
    void setStarred(bool s);

Q_SIGNALS:
    void albumChanged();
    void tracksChanged();
    void isStarredChanged();
    void busyChanged();

private:
    void clearData();
    void processData();

    sp_albumbrowse *m_sp_albumbrowse;

    QSpotifyAlbum *m_album;
    QSpotifyTrackList *m_albumTracks;

    QStringList m_review;

    bool m_hasMultipleArtists;

    bool m_busy;

    friend class QSpotifyPlaylist;
    friend class QSpotifyUser;
};

#endif // QSPOTIFYALBUMBROWSE_H
