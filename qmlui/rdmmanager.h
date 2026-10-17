/*
  Q Light Controller Plus
  rdmmanager.h

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

#ifndef RDMMANAGER_H
#define RDMMANAGER_H

#include <QQuickView>
#include <QVariant>
#include <QVector>
#include <QMap>

#include "qlcrdmservice.h"

class FixtureManager;
class QLCFixtureDef;
class ListModel;
class Doc;

/**
 * The QML side of the RDM fixtures import. The RDM service plugin discovers the fixtures
 * and assigns their DMX addresses: this class only presents them, keeps track of where
 * the user placed them in the layout grid and, at the end, creates the project fixtures.
 * The RDM service is optional (see available).
 */
class RDMManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool available READ available NOTIFY availableChanged FINAL)
    Q_PROPERTY(DiscoveryStatus discoveryStatus READ discoveryStatus WRITE setDiscoveryStatus NOTIFY discoveryStatusChanged FINAL)
    Q_PROPERTY(int fixturesFound READ fixturesFound NOTIFY fixturesFoundChanged FINAL)
    Q_PROPERTY(QVariant fixtureList READ fixtureList NOTIFY fixtureListChanged)
    Q_PROPERTY(QVariant gridList READ gridList NOTIFY gridListChanged)
    Q_PROPERTY(int gridWidth READ gridWidth WRITE setGridWidth NOTIFY gridSizeChanged FINAL)
    Q_PROPERTY(int gridHeight READ gridHeight WRITE setGridHeight NOTIFY gridSizeChanged FINAL)
    Q_PROPERTY(int placedCount READ placedCount NOTIFY gridListChanged FINAL)

public:
    explicit RDMManager(QQuickView *view, Doc *doc, FixtureManager *fixtureManager, QObject *parent = 0);
    ~RDMManager();

    enum DiscoveryStatus
    {
        Idle = 0,
        Running,
        Finished
    };
    Q_ENUM(DiscoveryStatus)

    /** True when the RDM service plugin is installed and loaded */
    bool available() const;

    Q_INVOKABLE void startDiscovery();

    /** Move a fixture to the given cell of the layout grid. If the cell is occupied,
     *  the two fixtures are swapped (or the other fixture goes back to the fixtures area) */
    Q_INVOKABLE void placeFixture(QString UID, int cellIndex);

    /** Take a fixture out of the layout grid, back to the fixtures area */
    Q_INVOKABLE void unplaceFixture(QString UID);

    /** Switch on/off the identify function of a fixture (e.g. flashing its light) */
    Q_INVOKABLE void setIdentify(QString UID, bool on);

    /** Assign DMX addresses to the fixtures in the grid, following the grid order */
    Q_INVOKABLE void autoPatch(int startAddress, int gap);

    /** Send the DMX addresses to the fixtures and add them to the project,
     *  placed in the 2D view following the layout grid. Returns false on error */
    Q_INVOKABLE bool applyLayout();

    int fixturesFound();

    /** Fixtures discovered and not yet placed in the layout grid */
    QVariant fixtureList();

    /** The layout grid cells, in row by row order */
    QVariant gridList();

    int gridWidth() const;
    void setGridWidth(int width);
    int gridHeight() const;
    void setGridHeight(int height);

    /** Number of fixtures placed in the layout grid */
    int placedCount() const;

    DiscoveryStatus discoveryStatus() const;
    void setDiscoveryStatus(DiscoveryStatus status);

protected slots:
    /** Look for the RDM service when a service plugin gets loaded */
    void slotServiceAvailable(const QString &id);

    void slotDeviceFound(QLCRDMDevice device);
    void slotMessage(QString title, QString text);
    void slotBusyChanged(bool busy);

private:
    /** A discovered fixture and its match in the fixture definitions library */
    struct DeviceEntry
    {
        QLCRDMDevice device;
        bool hasDefinition = false;
        QString defManufacturer;
        QString defModel;
        QString defMode;
    };

    /** Get the RDM service from the service loader, if loaded */
    void bindService();

    /** Look for a fixture definition and mode matching the given device */
    void matchDefinition(DeviceEntry &entry) const;

    void resizeGrid(int width, int height);

    /** Fixtures taken out of the layout grid get back the address they had before the autopatch */
    void releaseFixtures(const QStringList &uids);

    QVariantMap fixtureMap(const QString &UID) const;
    void updateLists();

signals:
    void availableChanged();
    void fixtureListChanged();
    void gridListChanged();
    void gridSizeChanged();
    void discoveryStatusChanged();
    void fixturesFoundChanged();

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Used to add the fixtures to the project */
    FixtureManager *m_fixtureManager;

    /** The RDM service plugin, nullptr if not installed */
    QLCRDMService *m_service;

    /** The models exposed to QML, with the roles:
     *  UID | manufacturer | modelName | dmxAddress | dmxChannels | personality | universe |
     *  ipAddress | hasDefinition | defMode | occupied | cellIndex
     */
    ListModel *m_fixtureList;
    ListModel *m_gridList;

    DiscoveryStatus m_discoveryStatus;

    /** Map of all the discovered fixtures by UID */
    QMap <QString, DeviceEntry> m_devices;

    /** The layout grid: the UID of the fixture in each cell, empty if free */
    int m_gridWidth;
    int m_gridHeight;
    QVector<QString> m_cells;
};
#endif
