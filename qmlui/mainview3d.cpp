/*
  Q Light Controller Plus
  mainview3d.cpp

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

#include <QDebug>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlComponent>

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>

#include "doc.h"
#include "qlcfile.h"
#include "qlcconfig.h"
#include "mainview3d.h"
#include "qlccapability.h"
#include "qlcfixturemode.h"
#include "monitorproperties.h"

MainView3D::MainView3D(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "3D", parent)
    , m_monProps(doc->monitorProperties())
    , m_scene3D(NULL)
    , m_sceneRootEntity(NULL)
    , m_quadMaterial(NULL)
    , m_ambientIntensity(0.8)
{
    setContextResource("qrc:/3DView.qml");
    setContextTitle(tr("3D View"));

    qRegisterMetaType<Qt3DCore::QEntity*>();

    m_fixtureComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/Fixture3DItem.qml"));
    if (m_fixtureComponent->isError())
        qDebug() << m_fixtureComponent->errors();

    m_selectionComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/SelectionEntity.qml"));
    if (m_selectionComponent->isError())
        qDebug() << m_selectionComponent->errors();

    setStageSize(m_monProps->gridSize());

    // the following two lists must always have the same items number
    m_stagesList << tr("Simple ground") << tr("Simple box") << tr("Rock stage");
    m_stageResourceList << "qrc:/StageSimple.qml" << "qrc:/StageBox.qml" << "qrc:/StageRock.qml";

    m_stageEntity = NULL;
    m_stageIndex = 0;
}

MainView3D::~MainView3D()
{
    resetItems();
}

void MainView3D::enableContext(bool enable)
{
    qDebug() << "Enable 3D context..." << enable;

    PreviewContext::enableContext(enable);
    if (enable == false)
    {
        resetItems();
        m_scene3D = NULL;
        m_sceneRootEntity = NULL;
        m_quadEntity = NULL;
        m_quadMaterial = NULL;

        if (m_stageEntity)
        {
            delete m_stageEntity;
            m_stageEntity = NULL;
        }
    }
}

void MainView3D::slotRefreshView()
{
    if (isEnabled() == false)
        return;

    resetItems();

    initialize3DProperties();

    qDebug() << "Refreshing 3D view...";

    foreach(Fixture *fixture, m_doc->fixtures())
    {
        if (m_monProps->hasFixturePosition(fixture->id()))
        {
            QVector3D fxPos = m_monProps->fixturePosition(fixture->id());
            createFixtureItem(fixture->id(), fxPos.x(), fxPos.y(), fxPos.z());
        }
        else
            createFixtureItem(fixture->id(), 0, 0, 0, false);
    }
}

void MainView3D::resetItems()
{
    qDebug() << "Resetting 3D items...";
    QMapIterator<quint32, FixtureMesh*> it(m_entitiesMap);
    while(it.hasNext())
    {
        it.next();
        FixtureMesh *e = it.value();
        //if (e->m_headItem)
        //    delete e->m_headItem;
        //if (e->m_armItem)
        //    delete e->m_armItem;
        delete e->m_rootItem;
        delete e->m_selectionBox;
    }

    //const auto end = m_entitiesMap.end();
    //for (auto it = m_entitiesMap.begin(); it != end; ++it)
    //    delete it.value();
    m_entitiesMap.clear();
}

QString MainView3D::meshDirectory() const
{
    QDir dir = QDir::cleanPath(QLCFile::systemDirectory(MESHESDIR).path());
    //qDebug() << "Absolute mesh path: " << dir.absolutePath();
    return QString("file:///") + dir.absolutePath() + QDir::separator();
}

void MainView3D::setUniverseFilter(quint32 universeFilter)
{
    PreviewContext::setUniverseFilter(universeFilter);

    /* TODO: hide/show fixture meshes */
}

/*********************************************************************
 * Fixtures
 *********************************************************************/

