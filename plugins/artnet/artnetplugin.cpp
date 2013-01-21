/*
  Q Light Controller
  artnetplugin.cpp

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

#include "artnetplugin.h"

#include <QNetworkInterface>
#include <QSettings>
#include <QDebug>

ArtNetPlugin::~ArtNetPlugin()
{
}

void ArtNetPlugin::init()
{
    QNetworkInterface interface;
    m_IPList = interface.allAddresses();
}

QString ArtNetPlugin::name()
{
    return QString("ArtNet");
}

int ArtNetPlugin::capabilities() const
{
    return QLCIOPlugin::Output;
}

QString ArtNetPlugin::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides input for devices supporting the ArtNet communication protocol.");
    str += QString("</P>");

    return str;
}

/*********************************************************************
 * Callbacks/Handlers
 *********************************************************************/
#if 0
void print_node_config(artnet_node_entry ne)
{
  qDebug() << QString("--------- %1.%2.%3.%4 -------------").arg(ne->ip[0]).arg(ne->ip[1]).arg(ne->ip[2]).arg(ne->ip[3]);
  qDebug() << QString().sprintf("Short Name:   %s", ne->shortname);
  qDebug() << QString().sprintf("Long Name:    %s", ne->longname);
  qDebug() << QString().sprintf("Node Report:  %s", ne->nodereport);
  qDebug() << QString().sprintf("Subnet:       0x%hhx", ne->sub);
  qDebug() << QString().sprintf("Numb Ports:   %d", ne->numbports);
  //qDebug() << QString("Input Addrs:  0x%hhx, 0x%hhx, 0x%hhx, 0x%hhx\n", ne->swin[0], ne->swin[1], ne->swin[2], ne->swin[3] );
  //qDebug() << QString("Output Addrs: 0x%hhx, 0x%hhx, 0x%hhx, 0x%hhx\n", ne->swout[0], ne->swout[1], ne->swout[2], ne->swout[3] );
  qDebug() << QString("----------------------------------");
}

int pollReplyHandler(artnet_node n, void *pp, void *user_data)
{
    Q_UNUSED(pp)

    if (user_data != NULL)
    {
        ArtNet_poll_info *info = (ArtNet_poll_info *)user_data;
        int output = info->output;
        ArtNetPlugin *plugin = info->plugin;

        artnet_node_list nl = artnet_get_nl(n);

        if (plugin->m_nodesFound[output] == artnet_nl_get_length(nl))
        {
            // this is not a new node, just a previously discovered one sending
            // another reply
            return 0;
        }
        else if(plugin->m_nodesFound[output] == 0)
        {
            // first node found
            plugin->m_nodesFound[output]++;
            print_node_config( artnet_nl_first(nl));
        }
        else
        {
            // new node
            plugin->m_nodesFound[output]++;
            print_node_config( artnet_nl_next(nl));
        }
  }
  return 0;
}
#endif

/*********************************************************************
 * Outputs
 *********************************************************************/
QStringList ArtNetPlugin::outputs()
{
    int idx = 0;
    QStringList list;
    for (int i = 0; i < m_IPList.length(); i++)
    {
        QHostAddress addr = m_IPList.at(i);
        if (addr.protocol() != QAbstractSocket::IPv6Protocol && addr != QHostAddress::LocalHost)
            list << QString(tr("%1: ArtNet network (%2)")).arg(idx++).arg(addr.toString());
    }
    return list;
}

QString ArtNetPlugin::outputInfo(quint32 output)
{
    Q_UNUSED(output);
    return QString();
}

void ArtNetPlugin::openOutput(quint32 output)
{
    if (output >= (quint32)m_IPList.length())
        return;

    int verbose = 0;
    char *ip_addr = NULL;

    if ((m_nodes[output] = artnet_new(ip_addr, verbose)) == NULL) {
      qDebug() << "artnet_new failed " << artnet_strerror();
      return;
    }

    artnet_set_short_name(m_nodes[output], "QLC+");
    artnet_set_long_name(m_nodes[output], "QLC+ ArtNet Controller");
    artnet_set_node_type(m_nodes[output], ARTNET_RAW);

    //artnet_set_handler(m_nodes[output], ARTNET_REPLY_HANDLER, pollReplyHandler, &m_pollCbkInfo[output]);
    artnet_set_port_type(m_nodes[output], 0, ARTNET_ENABLE_INPUT, ARTNET_PORT_DMX);
    artnet_set_port_addr(m_nodes[output], 0, ARTNET_INPUT_PORT, 0 /* port_addr*/);

    if( artnet_start(m_nodes[output]) != ARTNET_EOK)
    {
      qDebug() << "Failed to start ArtNet: " << artnet_strerror();
      return;
    }
/*
    int sd = artnet_get_sd(m_nodes[output]);

    if (artnet_send_poll(m_nodes[output], NULL, ARTNET_TTM_DEFAULT) != ARTNET_EOK)
    {
      qDebug() << "Poll send failed !";
      return;
    }
*/
}

void ArtNetPlugin::closeOutput(quint32 output)
{
    if (output >= (quint32)m_IPList.length())
        return;
	artnet_stop(m_nodes[output]);
	artnet_destroy(m_nodes[output]);
	m_nodes[output] = NULL;
}

void ArtNetPlugin::writeUniverse(quint32 output, const QByteArray& universe)
{
    artnet_send_dmx(m_nodes[output], 0, universe.length(), (uint8_t *)universe.data());
}

/*************************************************************************
  * Inputs
  *************************************************************************/  
void ArtNetPlugin::openInput(quint32 input)
{
    Q_UNUSED(input);
}

void ArtNetPlugin::closeInput(quint32 input)
{
    Q_UNUSED(input);
}

QStringList ArtNetPlugin::inputs()
{
    return QStringList();
}

QString ArtNetPlugin::inputInfo(quint32 input)
{
    Q_UNUSED(input);
    return QString();
}

/*********************************************************************
 * Configuration
 *********************************************************************/
void ArtNetPlugin::configure()
{
//    ConfigureArtNet conf(this);
//    conf.exec();
}

bool ArtNetPlugin::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(artnetplugin, ArtNetPlugin)
