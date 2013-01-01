/*
  Q Light Controller
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

    for (int i = 0; i < OSC_INPUTS; i++)
    {
        QString key = QString("OSCplugin/Input%1/server_port").arg(i);
        QVariant value = settings.value(key);
        if (value.isValid() == true)
            m_ports[i] = value.toString();
        else
            m_ports[i] = defaults.at(i);
        m_serv_threads[i] = NULL;
        /** Initialize the structure to be passed to the OSC callback */
        m_callbackInfo[i].input = i;
        m_callbackInfo[i].plugin = this;
    }
}

QString OSCPlugin::name()
{
    return QString("OSC");
}

int OSCPlugin::capabilities() const
{
    return QLCIOPlugin::Input;
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

/*************************************************************************
 * Inputs
 *************************************************************************/
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
            if (argc > 1)
            {
                QString newPath = QString("%1_%2").arg(path).arg(i);
                plugin->sendValueChanged(input, newPath, value);
            }
        }
        //lo_arg_pp((lo_type)types[i], argv[i]);
    }
    if (argc == 1)
        plugin->sendValueChanged(input, path, value);

    return 1;
}

void errorCallback(int num, const char *msg, const char *path)
{
    qDebug() << QString("liblo server error %1 in path %2: %3").arg(num).arg(path).arg(msg);
}

void OSCPlugin::openInput(quint32 input)
{
    if (input >= OSC_INPUTS)
        return;

    qDebug() << Q_FUNC_INFO << "Input " << input << " port: " << m_ports[input];

	/** Cleanup a previous server instance if started */
    if (m_serv_threads[input] != NULL)
    {
        lo_server_thread_stop(m_serv_threads[input]);
        lo_server_thread_free(m_serv_threads[input]);
        m_serv_threads[input] = NULL;
    }
    /* start a new server on the defined port */
    QByteArray p_bytes  = m_ports[input].toLatin1();
    const char *c_port = p_bytes.data();

    m_serv_threads[input] = lo_server_thread_new(c_port, errorCallback);

    if (m_serv_threads[input] != NULL)
	{
		/* add method that will match any path and args */
        lo_server_thread_add_method(m_serv_threads[input], NULL, NULL, messageCallback, &m_callbackInfo[input]);

        lo_server_thread_start(m_serv_threads[input]);
	}
}

void OSCPlugin::closeInput(quint32 input)
{
    if (input >= OSC_INPUTS)
        return;

    if (m_serv_threads[input] != NULL)
    {
        lo_server_thread_stop(m_serv_threads[input]);
        lo_server_thread_free(m_serv_threads[input]);
        m_serv_threads[input] = NULL;
    }
}

QStringList OSCPlugin::inputs()
{
    QStringList list;
    for (int i = 0; i < OSC_INPUTS; i++)
        list << QString("1: OSC Network %1").arg(i + 1);
    return list;
}

QString OSCPlugin::inputInfo(quint32 input)
{
    Q_UNUSED(input);
    return QString();
}

quint16 OSCPlugin::getHash(QString path)
{
    quint16 hash;
    if (m_hash.contains(path))
        hash = m_hash[path];
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
        m_hash[path] = hash;
    }

    return hash;
}

void OSCPlugin::sendValueChanged(quint32 input, QString path, uchar value)
{
    emit valueChanged(input, getHash(path), value, path);
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
    if (num >= OSC_INPUTS)
        return QString("7770");

    return m_ports[num];
}

void OSCPlugin::setPort(int num, QString port)
{
	qDebug() << Q_FUNC_INFO;
    if (num >= OSC_INPUTS)
        return;

    QSettings settings;
    QString key = QString("OSCplugin/Input%1/server_port").arg(num);

    settings.setValue(key, QVariant(port));

    if (port != m_ports[num])
	{
        m_ports[num] = port;
        openInput(num);
	}
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(oscplugin, OSCPlugin)
