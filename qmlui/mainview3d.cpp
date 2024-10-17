/*
  Q Light Controller Plus
  mainview3d.cpp

  Copyright (c) Massimo Callegari
  Copyright (c) Eric Arnebäck

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
#include <QTexture>
#include <QPainter>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlComponent>
#include <QSvgRenderer>

#include <Qt3DCore/QTransform>
#include <Qt3DCore/QNode>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
 #include <Qt3DRender/QGeometry>
 #include <Qt3DRender/QAttribute>
 #include <Qt3DRender/QBuffer>
#else
 #include <Qt3DCore/QAttribute>
#endif
#include <Qt3DRender/QParameter>
#include <Qt3DExtras/QPhongMaterial>

#include "doc.h"
#include "tardis.h"
#include "qlcfile.h"
#include "qlcconfig.h"
#include "listmodel.h"
#include "mainview3d.h"
#include "fixtureutils.h"
#include "qlccapability.h"
#include "qlcfixturemode.h"
#include "monitorproperties.h"

//#define SHOW_FRAMEGRAPH

MainView3D::MainView3D(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "3D", parent)
    , m_monProps(doc->monitorProperties())
    , m_fixtureComponent(nullptr)
    , m_genericComponent(nullptr)
    , m_selectionComponent(nullptr)
    , m_spotlightConeComponent(nullptr)
    , m_fillGBufferLayer(nullptr)
    , m_createItemCount(0)
    , m_frameAction(nullptr)
    , m_frameCount(0)
    , m_minFrameCount(0)
    , m_maxFrameCount(0)
    , m_avgFrameCount(1.0)
    , m_scene3D(nullptr)
    , m_sceneRootEntity(nullptr)
    , m_quadEntity(nullptr)
    , m_gBuffer(nullptr)
    , m_latestGenericID(0)
    , m_renderQuality(HighQuality)
    , m_stageEntity(nullptr)
    , m_ambientIntensity(0.6)
    , m_smokeAmount(0.8)
{
    setContextResource("qrc:/3DView.qml");
    setContextTitle(tr("3D View"));

    qRegisterMetaType<Qt3DCore::QEntity*>();
    qmlRegisterUncreatableType<MainView3D>("org.qlcplus.classes", 1, 0, "MainView3D", "Can't create an MainView3D!");

    // the following two lists must always have the same items number and must respect
    // the order of StageType enum in MonitorProperties class
    m_stagesList << tr("Simple ground") << tr("Simple box") << tr("Rock stage") << tr("Theatre stage");
    m_stageResourceList << "qrc:/StageSimple.qml" << "qrc:/StageBox.qml" << "qrc:/StageRock.qml" << "qrc:/StageTheatre.qml";

    m_genericItemsList = new ListModel(this);
    QStringList listRoles;
    listRoles << "itemID" << "name" << "isSelected";
    m_genericItemsList->setRoleNames(listRoles);

    resetCameraPosition();
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
        m_scene3D = nullptr;
        m_sceneRootEntity = nullptr;
        m_quadEntity = nullptr;

        if (m_stageEntity)
        {
            delete m_stageEntity;
            m_stageEntity = nullptr;
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

    for (Fixture *fixture : m_doc->fixtures())
    {
        if (m_monProps->containsFixture(fixture->id()))
        {
            for (quint32 subID : m_monProps->fixtureIDList(fixture->id()))
            {
                quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
                quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
                createFixtureItem(fixture->id(), headIndex, linkedIndex,
                                  m_monProps->fixturePosition(fixture->id(), headIndex, linkedIndex), true);
            }
        }
        else
        {
            createFixtureItems(fixture->id(), QVector3D(0, 0, 0), false);
        }
    }

    for (quint32 itemID : m_monProps->genericItemsID())
    {
        QString path = m_monProps->itemResource(itemID);
        createGenericItem(path, itemID);
    }
}

void MainView3D::resetItems()
{
    qDebug() << "Resetting 3D items...";

    QMetaObject::invokeMethod(m_scene3D, "updateFrameGraph", Q_ARG(QVariant, false));

    QMapIterator<quint32, SceneItem*> it(m_entitiesMap);
    while (it.hasNext())
    {
        it.next();
        SceneItem *e = it.value();
        delete e->m_goboTexture;
        delete e->m_selectionBox;
        // delete e->m_rootItem; // TODO: with this -> segfault
        if (e->m_rootItem)
            e->m_rootItem->setProperty("enabled", false); // workaround for the above
        delete e;
    }

    //const auto end = m_entitiesMap.end();
    //for (auto it = m_entitiesMap.begin(); it != end; ++it)
    //    delete it.value();
    m_entitiesMap.clear();

    QMapIterator<int, SceneItem*> it2(m_genericMap);
    while (it2.hasNext())
    {
        it2.next();
        SceneItem *e = it2.value();
        delete e->m_rootItem;
    }
    m_genericMap.clear();
    m_latestGenericID = 0;
    m_createItemCount = 0;

    m_frameCount = 0;
    m_minFrameCount = 0;
    m_maxFrameCount = 0;
    m_avgFrameCount = 1.0;
    setFrameCountEnabled(false);
}

void MainView3D::resetCameraPosition()
{
    setCameraPosition(QVector3D(0.0, 3.0, 7.5));
    setCameraUpVector(QVector3D(0.0, 1.0, 0.0));
    setCameraViewCenter(QVector3D(0.0, 1.0, 0.0));
}

QVector3D MainView3D::cameraPosition() const
{
    return m_cameraPosition;
}

void MainView3D::setCameraPosition(const QVector3D &newCameraPosition)
{
    if (m_cameraPosition == newCameraPosition)
        return;
    m_cameraPosition = newCameraPosition;
    emit cameraPositionChanged();
}

QVector3D MainView3D::cameraUpVector() const
{
    return m_cameraUpVector;
}

void MainView3D::setCameraUpVector(const QVector3D &newCameraUpVector)
{
    if (m_cameraUpVector == newCameraUpVector)
        return;
    m_cameraUpVector = newCameraUpVector;
    emit cameraUpVectorChanged();
}

QVector3D MainView3D::cameraViewCenter() const
{
    return m_cameraViewCenter;
}

void MainView3D::setCameraViewCenter(const QVector3D &newCameraViewCenter)
{
    if (m_cameraViewCenter == newCameraViewCenter)
        return;
    m_cameraViewCenter = newCameraViewCenter;
    emit cameraViewCenterChanged();
}

QString MainView3D::meshDirectory() const
{
    QDir dir = QDir::cleanPath(QLCFile::systemDirectory(MESHESDIR).path());
    //qDebug() << "Absolute mesh path: " << dir.absolutePath();
    return QString(QLCFile::fileUrlPrefix()) + dir.absolutePath() + QDir::separator();
}

QString MainView3D::goboDirectory() const
{
    QDir dir = QDir::cleanPath(QLCFile::systemDirectory(GOBODIR).path());
    return dir.absolutePath();
}

void MainView3D::setUniverseFilter(quint32 universeFilter)
{
    PreviewContext::setUniverseFilter(universeFilter);

    QMapIterator<quint32, SceneItem*> it(m_entitiesMap);
    while (it.hasNext())
    {
        it.next();
        quint32 itemID = it.key();

        quint32 fxID = FixtureUtils::itemFixtureID(itemID);

        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == nullptr)
            return;

        SceneItem *meshRef = m_entitiesMap.value(itemID, nullptr);

        if (meshRef == nullptr || meshRef->m_rootItem == nullptr)
            continue;

        if (universeFilter == Universe::invalid() || fixture->universe() == (quint32)universeFilter)
        {
            meshRef->m_rootItem->setProperty("enabled", true);
            meshRef->m_selectionBox->setProperty("enabled", true);
        }
        else
        {
            meshRef->m_rootItem->setProperty("enabled", false);
            meshRef->m_selectionBox->setProperty("enabled", false);
        }
    }
}

/*********************************************************************
 * Frame counter
 *********************************************************************/

bool MainView3D::frameCountEnabled() const
{
    return m_frameAction != nullptr ? true : false;
}

