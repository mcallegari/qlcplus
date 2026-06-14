/*
  Q Light Controller Plus
  idn.cpp

  Copyright (c) Daniel Schröder
  Updated by Mauritz Kauffmann, 2026

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
#include <QString>
#include <QDebug>
#include <QThread>

#include "idn.h"
#include "idnconfiguration.h"

static bool initialized = false;

#define MAX_INIT_RETRY 10

/*****************************************************************************
 * Initialization
 *****************************************************************************/

IdnPlugin::~IdnPlugin(){}

static QString settingsFilePath()
{
    return QDir::currentPath() + "/plugins/idn_settings.json";
}

void IdnPlugin::init()
{
  for (QNetworkInterface networkInterface : QNetworkInterface::allInterfaces())
  {
      for (QNetworkAddressEntry entry : networkInterface.addressEntries()){
        QHostAddress addr = entry.ip();
        if (addr.protocol() != QAbstractSocket::IPv6Protocol)
        {
          IdnOutput tmp;
          tmp.address = entry;
          tmp.controller = NULL;

          bool alreadyInList = false;

          for(int j = 0; j < m_Outputmapping.count(); j++){
            if (m_Outputmapping.at(j).address == tmp.address){
              alreadyInList = true;
              break;
            }
          }
          if (alreadyInList == false){
            m_Outputmapping.append(tmp);
          }
        }
      }
  }
  if(!initialized){
      initialized = true;
      QFileInfo checkFile(settingsFilePath());
      if(checkFile.exists() && checkFile.isFile()){
        loadSettings();
      }
  }
}

QString IdnPlugin::name() const
{
    return QString("IDN");
}

int IdnPlugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Infinite;
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

bool IdnPlugin::openOutput(quint32 output, quint32 universe)
{   
  if (requestLine(output, MAX_INIT_RETRY) == false)
        return false;
  
  if(m_Outputmapping[output].controller == NULL){
    IdnController *controller = new IdnController(m_Outputmapping.at(output).address,
                                                  output, m_manualClients, this);
    m_Outputmapping[output].controller = controller;

  }


  m_Outputmapping[output].controller->addUniverse(universe);

  addToMap(universe, output, Output);
  return true;
}

bool IdnPlugin::requestLine(quint32 line, int retries)
{
    int retryCount = 0;

    while (line >= (quint32)m_Outputmapping.length())
    {
        qDebug() << "[IDN] cannot open line" << line << "(available:" << m_Outputmapping.length() << ")";
		QThread::sleep(1);
        init();
        if (retryCount++ == retries)
            return false;
    }
    return true;
}

void IdnPlugin::closeOutput(quint32 output, quint32 universe)
{
  if (output >= (quint32)m_Outputmapping.length())
      return;
  
  removeFromMap(universe, output, Output);
  IdnController *controller = m_Outputmapping.at(output).controller;
  if(controller != NULL){
    if(controller->closeByUniverse(universe)){
      delete m_Outputmapping[output].controller;
      m_Outputmapping[output].controller = NULL; 
    }
  }
}

/*****************************************************************************
 * Fill Outputlist
 *****************************************************************************/

QStringList IdnPlugin::outputs()
{
    QStringList list;
    int j = 0;

    init();

    foreach(IdnOutput line, m_Outputmapping)
    {
        list << QString("%1: %2").arg(j + 1).arg(line.address.ip().toString());
        j++;
    }
    return list;
}

/*****************************************************************************
 * Plugin- and Output Info
 *****************************************************************************/

QString IdnPlugin::pluginInfo() const
{
  QString str;

  str += QString("<HTML>");
  str += QString("<HEAD>");
  str += QString("<TITLE>%1</TITLE>").arg(name());
  str += QString("</HEAD>");
  str += QString("<BODY>");

  str += QString("<P>");
  str += QString("<H3>%1</H3>").arg(name());
  str += tr("This plugin provides DMX output for devices supported by "
            "the IDN driver suite.");
  str += QString("</P>");

  return str;
}

QString IdnPlugin::outputInfo(quint32 output)
{
  Q_UNUSED(output);
    QString str;
    IdnController *controller = m_Outputmapping.at(output).controller;

    if (output != QLCIOPlugin::invalidLine())
    {

            str += QString("<H3>%1 %2</H3>").arg(tr("Output")).arg(outputs()[output]);
            str += QString("<P>");
            if(controller != NULL){
                str += QString("Status: Open");
                str += QString("<BR>");
                str += QString("Packets send: %1").arg(controller->getPacketSentNumber());
                str += QString("<BR>");
            }else{
                str += QString("Status: Not open");
            }
            str += QString("</P>");
    }
    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/*****************************************************************************
 * Send DMX Data to controller, that builds a IDN Packet and sends the Packet
 *****************************************************************************/

void IdnPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
  if (output >= (quint32) m_Outputmapping.count())
    return;

  IdnController *controller = m_Outputmapping.at(output).controller;
  if (controller != NULL) {
    controller->handleDmx(universe, data);
  }

  if(dataChanged) {

  }
}

/*****************************************************************************
* Reconfigure Controller after change of settings
*****************************************************************************/
 void IdnPlugin::reconfigureController(){
   //close all open outputs
   for(int i = 0; i < m_Outputmapping.length(); i++){
     closeOutput(i, 0);
   }
}

