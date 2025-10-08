/*
  Q Light Controller Plus
  os2lplugin.cpp

  Copyright (c) Massimo Callegari

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

#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>

#include "os2lplugin.h"
#include "os2lconfiguration.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OS2LPlugin::~OS2LPlugin()
{
    enableTCPServer(false);
}

void OS2LPlugin::init()
{
    m_inputUniverse = UINT_MAX;
    m_hostPort = OS2L_DEFAULT_PORT;
    m_tcpServer = NULL;
}

QString OS2LPlugin::name()
{
    return QString("OS2L");
}

int OS2LPlugin::capabilities() const
{
    return QLCIOPlugin::Input | QLCIOPlugin::Feedback | QLCIOPlugin::Beats;
}

QString OS2LPlugin::pluginInfo()
{
    /** Return a description of the purpose of this plugin
     *  in HTML format */
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides support for one OS2L host.");
    str += QString("</P>");

    return str;
}

/*************************************************************************
 * Inputs
 *************************************************************************/

bool OS2LPlugin::openInput(quint32 input, quint32 universe)
{
    if (input != 0)
        return false;

    m_inputUniverse = universe;

    addToMap(universe, input, Input);

    enableTCPServer(true);

    return true;
}

void OS2LPlugin::closeInput(quint32 input, quint32 universe)
{
    enableTCPServer(false);

    removeFromMap(input, universe, Input);

    m_inputUniverse = UINT_MAX;
}

QStringList OS2LPlugin::inputs()
{
    /**
     * Build a list of input line names. The names must be always in the
     * same order i.e. the first name is the name of input line number 0,
     * the next one is output line number 1, etc..
     */
    QStringList list;
    list << QString("OS2L line");
    return list;
}

QString OS2LPlugin::inputInfo(quint32 input)
{
    /**
     * Provide an informational text regarding the specified input line.
     * This text is in HTML format and it is shown to the user.
     */
    QString str;

    if (input != QLCIOPlugin::invalidLine())
        str += QString("<H3>%1</H3>").arg(inputs()[input]);

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

quint32 OS2LPlugin::universe() const
{
    return m_inputUniverse;
}

bool OS2LPlugin::enableTCPServer(bool enable)
{
    if (enable)
    {
        m_tcpServer = new QTcpServer(this);

        if (m_tcpServer->listen(QHostAddress::Any, m_hostPort) == false)
        {
            qDebug() << "[OS2L] Error listening TCP socket on" << m_hostPort;
            return false;
        }
        connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(slotProcessNewTCPConnection()));
        qDebug() << "[OS2L] listening on TCP port" << m_hostPort;
    }
    else
    {
        if (m_tcpServer == NULL)
            return true;

        disconnect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(slotProcessNewTCPConnection()));
        m_tcpServer->close();
        delete m_tcpServer;
        m_tcpServer = NULL;
        qDebug() << "[OS2L] stop listening on TCP";
    }

    return true;
}

quint16 OS2LPlugin::getHash(QString channel)
{
    quint16 hash;
    if (m_hashMap.contains(channel))
        hash = m_hashMap[channel];
    else
    {
        /** No existing hash found. Add a new key to the table */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        hash = qChecksum(channel.toUtf8().data(), channel.length());
#else
        QByteArrayView bav(channel.toUtf8().data(), channel.length());
        hash = qChecksum(bav);
#endif
        m_hashMap[channel] = hash;
    }

    return hash;
}

void OS2LPlugin::slotProcessNewTCPConnection()
{
    qDebug() << Q_FUNC_INFO;
    QTcpSocket *clientConnection = m_tcpServer->nextPendingConnection();
    if (clientConnection == NULL)
        return;

    QHostAddress senderAddress = clientConnection->peerAddress();
    qDebug() << "[slotProcessNewTCPConnection] Host connected:" << senderAddress.toString();
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(slotProcessTCPPackets()));
    connect(clientConnection, SIGNAL(disconnected()), this, SLOT(slotProcessTCPPackets()));
}

void OS2LPlugin::slotHostDisconnected()
{
    QTcpSocket *socket = (QTcpSocket *)sender();
    QHostAddress senderAddress = socket->peerAddress();
    qDebug() << "Host with address" << senderAddress.toString() << "disconnected!";
}

void OS2LPlugin::slotProcessTCPPackets()
{
    QTcpSocket *socket = (QTcpSocket *)sender();
    if (socket == NULL)
        return;

    QHostAddress senderAddress = QHostAddress(socket->peerAddress().toIPv4Address());

    while (1)
    {
        m_packetLeftOver.append(socket->readAll());

        int endIndex = m_packetLeftOver.indexOf("}");
        if (endIndex == -1)
        {
            if (socket->bytesAvailable())
                continue;
            else
                break;
        }

        QByteArray message = m_packetLeftOver.left(endIndex + 1);
        m_packetLeftOver.remove(0, endIndex + 1);
        QJsonDocument json = QJsonDocument::fromJson(message);

        qDebug() << "[TCP] Received" << message.length() << "bytes from" << senderAddress.toString();
        QJsonObject jsonObj = json.object();
        QJsonValue jEvent = jsonObj.value("evt");
        if (jEvent.isUndefined())
            return;

        QString event = jEvent.toString();

        if (event == "btn")
        {
            QJsonValue jName = jsonObj.value("name");
            QJsonValue jState = jsonObj.value("state");
            qDebug() << "Got button event with name" << jName.toString() << "and state" << jState.toString();
            uchar value = jState.toString() == "off" ? 0 : 255;
            emit valueChanged(m_inputUniverse, 0, getHash(jName.toString()), value, jName.toString());
        }
        else if (event == "cmd")
        {
            QJsonValue jId = jsonObj.value("id");
            QJsonValue jParam = jsonObj.value("param");
            qDebug() << "Got CMD message" << jId.toInt() << "with param" << jParam.toDouble();
            quint32 channel = quint32(jId.toInt());
            QString cmd = QString("cmd%1").arg(channel);
            emit valueChanged(m_inputUniverse, 0, quint32(jId.toInt()), uchar(jParam.toDouble()), cmd);
        }
        else if (event == "beat")
        {
           qDebug() << "Got beat message" << message;
           emit valueChanged(m_inputUniverse, 0, 8341, 255, "beat");
        }
    }
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void OS2LPlugin::configure()
{
    OS2LConfiguration conf(this);
    conf.exec();
}

bool OS2LPlugin::canConfigure()
{
    return true;
}

void OS2LPlugin::setParameter(quint32 universe, quint32 line, Capability type,
                             QString name, QVariant value)
{
    /** This method is provided to QLC+ to set the plugin specific settings.
     *  Those settings are saved in a project workspace and when it is loaded,
     *  this method is called after QLC+ has opened the input/output lines
     *  mapped in the project workspace as well.
     */

    if (name == OS2L_HOST_ADDRESS)
    {

    }
    else if (name == OS2L_HOST_PORT)
    {
        if (value.toInt() != m_hostPort)
        {
            m_hostPort = quint16(value.toUInt());

            /** restart the TCP server and listen on new port */
            enableTCPServer(false);
            enableTCPServer(true);
        }
    }

    /** Remember to call the base QLCIOPlugin method to actually inform
     *  QLC+ to store the parameter in the project workspace XML */
    QLCIOPlugin::setParameter(universe, line, type, name, value);
}
