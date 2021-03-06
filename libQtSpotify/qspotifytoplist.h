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


#ifndef QSPOTIFYTOPLIST_H
#define QSPOTIFYTOPLIST_H

#include <QObject>
#include <QDateTime>

struct sp_toplistbrowse;
class QSpotifyTrackList;

class QSpotifyToplist : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> tracks READ tracks NOTIFY resultsChanged)
    Q_PROPERTY(QList<QObject *> albums READ albums NOTIFY resultsChanged)
    Q_PROPERTY(QList<QObject *> artists READ artists NOTIFY resultsChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:

    QSpotifyToplist(QObject *parent = 0);
    ~QSpotifyToplist();

    QList<QObject *> tracks() const;
    QList<QObject *> albums() const { return m_albumResults; }
    QList<QObject *> artists() const { return m_artistResults; }

    bool busy() const { return m_busy; }

    Q_INVOKABLE void updateResults();

    bool event(QEvent *);

Q_SIGNALS:
    void resultsChanged();
    void busyChanged();

private:
    void clear();
    void populateResults(sp_toplistbrowse *tl);

    sp_toplistbrowse *m_sp_browsetracks;
    sp_toplistbrowse *m_sp_browseartists;
    sp_toplistbrowse *m_sp_browsealbums;

    bool m_busy;

    QSpotifyTrackList *m_trackResults;
    QList<QObject *> m_albumResults;
    QList<QObject *> m_artistResults;

    QDateTime m_lastUpdate;

};

#endif // QSPOTIFYTOPLIST_H