void MainView3D::initialize3DProperties()
{
    m_scene3D = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("scene3DItem"));

    qDebug() << Q_FUNC_INFO << m_scene3D;

    if (m_scene3D == NULL)
    {
        qDebug() << "Scene3DItem not found !";
        return;
    }

    m_sceneRootEntity = m_scene3D->findChild<QEntity *>("sceneRootEntity");
    if (m_sceneRootEntity == NULL)
    {
        qDebug() << "sceneRootEntity not found !";
        return;
    }

    m_quadEntity = m_scene3D->findChild<QEntity *>("quadEntity");
    if (m_quadEntity == NULL)
    {
        qDebug() << "quadEntity not found !";
        return;
    }

    m_quadMaterial = m_quadEntity->findChild<QMaterial *>("lightPassMaterial");
    if (m_quadMaterial == NULL)
    {
        qDebug() << "lightPassMaterial not found !";
        return;
    }

    qDebug() << m_sceneRootEntity << m_quadEntity << m_quadMaterial;

    if (m_stageEntity == NULL)
    {
        QQmlComponent *stageComponent = new QQmlComponent(m_view->engine(), QUrl(m_stageResourceList.at(m_stageIndex)));
        if (stageComponent->isError())
            qDebug() << stageComponent->errors();

        m_stageEntity = qobject_cast<QEntity *>(stageComponent->create());
        m_stageEntity->setParent(m_sceneRootEntity);

        QLayer *sceneDeferredLayer = m_sceneRootEntity->property("deferredLayer").value<QLayer *>();
        QEffect *sceneEffect = m_sceneRootEntity->property("geometryPassEffect").value<QEffect *>();
        m_stageEntity->setProperty("sceneLayer", QVariant::fromValue(sceneDeferredLayer));
        m_stageEntity->setProperty("effect", QVariant::fromValue(sceneEffect));
    }

    setStageSize(m_monProps->gridSize());
}

void MainView3D::sceneReady()
{
    qDebug() << "Scene entity ready";
}

void MainView3D::quadReady()
{
    qDebug() << "Quad material ready";

    QMetaObject::invokeMethod(this, "slotRefreshView", Qt::QueuedConnection);
}

void MainView3D::resetStage(QEntity *entity)
{
    if (entity == NULL)
        return;

    for (QEntity *child : entity->findChildren<QEntity *>())
    {
        QVariant prop = child->property("isDynamic");
        if (prop.isValid() && prop.toBool() == true)
            delete child;
    }
}

void MainView3D::slotCreateFixture(quint32 fxID)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    QString meshPath = meshDirectory() + "fixtures" + QDir::separator();
    if (fixture->type() == QLCFixtureDef::ColorChanger)
        meshPath.append("par.dae");
    else if (fixture->type() == QLCFixtureDef::MovingHead)
        meshPath.append("moving_head.dae");

    QEntity *newItem = qobject_cast<QEntity *>(m_fixtureComponent->create());
    newItem->setProperty("fixtureID", fxID);
    newItem->setProperty("itemSource", meshPath);
    newItem->setParent(m_sceneRootEntity);
}

void MainView3D::createFixtureItem(quint32 fxID, qreal x, qreal y, qreal z, bool mmCoords)
{
    Q_UNUSED(mmCoords)

    if (isEnabled() == false)
        return;

    if (m_quadEntity == NULL)
        initialize3DProperties();

    qDebug() << "[MainView3D] Creating fixture with ID" << fxID << "x:" << x << "y:" << y << "z:" << z;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    FixtureMesh *mesh = new FixtureMesh;
    mesh->m_rootItem = NULL;
    mesh->m_rootTransform = NULL;
    mesh->m_armItem = NULL;
    mesh->m_headItem = NULL;
    mesh->m_lightIndex = getNewLightIndex();
    mesh->m_selectionBox = NULL;
    m_entitiesMap[fxID] = mesh;

    QMetaObject::invokeMethod(this, "slotCreateFixture", Qt::QueuedConnection, Q_ARG(quint32, fxID));
}

Qt3DCore::QTransform *MainView3D::getTransform(QEntity *entity)
{
    if (entity == NULL)
        return NULL;

    for (QComponent *component : entity->components()) // C++11
    {
        //qDebug() << component->metaObject()->className();
        Qt3DCore::QTransform *transform = qobject_cast<Qt3DCore::QTransform *>(component);
        if (transform)
        {
            //qDebug() << "matrix:" << transform->matrix();
            //qDebug() << "translation:" << transform->translation();
            //qDebug() << "rotation:" << transform->rotation();
            //qDebug() << "scale:" << transform->scale3D();
            return transform;
        }
    }

    return NULL;
}

