/*
  Q Light Controller
  olaio.cpp

  Copyright (c) Simon Newton
                Heikki Junnila

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

#include <QApplication>
#include <QSettings>
#include <QString>
#include <QDebug>

#include "qlclogdestination.h"
#include "configureolaio.h"
#include "olaio.h"

#define SETTINGS_EMBEDDED "OlaIO/embedded"
#define UNIVERSE_COUNT 4

#ifndef QLC_OLA_FIRST_UNIVERSE
#define QLC_OLA_FIRST_UNIVERSE 1
#endif

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
    for (unsigned int i = 0; i < UNIVERSE_COUNT; ++i)
        m_outputs.append(i + QLC_OLA_FIRST_UNIVERSE);

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
            qWarning() << "[OLA] Running with embedded server";
            m_thread = new OlaEmbeddedServer();
        }
        else
        {
            m_thread = new OlaStandaloneClient();
        }

        if (!m_thread->start())
            qWarning() << "[OLA] Start thread failed";

        QSettings settings;
        settings.setValue(SETTINGS_EMBEDDED, m_embedServer);
    }
}

/****************************************************************************
 * Outputs
 ****************************************************************************/

bool OlaIO::openOutput(quint32 output, quint32 universe)
{
    if (output >= UNIVERSE_COUNT)
    {
        qWarning() << "[OLA] output" << output << "is out of range";
        return false;
    }
    addToMap(universe, output, Output);
    return true;
}

void OlaIO::closeOutput(quint32 output, quint32 universe)
{
    if (output >= UNIVERSE_COUNT)
        qWarning() << "[OLA] output" << output << "is out of range";
    else
        removeFromMap(output, universe, Output);
}

QStringList OlaIO::outputs()
{
    QStringList list;
    for (int i = 0; i < m_outputs.size(); ++i)
        list << QString("OLA Universe %1").arg(m_outputs[i]);
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
        str += tr("This is the output for OLA universe %1").arg(m_outputs[output]);
        str += QString("</P>");
    }

    str += QString("</BODY>");
    str += QString("</HTML>");
    return str;
}

void OlaIO::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(dataChanged)

    if (output > UNIVERSE_COUNT || !m_thread)
        return;
    else
        m_thread->write_dmx(m_outputs[output], data);
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

void OlaIO::setParameter(quint32 universe, quint32 line, Capability type,
                         QString name, QVariant value)
{
    QLCIOPlugin::setParameter(universe, line, type, name, value);
}
