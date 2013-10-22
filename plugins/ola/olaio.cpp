/*
  Q Light Controller
  olaio.cpp

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

#include <QApplication>
#include <QSettings>
#include <QString>
#include <QDebug>

#include "qlclogdestination.h"
#include "configureolaio.h"
#include "olaio.h"

#define SETTINGS_EMBEDDED "OlaIO/embedded"
#define UNIVERSE_COUNT 4

/****************************************************************************
 * Initialization
 ****************************************************************************/

OlaIO::~OlaIO()
{
    if (m_thread != NULL)
    {
        m_thread->stop();
        delete m_thread;
    }
    ola::InitLogging(ola::OLA_LOG_WARN, NULL);
}

/**
 * Start the plugin. It's hard to say if we want OLA running if there aren't
 * any output universes active.
 */
void OlaIO::init()
{
    m_embedServer = false;
    m_thread = NULL;
    ola::InitLogging(ola::OLA_LOG_WARN, new ola::QLCLogDestination());
    // TODO: load this from a savefile at some point
    for (unsigned int i = 1; i <= UNIVERSE_COUNT; ++i)
        m_outputs.append(i);

    bool es = false;
    QSettings settings;
    QVariant var = settings.value(SETTINGS_EMBEDDED);
    if (var.isValid() == true)
        es = settings.value(SETTINGS_EMBEDDED).toBool();

    // Make sure the thread is started the first time round
    m_embedServer = !es;
    // This should load from the settings when it is made
    setServerEmbedded(es);
}

QString OlaIO::name()
{
    return QString("OLA");
}

int OlaIO::capabilities() const
{
    return QLCIOPlugin::Output;
}

bool OlaIO::isServerEmbedded() const
{
    return m_embedServer;
}

void OlaIO::setServerEmbedded(bool embedServer)
{
    if (embedServer != m_embedServer)
    {
        if (m_thread != NULL)
        {
            m_thread->stop();
            delete m_thread;
        }

        m_embedServer = embedServer;
        if (m_embedServer)
        {
            qWarning() << "olaout: running as embedded";
            m_thread = new OlaEmbeddedServer();
        }
        else
        {
            m_thread = new OlaStandaloneClient();
        }

        if (!m_thread->start())
            qWarning() << "olaout: start thread failed";

        QSettings settings;
        settings.setValue(SETTINGS_EMBEDDED, m_embedServer);
    }
}

/****************************************************************************
 * Outputs
 ****************************************************************************/

void OlaIO::openOutput(quint32 output)
{
    if (output >= UNIVERSE_COUNT)
        qWarning() << Q_FUNC_INFO << "output" << output << "is out of range";
}

void OlaIO::closeOutput(quint32 output)
{
    if (output >= UNIVERSE_COUNT)
        qWarning() << Q_FUNC_INFO << "output" << output << "is out of range";
}

QStringList OlaIO::outputs()
{
    QStringList list;
    for (int i = 0; i < m_outputs.size(); ++i)
        list << QString("%1: OLA Universe %1").arg(i + 1);
    return list;
}

QString OlaIO::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output support for the Open Lighting Architecture (OLA).");
    str += QString("</P>");

    return str;
}

QString OlaIO::outputInfo(quint32 output)
{
    QString str;

    if (output != QLCIOPlugin::invalidLine())
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
        str += QString("<P>");
        str += tr("This is the output for OLA universe %1").arg(output + 1);
        str += QString("</P>");
    }

    str += QString("</BODY>");
    str += QString("</HTML>");
    return str;
}

void OlaIO::writeUniverse(quint32 output, const QByteArray& universe)
{
    if (output > UNIVERSE_COUNT || !m_thread)
        return;
    else
        m_thread->write_dmx(m_outputs[output], universe);
}

QList <uint> OlaIO::outputMapping() const
{
    return m_outputs;
}

void OlaIO::setOutputUniverse(quint32 output, unsigned int universe)
{
    if (output > UNIVERSE_COUNT)
        return;
    m_outputs[output] = universe;
}

/****************************************************************************
 * Configuration
 ****************************************************************************/

void OlaIO::configure()
{
    ConfigureOlaIO conf(this, NULL);
    conf.exec();
    emit configurationChanged();
}

bool OlaIO::canConfigure()
{
    return true;
}

/****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(olaio, OlaIO)
#endif
