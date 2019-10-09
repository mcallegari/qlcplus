/*
  Q Light Controller Plus
  palettemanager.h

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

#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include <QQuickView>
#include <QObject>

class Doc;
class ListModel;

class PaletteManager : public QObject
{
    Q_OBJECT

public:
    PaletteManager(QQuickView *view, Doc *doc, QObject *parent = nullptr);
    ~PaletteManager();

    Q_INVOKABLE QVariantList paletteList();

    Q_INVOKABLE void createPalette(int type, QString name, QVariant value1, QVariant value2);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
};

#endif // PALETTEMANAGER_H
