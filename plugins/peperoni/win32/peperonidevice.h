/*
  Q Light Controller
  peperonidevice.h

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

#ifndef PEPERONIDEVICE_H
#define PEPERONIDEVICE_H

#include <Windows.h>
#include <QObject>
#include <QMutex>

class PeperoniDevice : public QObject
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    PeperoniDevice(QObject* parent, struct usbdmx_functions* usbdmx,
                   int output);
    virtual ~PeperoniDevice();

protected:
    struct usbdmx_functions* m_usbdmx;

    /********************************************************************
     * Properties
     ********************************************************************/
public:
    QString name() const;
    int output() const;
    QString infoText() const;

protected:
    void extractName();

protected:
    QString m_name;
    int m_output;
    bool m_deviceOK;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    /** Open this device for DMX output */
    void open();

    /** Close this device */
    void close();

    /** Re-extract the device's name and reopen it if necessary */
    void rehash();

    HANDLE handle() const;

protected:
    HANDLE m_handle;

    /********************************************************************
     * Write
     ********************************************************************/
public:
    void outputDMX(const QByteArray& universe);
};

#endif