void MainView3D::setFrameCountEnabled(bool enable)
{
    if (enable)
    {
        m_frameAction = new QFrameAction();
        connect(m_frameAction, &QFrameAction::triggered, this, &MainView3D::slotFrameProcessed);
        if (m_sceneRootEntity)
            m_sceneRootEntity->addComponent(m_frameAction);
        m_fpsElapsed.start();
    }
    else
    {
        if (m_frameAction)
        {
            disconnect(m_frameAction, &QFrameAction::triggered, this, &MainView3D::slotFrameProcessed);
            delete m_frameAction;
            m_frameAction = nullptr;
        }
        m_frameCount = 0;
        m_minFrameCount = 0;
        m_maxFrameCount = 0;
        m_avgFrameCount = 0;
    }
    emit frameCountEnabledChanged();
}

void MainView3D::slotFrameProcessed()
{
    m_frameCount++;
    if (m_fpsElapsed.elapsed() >= 1000)
    {
        emit FPSChanged(m_frameCount);
        if (m_minFrameCount == 0 || m_frameCount < m_minFrameCount)
        {
            m_minFrameCount = m_frameCount;
            emit minFPSChanged(m_minFrameCount);
        }
        if (m_frameCount > m_maxFrameCount)
        {
            m_maxFrameCount = m_frameCount;
            emit maxFPSChanged(m_maxFrameCount);
        }

        // this is an exponential moving average with an alpha of 0.92
        m_avgFrameCount = 0.92 * m_avgFrameCount + 0.08 * m_frameCount;
        emit avgFPSChanged(m_avgFrameCount);

        m_frameCount = 0;
        m_fpsElapsed.restart();
    }
}

/*********************************************************************
 * Fixtures
 *********************************************************************/

void MainView3D::initialize3DProperties()
{
    if (m_fixtureComponent == nullptr)
    {
        m_fixtureComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/Fixture3DItem.qml"));
        if (m_fixtureComponent->isError())
            qDebug() << m_fixtureComponent->errors();
    }

    if (m_genericComponent == nullptr)
    {
        m_genericComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/Generic3DItem.qml"));
        if (m_genericComponent->isError())
            qDebug() << m_genericComponent->errors();
    }

    if (m_selectionComponent == nullptr)
    {
        m_selectionComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/SelectionEntity.qml"));
        if (m_selectionComponent->isError())
            qDebug() << m_selectionComponent->errors();
    }

    m_scene3D = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("scene3DItem"));

    qDebug() << Q_FUNC_INFO << m_scene3D;

    if (m_scene3D == nullptr)
    {
        qDebug() << "Scene3DItem not found!";
        return;
    }

    m_scene3DEntity = m_scene3D->findChild<QEntity *>("scene3DEntity");
    if (m_scene3DEntity == nullptr)
    {
        qDebug() << "m_scene3DEntity not found!";
        return;
    }

    m_sceneRootEntity = m_scene3D->findChild<QEntity *>("sceneRootEntity");
    if (m_sceneRootEntity == nullptr)
    {
        qDebug() << "sceneRootEntity not found!";
        return;
    }

    m_quadEntity = m_scene3D->findChild<QEntity *>("quadEntity");
    if (m_quadEntity == nullptr)
    {
        qDebug() << "quadEntity not found!";
        return;
    }

    m_gBuffer = m_scene3D->findChild<QRenderTarget *>("gBuffer");
    if (m_gBuffer == nullptr)
    {
        qDebug() << "gBuffer not found!";
        return;
    }

    if (m_frameAction)
        m_sceneRootEntity->addComponent(m_frameAction);

    qDebug() << m_sceneRootEntity << m_quadEntity << m_gBuffer;

    if (m_stageEntity == nullptr)
        createStage();

    QMetaObject::invokeMethod(m_scene3D, "updateFrameGraph", Q_ARG(QVariant, true));
}

QString MainView3D::makeShader(QString str) {

   QString prefix = R"(#version 150
#define GL3

#ifdef GL3
#define DECLARE_GBUFFER_OUTPUT out vec4 [3] gOutput;
#define DECLARE_FRAG_COLOR out vec4 fragColor;
#define VS_IN_ATTRIB in
#define VS_OUT_ATTRIB out
#define FS_IN_ATTRIB in
#define MGL_FRAG_COLOR fragColor
#define MGL_FRAG_DATA0 gOutput[0]
#define MGL_FRAG_DATA1 gOutput[1]
#define MGL_FRAG_DATA2 gOutput[2]
#define SAMPLE_TEX3D texture
#define SAMPLE_TEX2D texture
#else
#define DECLARE_GBUFFER_OUTPUT
#define DECLARE_FRAG_COLOR
#define VS_IN_ATTRIB attribute
#define VS_OUT_ATTRIB varying
#define FS_IN_ATTRIB varying
#define MGL_FRAG_COLOR gl_FragColor
#define MGL_FRAG_DATA0 gl_FragData[0]
#define MGL_FRAG_DATA1 gl_FragData[1]
#define MGL_FRAG_DATA2 gl_FragData[2]
#define SAMPLE_TEX3D texture3D
#define SAMPLE_TEX2D texture2D
#endif

)";

    return prefix + str;
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
    if (entity == nullptr)
        return;

    for (QEntity *child : entity->findChildren<QEntity *>())
    {
        QVariant prop = child->property("isDynamic");
        if (prop.isValid() && prop.toBool() == true)
            delete child;
    }
}

void MainView3D::createFixtureItems(quint32 fxID, QVector3D pos, bool mmCoords)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return;

    if (fixture->type() == QLCFixtureDef::Dimmer)
    {
        for (quint32 i = 0; i < fixture->channels(); i++)
            createFixtureItem(fixture->id(), i, 0, pos, mmCoords);
    }
    else
    {
        createFixtureItem(fixture->id(), 0, 0, pos, mmCoords);
    }
}

void MainView3D::createFixtureItem(quint32 fxID, quint16 headIndex, quint16 linkedIndex,
                                   QVector3D pos, bool mmCoords)
{
    Q_UNUSED(mmCoords)

    if (isEnabled() == false)
        return;

    if (m_quadEntity == nullptr)
        initialize3DProperties();

    qDebug() << "[MainView3D] Creating fixture with ID" << fxID << "pos:" << pos;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return;

    QLCFixtureMode *fxMode = fixture->fixtureMode();
    QString meshPath = meshDirectory() + "fixtures" + QDir::separator();
    QString openGobo = goboDirectory() + QDir::separator() + "Others/open.svg";
    quint32 itemID = FixtureUtils::fixtureItemID(fxID, headIndex, linkedIndex);
    QEntity *newItem = nullptr;

    SceneItem *mesh = new SceneItem;
    mesh->m_rootItem = nullptr;
    mesh->m_rootTransform = nullptr;
    mesh->m_armItem = nullptr;
    mesh->m_headItem = nullptr;
    mesh->m_selectionBox = nullptr;
    m_createItemCount++;

    if (fixture->type() == QLCFixtureDef::LEDBarBeams)
    {
        mesh->m_goboTexture = new GoboTextureImage(512, 512, openGobo);

        QQmlComponent *lbComp = new QQmlComponent(m_view->engine(), QUrl("qrc:/MultiBeams3DItem.qml"));
        if (lbComp->isError())
            qDebug() << lbComp->errors();

        newItem = qobject_cast<QEntity *>(lbComp->create());
        if (newItem == nullptr)
        {
            qDebug() << "Fixture 3D item creation failed !!";
            return;
        }
        newItem->setProperty("headsNumber", fixture->heads());

        if (fxMode != nullptr)
        {
            QLCPhysical phy = fxMode->physical();
            if (phy.layoutSize() != QSize(1, 1))
                newItem->setProperty("headsLayout", phy.layoutSize());
        }
    }
    else if (fixture->type() == QLCFixtureDef::LEDBarPixels)
    {
        mesh->m_goboTexture = nullptr;

        QQmlComponent *lbComp = new QQmlComponent(m_view->engine(), QUrl("qrc:/PixelBar3DItem.qml")); // TODO
        if (lbComp->isError())
            qDebug() << lbComp->errors();

        newItem = qobject_cast<QEntity *>(lbComp->create());
        if (newItem == nullptr)
        {
            qDebug() << "Fixture 3D item creation failed !!";
            return;
        }
        newItem->setProperty("headsNumber", fixture->heads());

        if (fxMode != nullptr)
        {
            QLCPhysical phy = fxMode->physical();
            if (phy.layoutSize() != QSize(1, 1))
                newItem->setProperty("headsLayout", phy.layoutSize());
        }
    }
    else
    {
        mesh->m_goboTexture = new GoboTextureImage(512, 512, openGobo);

        newItem = qobject_cast<QEntity *>(m_fixtureComponent->create());
        if (newItem == nullptr)
        {
            qDebug() << "Fixture 3D item creation failed !!";
            return;
        }
    }
    newItem->setParent(m_sceneRootEntity);

    switch (fixture->type())
    {
        case QLCFixtureDef::ColorChanger:
        case QLCFixtureDef::Dimmer:
            meshPath.append("par.dae");
            newItem->setProperty("meshType", FixtureMeshType::ParMeshType);
        break;
        case QLCFixtureDef::MovingHead:
            meshPath.append("moving_head.dae");
            newItem->setProperty("meshType", FixtureMeshType::MovingHeadMeshType);
        break;
        case QLCFixtureDef::Scanner:
            meshPath.append("scanner.dae");
            newItem->setProperty("meshType", FixtureMeshType::ScannerMeshType);
        break;
        case QLCFixtureDef::Hazer:
            meshPath.append("hazer.dae");
            newItem->setProperty("meshType", FixtureMeshType::DefaultMeshType);
        break;
        case QLCFixtureDef::Smoke:
            meshPath.append("smoke.dae");
            newItem->setProperty("meshType", FixtureMeshType::DefaultMeshType);
        break;
        case QLCFixtureDef::LEDBarBeams:
        case QLCFixtureDef::LEDBarPixels:
            meshPath.clear();
        break;
        default:
            qDebug() << "I don't know what to do with you :'(";
        break;
    }

    // at last, add the new fixture to the items map
    m_entitiesMap[itemID] = mesh;

    newItem->setProperty("itemID", itemID);
    if (meshPath.isEmpty() == false)
        newItem->setProperty("itemSource", meshPath);
}

