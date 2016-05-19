/*
  Q Light Controller Plus
  webaccessnetwork.cpp

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

#include <QNetworkInterface>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QFile>

#include "webaccessnetwork.h"
#include "commonjscss.h"
#include "qlcconfig.h"

#define IFACES_SYSTEM_FILE "/etc/network/interfaces"
#define DHCPCD_CONF_FILE "/etc/dhcpcd.conf"
#define WPA_SUPP_CONF_FILE "/etc/wpa_supplicant/wpa_supplicant.conf"

WebAccessNetwork::WebAccessNetwork(QObject *parent) :
    QObject(parent)
{
}

void WebAccessNetwork::resetInterface(InterfaceInfo *iface)
{
    iface->name = "";
    iface->isStatic = false;
    iface->isWireless = false;
    iface->address = "";
    iface->netmask  = "";
    iface->gateway = "";
    iface->enabled = false;
    iface->wpaConfFile = "";
    iface->ssid = "";
    iface->wpaPass = "";
    iface->dns1 = "";
    iface->dns2 = "";
}

void WebAccessNetwork::appendInterface(InterfaceInfo iface)
{
    if (iface.name.contains("wlan") || iface.name.contains("ra"))
        iface.isWireless = true;

    for (int i = 0; i < m_interfaces.count(); i++)
    {
        if (m_interfaces.at(i).name == iface.name)
        {
            m_interfaces[i].isStatic = iface.isStatic;
            m_interfaces[i].isWireless = iface.isWireless;
            m_interfaces[i].enabled = iface.enabled;

            if (!iface.address.isEmpty())
                m_interfaces[i].address = iface.address;
            if (!iface.gateway.isEmpty())
                m_interfaces[i].gateway = iface.gateway;
            if (!iface.netmask.isEmpty())
                m_interfaces[i].netmask = iface.netmask;
            if (!iface.dns1.isEmpty())
                m_interfaces[i].dns1 = iface.dns1;
            if (!iface.dns2.isEmpty())
                m_interfaces[i].dns2 = iface.dns2;

            if (!iface.ssid.isEmpty())
                m_interfaces[i].ssid = iface.ssid;
            if (!iface.wpaPass.isEmpty())
                m_interfaces[i].wpaPass = iface.wpaPass;

            return;
        }
    }

    // if we're here, it's a new interface. Just add it
    m_interfaces.append(iface);
}

QString WebAccessNetwork::getInterfaceHTML(InterfaceInfo *iface)
{
    QString dhcpChk = iface->isStatic ? QString() : QString("checked");
    QString staticChk = iface->isStatic ? QString("checked") : QString();
    QString visibility = iface->isStatic ? QString("visible") : QString("hidden");
    QString html = "<div style=\"margin: 20px 7% 20px 7%; width: 86%;\" >\n";
    html += "<div style=\"font-family: verdana,arial,sans-serif; padding: 5px 7px; font-size:20px; "
            "color:#CCCCCC; background:#222; border-radius: 7px;\">";

    html += tr("Network interface: ") + iface->name + "<br>\n";

    html += "<form style=\"margin: 5px 15px; color:#FFF;\">\n";
    if (iface->isWireless)
    {
        html += tr("Access point name (SSID): ") + "<input type=\"text\" id=\"" +
                iface->name + "SSID\" size=\"15\" value=\"" + iface->ssid + "\"><br>\n";
        html += tr("WPA-PSK Password: ") + "<input type=\"text\" id=\"" +
                iface->name + "WPAPSK\" size=\"15\" value=\"" + iface->wpaPass + "\"><br>\n";
    }
    /** IP mode radio buttons */
    html += "<input type=\"radio\" name=" + iface->name + "NetGroup onclick=\"showStatic('" +
            iface->name + "', false);\" value=\"dhcp\" " + dhcpChk + ">" + tr("Dynamic (DHCP)") + "<br>\n";
    html += "<input type=\"radio\" name=" + iface->name + "NetGroup onclick=\"showStatic('" +
            iface->name + "', true);\" value=\"static\" " + staticChk + ">" + tr("Static") + "<br>\n";

    /** Static IP fields */
    html += "<div id=\"" + iface->name + "StaticFields\" style=\"padding: 5px 30px; visibility:" + visibility + ";\">\n";
    html += tr("IP Address: ") + "<input type=\"text\" id=\"" +
            iface->name + "IPaddr\" size=\"15\" value=\"" + iface->address + "\"><br>\n";
    html += tr("Netmask: ") + "<input type=\"text\" id=\"" +
            iface->name + "Netmask\" size=\"15\" value=\"" + iface->netmask + "\"><br>\n";
    html += tr("Gateway: ") + "<input type=\"text\" size=\"15\" id=\"" +
            iface->name + "Gateway\" value=\"" + iface->gateway + "\"><br>\n";
    html += "</div>\n";
    html += "<input type=\"button\" value=\"" + tr("Apply changes") + "\" onclick=\"applyParams('" + iface->name + "');\" >\n";
    html += "</form></div></div>";

    return html;
}

