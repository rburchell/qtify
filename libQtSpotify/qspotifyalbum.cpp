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


#include "qspotifyalbum.h"
#include "qspotifyartist.h"
#include "qspotifysession.h"

#include <libspotify/api.h>
#include <QtCore/QDebug>

QSpotifyAlbum::QSpotifyAlbum(sp_album *album)
    : QSpotifyObject(true)
    , m_isAvailable(false)
    , m_year(0)
    , m_type(Unknown)
{
    connect(this, SIGNAL(dataChanged()), this, SIGNAL(albumDataChanged()));
    sp_album_add_ref(album);
    m_sp_album = album;
    metadataUpdated();
}

QSpotifyAlbum::~QSpotifyAlbum()
{
    sp_album_release(m_sp_album);
}

bool QSpotifyAlbum::isLoaded()
{
    return sp_album_is_loaded(m_sp_album);
}

bool QSpotifyAlbum::updateData()
{
    bool updated = false;

    bool isAvailable = sp_album_is_available(m_sp_album);
    sp_artist *a = sp_album_artist((m_sp_album));
    QString artist;
    if (a)
        artist = QString::fromUtf8(sp_artist_name(a));
    QString name = QString::fromUtf8(sp_album_name(m_sp_album));
    int year = sp_album_year(m_sp_album);
    Type type = Type(sp_album_type(m_sp_album));

    // Get cover
    const byte *album_cover_id = sp_album_cover(m_sp_album, SP_IMAGE_SIZE_NORMAL);
    if (album_cover_id != 0 && m_coverId.isEmpty()) {
        sp_link *link = sp_link_create_from_album_cover(m_sp_album, SP_IMAGE_SIZE_NORMAL);
        if (link) {
            char buffer[200];
            int uriSize = sp_link_as_string(link, &buffer[0], 200);
            m_coverId = QString::fromUtf8(&buffer[0], uriSize);
            sp_link_release(link);
            updated = true;
        }
    }

    if (isAvailable != m_isAvailable) {
        m_isAvailable = isAvailable;
        updated = true;
    }
    if (artist != m_artist) {
        m_artist = artist;
        updated = true;
    }
    if (name != m_name) {
        m_name = name;
        updated = true;
    }
    if (year != m_year) {
        m_year = year;
        updated = true;
    }
    if (type != m_type) {
        m_type = type;
        updated = true;
    }

    return updated;
}
