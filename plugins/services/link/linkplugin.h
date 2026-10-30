/*
  Q Light Controller Plus
  linkplugin.h

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

#ifndef LINKPLUGIN_H
#define LINKPLUGIN_H

#include <QObject>

#include "qlcserviceplugin.h"

class LinkService;

class LinkPlugin final : public QObject, public QLCServicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QLCServicePlugin_iid FILE "linkplugin.json")
    Q_INTERFACES(QLCServicePlugin)

public:
    LinkPlugin();
    ~LinkPlugin();

    /** @reimp */
    QString serviceId() const override;

    /** @reimp */
    QString name() const override;

    /** @reimp */
    QString version() const override;

    /** @reimp */
    int minHostApiVersion() const override;

    /** @reimp */
    bool init(QLCServiceHost *host) override;

    /** @reimp */
    QObject *service() override;

    /** @reimp */
    void shutdown() override;

private:
    LinkService *m_service;
};

#endif