/*****************************************************************************
 * Configuration
 ****************************************************************************/

void IdnPlugin::configure(){
  IdnConfiguration config(this);
  config.exec();
  //reconfigureController();
  emit configurationChanged();
}

bool IdnPlugin::canConfigure() const{
    return true;
}

QList<IdnOutput> IdnPlugin::getOutputmapping()
{
    return m_Outputmapping;
}


// void IdnPlugin::setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value){

//    if (line >= (quint32)m_Outputmapping.length()) {
//       return;
//    }

//    IdnController *controller = m_Outputmapping.at(line).controller;
//    if (controller == NULL) {
//     return;
//    }

//    if(type != Output){
//        qWarning() << Q_FUNC_INFO << name << "is not a valid IDN parameter";
//    }

//    QLCIOPlugin::setParameter(universe, line, type, name, value);
// }

/*****************************************************************************
 * Load Settings from idn.ini File
 *****************************************************************************/

QHash<IdnHostInfo, IdnSettings> IdnPlugin::loadSettings(){
  try
  {
    const std::string filePath = settingsFilePath().toStdString();
    std::ifstream file(filePath);
    if (!file.is_open())
    {
      qWarning() << "Could not open settings file:" << settingsFilePath();
      return m_manualClients;
    }


    nlohmann::json jsonSettings;
    file >> jsonSettings;

    for (const auto &entry : jsonSettings.items())
    {
      const std::string &ip = entry.key();
      const auto &clients = entry.value();
      for (const auto &client : clients)
      {
        // Validate required fields
        if (!client.contains("serviceID"))
          qWarning() << "[IDN] loadSettings: entry for IP" << QString::fromStdString(ip) << "is missing 'serviceID', defaulting to 0";
        if (!client.contains("universe"))
          qWarning() << "[IDN] loadSettings: entry for IP" << QString::fromStdString(ip) << "is missing 'universe', defaulting to 1";
        if (!client.contains("iface"))
          qWarning() << "[IDN] loadSettings: entry for IP" << QString::fromStdString(ip) << "is missing 'iface', defaulting to 127.0.0.1";

        IdnHostInfo hostInfo;
        hostInfo.address = QHostAddress(QString::fromStdString(ip));
        hostInfo.serviceId = client.value("serviceID", 0);

        IdnSettings idnSettings;
        idnSettings.iface = QHostAddress(QString::fromStdString(client.value("iface", "127.0.0.1")));
        idnSettings.universe = client.value("universe", 1);
        idnSettings.unitName = QString::fromStdString(client.value("unitName", ""));
        idnSettings.serviceName = QString::fromStdString(client.value("serviceName", ""));
        idnSettings.serviceID = client.value("serviceID", 0);
        idnSettings.serviceType = client.value("serviceType", 0);
        idnSettings.port = client.value("port", IDN_PORT);
        idnSettings.mode = client.value("mode", 4);
        idnSettings.idnChannel = client.value("idnChannel", 0);
        idnSettings.rangeBegin = client.value("rangeBegin", 1);
        idnSettings.rangeEnd = client.value("rangeEnd", 512);
        idnSettings.scan = client.value("scan", false);
        idnSettings.disabled = client.value("disabled", false);

        m_manualClients[hostInfo] = idnSettings;
      }
    }

    return m_manualClients;
  }
  catch (const std::exception &e)
  {
    qWarning() << "Error loading settings: " << e.what();
    return m_manualClients;
  }
}

void IdnPlugin::setSetting(QHash<IdnHostInfo, IdnSettings> settings){
    m_manualClients.clear();
    if(settings.size() > 0) {
      m_manualClients = settings;

      QHashIterator<IdnHostInfo, IdnSettings> it(settings);
      while (it.hasNext()) {
          it.next();
      } 
    }

    nlohmann::json jsonSettings;

  for (auto it = settings.constBegin(); it != settings.constEnd(); ++it)
  {
    const IdnHostInfo &hostInfo = it.key();
    const IdnSettings &idnSettings = it.value();

    nlohmann::json item;
    item["iface"] = idnSettings.iface.toString().toStdString();
    item["universe"] = idnSettings.universe;
    item["unitName"] = idnSettings.unitName.toStdString();
    item["serviceName"] = idnSettings.serviceName.toStdString();
    item["serviceID"] = idnSettings.serviceID;
    item["serviceType"] = idnSettings.serviceType;
    item["port"] = idnSettings.port;
    item["mode"] = idnSettings.mode;
    item["idnChannel"] = idnSettings.idnChannel;
    item["rangeBegin"] = idnSettings.rangeBegin;
    item["rangeEnd"] = idnSettings.rangeEnd;
    item["scan"] = idnSettings.scan;
    item["disabled"] = idnSettings.disabled;
    jsonSettings[hostInfo.address.toString().toStdString()][QString::number(hostInfo.serviceId).toStdString()] = item;
  }

  std::ofstream file(settingsFilePath().toStdString());
  if (!file.is_open())
  {
    qWarning() << "[IDN] setSetting: could not write settings file:" << settingsFilePath();
    return;
  }
  file << jsonSettings.dump(4);
}

QHash<IdnHostInfo, IdnSettings> IdnPlugin::getSetting(){
  return m_manualClients;
}




/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(idn, IdnPlugin)
#endif
