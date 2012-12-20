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

#define SETTINGS_OSC_PORT "OSCplugin/server_port"

OSCPlugin::~OSCPlugin()
{
}

void OSCPlugin::init()
{
    QSettings settings;

    QString key = QString(SETTINGS_OSC_PORT);
    QVariant value = settings.value(key);
    if (value.isValid() == true)
        m_port = value.toString();
    else
        m_port = "7770";
    m_serv_thread = NULL;
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
    Q_UNUSED(user_data);

    OSCPlugin *plugin = static_cast<OSCPlugin*>(user_data);
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
        //lo_arg_pp((lo_type)types[i], argv[i]);
    }
    plugin->sendValueChanged(0, path, value);

    return 1;
}

void errorCallback(int num, const char *msg, const char *path)
{
    qDebug() << QString("liblo server error %1 in path %2: %3").arg(num).arg(path).arg(msg);
}

void OSCPlugin::openInput(quint32 input)
{
	qDebug() << Q_FUNC_INFO << "port: " << m_port;
    if (input != 0)
        return;

	/** Cleanup a previous server instance if started */
    if (m_serv_thread != NULL)
    {
        lo_server_thread_stop(m_serv_thread);
        lo_server_thread_free(m_serv_thread);
        m_serv_thread = NULL;
    }
    /* start a new server on the defined port */
	QByteArray p_bytes  = m_port.toLatin1();
    const char *c_port = p_bytes.data();
	
    m_serv_thread = lo_server_thread_new(c_port, errorCallback);

	if (m_serv_thread != NULL)
	{
		/* add method that will match any path and args */
		lo_server_thread_add_method(m_serv_thread, NULL, NULL, messageCallback, this);

		lo_server_thread_start(m_serv_thread);
	}
}

void OSCPlugin::closeInput(quint32 input)
{
    if (input != 0)
        return;

    if (m_serv_thread != NULL)
    {
        lo_server_thread_stop(m_serv_thread);
        lo_server_thread_free(m_serv_thread);
        m_serv_thread = NULL;
    }
}

QStringList OSCPlugin::inputs()
{
    QStringList list;
    list << QString("1: OSC network");
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

QString OSCPlugin::getPort()
{
    return m_port;
}

void OSCPlugin::setPort(QString port)
{
	qDebug() << Q_FUNC_INFO;
    QSettings settings;

    settings.setValue(SETTINGS_OSC_PORT, QVariant(port));

	if (port != m_port)
	{
		m_port = port;
		openInput(0);
	}
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(oscplugin, OSCPlugin)