void MainView3D::setFixtureFlags(quint32 itemID, quint32 flags)
{
    SceneItem *meshRef = m_entitiesMap.value(itemID, nullptr);
    if (meshRef == nullptr)
        return;

    meshRef->m_rootItem->setProperty("enabled", (flags & MonitorProperties::HiddenFlag) ? false : true);
    meshRef->m_selectionBox->setProperty("enabled", (flags & MonitorProperties::HiddenFlag) ? false : true);

    meshRef->m_rootItem->setProperty("invertedPan", (flags & MonitorProperties::InvertedPanFlag) ? true : false);
    meshRef->m_rootItem->setProperty("invertedTilt", (flags & MonitorProperties::InvertedTiltFlag) ? true : false);
}

Qt3DCore::QTransform *MainView3D::getTransform(QEntity *entity)
{
    if (entity == nullptr)
        return nullptr;

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

    return nullptr;
}

QMaterial *MainView3D::getMaterial(QEntity *entity)
{
    if (entity == nullptr)
        return nullptr;

    for (QComponent *component : entity->components()) // C++11
    {
        //qDebug() << component->metaObject()->className();
        QMaterial *material = qobject_cast<QMaterial *>(component);
        if (material)
            return material;
    }

    return nullptr;
}

QVector3D MainView3D::lightPosition(quint32 itemID)
{
    SceneItem *meshRef = m_entitiesMap.value(itemID, nullptr);
    if (meshRef == nullptr)
        return QVector3D();

    return meshRef->m_rootItem->property("lightPos").value<QVector3D>();
}