QMaterial *MainView3D::getMaterial(QEntity *entity)
{
    if (entity == NULL)
        return NULL;

    for (QComponent *component : entity->components()) // C++11
    {
        //qDebug() << component->metaObject()->className();
        QMaterial *material = qobject_cast<QMaterial *>(component);
        if (material)
            return material;
    }

    return NULL;
}

unsigned int MainView3D::getNewLightIndex()
{
    unsigned int newIdx = 0;

    for (FixtureMesh *mesh : m_entitiesMap.values())
    {
        if (mesh->m_lightIndex >= newIdx)
            newIdx = mesh->m_lightIndex + 1;
    }

    return newIdx;
}

void MainView3D::updateLightPosition(FixtureMesh *meshRef)
{
    if (meshRef == NULL)
        return;

    QVector3D newLightPos = meshRef->m_rootTransform->translation();
    if (meshRef->m_armItem)
    {
        Qt3DCore::QTransform *armTransform = getTransform(meshRef->m_armItem);
        newLightPos += armTransform->translation();
    }
    if (meshRef->m_headItem)
    {
        Qt3DCore::QTransform *headTransform = getTransform(meshRef->m_headItem);
        newLightPos += headTransform->translation();
    }
    meshRef->m_rootItem->setProperty("lightPosition", newLightPos);
}

QVector3D MainView3D::lightPosition(quint32 fixtureID)
{
    FixtureMesh *meshRef = m_entitiesMap.value(fixtureID, NULL);
    if (meshRef == NULL)
        return QVector3D();

    return meshRef->m_rootItem->property("lightPosition").value<QVector3D>();
}

void MainView3D::getMeshCorners(QGeometryRenderer *mesh,
                                QVector3D &minCorner,
                                QVector3D &maxCorner)
{
    minCorner = QVector3D();
    maxCorner =  QVector3D();

    QGeometry *meshGeometry = mesh->geometry();

    if (!meshGeometry)
        return;

    Qt3DRender::QAttribute *vPosAttribute = nullptr;
    for (Qt3DRender::QAttribute *attribute : meshGeometry->attributes())
    {
        if (attribute->name() == Qt3DRender::QAttribute::defaultPositionAttributeName())
        {
            vPosAttribute = attribute;
            break;
        }
    }
    if (vPosAttribute)
    {
        const float *bufferPtr =
                reinterpret_cast<const float *>(vPosAttribute->buffer()->data().constData());
        uint stride = vPosAttribute->byteStride() / sizeof(float);
        uint offset = vPosAttribute->byteOffset() / sizeof(float);
        bufferPtr += offset;
        uint vertexCount = vPosAttribute->count();
        uint dataCount = vPosAttribute->buffer()->data().size() / sizeof(float);

        // Make sure we have valid data
        if (((vertexCount * stride) + offset) > dataCount)
            return;

        float minX = FLT_MAX;
        float minY = FLT_MAX;
        float minZ = FLT_MAX;
        float maxX = -FLT_MAX;
        float maxY = -FLT_MAX;
        float maxZ = -FLT_MAX;

        if (stride)
            stride = stride - 3; // Three floats per vertex

        for (uint i = 0; i < vertexCount; i++)
        {
            float xVal = *bufferPtr++;
            minX = qMin(xVal, minX);
            maxX = qMax(xVal, maxX);
            float yVal = *bufferPtr++;
            minY = qMin(yVal, minY);
            maxY = qMax(yVal, maxY);
            float zVal = *bufferPtr++;
            minZ = qMin(zVal, minZ);
            maxZ = qMax(zVal, maxZ);
            bufferPtr += stride;
        }

        minCorner = QVector3D(minX, minY, minZ);
        maxCorner = QVector3D(maxX, maxY, maxZ);
    }
}

