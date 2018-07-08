/*
  Q Light Controller Plus
  webaccessnetwork.h

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

#ifndef WEBACCESSNETWORK_H
#define WEBACCESSNETWORK_H

#include <QObject>

typedef struct
{
    bool enabled;
    QString name;
    bool isStatic;
    bool isWireless;
    QString address;
    QString netmask;
    QString gateway;
    QString dns1;
    QString dns2;
    QString wpaConfFile;
    QString ssid;
    QString wpaPass;
} InterfaceInfo;

class WebAccessNetwork: public QObject
{
public:
    WebAccessNetwork(QObject *parent = 0);

    void resetInterface(InterfaceInfo *iface);
    void appendInterface(InterfaceInfo iface);
    QString getInterfaceHTML(InterfaceInfo *iface);
    QString getNetworkHTML();
    QString getHTML();

    bool updateNetworkFile(QStringList cmdList);

protected:
    void parseWPAConfFile(InterfaceInfo *iface);
    bool writeNetworkFile();
    QString netmaskToString(int mask);
    int stringToNetmask(QString mask);

protected:
    QList<InterfaceInfo>m_interfaces;
    QStringList m_dhcpcdConfCache;
};

#endif // WEBACCESSNETWORK_H