QString WebAccessNetwork::getNetworkHTML()
{
    /* The complete picture of the current network interfaces status
     *  come from several places:
     * 1- Qt network interfaces
     * 2- /etc/network/interfaces
     * 3- /etc/dhcpcd.conf
     * 4- /etc/wpa_supplicant/wpa_supplicant.conf
     *
     * It is necessary to parse all of them and, only at the end,
     * produce the HTML code to be sent to a web browser
     */

    QString html = "";
    m_interfaces.clear();
    InterfaceInfo currInterface;
    resetInterface(&currInterface);

    // 1- gather the active network interface names with Qt
    QStringList systemDevs;
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        qDebug() << "Qt detected interface:" << interface.name();
        if (interface.name() != "lo")
            systemDevs.append(interface.name());
    }

    // 2- parse the interfaces file
    QFile interfacesFile(IFACES_SYSTEM_FILE);
    if (interfacesFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
        return "";

    QTextStream ifacesQTS(&interfacesFile);
    while (!ifacesQTS.atEnd())
    {
        QString line = ifacesQTS.readLine();
        line = line.simplified();
        // skip comments
        if (line.startsWith('#'))
            continue;

        QStringList ifaceRow = line.split(" ");
        if (ifaceRow.count() == 0)
            continue;

        QString keyword = ifaceRow.at(0);
        if (keyword == "iface")
        {
            if (currInterface.name.isEmpty() == false)
            {
                appendInterface(currInterface);
                resetInterface(&currInterface);
            }

            if (ifaceRow.count() < 4)
                continue;

            currInterface.name = ifaceRow.at(1);
            if (systemDevs.contains(currInterface.name))
                currInterface.enabled = true;

            if (ifaceRow.at(3) == "static")
                currInterface.isStatic = true;
        }
        else if (keyword == "wpa-conf")
        {
            currInterface.wpaConfFile = ifaceRow.at(1);
            parseWPAConfFile(&currInterface);
        }
        else if (keyword == "address")
            currInterface.address = ifaceRow.at(1);
        else if (keyword == "netmask")
            currInterface.netmask = ifaceRow.at(1);
        else if (keyword == "gateway")
            currInterface.gateway = ifaceRow.at(1);
        else if (keyword == "wpa-ssid")
            currInterface.ssid = ifaceRow.at(1);
        else if (keyword == "wpa-psk")
            currInterface.wpaPass = ifaceRow.at(1);
    }

    if (currInterface.name.isEmpty() == false)
    {
        appendInterface(currInterface);
        resetInterface(&currInterface);
    }

    interfacesFile.close();

    // 3- parse the dhcpcd.conf file
    bool qlcplusSectionFound = false;
    QFile dhcpcdFile(DHCPCD_CONF_FILE);
    if (dhcpcdFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
        return "";

    m_dhcpcdConfCache.clear();

    QTextStream dhcpQTS(&dhcpcdFile);
    while (!dhcpQTS.atEnd())
    {
        QString line = dhcpQTS.readLine();
        line = line.simplified();
        if (line.contains("QLC+"))
            qlcplusSectionFound = true;

        // cache the original file BEFORE the QLC+ section
        if (qlcplusSectionFound == false)
        {
            m_dhcpcdConfCache.append(line);
            continue;
        }

        line = line.simplified();

        QStringList ifaceRow = line.split(" ");
        if (ifaceRow.count() < 2)
            continue;

        QString keyword = ifaceRow.at(0);
        if (keyword == "interface")
        {
            if (currInterface.name.isEmpty() == false)
            {
                appendInterface(currInterface);
                resetInterface(&currInterface);
            }

            currInterface.name = ifaceRow.at(1);
            if (systemDevs.contains(currInterface.name))
                currInterface.enabled = true;
        }
        else if (keyword == "static")
        {
            QStringList params = ifaceRow.at(1).split("=");
            if (params.count() < 2)
                continue;

            currInterface.isStatic = true;

            QString paramKey = params.at(0);

            if (paramKey == "ip_address")
            {
                QStringList ipAddrVals = params.at(1).split("/");
                if (ipAddrVals.count() < 2)
                    continue;
                currInterface.address = ipAddrVals.at(0);
                currInterface.netmask = netmaskToString(ipAddrVals.at(1).toInt());
            }
            else if (paramKey == "routers")
            {
                currInterface.gateway = params.at(1);
            }
            else if (paramKey == "domain_name_servers")
            {
                currInterface.dns1 = params.at(1);
                if (ifaceRow.count() == 3)
                    currInterface.dns2 = ifaceRow.at(2);
            }
        }
    }

    if (currInterface.name.isEmpty() == false)
    {
        appendInterface(currInterface);
        resetInterface(&currInterface);
    }

    foreach (InterfaceInfo info, m_interfaces)
    {
        if (info.enabled)
            html += getInterfaceHTML(&info);
        qDebug() << "Interface:" << info.name << "isstatic:" << info.isStatic << "address:" << info.address
                 << "netmask:" << info.netmask << "gateway:" << info.gateway;
    }

    return html;
}

QString WebAccessNetwork::getHTML()
{
    QString m_JScode = "<script type=\"text/javascript\" src=\"websocket.js\"></script>\n";
    m_JScode += "<script type=\"text/javascript\" src=\"networkconfig.js\"></script>\n";

    QString m_CSScode = "<link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"common.css\">\n";
    m_CSScode += "<style type=\"text/css\" media=\"screen\">\n"
                 "html { height: 100%; background-color: #111; }\n"
                 "body {\n"
                 " margin: 0px;\n"
                 " background-image: linear-gradient(to bottom, #45484d 0%, #111 100%);\n"
                 " background-image: -webkit-linear-gradient(top, #45484d 0%, #111 100%);\n"
                 "}\n"
                 "</style>\n";

    QString bodyHTML = "<div class=\"controlBar\">\n"
                       "<a class=\"button button-blue\" href=\"/\"><span>" + tr("Back") + "</span></a>\n"
                       "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>\n"
                       "</div>\n";

    bodyHTML += "<div style=\"margin: 15px 7% 0px 7%; width: 86%; font-family: verdana,arial,sans-serif;"
                "font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("Network configuration") + "</div>\n";
    bodyHTML += getNetworkHTML();

    bodyHTML += "<div style=\"margin: 15px 7% 0px 7%; width: 86%; font-family: verdana,arial,sans-serif;"
                "font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("Project autostart") + "</div>\n";
    bodyHTML += "<div style=\"margin: 15px 7% 0px 7%; width: 86%; font-family: verdana,arial,sans-serif;"
                "font-size:18px; padding: 5px 0px; color:#CCCCCC; background:#222; border-radius: 7px;\">";
    bodyHTML += "<form style=\"margin: 5px 15px; color:#FFF;\">\n";
    bodyHTML += "<input type=\"radio\" name=autostart value=\"none\">" + tr("No project") + "\n";
    bodyHTML += "<input type=\"radio\" name=autostart value=\"current\" checked>" + tr("Use current project") + "\n";
    bodyHTML += "<input type=\"button\" value=\"" + tr("Apply changes") + "\" onclick=\"setAutostart();\" >\n";
    bodyHTML += "</form></div>\n";

    bodyHTML += "<div style=\"margin:5px 7%;\">\n";
    bodyHTML += "<a class=\"button button-blue\" href=\"\" onclick=\"javascript:websocket.send('QLC+SYS|REBOOT');\"><span>" + tr("Reboot") + "</span></a>\n";
    bodyHTML += "<a class=\"button button-blue\" href=\"\" onclick=\"javascript:websocket.send('QLC+SYS|HALT');\"><span>" + tr("Shutdown") + "</span></a>\n";
    bodyHTML += "</div>\n";

    QString str = HTML_HEADER + m_JScode + m_CSScode + "</head>\n<body>\n" + bodyHTML + "</body>\n</html>";

    return str;
}

bool WebAccessNetwork::updateNetworkFile(QStringList cmdList)
{
    for (int i = 0; i < m_interfaces.count(); i++)
    {
        if (m_interfaces.at(i).name == cmdList.at(2))
        {
            m_interfaces[i].enabled = true;
            if (cmdList.at(3) == "static")
                m_interfaces[i].isStatic = true;
            else
                m_interfaces[i].isStatic = false;
            m_interfaces[i].address = cmdList.at(4);
            m_interfaces[i].netmask = cmdList.at(5);
            m_interfaces[i].gateway = cmdList.at(6);
            if (m_interfaces[i].isWireless == true)
            {
                m_interfaces[i].ssid = cmdList.at(7);
                m_interfaces[i].wpaPass = cmdList.at(8);
            }
            return writeNetworkFile();
        }
    }
    return false;
}

void WebAccessNetwork::parseWPAConfFile(InterfaceInfo *iface)
{
    bool inNetwork = false;

    if (iface == NULL || iface->wpaConfFile.isEmpty())
        return;

    qDebug() << "Parsing WPA conf file" << iface->wpaConfFile;

    QFile wpaConfFile(iface->wpaConfFile);
    if (wpaConfFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
        return;

    QTextStream wpaConfQTS(&wpaConfFile);
    while (!wpaConfQTS.atEnd())
    {
        QString line = wpaConfQTS.readLine();
        line = line.simplified();

        if (line.startsWith("network"))
        {
            inNetwork = true;
            continue;
        }

        if (inNetwork)
        {
            if (line.contains("}"))
            {
                inNetwork = false;
                continue;
            }

            QStringList tokens = line.split("=");
            if (tokens.count() == 2)
            {
                QString param = tokens.at(0);
                QString value = tokens.at(1);

                //qDebug() << "Tokens:"<< param << value;

                if (param == "ssid")
                    iface->ssid = value.remove(QChar('"'));
                else if (param == "psk")
                    iface->wpaPass = value.remove(QChar('"'));
            }
        }

    }

    wpaConfFile.close();
}

bool WebAccessNetwork::writeNetworkFile()
{
    /* Here 3 things again:
     * 1- the /etc/network/interfaces file is left untouched
     * 2- /etc/dhcpcd.conf is written only if there are static IPs set
     * 3- the wpa_supplicant.conf file(s) are written for each wireless adapter
     */

    bool dhcpcdCacheWritten = false;
    QFile dhcpcdFile(DHCPCD_CONF_FILE);
    if (dhcpcdFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;

    foreach (InterfaceInfo iface, m_interfaces)
    {
        if (iface.enabled == false)
            continue;

        if (iface.isStatic == true)
        {
            if (dhcpcdCacheWritten == false && m_dhcpcdConfCache.isEmpty() == false)
            {
                foreach(QString line, m_dhcpcdConfCache)
                {
                    dhcpcdFile.write(line.toLatin1());
                    dhcpcdFile.write("\n");
                }
                dhcpcdFile.write("\n######### QLC+ parameters. Do not edit #########\n\n");
                dhcpcdCacheWritten = true;
            }
            else
                qDebug() << "[writeNetworkFile] ERROR. No dhcpcd cache found !";

            dhcpcdFile.write((QString("interface %1\n").arg(iface.name)).toLatin1());
            dhcpcdFile.write((QString("static ip_address=%1/%2\n").arg(iface.address).arg(stringToNetmask(iface.netmask))).toLatin1());
            dhcpcdFile.write((QString("static routers=%1\n").arg(iface.gateway)).toLatin1());
            if (iface.dns1.isEmpty() == false)
                dhcpcdFile.write((QString("static domain_name_servers=%1\n\n").arg(iface.dns1)).toLatin1());
            else
                dhcpcdFile.write(QString("static domain_name_servers=127.0.0.1\n\n").toLatin1());
        }

        if (iface.isWireless)
        {
            QString wpaConfName = iface.wpaConfFile.isEmpty() ? WPA_SUPP_CONF_FILE : iface.wpaConfFile;
            qDebug() << "[writeNetworkFile] Writing wpa conf file:" << wpaConfName;
            QFile wpaConfFile(wpaConfName);
            if (wpaConfFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
            {
                qDebug() << "[writeNetworkFile] Error opening file" << wpaConfName;
                return false;
            }

            wpaConfFile.write(QString("ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\n").toLatin1());
            wpaConfFile.write(QString("update_config=1\n\n").toLatin1());
            wpaConfFile.write(QString("network={\n").toLatin1());
            wpaConfFile.write(QString("scan_ssid=1\n").toLatin1());
            wpaConfFile.write((QString("ssid=\"%1\"\n").arg(iface.ssid)).toLatin1());
            wpaConfFile.write((QString("psk=\"%1\"\n").arg(iface.wpaPass)).toLatin1());
            wpaConfFile.write(QString("}\n").toLatin1());
            wpaConfFile.close();
        }
    }

    dhcpcdFile.close();

    return true;
}

#if 0 // old Wheezy configuration
bool WebAccessNetwork::writeNetworkFile()
{
    QFile netFile(IFACES_SYSTEM_FILE);
    if (netFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;

    netFile.write(QString("auto lo\n").toLatin1());
    netFile.write(QString("iface lo inet loopback\n").toLatin1());
    netFile.write(QString("allow-hotplug eth0\n").toLatin1());

    foreach (InterfaceInfo iface, m_interfaces)
    {
        if (iface.enabled == false)
            continue;

        if (iface.isWireless)
            netFile.write((QString("auto %1\n").arg(iface.name)).toLatin1());

        if (iface.isStatic == false)
            netFile.write((QString("iface %1 inet dhcp\n").arg(iface.name)).toLatin1());
        else
        {
            netFile.write((QString("iface %1 inet static\n").arg(iface.name)).toLatin1());
            netFile.write((QString("  address %1\n").arg(iface.address)).toLatin1());
            netFile.write((QString("  netmask %1\n").arg(iface.netmask)).toLatin1());
            netFile.write((QString("  gateway %1\n").arg(iface.gateway)).toLatin1());
        }
        if (iface.isWireless)
        {
            if (iface.ssid.isEmpty() == false)
                netFile.write((QString("  wpa-ssid %1\n").arg(iface.ssid)).toLatin1());
            if (iface.wpaPass.isEmpty() == false)
                netFile.write((QString("  wpa-psk %1\n").arg(iface.wpaPass)).toLatin1());
        }
    }

    netFile.close();

    return true;
}
#endif

QString WebAccessNetwork::netmaskToString(int mask)
{
    QString nmString;

    quint32 bitmask = 0;
    for (int i = 0; i < mask; i++)
        bitmask |= (1 << (31 - i));

    for (int n = 0; n < 4; n++)
    {
        if (nmString.isEmpty() == false)
            nmString.prepend(".");
        nmString.prepend(QString::number((bitmask >> (8 * n)) & 0x00FF));
    }
    return nmString;
}

int WebAccessNetwork::stringToNetmask(QString mask)
{
    quint32 lMask = 0;
    int nMask = 0;

    QStringList nibbles = mask.split(".");
    if (nibbles.count() != 4)
        return 24;

    for (int i = 0; i < 4; i++)
        lMask |= (nibbles.at(i).toInt() << (8 * (3 - i)));

    for (int b = 0; b < 32; b++)
    {
        if (lMask & (1 << (31 - b)))
            nMask++;
        else
            break;
    }

    return nMask;
}

