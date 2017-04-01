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
#include <Qt3DRender/QGeometryRenderer>

#include "doc.h"
#include "qlcconfig.h"
#include "mainview3d.h"
#include "qlccapability.h"
#include "qlcfixturemode.h"
#include "monitorproperties.h"

MainView3D::MainView3D(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "3D", parent)
{
    setContextResource("qrc:/3DView.qml");
    setContextTitle(tr("3D View"));

    qRegisterMetaType<Qt3DCore::QEntity*>();

    m_monProps = m_doc->monitorProperties();
    m_scene3D = NULL;
    m_rootEntity = NULL;

    fixtureComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/Fixture3DItem.qml"));
    if (fixtureComponent->isError())
        qDebug() << fixtureComponent->errors();
}

MainView3D::~MainView3D()
{
    resetItems();
}

void MainView3D::enableContext(bool enable)
{
    qDebug() << "Enable 3D context..." << enable;

    PreviewContext::enableContext(enable);
    if (enable == true)
        slotRefreshView();
    else
    {
        resetItems();
        m_scene3D = NULL;
        m_rootEntity = NULL;
    }
}

void MainView3D::setUniverseFilter(quint32 universeFilter)
{
    PreviewContext::setUniverseFilter(universeFilter);
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
    }

    //const auto end = m_entitiesMap.end();
    //for (auto it = m_entitiesMap.begin(); it != end; ++it)
    //    delete it.value();
    m_entitiesMap.clear();
}

void MainView3D::createFixtureItem(quint32 fxID, qreal x, qreal y, qreal z, bool mmCoords)
{
    Q_UNUSED(mmCoords)

    if (isEnabled() == false)
        return;

    if (m_scene3D == NULL || m_rootEntity == NULL)
        initialize3DProperties();

    qDebug() << "[MainView3D] Creating fixture with ID" << fxID << "x:" << x << "y:" << y << "z:" << z;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    QEntity *newFixtureItem = qobject_cast<QEntity *>(fixtureComponent->create());

    if (newFixtureItem != NULL)
    {
        QString meshPath = "file://" + QString(MESHESDIR) + "/fixtures";
        if (fixture->type() == QLCFixtureDef::ColorChanger)
            meshPath.append("/par.dae");
        else if (fixture->type() == QLCFixtureDef::MovingHead)
            meshPath.append("/moving_head.dae");

        newFixtureItem->setProperty("fixtureID", fxID);
        newFixtureItem->setProperty("itemSource", meshPath);

        newFixtureItem->setParent(m_rootEntity);

        FixtureMesh *mesh = new FixtureMesh;
        mesh->m_rootItem = newFixtureItem;
        mesh->m_rootTransform = NULL;
        mesh->m_armItem = NULL;
        mesh->m_headItem = NULL;
        m_entitiesMap[fxID] = mesh;
    }
}

void MainView3D::updateFixture(Fixture *fixture)
{
    if (m_enabled == false || fixture == NULL)
        return;

    if (m_entitiesMap.contains(fixture->id()) == false)
        return;

    QEntity *fxItem = m_entitiesMap[fixture->id()]->m_rootItem;

    bool setPosition = false;
    int panDegrees = 0;
    int tiltDegrees = 0;

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
            default:
            break;
        }
    }

    if (setPosition == true)
    {
        QMetaObject::invokeMethod(fxItem, "setPosition",
                Q_ARG(QVariant, panDegrees),
                Q_ARG(QVariant, tiltDegrees));
    }
}

void MainView3D::updateFixtureSelection(QList<quint32> fixtures)
{
    Q_UNUSED(fixtures)
}

void MainView3D::updateFixtureSelection(quint32 fxID, bool enable)
{
    qDebug() << "[View3D] fixture" << fxID << "selected:" << enable;

    // TODO: show/hide bounding box
}

