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


#ifndef QSPOTIFYARTISTBROWSE_H
#define QSPOTIFYARTISTBROWSE_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include "qspotifysearch.h"

struct sp_artistbrowse;
class QSpotifyAlbum;
class QSpotifyArtist;
class QSpotifyTrackList;

class QSpotifyArtistBrowse : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSpotifyArtist *artist READ artist WRITE setArtist NOTIFY artistChanged)
    Q_PROPERTY(QList<QObject *> topTracks READ topTracks NOTIFY dataChanged)
    Q_PROPERTY(QList<QObject *> albums READ albums NOTIFY dataChanged)
    Q_PROPERTY(int albumCount READ albumCount NOTIFY dataChanged)
    Q_PROPERTY(int singleCount READ singleCount NOTIFY dataChanged)
    Q_PROPERTY(int compilationCount READ compilationCount NOTIFY dataChanged)
    Q_PROPERTY(int appearsOnCount READ appearsOnCount NOTIFY dataChanged)
    Q_PROPERTY(QString pictureId READ pictureId NOTIFY dataChanged)
    Q_PROPERTY(QStringList biography READ biography NOTIFY dataChanged)
    Q_PROPERTY(QList<QObject *> similarArtists READ similarArtists NOTIFY dataChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    QSpotifyArtistBrowse(QObject *parent = 0);
    ~QSpotifyArtistBrowse();

    QSpotifyArtist *artist() const { return m_artist; }
    void setArtist(QSpotifyArtist *artist);

    QList<QObject *> topTracks() const;
    QList<QObject *> albums() const { return m_albums + m_singles + m_compilations + m_appearsOn; }

    int albumCount() const { return m_albums.count(); }
    int singleCount() const { return m_singles.count(); }
    int compilationCount() const { return m_compilations.count(); }
    int appearsOnCount() const { return m_appearsOn.count(); }

    QString pictureId() const { return m_pictureId; }

    QStringList biography() const { return m_biography; }

    QList<QObject *> similarArtists() const { return m_similarArtists; }

    bool busy() const { return m_busy; }

    bool event(QEvent *);

Q_SIGNALS:
    void artistChanged();
    void dataChanged();
    void busyChanged();

private Q_SLOTS:
    void searchArtists();
    void processTopHits();

private:
    void clearData();
    void processData();

    sp_artistbrowse *m_sp_artistbrowse;

    QSpotifyArtist *m_artist;
    QSpotifyTrackList *m_topTracks;
    QList<QObject *> m_albums;
    QList<QObject *> m_singles;
    QList<QObject *> m_compilations;
    QList<QObject *> m_appearsOn;
    QString m_pictureId;
    QStringList m_biography;
    QList<QObject *> m_similarArtists;
    bool m_busy;
    QSpotifySearch m_hackSearch;
    QSpotifySearch m_topHitsSearch;

    bool m_topHitsReady;
    bool m_dataReady;
};

#endif // QSPOTIFYARTISTBROWSE_H
