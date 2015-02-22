/*
  Q Light Controller Plus
  MenuBarEntry.qml

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

#ifndef INPUTOUTPUTOBJECT_H
#define INPUTOUTPUTOBJECT_H

#include <QObject>

class InputOutputObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString line READ line CONSTANT)
    Q_PROPERTY(QString plugin READ plugin CONSTANT)

public:
    InputOutputObject(QString name, QString line, QString plugin, QObject *parent=0)
        : QObject(parent)
        , m_name(name)
        , m_line(line)
        , m_plugin(plugin)
        { }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString line() const { return m_line; }
    void setLine(const QString &line) { m_line = line; }

    QString plugin() const { return m_plugin; }
    void setPlugin(const QString &plugin) { m_plugin = plugin; }

private:
    QString m_name;
    QString m_line;
    QString m_plugin;
};

#endif // INPUTOUTPUTOBJECT_H
