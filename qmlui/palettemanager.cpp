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
#include <QQmlEngine>

#include "palettemanager.h"
#include "contextmanager.h"
#include "listmodel.h"
#include "scene.h"
#include "doc.h"

PaletteManager::PaletteManager(QQuickView *view, Doc *doc,
                               ContextManager *ctxManager, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_contextManager(ctxManager)
    , m_typeFilter(QLCPalette::Undefined)
    , m_searchFilter(QString())
{
    m_view->rootContext()->setContextProperty("paletteManager", this);
    qmlRegisterUncreatableType<QLCPalette>("org.qlcplus.classes", 1, 0, "QLCPalette", "Can't create a QLCPalette!");

    m_paletteList = new ListModel(this);
    QStringList listRoles;
    listRoles << "paletteID" << "isSelected";
    m_paletteList->setRoleNames(listRoles);

    m_dimmerCount = m_colorCount = m_positionCount = m_position3DCount = 0;

    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));

    // Refresh the list when palettes are added/removed outside this manager
    // (e.g. by the Stage Wizard).
    connect(m_doc, &Doc::paletteAdded,   this, [this]() { updatePaletteList(); });
    connect(m_doc, &Doc::paletteRemoved, this, [this]() { updatePaletteList(); });
}

PaletteManager::~PaletteManager()
{
}

QVariant PaletteManager::paletteList() const
{
    return QVariant::fromValue(m_paletteList);
}

QLCPalette *PaletteManager::getPalette(quint32 id) const
{
    QLCPalette *palette = m_doc->palette(id);
    QQmlEngine::setObjectOwnership(palette, QQmlEngine::CppOwnership);
    return palette;
}

QLCPalette *PaletteManager::getEditingPalette(int type)
{
    qDebug() << "Requesting a palette for editing. Type" << type;

    if (m_editingMap.contains(type) == false)
    {
        QLCPalette *palette = new QLCPalette(QLCPalette::PaletteType(type));
        m_editingMap[type] = palette;
        QQmlEngine::setObjectOwnership(palette, QQmlEngine::CppOwnership);
    }

    return m_editingMap.value(type);
}

bool PaletteManager::releaseEditingPalette(int type)
{
    if (m_editingMap.contains(type))
    {
        QLCPalette *palette = m_editingMap.take(type);
        delete palette;
        return true;
    }

    return false;
}

quint32 PaletteManager::createPalette(QLCPalette *palette, QString name)
{
    if (palette == nullptr)
        return QLCPalette::invalidId();

    QVariantList pValues = palette->values();
    QLCPalette *newPalette = palette->createCopy();
    newPalette->setName(name);

    if (pValues.isEmpty())
        pValues.append(0);

    if (palette->type() == QLCPalette::Pan)
    {
        newPalette->resetValues();
        newPalette->setValue(pValues.at(0));
    }
    else if (palette->type() == QLCPalette::Tilt)
    {
        newPalette->resetValues();
        if (pValues.count() == 2)
            newPalette->setValue(pValues.at(1));
        else
            newPalette->setValue(pValues.at(0));
    }

    if (m_doc->addPalette(newPalette) == false)
    {
        qWarning() << "Failed to add palette";
        delete palette;
    }

    updatePaletteList();

    return newPalette->id();
}

quint32 PaletteManager::createPosition3DPalette(QString name, float x, float y, float z)
{
    QLCPalette *newPalette = new QLCPalette(QLCPalette::Position3D);
    newPalette->setName(name);
    newPalette->setValue(x, y, z);

    if (m_doc->addPalette(newPalette) == false)
    {
        qWarning() << "Failed to add Position3D palette";
        delete newPalette;
        return QLCPalette::invalidId();
    }

    updatePaletteList();
    return newPalette->id();
}

void PaletteManager::previewPalette(const QLCPalette *palette)
{
    if (palette == nullptr)
        return;

    m_contextManager->setChannelValues(palette->valuesFromFixtures(m_doc, m_contextManager->selectedFixtureIDList()));
}