void MainView3D::updateFixturePosition(quint32 fxID, QVector3D pos)
{
    if (isEnabled() == false || m_entitiesMap.contains(fxID) == false)
        return;

    FixtureMesh *mesh = m_entitiesMap.value(fxID);
    if (mesh->m_rootTransform == NULL)
        return;

    qDebug() << Q_FUNC_INFO << pos;

    mesh->m_rootTransform->setTranslation(QVector3D(pos.x() / 1000.0, pos.y() / 1000.0, pos.z() / 1000.0));
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

void MainView3D::createView()
{
    QMetaObject::invokeMethod(this, "slotRefreshView", Qt::QueuedConnection);
}

void MainView3D::slotRefreshView()
{
    if (isEnabled() == false)
        return;

    resetItems();

    if (m_scene3D == NULL || m_rootEntity == NULL)
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

void MainView3D::initializeFixture(quint32 fxID, QComponent *picker, Qt3DRender::QSceneLoader *loader)
{
    if (m_entitiesMap.contains(fxID) == false)
        return;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    QLCPhysical phy;
    QLCFixtureMode *fxMode = fixture->fixtureMode();

    if (fxMode != NULL)
        phy = fxMode->physical();

    // The QSceneLoader instance is a component of an entity. The loaded scene
    // tree is added under this entity.
    QVector<QEntity *> entities = loader->entities();

    if (entities.isEmpty())
        return;

    // Technically there could be multiple entities referencing the scene loader
    // but sharing is discouraged, and in our case there will be one anyhow.
    QEntity *root = entities[0];
#if 0
    for (QComponent *component : root->components()) // C++11
    {
        //qDebug() << component->metaObject()->className();
        Qt3DRender::QGeometryRenderer *renderer = qobject_cast<Qt3DRender::QGeometryRenderer*>(component);
        if (renderer)
        {
        }
    }
#endif
    qDebug() << "There are" << root->children().count() << "submeshes in the loaded fixture";

    FixtureMesh *meshRef = m_entitiesMap.value(fxID);

    QEntity *baseItem = root->findChild<QEntity *>("base");
    meshRef->m_armItem = root->findChild<QEntity *>("arm");
    meshRef->m_headItem = root->findChild<QEntity *>("head");

    if (baseItem != NULL)
        meshRef->m_rootTransform = getTransform(baseItem);

    if (meshRef->m_armItem != NULL)
    {
        int panDeg = phy.focusPanMax();
        if (panDeg == 0) panDeg = 360;

        qDebug() << "Fixture" << fxID << "has an arm entity !";
        if (fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB) != QLCChannel::invalid())
        {
            Qt3DCore::QTransform *transform = getTransform(meshRef->m_armItem);
            if (transform != NULL)
                QMetaObject::invokeMethod(loader, "bindPanTransform",
                        Q_ARG(QVariant, QVariant::fromValue(transform)),
                        Q_ARG(QVariant, panDeg));
        }
    }
#if 1
    if (meshRef->m_headItem != NULL)
    {
        int tiltDeg = phy.focusTiltMax();
        if (tiltDeg == 0) tiltDeg = 270;

        qDebug() << "Fixture" << fxID << "has an head entity !";
        Qt3DCore::QTransform *transform = getTransform(meshRef->m_headItem);

        if (baseItem != NULL)
        {
            if (fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB) != QLCChannel::invalid())
            {
                /* If there is a base item and a tilt channel,
                 * this is either a moving head or a scanner */
                if (transform != NULL)
                    QMetaObject::invokeMethod(loader, "bindTiltTransform",
                            Q_ARG(QVariant, QVariant::fromValue(transform)),
                            Q_ARG(QVariant, tiltDeg));
            }
        }
        else
        {
            meshRef->m_rootTransform = transform;
            baseItem = meshRef->m_headItem;
        }
    }
#endif
    if (m_monProps->hasFixturePosition(fixture->id()))
    {
        QVector3D fxPos = m_monProps->fixturePosition(fixture->id());
        meshRef->m_rootTransform->setTranslation(QVector3D(fxPos.x() / 1000.0, fxPos.y() / 1000.0, fxPos.z() / 1000.0));
    }

    /* Hook the object picker to the base entity */
    picker->setParent(baseItem);
    baseItem->addComponent(picker);
}

void MainView3D::initialize3DProperties()
{
    m_scene3D = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("scene3DItem"));

    qDebug() << Q_FUNC_INFO << m_scene3D;

    if (m_scene3D == NULL)
    {
        qDebug() << "Scene3DItem not found !";
        return;
    }

    m_rootEntity = m_scene3D->property("entity").value<Qt3DCore::QEntity *>();

    qDebug() << Q_FUNC_INFO << m_rootEntity;

    if (m_rootEntity == NULL)
    {
        qDebug() << "m_rootEntity not found !";
        return;
    }
}


