/*
  Q Light Controller Plus
  modeedit.h

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

#ifndef MODEEDIT_H
#define MODEEDIT_H

#include <QQuickView>

class QLCFixtureMode;
class PhysicalEdit;

class ModeEdit : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QVariantList channels READ channels NOTIFY channelsChanged)
    Q_PROPERTY(bool useGlobalPhysical READ useGlobalPhysical CONSTANT)
    Q_PROPERTY(PhysicalEdit *physical READ physical CONSTANT)

public:
    ModeEdit(QLCFixtureMode *mode, QObject *parent = nullptr);
    ~ModeEdit();

    /** Get/Set the name of the mode being edited */
    QString name() const;
    void setName(QString name);

    /** Return if the selected mode is using global or overridden
     *  physical information */
    bool useGlobalPhysical();

    /** Get an editor reference for the
     *  override physical properties */
    PhysicalEdit *physical();

    /** Get a list of all the available channels in the definition */
    QVariantList channels() const;

signals:
    void nameChanged();
    void channelsChanged();

private:
    /** Reference to the mode being edited */
    QLCFixtureMode *m_mode;
    /** Reference to the override physical properties */
    PhysicalEdit *m_physical;
};

#endif /* MODEEDIT_H */