void MainView3D::addVolumes(FixtureMesh *meshRef, QVector3D minCorner, QVector3D maxCorner)
{
    if (meshRef == NULL)
        return;

    // calculate the current bounding volume minimum/maximum positions
    float mminX = meshRef->m_volume.m_center.x() - (meshRef->m_volume.m_extents.x() / 2.0f);
    float mminY = meshRef->m_volume.m_center.y() - (meshRef->m_volume.m_extents.y() / 2.0f);
    float mminZ = meshRef->m_volume.m_center.z() - (meshRef->m_volume.m_extents.z() / 2.0f);
    float mmaxX = meshRef->m_volume.m_center.x() + (meshRef->m_volume.m_extents.x() / 2.0f);
    float mmaxY = meshRef->m_volume.m_center.y() + (meshRef->m_volume.m_extents.y() / 2.0f);
    float mmaxZ = meshRef->m_volume.m_center.z() + (meshRef->m_volume.m_extents.z() / 2.0f);

    //qDebug() << "volume pos" << mminX << mminY << mminZ << mmaxX << mmaxY << mmaxZ;

    // determine the minimum/maximum vertices positions between two volumes
    float vminX = qMin(mminX, minCorner.x());
    float vminY = qMin(mminY, minCorner.y());
    float vminZ = qMin(mminZ, minCorner.z());
    float vmaxX = qMax(mmaxX, maxCorner.x());
    float vmaxY = qMax(mmaxY, maxCorner.y());
    float vmaxZ = qMax(mmaxZ, maxCorner.z());

    //qDebug() << "vertices pos" << vminX << vminY << vminZ << vmaxX << vmaxY << vmaxZ;

    meshRef->m_volume.m_extents = QVector3D(vmaxX - vminX, vmaxY - vminY, vmaxZ - vminZ);
    meshRef->m_volume.m_center = QVector3D(vminX + meshRef->m_volume.m_extents.x() / 2.0f,
                                           vminY + meshRef->m_volume.m_extents.y() / 2.0f,
                                           vminZ + meshRef->m_volume.m_extents.z() / 2.0f);

    qDebug() << "-- extent" << meshRef->m_volume.m_extents << "-- center" << meshRef->m_volume.m_center;
}

QEntity *MainView3D::inspectEntity(QEntity *entity, FixtureMesh *meshRef,
                                   QLayer *layer, QEffect *effect,
                                   bool calculateVolume, QVector3D translation)
{
    if (entity == NULL)
        return NULL;

    QEntity *baseItem = NULL;
    QGeometryRenderer *geom = NULL;

    for (QComponent *component : entity->components()) // C++11
    {
        //qDebug() << component->metaObject()->className();

        QMaterial *material = qobject_cast<QMaterial *>(component);
        Qt3DCore::QTransform *transform = qobject_cast<Qt3DCore::QTransform *>(component);
        if (geom == NULL)
            geom = qobject_cast<QGeometryRenderer *>(component);

        if (material)
            material->setEffect(effect);

        if (transform)
            translation += transform->translation();
    }

    if (geom && calculateVolume)
    {
        QVector3D minCorner, maxCorner;
        getMeshCorners(geom, minCorner, maxCorner);
        minCorner += translation;
        maxCorner += translation;
        qDebug() << "Entity" << entity->objectName() << "translation:" << translation << ", min:" << minCorner << ", max:" << maxCorner;

        addVolumes(meshRef, minCorner, maxCorner);
    }

    for (QEntity *subEntity : entity->findChildren<QEntity *>(QString(), Qt::FindDirectChildrenOnly))
        baseItem = inspectEntity(subEntity, meshRef, layer, effect, calculateVolume, translation);

    entity->addComponent(layer);

    if (entity->objectName() == "base")
        baseItem = entity;
    else if (entity->objectName() == "arm")
        meshRef->m_armItem = entity;
    else if (entity->objectName() == "head")
        meshRef->m_headItem = entity;

    return baseItem;
}