void PaletteManager::updatePalette(QLCPalette *palette, QVariant value1)
{
    if (palette == nullptr)
        return;

    qDebug() << "[PaletteManager] Single value" << value1;
    QVariantList prevValues = palette->values();
    palette->setValue(value1);
    if (prevValues != palette->values() && palette->isTemporary() == false)
        m_doc->setModified();
}

void PaletteManager::updatePalette(QLCPalette *palette, QVariant value1, QVariant value2)
{
    if (palette == nullptr)
        return;

    qDebug() << "[PaletteManager] Double value" << value1 << value2;
    QVariantList prevValues = palette->values();
    palette->setValue(value1, value2);
    if (prevValues != palette->values() && palette->isTemporary() == false)
        m_doc->setModified();
}

void PaletteManager::updatePalette(QLCPalette *palette, QVariant value1, QVariant value2, QVariant value3)
{
    if (palette == nullptr)
        return;

    qDebug() << "[PaletteManager] Triple value" << value1 << value2 << value3;
    QVariantList prevValues = palette->values();
    palette->setValue(value1, value2, value3);
    if (prevValues != palette->values() && palette->isTemporary() == false)
        m_doc->setModified();
}

void PaletteManager::deletePalettes(QVariantList list)
{
    for (QVariant pID : list)
    {
        QLCPalette *p = m_doc->palette(pID.toInt());
        if (p == nullptr)
            continue;

        m_doc->deletePalette(pID.toInt());
    }

    updatePaletteList();
}

void PaletteManager::addPaletteToNewScene(quint32 id, QString sceneName)
{
    // check for palette existence
    if (m_doc->palette(id) == nullptr)
        return;

    Scene *scene = new Scene(m_doc);
    scene->setName(sceneName);
    scene->addPalette(id);
    m_doc->addFunction(scene);
}

int PaletteManager::typeFilter() const
{
    return int(m_typeFilter);
}

void PaletteManager::setTypeFilter(quint32 filter)
{
    if (filter == m_typeFilter)
        return;

    m_typeFilter = filter;

    updatePaletteList();
    emit typeFilterChanged();
}

void PaletteManager::setTypeFilter(int type, bool enable)
{
    quint32 newFilter = enable ? (m_typeFilter | quint32(type))
                               : (m_typeFilter & ~quint32(type));
    setTypeFilter(newFilter);
}

QString PaletteManager::searchFilter() const
{
    return m_searchFilter;
}

void PaletteManager::setSearchFilter(QString searchFilter)
{
    if (m_searchFilter == searchFilter)
        return;

    int currLen = m_searchFilter.length();

    m_searchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS ||
        (currLen >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS))
            updatePaletteList();

    emit searchFilterChanged();
}

QStringList PaletteManager::selectedItemNames(QVariantList list) const
{
    QStringList names;

    for (QVariant pID : list)
    {
        QLCPalette *p = m_doc->palette(pID.toInt());
        if (p == nullptr)
            continue;
        names.append(p->name());
    }

    return names;
}

void PaletteManager::updatePaletteList()
{
    m_paletteList->clear();
    m_dimmerCount = m_colorCount = m_positionCount = m_position3DCount = 0;

    for (QLCPalette *palette : m_doc->palettes())
    {
        // counts are always over all palettes, independent of the active filter
        switch (palette->type())
        {
            case QLCPalette::Dimmer:     m_dimmerCount++;     break;
            case QLCPalette::Color:      m_colorCount++;      break;
            case QLCPalette::Pan:
            case QLCPalette::Tilt:
            case QLCPalette::PanTilt:    m_positionCount++;   break;
            case QLCPalette::Position3D: m_position3DCount++; break;
            default: break;
        }

        if ((m_typeFilter == QLCPalette::Undefined || m_typeFilter & palette->type()) &&
            (m_searchFilter.length() < SEARCH_MIN_CHARS || palette->name().toLower().contains(m_searchFilter)))
        {
            QVariantMap funcMap;
            funcMap.insert("paletteID", palette->id());
            funcMap.insert("isSelected", false);
            m_paletteList->addDataMap(funcMap);
        }
    }

    emit dimmerCountChanged();
    emit colorCountChanged();
    emit positionCountChanged();
    emit position3DCountChanged();

    emit paletteListChanged();
}

void PaletteManager::slotDocLoaded()
{
    updatePaletteList();
}
