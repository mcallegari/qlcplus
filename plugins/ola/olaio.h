/*
  Q Light Controller
  olaio.h

  Copyright (c) Simon Newton
                Heikki Junnila

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

#ifndef OLAIO_H
#define OLAIO_H

#include <QObject>
#include <QDebug>
#include <QList>
#include <ola/Logging.h>

#include "qlcioplugin.h"
#include "olaoutthread.h"

class ConfigureOlaIO;

class OlaIO : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)

    friend class ConfigureOlaIO;

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** @reimp */
    ~OlaIO();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

private:
    /** Is the plugin currently running as a stand alone daemon. */
    bool isServerEmbedded() const;

    /** Set whether or not to run as a standalone daemon. */
    void setServerEmbedded(bool embedServer);

    /************************************************************************
     * Outputs
     ************************************************************************/
public:
    /** @reimp */
    void openOutput(quint32 output);

    /** @reimp */
    void closeOutput(quint32 output);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 output, const QByteArray& universe);

private:
    /** Return the output: universe mapping */
    QList <uint> outputMapping() const;

    /**
     * Set the OLA universe for an output
     * @param output the id of the output to change
     * @param universe the OLA universe id
     */
    void setOutputUniverse(quint32 output, unsigned int universe);

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    void openInput(quint32 input) { Q_UNUSED(input); }

    /** @reimp */
    void closeInput(quint32 input) { Q_UNUSED(input); }

    /** @reimp */
    QStringList inputs() { return QStringList(); }

    /** @reimp */
    QString inputInfo(quint32 input) { Q_UNUSED(input); return QString(); }

    /** @reimp */
    void sendFeedBack(quint32 input, quint32 channel, uchar value, const QString& key)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); Q_UNUSED(key); }

    /************************************************************************
     * Configuration
     ************************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

private:
    QString m_configDir;
    OlaOutThread *m_thread;
    QList <uint> m_outputs;
    bool m_embedServer;
};

#endif
