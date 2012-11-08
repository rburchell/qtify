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


#ifndef QSPOTIFY_QMLPLUGIN_H
#define QSPOTIFY_QMLPLUGIN_H

#include <QtSpotify>
#include <QtQml/qqml.h>

void registerQmlTypes()
{
    QLatin1String uri("QtSpotify");

    qmlRegisterUncreatableType<QSpotifySession>(uri.latin1(), 1, 0, "SpotifySession", QLatin1String("Use the Context property instead."));
    qmlRegisterUncreatableType<QSpotifyUser>(uri.latin1(), 1, 0, "SpotifyUser", QLatin1String("Retrieve it from the SpotifySession"));
    qmlRegisterUncreatableType<QSpotifyPlaylist>(uri.latin1(), 1, 0, "SpotifyPlaylist", QLatin1String("Retrieve it from the SpotifySession"));
    qmlRegisterUncreatableType<QSpotifyTrack>(uri.latin1(), 1, 0, "SpotifyTrack", QLatin1String("Retrieve it from the SpotifySession"));
    qmlRegisterUncreatableType<QSpotifyAlbum>(uri.latin1(), 1, 0, "SpotifyAlbum", QLatin1String("Retrieve it from the SpotifySession"));
    qmlRegisterUncreatableType<QSpotifyArtist>(uri.latin1(), 1, 0, "SpotifyArtist", QLatin1String("Retrieve it from the SpotifySession"));
    qmlRegisterUncreatableType<QSpotifyPlayQueue>(uri.latin1(), 1, 0, "SpotifyPlayQueue", QLatin1String("Retrieve it from the SpotifySession"));

    qmlRegisterType<QSpotifySearch>(uri.latin1(), 1, 0, "SpotifySearch");
    qmlRegisterType<QSpotifyAlbumBrowse>(uri.latin1(), 1, 0, "SpotifyAlbumBrowse");
    qmlRegisterType<QSpotifyArtistBrowse>(uri.latin1(), 1, 0, "SpotifyArtistBrowse");
    qmlRegisterType<QSpotifyToplist>(uri.latin1(), 1, 0, "SpotifyToplist");
}

#endif // QSPOTIFY_QMLPLUGIN_H
