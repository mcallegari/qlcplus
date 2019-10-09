/*
  Q Light Controller Plus
  palettemanager.cpp

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

#include <QQmlContext>

#include "palettemanager.h"
#include "qlcpalette.h"
#include "doc.h"

PaletteManager::PaletteManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    m_view->rootContext()->setContextProperty("paletteManager", this);
    qmlRegisterUncreatableType<QLCPalette>("org.qlcplus.classes", 1, 0, "QLCPalette", "Can't create a QLCPalette!");
}

PaletteManager::~PaletteManager()
{

}

QVariantList PaletteManager::paletteList()
{
    QVariantList list;

    for (QLCPalette *palette : m_doc->palettes())
        list.append(QVariant::fromValue(palette));

    return list;
}

void PaletteManager::createPalette(int type, QString name, QVariant value1, QVariant value2)
{
    QLCPalette *palette = new QLCPalette(QLCPalette::PaletteType(type));
    palette->setName(name);

    if (type == QLCPalette::Position)
        palette->setValue(value1, value2);
    else
        palette->setValue(value1);

    if (m_doc->addPalette(palette) == false)
    {
        qWarning() << "Failed to add palette";
        delete palette;
    }
}