void MainView3D::initializeFixture(quint32 fxID, QEntity *fxEntity, QComponent *picker, QSceneLoader *loader)
{
    if (m_entitiesMap.contains(fxID) == false)
        return;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    QLCPhysical phy;
    QLCFixtureMode *fxMode = fixture->fixtureMode();
    int panDeg = 0;
    int tiltDeg = 0;
    bool calculateVolume = false;

    if (fxMode != NULL)
    {
        phy = fxMode->physical();

        panDeg = phy.focusPanMax();
        if (panDeg == 0)
            panDeg = 360;

        tiltDeg = phy.focusTiltMax();
        if (tiltDeg == 0)
            tiltDeg = 270;
    }

    qDebug() << "Initialize fixture" << fixture->id();

    // The QSceneLoader instance is a component of an entity. The loaded scene
    // tree is added under this entity.
    QVector<QEntity *> entities = loader->entities();

    if (entities.isEmpty())
        return;

    // Technically there could be multiple entities referencing the scene loader
    // but sharing is discouraged, and in our case there will be one anyhow.
    QEntity *root = entities[0];

    qDebug() << "There are" << root->children().count() << "components in the loaded fixture";

    fxEntity->setParent(m_sceneRootEntity);

    QLayer *sceneDeferredLayer = m_sceneRootEntity->property("deferredLayer").value<QLayer *>();
    QEffect *sceneEffect = m_sceneRootEntity->property("geometryPassEffect").value<QEffect *>();

    QVector3D translation;
    FixtureMesh *meshRef = m_entitiesMap.value(fxID);
    meshRef->m_rootItem = fxEntity;
    meshRef->m_rootTransform = getTransform(meshRef->m_rootItem);

    // If this model has been already loaded, re-use the cached bounding volume
    if (m_boundingVolumesMap.contains(loader->source()))
        meshRef->m_volume = m_boundingVolumesMap[loader->source()];
    else
        calculateVolume = true;

    // Walk through the scene tree and add each mesh to the deferred pipeline.
    // If needed, calculate also the bounding volume */
    QEntity *baseItem = inspectEntity(root, meshRef, sceneDeferredLayer, sceneEffect, calculateVolume, translation);

    qDebug() << "Calculated volume" << meshRef->m_volume.m_extents << meshRef->m_volume.m_center;

    if (calculateVolume)
        m_boundingVolumesMap[loader->source()] = meshRef->m_volume;

    if (meshRef->m_armItem)
    {
        qDebug() << "Fixture" << fxID << "has an arm entity";
        if (fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB) != QLCChannel::invalid())
        {
            Qt3DCore::QTransform *transform = getTransform(meshRef->m_armItem);
            if (transform != NULL)
                QMetaObject::invokeMethod(meshRef->m_rootItem, "bindPanTransform",
                        Q_ARG(QVariant, QVariant::fromValue(transform)),
                        Q_ARG(QVariant, panDeg));
        }
    }

    if (meshRef->m_headItem)
    {
        qDebug() << "Fixture" << fxID << "has a head entity";
        Qt3DCore::QTransform *transform = getTransform(meshRef->m_headItem);

        if (baseItem != NULL)
        {
            if (fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB) != QLCChannel::invalid())
            {
                // If there is a base item and a tilt channel,
                // this is either a moving head or a scanner
                if (transform != NULL)
                    QMetaObject::invokeMethod(meshRef->m_rootItem, "bindTiltTransform",
                            Q_ARG(QVariant, QVariant::fromValue(transform)),
                            Q_ARG(QVariant, tiltDeg));
            }
        }

        if (m_quadEntity)
        {
            QMetaObject::invokeMethod(m_quadMaterial, "addLight",
                    Q_ARG(QVariant, QVariant::fromValue(meshRef->m_rootItem)),
                    Q_ARG(QVariant, meshRef->m_lightIndex + 1),
                    Q_ARG(QVariant, QVariant::fromValue(meshRef->m_rootTransform)));
        }
    }

    /* Set the fixture position */
    QVector3D fxPos = QVector3D(m_stageSize.x() * 500, 1000.0, m_stageSize.z() * 500);
    if (m_monProps->hasFixturePosition(fixture->id()))
        fxPos = m_monProps->fixturePosition(fixture->id());
    else
        m_monProps->setFixturePosition(fixture->id(), fxPos);

    updateFixturePosition(fxID, fxPos);

    /* Hook the object picker to the base entity */
    picker->setParent(meshRef->m_rootItem);
    meshRef->m_rootItem->addComponent(picker);

    updateFixture(fixture);

    QLayer *selectionLayer = m_sceneRootEntity->property("selectionLayer").value<QLayer *>();
    QGeometryRenderer *selectionMesh = m_sceneRootEntity->property("selectionMesh").value<QGeometryRenderer *>();

    meshRef->m_selectionBox = qobject_cast<QEntity *>(m_selectionComponent->create());
    meshRef->m_selectionBox->setParent(m_sceneRootEntity);

    meshRef->m_selectionBox->setProperty("selectionLayer", QVariant::fromValue(selectionLayer));
    meshRef->m_selectionBox->setProperty("geometryPassEffect", QVariant::fromValue(sceneEffect));
    meshRef->m_selectionBox->setProperty("selectionMesh", QVariant::fromValue(selectionMesh));

    meshRef->m_selectionBox->setProperty("extents", meshRef->m_volume.m_extents);
    meshRef->m_selectionBox->setProperty("center", meshRef->m_volume.m_center);

    if (meshRef->m_rootTransform != NULL)
        QMetaObject::invokeMethod(meshRef->m_selectionBox, "bindFixtureTransform",
                Q_ARG(QVariant, fixture->id()),
                Q_ARG(QVariant, QVariant::fromValue(meshRef->m_rootTransform)));
}

