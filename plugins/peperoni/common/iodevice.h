/*
  Q Light Controller
  iodevice.h

  Copyright (c) Heikki Junnila

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

#ifndef IODEVICE_H
#define IODEVICE_H

#include <QVariant>
#include <QObject>

class IODevice : public QObject
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    IODevice(const QVariant& uid, const QString& name, QObject* parent = 0);
    virtual ~IODevice();

    QVariant uid() const;
    QString name() const;

private:
    const QVariant m_uid;
    const QString m_name;

    /************************************************************************
     * Virtual Open/Close
     ************************************************************************/
public:
    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
};

#endif
