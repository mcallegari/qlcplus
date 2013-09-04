/*
  Q Light Controller Plus
  oscplugin.cpp

  Copyright (c) Massimo Callegari

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

#include "oscplugin.h"
#include "configureosc.h"

#include <QSettings>
#include <QDebug>

OSCPlugin::~OSCPlugin()
{
}

void OSCPlugin::init()
{
    QSettings settings;
    QStringList defaults;
    defaults << "7770" << "8000" << "9000" << "9990";

    for (int i = 0; i < QLCIOPLUGINS_UNIVERSES; i++)
    {
        QString key = QString("OSCplugin/Input%1/server_port").arg(i);
        QVariant value = settings.value(key);
        if (value.isValid() == true)
            m_nodes[i].m_port = value.toString();
        else
            m_nodes[i].m_port = defaults.at(i);

        QString outAddrkey = QString("OSCplugin/Output%1/output_addr").arg(i);
        QVariant outValue = settings.value(outAddrkey);
        if (outValue.isValid() == true)
        {
            QString strAddr = outValue.toString();
            if (strAddr.contains(':'))
            {
                QStringList strList = strAddr.split(':');
                m_nodes[i].m_outAddr = lo_address_new(strList.at(0).toStdString().c_str(), strList.at(1).toStdString().c_str());
            }
            else
                m_nodes[i].m_outAddr = lo_address_new(strAddr.toStdString().c_str(), defaults.at(i).toStdString().c_str());

            m_nodes[i].m_outAddrStr = strAddr;
        }
        else
            m_nodes[i].m_outAddrStr = QString();

        // Initialize DMX values to 0
        for (int d = 0; d < 512; d++)
            m_nodes[i].m_dmxValues.append((char)0x00);

        m_nodes[i].m_serv_thread = NULL;

        /** Initialize the structure to be passed to the OSC callback */
        m_nodes[i].m_callbackInfo.input = i;
        m_nodes[i].m_callbackInfo.plugin = this;
    }
}

QString OSCPlugin::name()
{
    return QString("OSC");
}

int OSCPlugin::capabilities() const
{
    return QLCIOPlugin::Input | QLCIOPlugin::Output | QLCIOPlugin::Feedback;
}

QString OSCPlugin::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides input for devices supporting the OSC transmission protocol.");
    str += QString("</P>");

    return str;
}


/*********************************************************************
 * LibLO callbacks
 *********************************************************************/

void errorCallback(int num, const char *msg, const char *path)
{
    qDebug() << QString("liblo server error %1 in path %2: %3").arg(num).arg(path).arg(msg);
}

int messageCallback(const char *path, const char *types, lo_arg **argv,
                     int argc, void *data, void *user_data)
{
    Q_UNUSED(data);

    OSC_cbk_info *info = (OSC_cbk_info *)user_data;
    int input = info->input;
    OSCPlugin *plugin = info->plugin;
    int i;
    uchar value = 0;

    //qDebug() << "path: " << path;
    for (i = 0; i < argc; i++)
    {
        if (types[i] == 'f')
        {
            float f_val = argv[i]->f;
            //qDebug() << "arg: " << i << ", type: '" << types[i] << "'', val: " << f_val;
            value = 255 * f_val;
        }
        else if (types[i] == 'i')
        {
            int32_t i_val = argv[i]->i;
            if (i_val < 256)
                value = (uchar)i_val;
            else
                value = i_val / 0xFFFFFF;
        }
        if (argc > 1)
        {
            QString newPath = QString("%1_%2").arg(path).arg(i);
            plugin->sendValueChanged(input, newPath, value);
        }
        //lo_arg_pp((lo_type)types[i], argv[i]);
    }
    if (argc == 1)
        plugin->sendValueChanged(input, path, value);

    return 1;
}

/*********************************************************************
 * Outputs
 *********************************************************************/
void OSCPlugin::openOutput(quint32 output)
{
    if (output >= QLCIOPLUGINS_UNIVERSES)
        return;

    qDebug() << Q_FUNC_INFO << "Output on " << m_nodes[output].m_outAddrStr;
}