QMatrix4x4 MainView3D::lightMatrix(quint32 itemID)
{
    SceneItem *meshRef = m_entitiesMap.value(itemID, nullptr);
    if (meshRef == nullptr)
        return QMatrix4x4();

    return meshRef->m_rootItem->property("lightMatrix").value<QMatrix4x4>();
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QAttribute *vPosAttribute = nullptr;
    for (Qt3DRender::QAttribute *attribute : meshGeometry->attributes())
    {
        if (attribute->name() == Qt3DRender::QAttribute::defaultPositionAttributeName())
        {
            vPosAttribute = attribute;
            break;
        }
    }
#else
    Qt3DCore::QAttribute *vPosAttribute = nullptr;
    for (Qt3DCore::QAttribute *attribute : meshGeometry->attributes())
    {
        if (attribute->name() == Qt3DCore::QAttribute::defaultPositionAttributeName())
        {
            vPosAttribute = attribute;
            break;
        }
    }
#endif
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

void MainView3D::addVolumes(SceneItem *meshRef, QVector3D minCorner, QVector3D maxCorner)
{
    if (meshRef == nullptr)
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

    //qDebug() << "-- extent" << meshRef->m_volume.m_extents << "-- center" << meshRef->m_volume.m_center;
}

QEntity *MainView3D::inspectEntity(QEntity *entity, SceneItem *meshRef,
                                   QLayer *layer, QEffect *effect,
                                   bool calculateVolume, QVector3D translation)
{
    if (entity == nullptr)
        return nullptr;

    QEntity *baseItem = nullptr;
    QGeometryRenderer *geom = nullptr;

    for (QComponent *component : entity->components()) // C++11
    {
        //qDebug() << "Class name:" << component->metaObject()->className();

        QMaterial *material = qobject_cast<QMaterial *>(component);
        Qt3DCore::QTransform *transform = qobject_cast<Qt3DCore::QTransform *>(component);

        if (geom == nullptr)
            geom = qobject_cast<QGeometryRenderer *>(component);

        if (material)
        {
            material->setEffect(effect);

            //for (QParameter *par : material->parameters())
            //    qDebug() << "Material parameter:" << par->name() << par->value();

            Qt3DExtras::QPhongMaterial *pMaterial = qobject_cast<Qt3DExtras::QPhongMaterial *>(component);

            if (pMaterial)
            {
                material->addParameter(new QParameter("diffuse", pMaterial->diffuse()));
                material->addParameter(new QParameter("specular", pMaterial->specular()));
                material->addParameter(new QParameter("shininess", pMaterial->shininess() ? pMaterial->shininess() : 1.0));
                material->addParameter(new QParameter("bloom", 0));
            }
            else
            {
                material->addParameter(new QParameter("diffuse", QVector3D(0.64f, 0.64f, 0.64f)));
                material->addParameter(new QParameter("specular", QVector3D(0.64f, 0.64f, 0.64f)));
                material->addParameter(new QParameter("shininess", 1.0));
                material->addParameter(new QParameter("bloom", 0));
            }
        }

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

#ifdef SHOW_FRAMEGRAPH
void MainView3D::walkNode(QNode *e, int depth)
{
    QNodeVector nodes = e->childNodes();
    for (int i = 0; i < nodes.count(); ++i)
    {
        QNode *node = nodes[i];
        QEntity *entity = qobject_cast<QEntity *>(node);
        QString indent;
        indent.fill(' ', depth * 2);
        if (entity)
            qDebug().noquote() << indent << "Entity:" << entity;
        else
            qDebug().noquote() << indent << "Node:" << node->metaObject()->className();
        walkNode(node, depth + 1);
    }
}
#endif

void MainView3D::initializeFixture(quint32 itemID, QEntity *fxEntity, QSceneLoader *loader)
{
    if (m_entitiesMap.contains(itemID) == false)
        return;

    quint32 fxID = FixtureUtils::itemFixtureID(itemID);
    quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
    quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return;

    quint32 itemFlags = m_monProps->fixtureFlags(fxID, headIndex, linkedIndex);
    QLCFixtureMode *fxMode = fixture->fixtureMode();
    QVector3D fxSize(0.3, 0.3, 0.3);
    int panDeg = 0;
    int tiltDeg = 0;
    qreal focusMax = 30;
    qreal focusMin = 10;
    bool calculateVolume = false;
    QEntity *root = nullptr;
    QEntity *baseItem = nullptr;

    if (fxMode != nullptr)
    {
        QLCPhysical phy = fxMode->physical();

        if (phy.width())
            fxSize.setX(phy.width() / 1000.0);
        if (phy.height())
            fxSize.setY(phy.height() / 1000.0);
        if (phy.depth())
            fxSize.setZ(phy.depth() / 1000.0);

        panDeg = phy.focusPanMax() ? phy.focusPanMax() : 360;
        tiltDeg = phy.focusTiltMax() ? phy.focusTiltMax() : 270;
        focusMin = phy.lensDegreesMin() ? phy.lensDegreesMin() : 10;
        focusMax = phy.lensDegreesMax() ? phy.lensDegreesMax() : 30;
    }

    qDebug() << "Initialize fixture" << fixture->id();

    if (loader)
    {
        // The QSceneLoader instance is a component of an entity. The loaded scene
        // tree is added under this entity.
        QVector<QEntity *> entities = loader->entities();

        if (entities.isEmpty())
            return;

        // Technically there could be multiple entities referencing the scene loader
        // but sharing is discouraged, and in our case there will be one anyhow.
        root = entities[0];
        qDebug() << "There are" << root->children().count() << "components in the loaded fixture";
    }
    else
    {
        root = fxEntity;
    }

    QLayer *sceneDeferredLayer = m_sceneRootEntity->property("deferredLayer").value<QLayer *>();
    QEffect *sceneEffect = m_sceneRootEntity->property("geometryPassEffect").value<QEffect *>();

    qDebug() << sceneDeferredLayer << sceneEffect;

    QVector3D translation;
    SceneItem *meshRef = m_entitiesMap.value(itemID);
    meshRef->m_rootItem = fxEntity;
    meshRef->m_rootTransform = getTransform(meshRef->m_rootItem);

    if (meshRef->m_goboTexture != nullptr)
    {
        QTexture2D *tex = fxEntity->property("goboTexture").value<QTexture2D *>();
        //tex->setFormat(Qt3DRender::QAbstractTexture::RGBA8U);
        tex->addTextureImage(meshRef->m_goboTexture);
    }

    // If this model has been already loaded, re-use the cached bounding volume
    if (loader && m_boundingVolumesMap.contains(loader->source()))
        meshRef->m_volume = m_boundingVolumesMap[loader->source()];
    else
        calculateVolume = true;

    if (loader)
    {
        // Walk through the scene tree and add each mesh to the deferred pipeline.
        // If needed, calculate also the bounding volume */
        baseItem = inspectEntity(root, meshRef, sceneDeferredLayer, sceneEffect, calculateVolume, translation);
    }
    else
    {
        meshRef->m_headItem = fxEntity->findChild<QEntity *>("headEntity");
        // root item is already the whole mesh. Add it to the pipeline
        fxEntity->setProperty("sceneLayer", QVariant::fromValue(sceneDeferredLayer));
        fxEntity->setProperty("sceneEffect", QVariant::fromValue(sceneEffect));
        // set physical dimension
        meshRef->m_volume.m_extents = fxSize;
        fxEntity->setProperty("phySize", QVariant::fromValue(fxSize));
    }

    qDebug() << "Calculated volume" << meshRef->m_volume.m_extents << meshRef->m_volume.m_center;

    if (loader && calculateVolume)
        m_boundingVolumesMap[loader->source()] = meshRef->m_volume;

    if (meshRef->m_armItem)
    {
        qDebug() << "Fixture" << fxID << "has an arm entity";
        if (fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB) != QLCChannel::invalid())
        {
            Qt3DCore::QTransform *transform = getTransform(meshRef->m_armItem);
            if (transform != nullptr)
                QMetaObject::invokeMethod(meshRef->m_rootItem, "bindPanTransform",
                        Q_ARG(QVariant, QVariant::fromValue(transform)),
                        Q_ARG(QVariant, panDeg));
        }
    }

    if (meshRef->m_headItem)
    {
        qDebug() << "Fixture" << fxID << "has a head entity";

        Qt3DCore::QTransform *transform = getTransform(meshRef->m_headItem);

        if (baseItem != nullptr)
        {
            if (fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB) != QLCChannel::invalid())
            {
                // If there is a base item and a tilt channel,
                // this is either a moving head or a scanner
                if (transform != nullptr)
                    QMetaObject::invokeMethod(meshRef->m_rootItem, "bindTiltTransform",
                            Q_ARG(QVariant, QVariant::fromValue(transform)),
                            Q_ARG(QVariant, tiltDeg));
            }
        }

        meshRef->m_rootItem->setProperty("focusMinDegrees", focusMin);
        meshRef->m_rootItem->setProperty("focusMaxDegrees", focusMax);

        QMetaObject::invokeMethod(meshRef->m_rootItem, "setupScattering",
                                  Q_ARG(QVariant, QVariant::fromValue(m_sceneRootEntity)));
    }

    /* Set the fixture position */
    QVector3D fxPos;
    if (m_monProps->containsItem(fxID, headIndex, linkedIndex))
    {
        fxPos = m_monProps->fixturePosition(fxID, headIndex, linkedIndex);
    }
    else
    {
        QSizeF size = FixtureUtils::item2DDimension(fxMode, MonitorProperties::TopView);
        QPointF itemPos = FixtureUtils::available2DPosition(m_doc, MonitorProperties::TopView,
                                                            QRectF(0, 0, size.width(), size.height()));
        // add the new fixture to the Doc monitor properties
        fxPos = QVector3D(itemPos.x(), 1000.0, itemPos.y());
        m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, fxPos);
        m_monProps->setFixtureFlags(fxID, headIndex, linkedIndex, 0);
        Tardis::instance()->enqueueAction(Tardis::FixtureSetPosition, itemID, QVariant(QVector3D(0, 0, 0)), QVariant(fxPos));
    }

    // scaling is not needed for dynamic meshes
    if (loader)
        updateFixtureScale(itemID, fxSize);
    updateFixturePosition(itemID, fxPos);
    updateFixtureRotation(itemID, m_monProps->fixtureRotation(fxID, headIndex, linkedIndex));

    QLayer *selectionLayer = m_sceneRootEntity->property("selectionLayer").value<QLayer *>();
    QGeometryRenderer *selectionMesh = m_sceneRootEntity->property("selectionMesh").value<QGeometryRenderer *>();

    meshRef->m_selectionBox = qobject_cast<QEntity *>(m_selectionComponent->create());
    meshRef->m_selectionBox->setParent(m_sceneRootEntity);

    meshRef->m_selectionBox->setProperty("selectionLayer", QVariant::fromValue(selectionLayer));
    meshRef->m_selectionBox->setProperty("geometryPassEffect", QVariant::fromValue(sceneEffect));
    meshRef->m_selectionBox->setProperty("selectionMesh", QVariant::fromValue(selectionMesh));
    meshRef->m_selectionBox->setProperty("extents", meshRef->m_volume.m_extents);
    meshRef->m_selectionBox->setProperty("center", meshRef->m_volume.m_center);

    if (meshRef->m_rootTransform != nullptr)
    {
        QMetaObject::invokeMethod(meshRef->m_selectionBox, "bindItemTransform",
                Q_ARG(QVariant, itemID),
                Q_ARG(QVariant, QVariant::fromValue(meshRef->m_rootTransform)));
    }

    if (itemFlags & MonitorProperties::HiddenFlag)
    {
        meshRef->m_rootItem->setProperty("enabled", false);
        meshRef->m_selectionBox->setProperty("enabled", false);
    }

    if (itemFlags & MonitorProperties::InvertedPanFlag)
        meshRef->m_rootItem->setProperty("invertedPan", true);

    if (itemFlags & MonitorProperties::InvertedTiltFlag)
        meshRef->m_rootItem->setProperty("invertedTilt", true);

    m_createItemCount--;

    // Update the Scene Graph only when the last fixture has been added to the Scene
    if (m_createItemCount == 0)
    {
        QMetaObject::invokeMethod(m_scene3D, "updateFrameGraph", Q_ARG(QVariant, true));
#ifdef SHOW_FRAMEGRAPH
        if (m_scene3DEntity)
            walkNode(m_scene3DEntity, 0);
#endif
    }

    // at last, preview the fixture channels
    QByteArray values;
    updateFixture(fixture, values);
}

