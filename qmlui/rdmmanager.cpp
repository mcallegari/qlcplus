/*
  Q Light Controller Plus
  rdmmanager.cpp

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

#include <QQmlContext>
#include <QDebug>

#include "rdmmanager.h"
#include "fixturemanager.h"
#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "serviceloader.h"
#include "listmodel.h"
#include "universe.h"
#include "doc.h"

#define DEFAULT_GRID_WIDTH  6
#define DEFAULT_GRID_HEIGHT 4
#define MAX_GRID_SIZE       50

/** Lower case and keep only letters and numbers, to compare names written in different ways */
static QString normalizedName(const QString &name)
{
    QString result;
    for (const QChar &c : name.toLower())
    {
        if (c.isLetterOrNumber())
            result.append(c);
    }
    return result;
}

RDMManager::RDMManager(QQuickView *view, Doc *doc, FixtureManager *fixtureManager, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_fixtureManager(fixtureManager)
    , m_service(nullptr)
    , m_discoveryStatus(Idle)
    , m_gridWidth(DEFAULT_GRID_WIDTH)
    , m_gridHeight(DEFAULT_GRID_HEIGHT)
{
    m_view->rootContext()->setContextProperty("rdmManager", this);
    qmlRegisterUncreatableType<RDMManager>("org.qlcplus.classes", 1, 0, "RDMManager", "Can't create a RDMManager!");

    QStringList listRoles;
    listRoles << "UID" << "manufacturer" << "modelName" << "dmxAddress" << "dmxChannels"
              << "personality" << "universe" << "ipAddress" << "hasDefinition" << "defMode"
              << "occupied" << "cellIndex";

    m_fixtureList = new ListModel(this);
    m_fixtureList->setRoleNames(listRoles);
    m_gridList = new ListModel(this);
    m_gridList->setRoleNames(listRoles);

    m_cells.fill(QString(), m_gridWidth * m_gridHeight);

    // the RDM service is an optional plugin: it might be already loaded or not
    connect(m_doc->serviceLoader(), &ServiceLoader::serviceAvailable,
            this, &RDMManager::slotServiceAvailable);
    bindService();
}

RDMManager::~RDMManager()
{
}

bool RDMManager::available() const
{
    return m_service != nullptr;
}

void RDMManager::slotServiceAvailable(const QString &id)
{
    if (id == QLCRDM_SERVICE_ID)
        bindService();
}

void RDMManager::bindService()
{
    if (m_service != nullptr)
        return;

    QObject *serviceObject = m_doc->serviceLoader()->service(QLCRDM_SERVICE_ID);
    QLCRDMService *service = qobject_cast<QLCRDMService*>(serviceObject);
    if (service == nullptr)
        return;

    m_service = service;

    // the service is a QObject implementing the QLCRDMService interface
    connect(serviceObject, SIGNAL(deviceFound(QLCRDMDevice)),
            this, SLOT(slotDeviceFound(QLCRDMDevice)));
    connect(serviceObject, SIGNAL(message(QString, QString)),
            this, SLOT(slotMessage(QString, QString)));
    connect(serviceObject, SIGNAL(busyChanged(bool)),
            this, SLOT(slotBusyChanged(bool)));

    emit availableChanged();
}

void RDMManager::startDiscovery()
{
    if (m_service == nullptr)
        return;

    m_devices.clear();
    m_cells.fill(QString());
    emit fixturesFoundChanged();
    setDiscoveryStatus(Running);

    // the service launches a discovery on every patched line supporting RDM
    m_service->discoverAll();
}

int RDMManager::fixturesFound()
{
    return m_devices.count();
}

QVariant RDMManager::fixtureList()
{
    return QVariant::fromValue(m_fixtureList);
}

QVariant RDMManager::gridList()
{
    return QVariant::fromValue(m_gridList);
}

void RDMManager::setDiscoveryStatus(DiscoveryStatus status)
{
    if (status == m_discoveryStatus)
        return;

    m_discoveryStatus = status;
    emit discoveryStatusChanged();
}

RDMManager::DiscoveryStatus RDMManager::discoveryStatus() const
{
    return m_discoveryStatus;
}

/*********************************************************************
 * Discovered fixtures
 *********************************************************************/

void RDMManager::slotDeviceFound(QLCRDMDevice device)
{
    DeviceEntry entry;
    entry.device = device;
    matchDefinition(entry);
    m_devices[device.uid] = entry;

    emit fixturesFoundChanged();

    // while the discovery is running the lists are built at the end,
    // afterwards (e.g. after the addresses assignment) keep them up to date
    if (m_discoveryStatus != Running)
        updateLists();
}

void RDMManager::slotMessage(QString title, QString text)
{
    qWarning() << "[RDM]" << title << text;
}

void RDMManager::slotBusyChanged(bool busy)
{
    if (busy)
        return;

    // the lists are rebuilt when the discovery ends only: other requests (e.g. identify)
    // end while the user is interacting with the fixtures and must not disturb the view
    if (m_discoveryStatus == Running)
    {
        setDiscoveryStatus(Finished);
        updateLists();
    }
}

void RDMManager::matchDefinition(DeviceEntry &entry) const
{
    entry.hasDefinition = false;

    const QString nManuf = normalizedName(entry.device.manufacturer);
    const QString nModel = normalizedName(entry.device.model);
    const QString nPersonality = normalizedName(entry.device.personality);

    if (nModel.isEmpty())
        return;

    QLCFixtureDefCache *cache = m_doc->fixtureDefCache();

    const QStringList manufacturers = cache->manufacturers();
    for (const QString &manufacturer : manufacturers)
    {
        // RDM manufacturer labels are written in many ways: accept partial matches
        const QString nName = normalizedName(manufacturer);
        if (nManuf.isEmpty() == false && nName != nManuf &&
            nName.contains(nManuf) == false && nManuf.contains(nName) == false)
            continue;

        const QStringList models = cache->models(manufacturer);
        for (const QString &model : models)
        {
            if (normalizedName(model) != nModel)
                continue;

            QLCFixtureDef *def = cache->fixtureDef(manufacturer, model);
            if (def == nullptr)
                continue;

            // the personality name first, then a mode with the same channels count
            QString modeName;
            const QList<QLCFixtureMode*> modes = def->modes();
            for (QLCFixtureMode *mode : modes)
            {
                if (nPersonality.isEmpty() == false && normalizedName(mode->name()) == nPersonality)
                {
                    modeName = mode->name();
                    break;
                }
            }
            if (modeName.isEmpty() && entry.device.channels > 0)
            {
                for (QLCFixtureMode *mode : modes)
                {
                    if (mode->channels().count() == entry.device.channels)
                    {
                        modeName = mode->name();
                        break;
                    }
                }
            }

            if (modeName.isEmpty())
                continue;

            entry.hasDefinition = true;
            entry.defManufacturer = def->manufacturer();
            entry.defModel = def->model();
            entry.defMode = modeName;
            return;
        }
    }
}

/*********************************************************************
 * Layout grid
 *********************************************************************/

int RDMManager::gridWidth() const
{
    return m_gridWidth;
}

void RDMManager::setGridWidth(int width)
{
    resizeGrid(width, m_gridHeight);
}

int RDMManager::gridHeight() const
{
    return m_gridHeight;
}

void RDMManager::setGridHeight(int height)
{
    resizeGrid(m_gridWidth, height);
}

void RDMManager::resizeGrid(int width, int height)
{
    width = qBound(1, width, MAX_GRID_SIZE);
    height = qBound(1, height, MAX_GRID_SIZE);

    if (width == m_gridWidth && height == m_gridHeight)
        return;

    // keep every fixture in its row and column. The ones that
    // don't fit anymore go back to the fixtures area
    QVector<QString> newCells(width * height);
    for (int y = 0; y < qMin(height, m_gridHeight); y++)
    {
        for (int x = 0; x < qMin(width, m_gridWidth); x++)
            newCells[y * width + x] = m_cells.at(y * m_gridWidth + x);
    }

    QStringList removed;
    for (const QString &uid : m_cells)
    {
        if (uid.isEmpty() == false && newCells.contains(uid) == false)
            removed.append(uid);
    }

    m_cells = newCells;
    m_gridWidth = width;
    m_gridHeight = height;

    releaseFixtures(removed);

    emit gridSizeChanged();
    updateLists();
}

void RDMManager::releaseFixtures(const QStringList &uids)
{
    if (m_service != nullptr && uids.isEmpty() == false)
        m_service->cancelOperations(uids);
}

int RDMManager::placedCount() const
{
    int count = 0;
    for (const QString &uid : m_cells)
    {
        if (uid.isEmpty() == false)
            count++;
    }
    return count;
}

void RDMManager::placeFixture(QString UID, int cellIndex)
{
    if (m_devices.contains(UID) == false || cellIndex < 0 || cellIndex >= m_cells.count())
        return;

    const int previousIndex = m_cells.indexOf(UID);
    if (previousIndex == cellIndex)
        return;

    const QString other = m_cells.at(cellIndex);
    m_cells[cellIndex] = UID;

    // a fixture coming from the grid swaps with the one in the target cell,
    // a fixture coming from the fixtures area sends the other one back there
    if (previousIndex >= 0)
        m_cells[previousIndex] = other;
    else if (other.isEmpty() == false)
        releaseFixtures(QStringList() << other);

    updateLists();
}

void RDMManager::unplaceFixture(QString UID)
{
    const int index = m_cells.indexOf(UID);
    if (index < 0)
        return;

    m_cells[index].clear();
    releaseFixtures(QStringList() << UID);
    updateLists();
}

void RDMManager::setIdentify(QString UID, bool on)
{
    if (m_service == nullptr)
        return;

    // PID_IDENTIFY_DEVICE (0x1000) takes a single byte
    m_service->writePid(UID, "0x1000", QLCRDMService::ByteArg, on ? "1" : "0");
}

void RDMManager::autoPatch(int startAddress, int gap)
{
    if (m_service == nullptr)
        return;

    QStringList ordered;
    for (const QString &uid : m_cells)
    {
        if (uid.isEmpty() == false)
            ordered.append(uid);
    }

    if (ordered.isEmpty())
        return;

    // the new addresses come back with deviceFound
    m_service->autoPatch(ordered, startAddress, gap);
}

/*********************************************************************
 * Apply
 *********************************************************************/

bool RDMManager::applyLayout()
{
    if (m_service == nullptr)
        return false;

    // send the new addresses to the physical fixtures
    m_service->executeOperations();

    bool result = true;

    for (int i = 0; i < m_cells.count(); i++)
    {
        const QString uid = m_cells.at(i);
        if (uid.isEmpty())
            continue;

        const DeviceEntry &entry = m_devices.value(uid);
        const QLCRDMDevice &dev = entry.device;

        QString manufacturer = entry.defManufacturer;
        QString model = entry.defModel;
        QString mode = entry.defMode;
        QString name = dev.model.isEmpty() ? QString("RDM fixture") : dev.model;

        // fixtures not found in the library are added as generic dimmers
        if (entry.hasDefinition == false)
        {
            manufacturer = "Generic";
            model = "Generic Dimmer";
            mode.clear();
        }

        // RDM addresses start from 1. The 2D view position is in grid units,
        // one unit per cell, with the first cell centered at the grid origin
        const int column = i % m_gridWidth;
        const int row = i / m_gridWidth;

        if (m_fixtureManager->addFixture(manufacturer, model, mode, name,
                                         dev.universe, qMax(0, int(dev.address) - 1),
                                         qMax(1, int(dev.channels)), 1, 0,
                                         column + 0.5, row + 0.5) == false)
        {
            qWarning() << "[RDM] Cannot add fixture" << uid;
            result = false;
        }
    }

    return result;
}

/*********************************************************************
 * Models
 *********************************************************************/

QVariantMap RDMManager::fixtureMap(const QString &UID) const
{
    QVariantMap map;

    if (UID.isEmpty())
    {
        map["UID"] = "";
        map["manufacturer"] = "";
        map["modelName"] = "";
        map["dmxAddress"] = 0;
        map["dmxChannels"] = 0;
        map["personality"] = "";
        map["universe"] = 0;
        map["ipAddress"] = "";
        map["hasDefinition"] = false;
        map["defMode"] = "";
        map["occupied"] = false;
        return map;
    }

    const DeviceEntry &entry = m_devices.value(UID);
    const QLCRDMDevice &dev = entry.device;

    map["UID"] = UID;
    map["manufacturer"] = dev.manufacturer;
    map["modelName"] = dev.model;
    map["dmxAddress"] = dev.address;
    map["dmxChannels"] = dev.channels;
    map["personality"] = dev.personality;
    map["universe"] = dev.universe;
    map["ipAddress"] = dev.ipAddress;
    map["hasDefinition"] = entry.hasDefinition;
    map["defMode"] = entry.defMode;
    map["occupied"] = true;
    return map;
}

void RDMManager::updateLists()
{
    m_fixtureList->clear();
    m_gridList->clear();

    for (auto i = m_devices.cbegin(), end = m_devices.cend(); i != end; ++i)
    {
        if (m_cells.contains(i.key()) == false)
        {
            QVariantMap fxMap = fixtureMap(i.key());
            fxMap["cellIndex"] = -1;
            m_fixtureList->addDataMap(fxMap);
        }
    }

    for (int c = 0; c < m_cells.count(); c++)
    {
        QVariantMap cellMap = fixtureMap(m_cells.at(c));
        cellMap["cellIndex"] = c;
        m_gridList->addDataMap(cellMap);
    }

    emit fixtureListChanged();
    emit gridListChanged();
}