void OSCPlugin::closeOutput(quint32 output)
{
    if (output >= QLCIOPLUGINS_UNIVERSES)
        return;

    if (m_nodes[output].m_serv_thread != NULL)
    {
        lo_server_thread_stop(m_nodes[output].m_serv_thread);
        lo_server_thread_free(m_nodes[output].m_serv_thread);
        m_nodes[output].m_serv_thread = NULL;
    }
}

QStringList OSCPlugin::outputs()
{
    QStringList list;
    for (int i = 0; i < QLCIOPLUGINS_UNIVERSES; i++)
        list << QString("%1: %2 %1").arg(i + 1).arg(tr("OSC Network"));
    return list;
}

QString OSCPlugin::outputInfo(quint32 output)
{
    if (output >= QLCIOPLUGINS_UNIVERSES)
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Output")).arg(outputs()[output]);
    str += QString("<P>");
    if (m_nodes[output].m_outAddrStr.isEmpty() == true)
        str += tr("Status: Not ready");
    else
    {
        str += tr("Address: ");
        str += m_nodes[output].m_outAddrStr;
        str += "<BR>";
        str += tr("Status: Ready");
    }
    str += QString("</P>");
    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void OSCPlugin::writeUniverse(quint32 output, const QByteArray &universe)
{
    if (output >= QLCIOPLUGINS_UNIVERSES)
        return;

    for (int i = 0; i < universe.length(); i++)
    {
        if (universe[i] != m_nodes[output].m_dmxValues[i])
        {
            //send data here
            m_nodes[output].m_dmxValues[i] = universe[i];
            QString str = QString("/%1/dmx/%2").arg(output).arg(i);
            //qDebug() << "[OSC writeUniverse] Send channel : " << str << ", value: " << universe[i];
            lo_send(m_nodes[output].m_outAddr, str.toStdString().c_str(), "f", (float)((uchar)universe[i]) / 255);
        }
    }
}

/*************************************************************************
 * Inputs
 *************************************************************************/

void OSCPlugin::openInput(quint32 input)
{
    if (input >= QLCIOPLUGINS_UNIVERSES)
        return;

    qDebug() << Q_FUNC_INFO << "Input " << input << " port: " << m_nodes[input].m_port;

	/** Cleanup a previous server instance if started */
    if (m_nodes[input].m_serv_thread != NULL)
    {
        lo_server_thread_stop(m_nodes[input].m_serv_thread);
        lo_server_thread_free(m_nodes[input].m_serv_thread);
        m_nodes[input].m_serv_thread = NULL;
    }
    /* start a new server on the defined port */
    QByteArray p_bytes  = m_nodes[input].m_port.toLatin1();
    const char *c_port = p_bytes.data();

    m_nodes[input].m_serv_thread = lo_server_thread_new(c_port, errorCallback);

    if (m_nodes[input].m_serv_thread != NULL)
	{
		/* add method that will match any path and args */
        lo_server_thread_add_method(m_nodes[input].m_serv_thread, NULL, NULL, messageCallback, &m_nodes[input].m_callbackInfo);

        lo_server_thread_start(m_nodes[input].m_serv_thread);
	}
}

void OSCPlugin::closeInput(quint32 input)
{
    if (input >= QLCIOPLUGINS_UNIVERSES)
        return;

    if (m_nodes[input].m_serv_thread != NULL)
    {
        lo_server_thread_stop(m_nodes[input].m_serv_thread);
        lo_server_thread_free(m_nodes[input].m_serv_thread);
        m_nodes[input].m_serv_thread = NULL;
    }
}

QStringList OSCPlugin::inputs()
{
    QStringList list;
    for (int i = 0; i < QLCIOPLUGINS_UNIVERSES; i++)
        list << QString("%1: %2 %1").arg(i + 1).arg(tr("OSC Network"));
    return list;
}

QString OSCPlugin::inputInfo(quint32 input)
{
    if (input >= QLCIOPLUGINS_UNIVERSES)
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Input")).arg(inputs()[input]);
    str += QString("<P>");
    if (m_nodes[input].m_serv_thread == NULL)
        str += tr("Status: Not ready");
    else
    {
        str += tr("Status: Ready");
    }
    str += QString("</P>");
    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void OSCPlugin::sendFeedBack(quint32 input, quint32 channel, uchar value, const QString &key)
{
    if (input >= QLCIOPLUGINS_UNIVERSES || m_nodes[input].m_outAddrStr.isEmpty())
        return;

    qDebug() << "[OSC sendFeedBack] Key:" << key << "value:" << value;
    QString path = key;
    // on invalid key try to retrieve the OSC path from the hash table.
    // This works only if the OSC widget has been previously moved by the user
    if (key.isEmpty())
        path = m_nodes[input].m_hash.key(channel);

    if (path.contains("_0"))
    {
        m_nodes[input].m_multiDataFirst = value;
        return;
    }
    else if (path.contains("_1"))
    {
        path.chop(2);
        lo_send(m_nodes[input].m_outAddr, path.toStdString().c_str(), "ff", (float)m_nodes[input].m_multiDataFirst / 255, (float)value / 255);
        return;
    }
    //lo_send_from(destAddr, m_nodes[input].m_serv_thread, LO_TT_IMMEDIATE, "/1/fader1", "f", 0.5f /*(float)value / 255*/);
    lo_send(m_nodes[input].m_outAddr, path.toStdString().c_str(), "f", (float)value / 255);
}

quint16 OSCPlugin::getHash(quint32 line, QString path)
{
    quint16 hash;
    if (m_nodes[line].m_hash.contains(path))
        hash = m_nodes[line].m_hash[path];
    else
    {
        /*
        #include <QByteArray>
        #include <zlib.h>

        QByteArray ba("This is a Test 123 :)!");
        ulong crc = crc32(0, NULL, 0);
        crc = crc32(crc, (const Bytef *)ba.data(), ba.size());
        printf("CRC-32 = 0x%x\n", crc);

        */

        /** No existing hash found. Add a new key to the table */
        hash = qChecksum(path.toUtf8().data(), path.length());
        m_nodes[line].m_hash[path] = hash;
    }

    return hash;
}

void OSCPlugin::sendValueChanged(quint32 input, QString path, uchar value)
{
    emit valueChanged(input, getHash(input, path), value, path);
}

/*********************************************************************
 * Configuration
 *********************************************************************/
void OSCPlugin::configure()
{
    ConfigureOSC conf(this);
    conf.exec();
}

bool OSCPlugin::canConfigure()
{
    return true;
}

QString OSCPlugin::getPort(int num)
{
    if (num >= QLCIOPLUGINS_UNIVERSES)
        return QString("7770");

    return m_nodes[num].m_port;
}

void OSCPlugin::setPort(int num, QString port)
{
    qDebug() << Q_FUNC_INFO;

    if (num >= QLCIOPLUGINS_UNIVERSES)
        return;

    QSettings settings;
    QString key = QString("OSCplugin/Input%1/server_port").arg(num);

    settings.setValue(key, QVariant(port));

    if (port != m_nodes[num].m_port)
    {
        m_nodes[num].m_port = port;
        openInput(num);
    }
}

QString OSCPlugin::getOutputAddress(int num)
{
    if (num >= QLCIOPLUGINS_UNIVERSES)
        return QString();

    return m_nodes[num].m_outAddrStr;
}

void OSCPlugin::setOutputAddress(int num, QString addr)
{
    qDebug() << Q_FUNC_INFO;

    if (num >= QLCIOPLUGINS_UNIVERSES)
        return;

    QSettings settings;
    QString key = QString("OSCplugin/Output%1/output_addr").arg(num);
    settings.setValue(key, QVariant(addr));
    if (addr.contains(':'))
    {
        QStringList strList = addr.split(':');
        m_nodes[num].m_outAddr = lo_address_new(strList.at(0).toStdString().c_str(), strList.at(1).toStdString().c_str());
    }
    else
        m_nodes[num].m_outAddr = lo_address_new(addr.toStdString().c_str(), "6666");

    m_nodes[num].m_outAddrStr = addr;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(oscplugin, OSCPlugin)
