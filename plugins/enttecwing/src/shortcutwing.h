/*
  Q Light Controller
  eshortcutwing.h

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
