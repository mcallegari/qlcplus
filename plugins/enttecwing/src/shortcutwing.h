/*
  Q Light Controller
  eshortcutwing.h

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

#ifndef SHORTCUTWING_H
#define SHORTCUTWING_H

#include <QHostAddress>
#include <QByteArray>
#include <QObject>

#include "qlcmacros.h"
#include "wing.h"

/****************************************************************************
 * ShortcutWing
 ****************************************************************************/

class QLC_DECLSPEC ShortcutWing : public Wing
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /**
     * Construct a new ShortcutWing object. This object represents an
     * ENTTEC Shortcut Wing at the given IP address.
     *
     * @param parent The parent object that owns the new wing object.
     * @param address The address of the physical wing board.
     * @param data A UDP datagram packet originating from a wing.
     */
    ShortcutWing(QObject* parent, const QHostAddress& address,
                  const QByteArray& data);

    /**
     * Destructor.
     */
    ~ShortcutWing();

    /** @reimp */
    QString name() const;

    /************************************************************************
     * Input data
     ************************************************************************/
public:
    /** @reimp */
    void parseData(const QByteArray& data);

    /** Check if page buttons were pressed and increase/decrease page number */
    void applyPageButtons(const QByteArray& data);

    /** Send current page number back to the wing */
    void sendPageData();
};

#endif
