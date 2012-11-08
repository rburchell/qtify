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


#ifndef QSPOTIFYALBUM_H
#define QSPOTIFYALBUM_H

#include "qspotifyobject.h"

struct sp_album;
struct sp_artist;
class QSpotifyArtist;

class QSpotifyAlbum : public QSpotifyObject
{
    Q_OBJECT

    Q_PROPERTY(bool isAvailable READ isAvailable NOTIFY albumDataChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY albumDataChanged)
    Q_PROPERTY(QString name READ name NOTIFY albumDataChanged)
    Q_PROPERTY(int year READ year NOTIFY albumDataChanged)
    Q_PROPERTY(Type type READ type NOTIFY albumDataChanged)
    Q_PROPERTY(QString sectionType READ sectionType NOTIFY albumDataChanged)
    Q_PROPERTY(QString coverId READ coverId NOTIFY albumDataChanged)
    Q_ENUMS(Type)
public:
    enum Type {
        Album = 0,
        Single = 1,
        Compilation = 2,
        Unknown = 3
    };

    ~QSpotifyAlbum();

    bool isLoaded();

    bool isAvailable() const { return m_isAvailable; }
    QString artist() const { return m_artist; }
    QString name() const { return m_name; }
    int year() const { return m_year; }
    Type type() const { return m_type; }
    void setSectionType(const QString &t) { m_sectionType = t; }
    QString sectionType() const { return m_sectionType; }
    QString coverId() const { return m_coverId; }

    sp_album *spalbum() const { return m_sp_album; }

Q_SIGNALS:
    void albumDataChanged();

protected:
    bool updateData();

private:
    QSpotifyAlbum(sp_album *album);

    sp_album *m_sp_album;

    bool m_isAvailable;
    QString m_artist;
    QString m_name;
    int m_year;
    Type m_type;
    QString m_sectionType;
    QString m_coverId;

    friend class QSpotifySession;
    friend class QSpotifyTrack;
    friend class QSpotifyArtistBrowse;
    friend class QSpotifySearch;
    friend class QSpotifyToplist;
};

#endif // QSPOTIFYALBUM_H
