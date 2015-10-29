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
    iface->gateway = "";
    iface->gateway = "";
    iface->enabled = false;
    iface->ssid = "";
    iface->wpaPass = "";
}

void WebAccessNetwork::appendInterface(InterfaceInfo iface)
{
    m_interfaces.append(iface);
}

QString WebAccessNetwork::getInterfaceHTML(InterfaceInfo *iface)
{
    QString dhcpChk = iface->isStatic?QString():QString("checked");
    QString staticChk = iface->isStatic?QString("checked"):QString();
    QString visibility = iface->isStatic?QString("visible"):QString("hidden");
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
    QStringList systemDevs;
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
        systemDevs.append(interface.name());

    QFile netFile(IFACES_SYSTEM_FILE);
    if (netFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
        return "";

    m_interfaces.clear();
    InterfaceInfo currInterface;
    resetInterface(&currInterface);

    QString html = "";

    QTextStream in(&netFile);
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        line = line.simplified();
        // ignore comments
        if (line.startsWith('#'))
            continue;

        QStringList ifaceRow = line.split(" ");
        if (ifaceRow.count() == 0)
            continue;

        QString keyword = ifaceRow.at(0);
        if (keyword == "iface")
        {
            if (currInterface.isStatic == true)
            {
                html += getInterfaceHTML(&currInterface);
                appendInterface(currInterface);
                resetInterface(&currInterface);
            }

            if (ifaceRow.count() < 4)
                continue;

            currInterface.name = ifaceRow.at(1);
            // remove the interface from the system list
            // the remaining interfaces will be added at the
            // end as disabled interfaces
            systemDevs.removeOne(currInterface.name);

            if (currInterface.name.contains("wlan") || currInterface.name.contains("ra"))
                currInterface.isWireless = true;

            if (ifaceRow.at(3) == "dhcp")
            {
                html += getInterfaceHTML(&currInterface);
                appendInterface(currInterface);
                resetInterface(&currInterface);
            }
            else if (ifaceRow.at(3) == "static")
                currInterface.isStatic = true;
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

        currInterface.enabled = true;
    }

    netFile.close();

    if (currInterface.isStatic == true)
    {
        html += getInterfaceHTML(&currInterface);
        appendInterface(currInterface);
        resetInterface(&currInterface);
    }

    foreach(QString dev, systemDevs)
    {
        currInterface.name = dev;
        currInterface.enabled = false;
        if (currInterface.name.contains("wlan") || currInterface.name.contains("ra"))
            currInterface.isWireless = true;
        html += getInterfaceHTML(&currInterface);
        appendInterface(currInterface);
        resetInterface(&currInterface);
    }

    foreach (InterfaceInfo info, m_interfaces)
    {
        qDebug() << "Interface:" << info.name << "isstatic:" << info.isStatic;
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

