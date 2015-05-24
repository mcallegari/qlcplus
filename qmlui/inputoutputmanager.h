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
class Universe;
class InputOutputMap;

class InputOutputManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<Universe> universes READ universes CONSTANT)
    Q_PROPERTY(QStringList universeNames READ universeNames CONSTANT)

public:
    InputOutputManager(Doc *doc, QObject *parent = 0);

    QQmlListProperty<Universe> universes();
    QStringList universeNames() const;
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
    /** List of the current Fixtures in Doc */
    QList<Universe *> m_universeList;
    /** List of the available input sources of a universe */
    QList<QObject*> m_inputSources;
    /** List of the available input profiles of a universe */
    QList<QObject*> m_inputProfiles;
    /** List of the available output sources of a universe */
    QList<QObject*> m_outputSources;

    QQuickItem *m_selectedItem;
    int m_selectedUniverseIndex;
};

#endif // INPUTOUTPUTMANAGER_H
