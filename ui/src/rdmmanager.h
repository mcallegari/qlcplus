/*
  Q Light Controller Plus
  rdmmanager.h

  Copyright (c) Massimo Callegari

  Licensed under the Apache License Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing software
  distributed under the License is distributed on an "AS IS" BASIS
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/


#ifndef RDMMANAGER_H
#define RDMMANAGER_H

#include "ui_rdmmanager.h"
#include "qlcrdmservice.h"

/**
 * The RDM view. It only displays what the RDM service plugin provides and forwards
 * the user requests to it: all the RDM logic lives in the service.
 */
class RDMManager final : public QWidget, public Ui_RDMManager
{
    Q_OBJECT

public:
    /** @param service the RDM service object, implementing QLCRDMService */
    explicit RDMManager(QWidget *parent, QObject *service);
    ~RDMManager();

private slots:
    /** Triggers the RDM discovery on all universes */
    void slotRefresh();

    /** Triggers the download of all the supported
     *  PID and information from the first selected fixture */
    void slotGetInfo();

    /** Read a single PID and return the result */
    void slotReadPID();

    /** Write a single PID and return the result */
    void slotWritePID();

    void slotDeviceFound(QLCRDMDevice device);

    void slotDeviceInfoReady(QString html);

    void slotSelectionChanged();

    void slotUpdatePidInfo(QString info);

    void slotDisplayMessage(QString title, QString text);

    void slotBusyChanged(bool busy);

signals:
    void fixtureInfoReady(QString &info);

private:
    QString selectedUID() const;

private:
    /** The RDM service plugin */
    QLCRDMService *m_service;
};

#endif // RDMMANAGER_H