void MainView3D::updateFixture(Fixture *fixture, QByteArray &previous)
{
    if (m_enabled == false || fixture == nullptr)
        return;

    for (quint32 subID : m_monProps->fixtureIDList(fixture->id()))
    {
        quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
        quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
        updateFixtureItem(fixture, headIndex, linkedIndex, previous);
    }
}

void MainView3D::updateFixtureItem(Fixture *fixture, quint16 headIndex, quint16 linkedIndex, QByteArray &previous)
{
    quint32 itemID = FixtureUtils::fixtureItemID(fixture->id(), headIndex, linkedIndex);
    SceneItem *meshItem = m_entitiesMap.value(itemID, nullptr);
    QColor color;
    bool setPosition = false;
    bool colorSet = false;
    bool goboSet = false;
    int panValue = 0;
    int tiltValue = 0;

    if (meshItem == nullptr)
        return;

    QEntity *fixtureItem = meshItem->m_rootItem;
    if (fixtureItem == nullptr)
        return;

    // in case of a dimmer pack, headIndex is actually the fixture channel
    // so treat this as a special case and go straight to the point
    if (fixture->type() == QLCFixtureDef::Dimmer)
    {
        qreal value = qreal(fixture->channelValueAt(headIndex)) / 255.0;
        QMetaObject::invokeMethod(fixtureItem, "setHeadIntensity",
                Q_ARG(QVariant, 0),
                Q_ARG(QVariant, value));

        QColor gelColor = m_monProps->fixtureGelColor(fixture->id(), headIndex, linkedIndex);
        if (gelColor.isValid() == false)
            gelColor = Qt::white;

        QMetaObject::invokeMethod(fixtureItem, "setHeadRGBColor",
                Q_ARG(QVariant, 0),
                Q_ARG(QVariant, gelColor));

        return;
    }

    quint32 masterDimmerChannel = fixture->masterIntensityChannel();
    qreal masterDimmerValue = masterDimmerChannel != QLCChannel::invalid() ?
                              qreal(fixture->channelValueAt(int(masterDimmerChannel))) / 255.0 : 1.0;

    for (int headIdx = 0; headIdx < fixture->heads(); headIdx++)
    {
        quint32 headDimmerChannel = fixture->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, headIdx);
        if (headDimmerChannel == QLCChannel::invalid())
            headDimmerChannel = masterDimmerChannel;

        qreal intensityValue = 1.0;
        bool hasDimmer = false;

        if (headDimmerChannel != QLCChannel::invalid())
        {
            intensityValue = qreal(fixture->channelValueAt(int(headDimmerChannel))) / 255.0;
            hasDimmer = true;
        }

        if (headDimmerChannel != masterDimmerChannel)
            intensityValue *= masterDimmerValue;

        //qDebug() << "Head" << headIdx << "dimmer channel:" << headDimmerIndex << "intensity" << intensityValue;

        QMetaObject::invokeMethod(fixtureItem, "setHeadIntensity",
                Q_ARG(QVariant, headIdx),
                Q_ARG(QVariant, intensityValue));

        color = FixtureUtils::headColor(fixture, hasDimmer, headIdx);

        QMetaObject::invokeMethod(fixtureItem, "setHeadRGBColor",
                                  Q_ARG(QVariant, headIdx),
                                  Q_ARG(QVariant, color));
        colorSet = true;
    } // for heads

    // now scan all the channels for "common" capabilities
    for (int i = 0; i < int(fixture->channels()); i++)
    {
        const QLCChannel *ch = fixture->channel(quint32(i));
        if (ch == nullptr)
            continue;

        uchar value = fixture->channelValueAt(i);

        switch (ch->group())
        {
            case QLCChannel::Pan:
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    panValue += (value << 8);
                else
                    panValue += (value);

                if (previous.isEmpty() || value != uchar(previous.at(i)))
                    setPosition = true;
            }
            break;
            case QLCChannel::Tilt:
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    tiltValue += (value << 8);
                else
                    tiltValue += (value);

                if (previous.isEmpty() || value != uchar(previous.at(i)))
                    setPosition = true;
            }
            break;
            case QLCChannel::Speed:
            {
                if (previous.length() && value == uchar(previous.at(i)))
                    break;

                int panSpeed, tiltSpeed;
                FixtureUtils::positionTimings(ch, value, panSpeed, tiltSpeed);
                QMetaObject::invokeMethod(fixtureItem, "setPositionSpeed",
                                          Q_ARG(QVariant, panSpeed),
                                          Q_ARG(QVariant, tiltSpeed));
            }
            break;
            case QLCChannel::Colour:
            {
                if (colorSet && value == 0)
                    break;

                QLCCapability *cap = ch->searchCapability(value);

                if (cap == nullptr ||
                   (cap->presetType() != QLCCapability::SingleColor &&
                    cap->presetType() != QLCCapability::DoubleColor))
                    break;

                QColor wheelColor1 = cap->resource(0).value<QColor>();
                //QColor wheelColor2 = cap->resource(1).value<QColor>();

                if (wheelColor1.isValid())
                {
                    color = FixtureUtils::applyColorFilter(color, wheelColor1);
                    QMetaObject::invokeMethod(fixtureItem, "setHeadRGBColor",
                                              Q_ARG(QVariant, 0),
                                              Q_ARG(QVariant, color));
                }
            }
            break;
            case QLCChannel::Beam:
            {
                if (previous.length() && value == uchar(previous.at(i)))
                    break;

                switch (ch->preset())
                {
                    case QLCChannel::BeamZoomSmallBig:
                        QMetaObject::invokeMethod(fixtureItem, "setZoom", Q_ARG(QVariant, value));
                    break;
                    case QLCChannel::BeamZoomBigSmall:
                        QMetaObject::invokeMethod(fixtureItem, "setZoom", Q_ARG(QVariant, 255 - value));
                    break;
                    default:
                    break;
                }
            }
            break;
            case QLCChannel::Gobo:
            {
                if (previous.length() && value == uchar(previous.at(i)))
                    break;

                QLCCapability *cap = ch->searchCapability(value);

                if (cap == nullptr)
                    break;

                switch (cap->preset())
                {
                    case QLCCapability::GoboMacro:
                    case QLCCapability::GoboShakeMacro:
                    {
                        QString resName = cap->resource(0).toString();

                        if (goboSet || resName.isEmpty())
                            break;

                        if (meshItem->m_goboTexture)
                            meshItem->m_goboTexture->setSource(resName);

                        // here we don't look for any other gobos, so if a
                        // fixture has more than one gobo wheel, the second
                        // one will be skipped if the first one has been set
                        // to a non-open gobo
                        goboSet = true;
                    }
                    break;
                    default:
                    {
                        int speed;
                        bool clockwise = FixtureUtils::goboTiming(cap, value, speed);
                        if (speed == -1)
                            break;

                        QMetaObject::invokeMethod(fixtureItem, "setGoboSpeed",
                                Q_ARG(QVariant, clockwise), Q_ARG(QVariant, speed));
                    }
                    break;
                }
            }
            break;
            case QLCChannel::Shutter:
            {
                if (previous.length() && value == uchar(previous.at(i)))
                    break;

                int high = 200, low = 800;
                int capPreset = FixtureUtils::shutterTimings(ch, value, high, low);

                QMetaObject::invokeMethod(fixtureItem, "setShutter",
                        Q_ARG(QVariant, capPreset),
                        Q_ARG(QVariant, low), Q_ARG(QVariant, high));
            }
            break;
            default:
            break;
        }
    }

    if (setPosition)
    {
        QMetaObject::invokeMethod(fixtureItem, "setPosition",
                Q_ARG(QVariant, panValue),
                Q_ARG(QVariant, tiltValue));
    }
}

