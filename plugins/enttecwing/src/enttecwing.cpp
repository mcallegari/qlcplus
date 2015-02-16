/*
  Q Light Controller
  enttecwing.cpp

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

#include <QStringList>
#include <QUdpSocket>
#include <QDebug>

#include "playbackwing.h"
#include "shortcutwing.h"
#include "programwing.h"
#include "enttecwing.h"
#include "wing.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void EnttecWing::init()
{
    /* Create a new UDP socket and start listening to packets coming to
       any local address. */
    m_socket = new QUdpSocket(this);
    reBindSocket();
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadSocket()));
}

EnttecWing::~EnttecWing()
{
    while (m_devices.isEmpty() == false)
        delete m_devices.takeFirst();
}

QString EnttecWing::name()
{
    return QString("ENTTEC Wing");
}

int EnttecWing::capabilities() const
{
    return QLCIOPlugin::Input;
}

bool EnttecWing::reBindSocket()
{
    if (m_socket->state() == QAbstractSocket::BoundState)
        m_socket->close();

    if (m_socket->bind(QHostAddress::Any, Wing::UDPPort) == false)
    {
        m_errorString = m_socket->errorString();
        qWarning() << Q_FUNC_INFO << m_errorString;
        return false;
    }
    else
    {
        m_errorString.clear();
    }
    return true;
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

bool EnttecWing::openInput(quint32 input)
{
    Q_UNUSED(input);
    return reBindSocket();
}

void EnttecWing::closeInput(quint32 input)
{
    Q_UNUSED(input);
}

QStringList EnttecWing::inputs()
{
    QStringList list;
    QListIterator <Wing*> it(m_devices);
    while (it.hasNext() == true)
        list << it.next()->name();
    return list;
}

QString EnttecWing::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides input support for Enttec Playback "
              "and Enttec Shortcut Wings.");
    str += QString("</P>");

    return str;
}

QString EnttecWing::inputInfo(quint32 input)
{
    QString str;

    if (input == QLCIOPlugin::invalidLine())
    {
        /* Plugin or just an invalid input selected. Display the error. */
        if (m_socket->state() != QAbstractSocket::BoundState)
        {
            str += QString("<P>");
            str += tr("Unable to bind to UDP port %1:").arg(Wing::UDPPort);
            str += QString(" %1.").arg(m_errorString);
            str += QString("</P>");
        }
        else
        {
            str += QString("<P>");
            str += tr("Listening to UDP port %1.").arg(Wing::UDPPort);
            str += QString("</P>");
        }
    }
    else
    {
        /* A specific input line selected. Display its information if
           available. */
        Wing* dev = device(input);
        if (dev != NULL)
            str += dev->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void EnttecWing::sendFeedBack(quint32 input, quint32 channel, uchar value, const QString &)
{
    Wing* wing = device(input);
    if (wing != NULL)
        wing->feedBack(channel, value);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void EnttecWing::configure()
{
    reBindSocket();
    emit configurationChanged();
}

bool EnttecWing::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

Wing* EnttecWing::createWing(QObject* parent, const QHostAddress& address,
                              const QByteArray& data)
{
    Wing* wing = NULL;

    /* Check, that the message is from an ENTTEC Wing */
    if (Wing::isOutputData(data) == false)
        return NULL;

    switch (Wing::resolveType(data))
    {
    case Wing::Playback:
        wing = new PlaybackWing(parent, address, data);
        break;
    case Wing::Shortcut:
        wing = new ShortcutWing(parent, address, data);
        break;
    case Wing::Program:
        wing = new ProgramWing(parent, address, data);
        break;
    default:
        wing = NULL;
        break;
    }

    return wing;
}

Wing* EnttecWing::device(const QHostAddress& address, Wing::Type type)
{
    QListIterator <Wing*> it(m_devices);
    while (it.hasNext() == true)
    {
        Wing* dev = it.next();
        if (dev->address() == address && dev->type() == type)
            return dev;
    }

    return NULL;
}

Wing* EnttecWing::device(quint32 index)
{
    if (index < quint32(m_devices.count()))
        return m_devices.at(index);
    else
        return NULL;
}

static bool wing_device_sort(const Wing* d1, const Wing* d2)
{
    /* Sort devices based on their addresses. Lexical sorting is enough. */
    return (d1->address().toString() < d2->address().toString());
}

void EnttecWing::addDevice(Wing* device)
{
    Q_ASSERT(device != NULL);

    connect(device, SIGNAL(valueChanged(quint32,uchar)),
            this, SLOT(slotValueChanged(quint32,uchar)));

    m_devices.append(device);

    /* To maintain some persistency with the indices of multiple devices
       between sessions they need to be sorted according to some
       (semi-)permanent criteria. Their addresses shouldn't change too
       often, so let's use that. */
    qSort(m_devices.begin(), m_devices.end(), wing_device_sort);

    emit configurationChanged();
}

void EnttecWing::removeDevice(Wing* device)
{
    Q_ASSERT(device != NULL);
    m_devices.removeAll(device);
    delete device;

    emit configurationChanged();
}

void EnttecWing::slotReadSocket()
{
    while (m_socket->hasPendingDatagrams() == true)
    {
        QHostAddress sender;
        QByteArray data;
        Wing* wing;

        /* Read data from socket */
        data.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(data.data(), data.size(), &sender);

        /* Check, whether we already have a device from this address */
        wing = device(sender, Wing::resolveType(data));
        if (wing == NULL)
        {
            /* New address. Create a new device. */
            wing = createWing(this, sender, data);
            if (wing != NULL)
                addDevice(wing);
        }
        else
        {
            // Since creating a wing already does parseData, don't do it again
            wing->parseData(data);
        }
    }
}

void EnttecWing::slotValueChanged(quint32 channel, uchar value)
{
    Wing* wing = qobject_cast<Wing*> (QObject::sender());
    emit valueChanged(UINT_MAX, m_devices.indexOf(wing), channel, value);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(enttecwing, EnttecWing)
#endif
