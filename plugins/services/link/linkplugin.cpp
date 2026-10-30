/*
  Q Light Controller Plus
  linkplugin.cpp

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

#include "linkplugin.h"
#include "linkservice.h"

#define LINK_PLUGIN_VERSION "1.0.0"

LinkPlugin::LinkPlugin()
    : m_service(nullptr)
{
}

LinkPlugin::~LinkPlugin()
{
    delete m_service;
}

QString LinkPlugin::serviceId() const
{
    return QString(QLCLINK_SERVICE_ID);
}

QString LinkPlugin::name() const
{
    return QString("Ableton Link");
}

QString LinkPlugin::version() const
{
    return QString(LINK_PLUGIN_VERSION);
}

int LinkPlugin::minHostApiVersion() const
{
    return 1;
}

bool LinkPlugin::init(QLCServiceHost *host)
{
    Q_UNUSED(host)

    /* Link stays disabled (no network traffic) until
     * the user selects it as beat generator */
    if (m_service == nullptr)
        m_service = new LinkService();
    return true;
}

QObject *LinkPlugin::service()
{
    return m_service;
}

void LinkPlugin::shutdown()
{
    delete m_service;
    m_service = nullptr;
}