void MainView3D::updateFixtureSelection(QList<quint32> fixtures)
{
    QMapIterator<quint32, SceneItem*> it(m_entitiesMap);
    while (it.hasNext())
    {
        it.next();
        quint32 fxID = it.key();
        SceneItem *meshRef = m_entitiesMap.value(fxID, nullptr);

        if (meshRef == nullptr || meshRef->m_rootItem == nullptr)
            return;

        if (fixtures.contains(fxID))
        {
            meshRef->m_rootItem->setProperty("isSelected", true);
            meshRef->m_selectionBox->setProperty("isSelected", true);
        }
        else
        {
            meshRef->m_rootItem->setProperty("isSelected", false);
            meshRef->m_selectionBox->setProperty("isSelected", false);
        }
    }
}

void MainView3D::updateFixtureSelection(quint32 itemID, bool enable)
{
    qDebug() << "[View3D] item" << itemID << "selected:" << enable;

    SceneItem *meshRef = m_entitiesMap.value(itemID, nullptr);
    if (meshRef && meshRef->m_rootItem)
    {
        meshRef->m_rootItem->setProperty("isSelected", enable);
        meshRef->m_selectionBox->setProperty("isSelected", enable);
    }
}

void MainView3D::updateFixturePosition(quint32 itemID, QVector3D pos)
{
    if (isEnabled() == false)
        return;

    SceneItem *mesh = m_entitiesMap.value(itemID, nullptr);
    if (mesh == nullptr || mesh->m_rootTransform == nullptr)
        return;

    //qDebug() << "Update 3D fixture position" << pos;

    float x = (pos.x() / 1000.0) - (m_monProps->gridSize().x() / 2) + (mesh->m_volume.m_extents.x() / 2);
    float y = (pos.y() / 1000.0) + (mesh->m_volume.m_extents.y() / 2);
    float z = (pos.z() / 1000.0) - (m_monProps->gridSize().z() / 2) + (mesh->m_volume.m_extents.z() / 2);

    /* move the root mesh first */
    mesh->m_rootTransform->setTranslation(QVector3D(x, y, z));

    updateLightMatrix(mesh);
}

void MainView3D::updateFixtureRotation(quint32 itemID, QVector3D degrees)
{
    if (isEnabled() == false)
        return;

    SceneItem *mesh = m_entitiesMap.value(itemID, nullptr);
    if (mesh == nullptr || mesh->m_rootTransform == nullptr)
        return;

    qDebug() << Q_FUNC_INFO << degrees;

    QQuaternion qRotation;
    qRotation = Qt3DCore::QTransform::fromAxesAndAngles(QVector3D(1, 0, 0), -degrees.x(),
                                                        QVector3D(0, 1, 0), -degrees.y(),
                                                        QVector3D(0, 0, 1), -degrees.z());
    mesh->m_rootTransform->setRotation(qRotation);

    updateLightMatrix(mesh);
}

void MainView3D::updateLightMatrix(SceneItem *mesh)
{
    // no head ? Nothing to do
    if (mesh->m_headItem == nullptr)
        return;

    // below, we extract a rotation matrix and position, which we need for properly
    // positioning and rotating the spotlight cone.
    QMatrix4x4 m = (mesh->m_rootTransform->matrix());

    if (mesh->m_armItem)
    {
        QMatrix4x4 armTransform = getTransform(mesh->m_armItem)->matrix();
        m.translate(armTransform.data()[12], armTransform.data()[13], armTransform.data()[14]);
    }

    if (mesh->m_headItem)
    {
        QMatrix4x4 headTransform = getTransform(mesh->m_headItem)->matrix();
        m.translate(headTransform.data()[12], headTransform.data()[13], headTransform.data()[14]);
    }

    QVector4D xb = m * QVector4D(1, 0, 0, 0);
    QVector4D yb = m * QVector4D(0, 1, 0, 0);
    QVector4D zb = m * QVector4D(0, 0, 1, 0);

    QVector3D xa = QVector3D(xb.x(), xb.y(), xb.z()).normalized();
    QVector3D ya = QVector3D(yb.x(), yb.y(), yb.z()).normalized();
    QVector3D za = QVector3D(zb.x(), zb.y(), zb.z()).normalized();

    QMatrix4x4 lightMatrix = QMatrix4x4(
        xa.x(), xa.y(), xa.z(), 0,
        ya.x(), ya.y(), ya.z(), 0,
        za.x(), za.y(), za.z(), 0,
        0, 0, 0, 1
    ).transposed();

    QVector4D result = m * QVector4D(0, 0, 0, 1);

    QMetaObject::invokeMethod(mesh->m_rootItem, "setHeadLightProps",
            Q_ARG(QVariant, 0),
            Q_ARG(QVariant, QVariant::fromValue(QVector3D(result.x(), result.y(), result.z()))),
            Q_ARG(QVariant, QVariant::fromValue(lightMatrix)));
}

void MainView3D::updateFixtureScale(quint32 itemID, QVector3D origSize)
{
    if (isEnabled() == false)
        return;

    SceneItem *mesh = m_entitiesMap.value(itemID, nullptr);
    if (mesh == nullptr || mesh->m_rootTransform == nullptr)
        return;

    QVector3D meshSize = mesh->m_volume.m_extents;

    float xScale = origSize.x() / meshSize.x();
    float yScale = origSize.y() / meshSize.y();
    float zScale = origSize.z() / meshSize.z();

    float minScale = qMin(xScale, qMin(yScale, zScale));

    mesh->m_rootTransform->setScale3D(QVector3D(minScale, minScale, minScale));

    // warning: after this, the original mesh size is lost forever
    mesh->m_volume.m_extents *= minScale;
    mesh->m_volume.m_center *= minScale;
}

void MainView3D::removeFixtureItem(quint32 itemID)
{
    if (isEnabled() == false || m_entitiesMap.contains(itemID) == false)
        return;

    SceneItem *mesh = m_entitiesMap.take(itemID);

    delete mesh->m_goboTexture;
    delete mesh->m_selectionBox;
    delete mesh->m_rootTransform;
//    delete mesh->m_rootItem; // this will cause a segfault
    mesh->m_rootItem->setProperty("enabled", false); // workaround for the above

    delete mesh;
}

/*********************************************************************
 * Generic items
 *********************************************************************/

void MainView3D::createGenericItem(QString filename, int itemID)
{
    if (isEnabled() == false)
        return;

    qDebug() << "File URL is:" << filename;

    if (m_quadEntity == nullptr)
        initialize3DProperties();

    if (itemID == -1)
    {
        QString resFile = QString(filename);
        if (resFile.startsWith(meshDirectory()))
            resFile = QUrl(filename).toLocalFile();
        else
            resFile.remove(QLCFile::fileUrlPrefix());

        QVector3D envSize = m_monProps->gridSize();
        QVector3D newPos((envSize.x() / 2) * 1000.0, 1000.0, (envSize.z() / 2) * 1000.0);
        m_monProps->setItemPosition(m_latestGenericID, newPos);
        m_monProps->setItemScale(m_latestGenericID, QVector3D(1.0, 1.0, 1.0));
        m_monProps->setItemResource(m_latestGenericID, resFile);
    }
    else
    {
        // check file existence
        // 1- absolute path first
        QFileInfo fInfo(filename);
        if (fInfo.exists() == false)
        {
            // 2 - mesh directory relative path
            QString meshFile = meshDirectory() + filename;
            meshFile.remove(QLCFile::fileUrlPrefix());
            fInfo.setFile(meshFile);
            if (fInfo.exists() == false)
            {
                // 3 - project workspace relative path
                QString pFile = m_doc->denormalizeComponentPath(filename);
                fInfo.setFile(pFile);
                // 4 - give up
                if (fInfo.exists() == false)
                {
                    qWarning() << "File doesn't exist:" << filename;
                    return;
                }
                else
                {
                    filename = QLCFile::fileUrlPrefix() + m_doc->getWorkspacePath() + QDir::separator() + filename;
                }
            }
            else
            {
                filename = meshDirectory() + filename;
            }
        }
        else
        {
            filename = QLCFile::fileUrlPrefix() + filename;
        }

        m_latestGenericID = itemID;
    }

    SceneItem *mesh = new SceneItem;
    mesh->m_rootItem = nullptr;
    mesh->m_rootTransform = nullptr;
    mesh->m_armItem = nullptr;
    mesh->m_headItem = nullptr;
    mesh->m_selectionBox = nullptr;
    mesh->m_goboTexture = nullptr;

    QEntity *newItem = qobject_cast<QEntity *>(m_genericComponent->create());
    newItem->setParent(m_sceneRootEntity);

    newItem->setProperty("itemID", m_latestGenericID);
    newItem->setProperty("itemSource", filename);

    // at last, add the new item to the generic map
    m_genericMap[m_latestGenericID] = mesh;

    m_latestGenericID++;
}