void MainView3D::updateFixture(Fixture *fixture)
{
    if (m_enabled == false || fixture == NULL)
        return;

    if (m_entitiesMap.contains(fixture->id()) == false)
        return;

    QEntity *fixtureItem = m_entitiesMap[fixture->id()]->m_rootItem;
    QColor color;

    bool setPosition = false;
    bool setColor = false;
    int panDegrees = 0;
    int tiltDegrees = 0;

    quint32 headDimmerIndex = fixture->channelNumber(QLCChannel::Intensity, QLCChannel::MSB);
    //qDebug() << "Head" << headIdx << "dimmer channel:" << mdIndex;
    qreal intValue = 1.0;
    if (headDimmerIndex != QLCChannel::invalid())
        intValue = (qreal)fixture->channelValueAt(headDimmerIndex) / 255;

    fixtureItem->setProperty("intensity", intValue);

    QVector <quint32> rgbCh = fixture->rgbChannels();
    if (rgbCh.size() == 3)
    {
        color = QColor(fixture->channelValueAt(rgbCh.at(0)),
                       fixture->channelValueAt(rgbCh.at(1)),
                       fixture->channelValueAt(rgbCh.at(2)));
        setColor = true;
    }

    QVector <quint32> cmyCh = fixture->cmyChannels();
    if (cmyCh.size() == 3)
    {
        color.setCmyk(fixture->channelValueAt(cmyCh.at(0)),
                      fixture->channelValueAt(cmyCh.at(1)),
                      fixture->channelValueAt(cmyCh.at(2)), 0);
        setColor = true;
    }

    // now scan all the channels for "common" capabilities
    for (quint32 i = 0; i < fixture->channels(); i++)
    {
        const QLCChannel *ch = fixture->channel(i);
        if (ch == NULL)
            continue;

        uchar value = fixture->channelValueAt(i);

        switch (ch->group())
        {
            case QLCChannel::Pan:
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    panDegrees += (value << 8);
                else
                    panDegrees += (value);
                setPosition = true;
            }
            break;
            case QLCChannel::Tilt:
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    tiltDegrees += (value << 8);
                else
                    tiltDegrees += (value);
                setPosition = true;
            }
            break;
            case QLCChannel::Colour:
            {
                if (value == 0)
                    break;

                foreach(QLCCapability *cap, ch->capabilities())
                {
                    if (value >= cap->min() && value <= cap->max())
                    {
                        if (cap->resourceColor1().isValid())
                        {
                            color = cap->resourceColor1();
                            setColor = true;
                        }
                        break;
                    }
                }
            }
            break;
            default:
            break;
        }
    }

    if (setPosition)
    {
        QMetaObject::invokeMethod(fixtureItem, "setPosition",
                Q_ARG(QVariant, panDegrees),
                Q_ARG(QVariant, tiltDegrees));
    }

    if (setColor)
        fixtureItem->setProperty("lightColor", color);
}

void MainView3D::updateFixtureSelection(QList<quint32> fixtures)
{
    QMapIterator<quint32, FixtureMesh*> it(m_entitiesMap);
    while(it.hasNext())
    {
        it.next();
        quint32 fxID = it.key();
        FixtureMesh *meshRef = m_entitiesMap.value(fxID);

        if(fixtures.contains(fxID))
            meshRef->m_selectionBox->setProperty("isSelected", true);
        else
            meshRef->m_selectionBox->setProperty("isSelected", false);
    }
}

