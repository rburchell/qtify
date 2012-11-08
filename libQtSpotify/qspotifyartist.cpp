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


#include "qspotifyartist.h"

#include <QtCore/QDebug>
#include <libspotify/api.h>

QSpotifyArtist::QSpotifyArtist(sp_artist *artist)
    : QSpotifyObject(true)
{
    connect(this, SIGNAL(dataChanged()), this, SIGNAL(artistDataChanged()));

    sp_artist_add_ref(artist);
    m_sp_artist = artist;
    metadataUpdated();
}

QSpotifyArtist::~QSpotifyArtist()
{
    sp_artist_release(m_sp_artist);
}

bool QSpotifyArtist::isLoaded()
{
    return sp_artist_is_loaded(m_sp_artist);
}

bool QSpotifyArtist::updateData()
{
    bool updated = false;

    QString name = QString::fromUtf8(sp_artist_name(m_sp_artist));
    if (m_name != name) {
        m_name = name;
        updated = true;
    }

    if (m_pictureId.isEmpty()) {
        sp_link *link = sp_link_create_from_artist_portrait(m_sp_artist, SP_IMAGE_SIZE_NORMAL);
        if (link) {
            char buffer[200];
            int uriSize = sp_link_as_string(link, &buffer[0], 200);
            m_pictureId = QString::fromUtf8(&buffer[0], uriSize);
            sp_link_release(link);
            updated = true;
        }
    }

    return updated;
}
