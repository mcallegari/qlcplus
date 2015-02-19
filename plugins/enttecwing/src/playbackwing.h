/*
  Q Light Controller
  playbackwing.h

  Copyright (c) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef PLAYBACKWING_H
#define PLAYBACKWING_H

#include <QHostAddress>
#include <QByteArray>
#include <QObject>
#include <QMap>

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

protected:
    /**
     * Since some of the channels in a playback wing seem to be in a weird
     * order, this map is used to convert this weird order to a sequential
     * order.
     */
    QMap <int,int> m_channelMap;
};

#endif
