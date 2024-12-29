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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <QTextStream>
#include <QStringList>
#include <QProcess>
#include <QDebug>
#include <QFile>

#include "webaccessnetwork.h"
#include "commonjscss.h"
#include "qlcconfig.h"

#define IFACES_SYSTEM_FILE "/etc/network/interfaces"
#define DHCPCD_CONF_FILE "/etc/dhcpcd.conf"
#define WPA_SUPP_CONF_FILE "/etc/wpa_supplicant/wpa_supplicant.conf"
#define HOTSPOT_CON_NAME QString("QLCHOTSPOT")

WebAccessNetwork::WebAccessNetwork(QObject *parent) :
    QObject(parent)
{
}

void WebAccessNetwork::resetInterface(InterfaceInfo *iface)
{
    iface->devName = "";
    iface->connName = "";
    iface->connUUID = "";
    iface->isStatic = false;
    iface->isWireless = false;
    iface->isHotspot = false;
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
    if (iface.devName.contains("wlan") || iface.devName.contains("ra"))
        iface.isWireless = true;

    for (int i = 0; i < m_interfaces.count(); i++)
    {
        if (m_interfaces.at(i).devName == iface.devName)
        {
            m_interfaces[i].isStatic = iface.isStatic;
            m_interfaces[i].isWireless = iface.isWireless;
            m_interfaces[i].isHotspot = iface.isHotspot;
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
    QString editable = iface->isStatic ? QString("") : QString("disabled");
    QString html = "<div style=\"margin: 20px 7% 20px 7%; width: 86%;\" >\n";
    html += "<div style=\"font-family: verdana,arial,sans-serif; padding: 5px 7px; font-size:20px; "
            "color:#CCCCCC; background:#222; border-radius: 7px;\">";

    html += "<table><tr>";

    if (iface->isHotspot)
    {
        html += "<td style=\"padding: 0 37px; text-align: center; background: #333;\">";
        html += tr("Wi-Fi Hotspot") + "</td>\n";

        html += "<td><form style=\"margin: 5px 15px; color:#FFF;\">\n";
        html += "<table><tr>";
        html += "<td>" + tr("Access point name (SSID): ") + "</td><td><input type=\"text\" id=\"" +
                "hotspotSSID\" size=\"15\" value=\"" + iface->ssid + "\"></td></tr>\n";
        html += "<td>" + tr("WPA-PSK Password: ") + "</td><td><input type=\"text\" id=\"" +
                "hotspotWPAPSK\" size=\"15\" value=\"\"></td></tr>\n";
        html += "</tr></table>";

        html += "<input type=\"button\" value=\"" + tr("Enable") + "\" onclick=\"enableHotspot(1);\" >\n";
        html += "<input type=\"button\" value=\"" + tr("Disable") + "\" onclick=\"enableHotspot(0);\" >\n";
    }
    else
    {
        html += "<td style=\"padding: 0 20px; text-align: center; background: #333;\">";
        html += tr("Network interface") + "<br>" + iface->devName + "</td>\n";

        html += "<td><form style=\"margin: 5px 15px; color:#FFF;\">\n";
        if (iface->isWireless)
        {
            html += "<table><tr>";
            html += "<td>" + tr("Access point name (SSID): ") + "</td><td><input type=\"text\" id=\"" +
                    iface->devName + "SSID\" size=\"15\" value=\"" + iface->ssid + "\"></td></tr>\n";
            html += "<td>" + tr("WPA-PSK Password: ") + "</td><td><input type=\"text\" id=\"" +
                    iface->devName + "WPAPSK\" size=\"15\" value=\"" + iface->wpaPass + "\"></td></tr>\n";
            html += "</tr></table>";
        }
        /** IP mode radio buttons */
        html += "<input type=\"radio\" name=" + iface->devName + "NetGroup onclick=\"showStatic('" +
                iface->devName + "', false);\" value=\"dhcp\" " + dhcpChk + ">" + tr("Dynamic (DHCP)") + "<br>\n";
        html += "<input type=\"radio\" name=" + iface->devName + "NetGroup onclick=\"showStatic('" +
                iface->devName + "', true);\" value=\"static\" " + staticChk + ">" + tr("Static") + "<br>\n";

        /** Static IP fields */
        html += "<div id=\"" + iface->devName + "StaticFields\" style=\"padding: 5px 30px;\">\n";
        html += "<table><tr>";
        html += "<td>" + tr("IP Address: ") + "</td><td><input type=\"text\" id=\"" +
                iface->devName + "IPaddr\" size=\"15\" value=\"" + iface->address + "\" " + editable + "></td></tr>\n";
        html += "<td>" + tr("Netmask: ") + "</td><td><input type=\"text\" id=\"" +
                iface->devName + "Netmask\" size=\"15\" value=\"" + iface->netmask + "\" " + editable + "></td></tr>\n";
        html += "<td>" + tr("Gateway: ") + "</td><td><input type=\"text\" size=\"15\" id=\"" +
                iface->devName + "Gateway\" value=\"" + iface->gateway + "\" " + editable + "></td></tr>\n";
        html += "</table></div>\n";
        html += "<input type=\"button\" value=\"" + tr("Apply changes") + "\" onclick=\"applyParams('" + iface->devName + "');\" >\n";
    }

    html += "</form></td></tr></table></div></div>";

    return html;
}

QStringList WebAccessNetwork::getNmcliOutput(QStringList args, bool verbose)
{
    QStringList outputLines;
    QProcess process;

    args.prepend("nmcli");
    qDebug() << "Executing command line: " << args.join(' ');
    process.start("sudo", args);

    if (process.waitForFinished())
    {
        process.setReadChannel(QProcess::StandardOutput);
        while (process.canReadLine())
        {
            QString line = process.readLine().simplified();
            if (verbose)
                qDebug() << "Output::" << line;

            outputLines << line;
        }
    }

    return outputLines;
}

void WebAccessNetwork::refreshConnectionsList()
{
    InterfaceInfo currInterface;

    m_interfaces.clear();
    resetInterface(&currInterface);

    // execute "nmcli -t device status" to list all avilable devices
    QStringList devStatusOutput = getNmcliOutput(QStringList() << "-t" << "device" << "status");

    foreach (QString dLine, devStatusOutput)
    {
        QStringList devTokens = dLine.split(':');
        qDebug() << "output " << devTokens.at(0) << devTokens.at(3);

        if (!currInterface.devName.isEmpty())
            appendInterface(currInterface);

        resetInterface(&currInterface);
        currInterface.enabled = true;
        currInterface.devName = devTokens.at(0);
        currInterface.connName = devTokens.at(3);

        // skip loopback and disconnected interfaces
        if (currInterface.devName == "lo" || currInterface.devName.contains("p2p"))
        {
            currInterface.devName = "";
            continue;
        }

        if (currInterface.connName.isEmpty())
            continue;

        if (devTokens.at(3) == HOTSPOT_CON_NAME)
            currInterface.isHotspot = true;

        // run "nmcli -t con show CONN_NAME" to retrieve everything about a connection
        QStringList conShowOuput = getNmcliOutput(QStringList() << "-t" << "con" << "show" << currInterface.connName);
        foreach (QString cLine, conShowOuput)
        {
            QStringList params = cLine.split(':');
            if (params.at(0) == "connection.uuid")
            {
                currInterface.connUUID = params.at(1);
            }
            else if (params.at(0) == "ipv4.method")
            {
                currInterface.isStatic = params.at(1) == "auto" ? false : true;
            }
            else if (params.at(0).startsWith("IP4.ADDRESS"))
            {
                QStringList ipAddrTokens = params.at(1).split("/");
                if (ipAddrTokens.count() == 2)
                {
                    currInterface.address = ipAddrTokens.at(0);
                    unsigned long mask = (0xFFFFFFFF << (32 - ipAddrTokens.at(1).toUInt())) & 0xFFFFFFFF;
                    currInterface.netmask = QString::number(mask >> 24) + '.' +
                                            QString::number((mask >> 16) & 0xFF) + '.' +
                                            QString::number((mask >> 8) & 0xFF) + '.' +
                                            QString::number(mask & 0xFF);
                }
            }
            else if (params.at(0).startsWith("IP4.GATEWAY"))
            {
                currInterface.gateway = params.at(1);
            }
            else if (params.at(0).startsWith("IP4.DNS"))
            {
                if (currInterface.dns1.isEmpty())
                    currInterface.dns1 = params.at(1);
                else
                    currInterface.dns2 = params.at(1);
            }
            else if (params.at(0) == "802-11-wireless.ssid")
            {
                currInterface.ssid = params.at(1);
            }
        }
    }
}

QString WebAccessNetwork::getNetworkHTML()
{
    QString html = "";
    bool hotspotFound = false;

    refreshConnectionsList();

    foreach (InterfaceInfo info, m_interfaces)
    {
        if (info.enabled)
            html += getInterfaceHTML(&info);
        qDebug() << "Interface:" << info.devName << "isStatic:" << info.isStatic << "address:" << info.address
                 << "netmask:" << info.netmask << "gateway:" << info.gateway;
        if (info.isHotspot)
            hotspotFound = true;
    }

    // add the possibility to activate a Wi-Fi hotspot
    if (hotspotFound == false)
    {
        InterfaceInfo hs;
        hs.isHotspot = true;
        html += getInterfaceHTML(&hs);
    }

    return html;
}

QString WebAccessNetwork::getHTML()
{
    QString m_JScode = "<script type=\"text/javascript\" src=\"websocket.js\"></script>\n";
    m_JScode += "<script type=\"text/javascript\" src=\"networkconfig.js\"></script>\n";

    QString m_CSScode = "<link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"common.css\">\n";
    m_CSScode += "<style type=\"text/css\" media=\"screen\">\n"
                 "html { height: 100%; background-color: #222; }\n"
                 "body {\n"
                 " margin: 0px;\n"
                 " background: #222;\n"
                 "}\n"
                 "input[type=button] {\n"
                 "background-color: #364e5e;\n"
                 "border: none;\n"
                 "border-radius: .2em;\n"
                 "color: white;\n"
                 "font: 20px/1.0em 'Trebuchet MS',Arial, Helvetica;\n"
                 "padding: 10px 16px;\n"
                 "text-decoration: none;\n"
                 "margin: 4px 2px;\n"
                 "cursor: pointer;\n"
                 "}\n"
                 "</style>\n";

    QString bodyHTML = "<div class=\"controlBar\">\n"
                       "<a class=\"button button-blue\" href=\"/\"><span>" + tr("Back") + "</span></a>\n"
                       "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>\n"
                       "</div>\n";

    bodyHTML += "<div style=\"margin: 15px 7% 0px 7%; width: 86%; font-family: verdana,arial,sans-serif;"
                "font-size: 20px; text-align: center; color: #CCCCCC; background: #333; padding: 7px;\">";
    bodyHTML += tr("Network configuration") + "</div>\n";
    bodyHTML += getNetworkHTML();

    bodyHTML += "<div style=\"margin: 15px 7% 0px 7%; width: 86%; font-family: verdana,arial,sans-serif;"
                "font-size: 20px; text-align: center; color: #CCCCCC; background: #333; padding: 7px;\">";
    bodyHTML += tr("Project autostart") + "</div>\n";
    bodyHTML += "<div style=\"margin: 15px 7% 0px 7%; width: 86%; font-family: verdana,arial,sans-serif;"
                "font-size:18px; padding: 5px 0px; color:#CCCCCC; background:#222; border-radius: 7px;\">";
    bodyHTML += "<form style=\"margin: 5px 15px; color:#FFF;\">\n";
    bodyHTML += "<input type=\"radio\" name=autostart value=\"none\">" + tr("No project") + "\n";
    bodyHTML += "<input type=\"radio\" name=autostart value=\"current\" checked>" + tr("Use current project") + "\n";
    bodyHTML += "<input type=\"button\" value=\"" + tr("Apply changes") + "\" onclick=\"setAutostart();\" >\n";
    bodyHTML += "</form></div>\n";

    bodyHTML += "<div style=\"margin:5px 7%;\">\n";
    bodyHTML += "<input type=\"button\" value=\"" + tr("Reboot") + "\" onclick=\"javascript:websocket.send('QLC+SYS|REBOOT');\">\n";
    bodyHTML += "<input type=\"button\" value=\"" + tr("Shutdown") + "\" onclick=\"javascript:websocket.send('QLC+SYS|HALT');\">";
    bodyHTML += "</div>\n";

    QString str = HTML_HEADER + m_JScode + m_CSScode + "</head>\n<body>\n" + bodyHTML + "</body>\n</html>";

    return str;
}

bool WebAccessNetwork::updateNetworkSettings(QStringList cmdList)
{
    for (int i = 0; i < m_interfaces.count(); i++)
    {
        if (m_interfaces.at(i).devName == cmdList.at(2))
        {
            if (!m_interfaces[i].connName.isEmpty())
            {
                // first off, delete the current connection profile
                getNmcliOutput(QStringList() << "con" << "del" << m_interfaces[i].connName);
            }

            m_interfaces[i].enabled = true;
            bool staticRequest = cmdList.at(3) == "static" ? true : false;
            QString args = "con add con-name qlcplus" + m_interfaces[i].devName + " ifname " + m_interfaces[i].devName;

            if (staticRequest)
            {
                // convert netmask to bitwise notation
                uint32_t intNetMask;
                uint32_t bitCount = 0;

                if (inet_pton(AF_INET, cmdList.at(5).toUtf8().constData(), &intNetMask) == 0)
                {
                    qDebug() << "Invalid netmask";
                    return false;
                }

                while (intNetMask > 0)
                {
                    intNetMask = intNetMask >> 1;
                    bitCount++;
                }

                if (m_interfaces[i].isWireless)
                    args = args + " type wifi ssid " + cmdList.at(7);
                else
                    args = args + " type ethernet";

                args = args + " ip4 " + cmdList.at(4) + "/" + QString::number(bitCount) + " gw4 " + cmdList.at(6);
            }
            else // DHCP
            {
                if (m_interfaces[i].isWireless)
                {
                    //m_interfaces[i].ssid = cmdList.at(7);
                    //m_interfaces[i].wpaPass = cmdList.at(8);
                    args = args + " type wifi ssid " + cmdList.at(7);
                }
                else
                {
                    args += " type ethernet";
                }
            }

            // add the new/updated connection profile
            getNmcliOutput(args.split(" "));

            // if a password is set, modify the just created connection
            if (m_interfaces[i].isWireless && !cmdList.at(8).isEmpty())
            {
                args = "con mod qlcplus" + m_interfaces[i].devName + " wifi-sec.key-mgmt wpa-psk wifi-sec.psk " + cmdList.at(8);
                getNmcliOutput(args.split(" "));
            }

            // finally, activate the connection
            args = "con up qlcplus" + m_interfaces[i].devName;
            getNmcliOutput(args.split(" "));

            refreshConnectionsList();

            return true;
        }
    }
    return false;
}

bool WebAccessNetwork::createWiFiHotspot(QString SSID, QString password)
{
    // first off, delete the current connection profile
    getNmcliOutput(QStringList() << "con" << "del" << HOTSPOT_CON_NAME);

    // create the connection
    QString args = "con add type wifi ifname wlan0 mode ap con-name " + HOTSPOT_CON_NAME + " autoconnect yes ssid \"" + SSID + "\"";
    getNmcliOutput(args.split(" "));

    // modify with proper parameters
    args = "con modify " + HOTSPOT_CON_NAME + " 802-11-wireless.ssid " + SSID;
    getNmcliOutput(args.split(" "));
    args = "con modify " + HOTSPOT_CON_NAME + " 802-11-wireless.band bg";
    getNmcliOutput(args.split(" "));
    args = "con modify " + HOTSPOT_CON_NAME + " 802-11-wireless-security.key-mgmt wpa-psk";
    getNmcliOutput(args.split(" "));
    args = "con modify " + HOTSPOT_CON_NAME + " 802-11-wireless-security.proto rsn";
    getNmcliOutput(args.split(" "));
    args = "con modify " + HOTSPOT_CON_NAME + " 802-11-wireless-security.group ccmp";
    getNmcliOutput(args.split(" "));
    args = "con modify " + HOTSPOT_CON_NAME + " 802-11-wireless-security.pairwise ccmp";
    getNmcliOutput(args.split(" "));
    args = "con modify " + HOTSPOT_CON_NAME + " 802-11-wireless-security.psk " + password;
    getNmcliOutput(args.split(" "));
    args = "con modify " + HOTSPOT_CON_NAME + " ipv4.method shared";
    getNmcliOutput(args.split(" "));

    // activate the connection
    args = "con up " + HOTSPOT_CON_NAME;
    getNmcliOutput(args.split(" "));

    return true;
}

bool WebAccessNetwork::deleteWiFiHotspot()
{
    getNmcliOutput(QStringList() << "con" << "del" << HOTSPOT_CON_NAME);
    return true;
}
