/*
  Q Light Controller Plus
  inputoutputmanager.h

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

#ifndef INPUTOUTPUTMANAGER_H
#define INPUTOUTPUTMANAGER_H

#include <QStringList>
#include <QQuickItem>
#include <QVariant>
#include <QObject>

class Doc;
class InputOutputMap;

class InputOutputManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList universes READ universes NOTIFY universesChanged)

public:
    InputOutputManager(Doc *doc, QObject *parent = 0);

    QStringList universes();
    Q_INVOKABLE QVariant audioInputSources();
    Q_INVOKABLE QVariant audioOutputSources();

    Q_INVOKABLE QVariant universeInputSources(int universe);
    Q_INVOKABLE QVariant universeOutputSources(int universe);
    Q_INVOKABLE QVariant universeInputProfiles(int universe);

    Q_INVOKABLE void setSelectedItem(QQuickItem *item, int index);

private:
    void clearInputList();
    void clearOutputList();

signals:
    void universesChanged();

public slots:

private:
    Doc *m_doc;
    InputOutputMap* m_ioMap;
    QList<QObject*> m_inputSources;
    QList<QObject*> m_inputProfiles;
    QList<QObject*> m_outputSources;

    QQuickItem *m_selectedItem;
    int m_selectedUniverseIndex;
};

#endif // INPUTOUTPUTMANAGER_H
