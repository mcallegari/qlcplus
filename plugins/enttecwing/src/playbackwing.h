/*
  Q Light Controller
  playbackwing.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef PLAYBACKWING_H
#define PLAYBACKWING_H

#include <QHostAddress>
#include <QByteArray>
#include <QObject>

#include "qlcmacros.h"
#include "wing.h"

/****************************************************************************
 * PlaybackWing
 ****************************************************************************/

class QLC_DECLSPEC PlaybackWing : public Wing
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /**
     * Construct a new PlaybackWing object. This object represents an
     * ENTTEC Playback Wing at the given IP address.
     *
     * @param parent The parent object that owns the new wing object.
     * @param address The address of the physical wing board.
     * @param data A UDP datagram packet originating from a wing.
     */
    PlaybackWing(QObject* parent, const QHostAddress& address,
                 const QByteArray& data);

    /**
     * Destructor.
     */
    ~PlaybackWing();

    /** @reimp */
    QString name() const;

    /********************************************************************
     * Input data
     ********************************************************************/
public:
    /** @reimp */
    void parseData(const QByteArray& data);

    /** Check if extra buttons were pressed and act accordingly */
    void applyExtraButtons(const QByteArray& data);

    /** Send current page number back to the wing */
    void sendPageData();

    /** @reimp */
    quint32 pageSize() const;
protected:
    /**
     * Since some of the channels in a playback wing seem to be in a weird
     * order, this map is used to convert this weird order to a sequential
     * order.
     */
    QMap <int,int> m_channelMap;
};

#endif