void MainView3D::initializeItem(int itemID, QEntity *itemEntity, QSceneLoader *loader)
{
    if (m_genericMap.contains(itemID) == false)
        return;

    // The QSceneLoader instance is a component of an entity. The loaded scene
    // tree is added under this entity.
    QVector<QEntity *> entities = loader->entities();

    if (entities.isEmpty())
        return;

    // Technically there could be multiple entities referencing the scene loader
    // but sharing is discouraged, and in our case there will be one anyhow.
    QEntity *root = entities[0];
    bool calculateVolume = false;

    qDebug() << "There are" << root->children().count() << "components in the loaded fixture";

    QLayer *sceneDeferredLayer = m_sceneRootEntity->property("deferredLayer").value<QLayer *>();
    QEffect *sceneEffect = m_sceneRootEntity->property("geometryPassEffect").value<QEffect *>();

    QVector3D translation;
    SceneItem *meshRef = m_genericMap.value(itemID);
    meshRef->m_rootItem = itemEntity;
    meshRef->m_rootTransform = getTransform(meshRef->m_rootItem);

    // If this model has been already loaded, re-use the cached bounding volume
    if (m_boundingVolumesMap.contains(loader->source()))
        meshRef->m_volume = m_boundingVolumesMap[loader->source()];
    else
        calculateVolume = true;

    // Walk through the scene tree and add each mesh to the deferred pipeline.
    // If needed, calculate also the bounding volume */
    inspectEntity(root, meshRef, sceneDeferredLayer, sceneEffect, calculateVolume, translation);

    qDebug() << "Calculated volume:" << meshRef->m_volume.m_extents << ", center:" << meshRef->m_volume.m_center;
    qDebug() << "Translation:" << translation;

    if (calculateVolume)
        m_boundingVolumesMap[loader->source()] = meshRef->m_volume;

    QLayer *selectionLayer = m_sceneRootEntity->property("selectionLayer").value<QLayer *>();
    QGeometryRenderer *selectionMesh = m_sceneRootEntity->property("selectionMesh").value<QGeometryRenderer *>();

    meshRef->m_selectionBox = qobject_cast<QEntity *>(m_selectionComponent->create());
    meshRef->m_selectionBox->setParent(m_sceneRootEntity);

    meshRef->m_selectionBox->setProperty("selectionLayer", QVariant::fromValue(selectionLayer));
    meshRef->m_selectionBox->setProperty("geometryPassEffect", QVariant::fromValue(sceneEffect));
    meshRef->m_selectionBox->setProperty("selectionMesh", QVariant::fromValue(selectionMesh));
    meshRef->m_selectionBox->setProperty("extents", meshRef->m_volume.m_extents);
    meshRef->m_selectionBox->setProperty("center", meshRef->m_volume.m_center);
    meshRef->m_selectionBox->setProperty("color", QVector4D(0, 1, 0, 2.0));

    updateGenericItemScale(itemID, m_monProps->itemScale(itemID));
    updateGenericItemPosition(itemID, m_monProps->itemPosition(itemID));
    updateGenericItemRotation(itemID, m_monProps->itemRotation(itemID));

    if (meshRef->m_rootTransform != nullptr)
    {
        QMetaObject::invokeMethod(meshRef->m_selectionBox, "bindItemTransform",
                Q_ARG(QVariant, itemID),
                Q_ARG(QVariant, QVariant::fromValue(meshRef->m_rootTransform)));
    }

    itemEntity->setProperty("sceneLayer", QVariant::fromValue(sceneDeferredLayer));
    itemEntity->setProperty("effect", QVariant::fromValue(sceneEffect));
    updateGenericItemsList();
}

void MainView3D::setItemSelection(int itemID, bool enable, int keyModifiers)
{
    if (isEnabled() == false)
        return;

    if (enable && keyModifiers == 0)
    {
        for (int &id : m_genericSelectedItems)
        {
            SceneItem *meshRef = m_genericMap.value(id, nullptr);
            if (meshRef)
            {
                meshRef->m_rootItem->setProperty("isSelected", false);
                meshRef->m_selectionBox->setProperty("isSelected", false);
            }
        }
        m_genericSelectedItems.clear();
        emit genericSelectedCountChanged();
    }

    SceneItem *meshRef = m_genericMap.value(itemID, nullptr);
    if (meshRef)
    {
        meshRef->m_rootItem->setProperty("isSelected", enable);
        meshRef->m_selectionBox->setProperty("isSelected", enable);
    }

    if (enable)
        m_genericSelectedItems.append(itemID);
    else
        m_genericSelectedItems.removeAll(itemID);

    emit genericSelectedCountChanged();
}

int MainView3D::genericSelectedCount() const
{
    return m_genericSelectedItems.count();
}

void MainView3D::removeSelectedGenericItems()
{
    for (int &id : m_genericSelectedItems)
    {
        SceneItem *meshRef = m_genericMap.take(id);
        if (meshRef)
        {
            delete meshRef->m_rootItem;
            delete meshRef->m_selectionBox;
        }
        m_monProps->removeItem(id);
    }
    m_genericSelectedItems.clear();
    emit genericSelectedCountChanged();
    updateGenericItemsList();
}

void MainView3D::normalizeSelectedGenericItems()
{
    for (int &id : m_genericSelectedItems)
    {
        SceneItem *meshRef = m_genericMap.value(id, nullptr);
        if (meshRef)
        {
            // adjust the item scale to target size
            float factor = 1.0;
            float targetSize = 2.0;

            if (targetSize / meshRef->m_volume.m_extents.x() < factor)
                factor = targetSize / meshRef->m_volume.m_extents.x();

            if (targetSize / meshRef->m_volume.m_extents.y() < factor)
                factor = targetSize / meshRef->m_volume.m_extents.y();

            if (targetSize / meshRef->m_volume.m_extents.z() < factor)
                factor = targetSize / meshRef->m_volume.m_extents.z();

            updateGenericItemScale(id, QVector3D(factor, factor, factor));

            // adjust the item position to scene origin
            updateGenericItemPosition(id, QVector3D(meshRef->m_volume.m_center.x() * -1000.0,
                                                    meshRef->m_volume.m_center.y() * -1000.0,
                                                    meshRef->m_volume.m_center.z() * -1000.0));
        }
    }
}

void MainView3D::updateGenericItemsList()
{
    m_genericItemsList->clear();

    for (quint32 &itemID : m_monProps->genericItemsID())
    {
        QVariantMap itemMap;
        itemMap.insert("itemID", itemID);
        itemMap.insert("name", m_monProps->itemName(itemID));
        itemMap.insert("isSelected", false);
        m_genericItemsList->addDataMap(itemMap);
    }

    emit genericItemsListChanged();
}

QVariant MainView3D::genericItemsList() const
{
    return QVariant::fromValue(m_genericItemsList);
}

void MainView3D::updateGenericItemPosition(quint32 itemID, QVector3D pos)
{
    if (isEnabled() == false)
        return;

    QVector3D currPos = m_monProps->itemPosition(itemID);
    Tardis::instance()->enqueueAction(Tardis::GenericItemSetPosition, itemID, QVariant(currPos), QVariant(pos));

    m_monProps->setItemPosition(itemID, pos);

    SceneItem *item = m_genericMap.value(itemID, nullptr);
    if (item == nullptr || item->m_rootTransform == nullptr)
        return;

    float x = (pos.x() / 1000.0) - (m_monProps->gridSize().x() / 2) + (item->m_volume.m_extents.x() / 2);
    float y = (pos.y() / 1000.0) + (item->m_volume.m_extents.y() / 2);
    float z = (pos.z() / 1000.0) - (m_monProps->gridSize().z() / 2) + (item->m_volume.m_extents.z() / 2);
    item->m_rootTransform->setTranslation(QVector3D(x, y, z));
}

