/*
  Q Light Controller Plus
  fixturebrowser.h

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

#ifndef FIXTUREBROWSER_H
#define FIXTUREBROWSER_H

#include <QQuickView>
#include <QDebug>

class QLCFixtureMode;
class QLCFixtureDef;
class Doc;

class FixtureBrowser : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer NOTIFY manufacturerChanged)
    Q_PROPERTY(QStringList manufacturers READ manufacturers)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QStringList models READ models)
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QStringList modes READ modes)
    Q_PROPERTY(int modeChannels READ modeChannels NOTIFY modeChannelsChanged)

public:
    FixtureBrowser(QQuickView *view, Doc *doc, QObject *parent = 0);

    QString manufacturer() const { return m_manufacturer; }
    void setManufacturer(QString manufacturer) { m_manufacturer = manufacturer; }
    QStringList manufacturers();

    QString model() const { return m_model; }
    void setModel(QString model);
    QStringList models();

    QString mode() const;
    void setMode(QString name);
    QStringList modes();

    int modeChannels();

signals:
    void manufacturerChanged();
    void modelChanged();
    void modeChanged();
    void modeChannelsChanged();

protected slots:
    //void slotUiEditorLoaded();

private:
    Doc *m_doc;
    QQuickView *m_view;
    QString m_manufacturer;
    QString m_model;
    QLCFixtureDef *m_definition;
    QLCFixtureMode *m_mode;
};

#endif // FIXTUREBROWSER_H