void MainView3D::updateFixtureSelection(quint32 fxID, bool enable)
{
    qDebug() << "[View3D] fixture" << fxID << "selected:" << enable;

    FixtureMesh *meshRef = m_entitiesMap.value(fxID, NULL);
    if (meshRef)
        meshRef->m_selectionBox->setProperty("isSelected", enable);
}

void MainView3D::updateFixturePosition(quint32 fxID, QVector3D pos)
{
    if (isEnabled() == false || m_entitiesMap.contains(fxID) == false)
        return;

    FixtureMesh *mesh = m_entitiesMap.value(fxID);
    if (mesh->m_rootTransform == NULL)
        return;

    qDebug() << Q_FUNC_INFO << pos;

    /* move the root mesh first */
    mesh->m_rootTransform->setTranslation(QVector3D((pos.x() / 1000.0) - (m_stageSize.x() / 2),
                                                    pos.y() / 1000.0,
                                                    (pos.z() / 1000.0) - (m_stageSize.z() / 2)));
    /* recalculate the light position */
    if (mesh->m_headItem)
        updateLightPosition(mesh);
}

void MainView3D::updateFixtureRotation(quint32 fxID, QVector3D degrees)
{
    if (isEnabled() == false || m_entitiesMap.contains(fxID) == false)
        return;

    FixtureMesh *mesh = m_entitiesMap.value(fxID);
    if (mesh->m_rootTransform == NULL)
        return;

    qDebug() << Q_FUNC_INFO << degrees;

    mesh->m_rootTransform->setRotationX(degrees.x());
    mesh->m_rootTransform->setRotationY(degrees.y());
    mesh->m_rootTransform->setRotationZ(degrees.z());
}

void MainView3D::removeFixtureItem(quint32 fxID)
{
    if (isEnabled() == false || m_entitiesMap.contains(fxID) == false)
        return;

    FixtureMesh *mesh = m_entitiesMap.take(fxID);

    delete mesh->m_rootItem;
    delete mesh->m_selectionBox;

    delete mesh;
}

/*********************************************************************
 * Environment
 *********************************************************************/

QVector3D MainView3D::stageSize() const
{
    return m_stageSize;
}

void MainView3D::setStageSize(QVector3D stageSize)
{
    if (m_stageSize == stageSize)
        return;

    m_stageSize = stageSize;
    m_monProps->setGridSize(stageSize);
    emit stageSizeChanged(m_stageSize);
}

float MainView3D::ambientIntensity() const
{
    return m_ambientIntensity;
}

void MainView3D::setAmbientIntensity(float ambientIntensity)
{
    if (m_ambientIntensity == ambientIntensity)
        return;

    m_ambientIntensity = ambientIntensity;
    emit ambientIntensityChanged(m_ambientIntensity);
}

QStringList MainView3D::stagesList() const
{
    return m_stagesList;
}

int MainView3D::stageIndex() const
{
    return m_stageIndex;
}

void MainView3D::setStageIndex(int stageIndex)
{
    if (m_stageIndex == stageIndex)
        return;

    m_stageIndex = stageIndex;

    if (m_stageEntity)
        delete m_stageEntity;

    QQmlComponent *stageComponent = new QQmlComponent(m_view->engine(), QUrl(m_stageResourceList.at(m_stageIndex)));
    if (stageComponent->isError())
        qDebug() << stageComponent->errors();

    m_stageEntity = qobject_cast<QEntity *>(stageComponent->create());
    m_stageEntity->setParent(m_sceneRootEntity);

    QLayer *sceneDeferredLayer = m_sceneRootEntity->property("deferredLayer").value<QLayer *>();
    QEffect *sceneEffect = m_sceneRootEntity->property("geometryPassEffect").value<QEffect *>();
    m_stageEntity->setProperty("sceneLayer", QVariant::fromValue(sceneDeferredLayer));
    m_stageEntity->setProperty("effect", QVariant::fromValue(sceneEffect));

    emit stageIndexChanged(m_stageIndex);
}