QVector3D MainView3D::genericItemsPosition()
{
    if (m_genericSelectedItems.count() == 1)
        return m_monProps->itemPosition(m_genericSelectedItems.first());

    return QVector3D(0, 0, 0);
}

void MainView3D::setGenericItemsPosition(QVector3D pos)
{
    if (m_genericSelectedItems.isEmpty())
        return;

    if (m_genericSelectedItems.count() == 1)
    {
        updateGenericItemPosition(m_genericSelectedItems.first(), pos);
    }
    else
    {
        // relative position change
        for (int &itemID : m_genericSelectedItems)
        {
            QVector3D newPos = m_monProps->itemPosition(itemID) + pos;
            updateGenericItemPosition(itemID, newPos);
        }
    }

    emit genericItemsPositionChanged();
}

void MainView3D::updateGenericItemRotation(quint32 itemID, QVector3D rot)
{
    if (isEnabled() == false)
        return;

    QVector3D currRot = m_monProps->itemRotation(itemID);
    Tardis::instance()->enqueueAction(Tardis::GenericItemSetRotation, itemID, QVariant(currRot), QVariant(rot));

    m_monProps->setItemRotation(itemID, rot);
    SceneItem *item = m_genericMap.value(itemID, nullptr);
    if (item == nullptr || item->m_rootTransform == nullptr)
        return;

    QQuaternion qRotation;
    qRotation = Qt3DCore::QTransform::fromAxesAndAngles(QVector3D(1, 0, 0), rot.x(),
                                                        QVector3D(0, 1, 0), rot.y(),
                                                        QVector3D(0, 0, 1), rot.z());
    item->m_rootTransform->setRotation(qRotation);
}

QVector3D MainView3D::genericItemsRotation()
{
    if (m_genericSelectedItems.count() == 1)
        return m_monProps->itemRotation(m_genericSelectedItems.first());

    return QVector3D(0, 0, 0);
}

void MainView3D::setGenericItemsRotation(QVector3D rot)
{
    if (m_genericSelectedItems.isEmpty())
        return;

    if (m_genericSelectedItems.count() == 1)
    {
        updateGenericItemRotation(m_genericSelectedItems.first(), rot);
    }
    else
    {
        // relative position change
        for (int &itemID : m_genericSelectedItems)
        {
            QVector3D newRot = m_monProps->itemRotation(itemID) + rot;

            // normalize back to a 0-359 range
            if (newRot.x() < 0) newRot.setX(newRot.x() + 360);
            else if (newRot.x() >= 360) newRot.setX(newRot.x() - 360);

            if (newRot.y() < 0) newRot.setY(newRot.y() + 360);
            else if (newRot.y() >= 360) newRot.setY(newRot.y() - 360);

            if (newRot.z() < 0) newRot.setZ(newRot.z() + 360);
            else if (newRot.z() >= 360) newRot.setZ(newRot.z() - 360);

            updateGenericItemRotation(itemID, newRot);
        }
    }
    emit genericItemsRotationChanged();
}

void MainView3D::updateGenericItemScale(quint32 itemID, QVector3D scale)
{
    if (isEnabled() == false)
        return;

    QVector3D currScale = m_monProps->itemScale(itemID);
    Tardis::instance()->enqueueAction(Tardis::GenericItemSetScale, itemID, QVariant(currScale), QVariant(scale));

    m_monProps->setItemScale(itemID, scale);
    SceneItem *item = m_genericMap.value(itemID, nullptr);
    if (item == nullptr || item->m_rootTransform == nullptr)
        return;

    item->m_rootTransform->setScale3D(scale);
    if (item->m_selectionBox)
        item->m_selectionBox->setProperty("modScale", scale);
}

QVector3D MainView3D::genericItemsScale()
{
    if (m_genericSelectedItems.count() == 1)
    {
        QVector3D scale = m_monProps->itemScale(m_genericSelectedItems.first());
        return QVector3D(scale.x() * 100.0, scale.y() * 100.0, scale.z() * 100.0);
    }

    return QVector3D(100.0, 100.0, 100.0);
}

void MainView3D::setGenericItemsScale(QVector3D scale)
{
    if (m_genericSelectedItems.isEmpty())
        return;

    QVector3D normScale(scale.x() / 100.0, scale.y() / 100.0, scale.z() / 100.0);
    if (m_genericSelectedItems.count() == 1)
    {
        updateGenericItemScale(m_genericSelectedItems.first(), normScale);
    }
    else
    {
        for (int &itemID : m_genericSelectedItems)
        {
            QVector3D newScale = m_monProps->itemScale(itemID) + normScale;
            updateGenericItemScale(itemID, newScale);
        }
    }

    emit genericItemsScaleChanged();
}

/*********************************************************************
 * Environment
 *********************************************************************/

MainView3D::RenderQuality MainView3D::renderQuality() const
{
    return m_renderQuality;
}

void MainView3D::setRenderQuality(MainView3D::RenderQuality renderQuality)
{
    if (m_renderQuality == renderQuality)
        return;

    m_renderQuality = renderQuality;
    emit renderQualityChanged(m_renderQuality);
}

QStringList MainView3D::stagesList() const
{
    return m_stagesList;
}

int MainView3D::stageIndex() const
{
    return m_monProps->stageType();
}

void MainView3D::setStageIndex(int stageIndex)
{
    if (stageIndex == m_monProps->stageType())
        return;

    m_monProps->setStageType(MonitorProperties::StageType(stageIndex));
    createStage();

    emit stageIndexChanged(stageIndex);
}

void MainView3D::createStage()
{
    if (m_stageEntity)
        delete m_stageEntity;

    QQmlComponent *stageComponent = new QQmlComponent(m_view->engine(), QUrl(m_stageResourceList.at(m_monProps->stageType())));
    if (stageComponent->isError())
        qDebug() << stageComponent->errors();

    m_stageEntity = qobject_cast<QEntity *>(stageComponent->create());
    m_stageEntity->setParent(m_sceneRootEntity);

    QLayer *sceneDeferredLayer = m_sceneRootEntity->property("deferredLayer").value<QLayer *>();
    QEffect *sceneEffect = m_sceneRootEntity->property("geometryPassEffect").value<QEffect *>();
    m_stageEntity->setProperty("sceneLayer", QVariant::fromValue(sceneDeferredLayer));
    m_stageEntity->setProperty("effect", QVariant::fromValue(sceneEffect));
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

float MainView3D::smokeAmount() const
{
    return m_smokeAmount;
}

void MainView3D::setSmokeAmount(float smokeAmount)
{
    if (m_smokeAmount == smokeAmount)
        return;

    m_smokeAmount = smokeAmount;
    emit smokeAmountChanged(m_smokeAmount);
}

/** *********************************************************************************
 *                          GOBO TEXTURE CLASS METHODS
 *  ********************************************************************************* */

GoboTextureImage::GoboTextureImage(int w, int h, QString filename)
    : m_renderer(nullptr)
{
    setSize(QSize(w, h));
    setSource(filename);
}

QString GoboTextureImage::source() const
{
    return m_source;
}

void GoboTextureImage::setSource(QString filename)
{
    if (filename == m_source)
        return;

    m_source = filename;

    if (filename.endsWith(".svg"))
    {
        if (m_renderer == nullptr)
            m_renderer = new QSvgRenderer();

        if (m_renderer->load(m_source) == false)
            qWarning() << "Failed to load SVG gobo" << m_source;
    }
    else
    {
        if (m_renderer)
            delete m_renderer;
        m_renderer = nullptr;
    }
    update();
}

void GoboTextureImage::paint(QPainter *painter)
{
    int w = painter->device()->width();
    int h = painter->device()->height();

    painter->fillRect(0, 0, w, h, Qt::black);
    painter->setBrush(QBrush(Qt::white));
    painter->drawEllipse(2, 2, w - 4, h - 4);
    if (m_renderer)
    {
        m_renderer->render(painter, QRect(1, 1, w - 2, h - 2));
    }
    else
    {
        QIcon goboFile(m_source);
        painter->drawPixmap(1, 1, w - 2, h - 2, goboFile.pixmap(QSize(w - 2, h - 2)));
    }
}
