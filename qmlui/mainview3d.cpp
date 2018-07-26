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

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QParameter>
#include <Qt3DExtras/QPhongMaterial>

#include "doc.h"
#include "tardis.h"
#include "qlcfile.h"
#include "qlcconfig.h"
#include "mainview3d.h"
#include "fixtureutils.h"
#include "qlccapability.h"
#include "qlcfixturemode.h"
#include "monitorproperties.h"

MainView3D::MainView3D(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "3D", parent)
    , m_monProps(doc->monitorProperties())
    , m_fixtureComponent(NULL)
    , m_genericComponent(NULL)
    , m_selectionComponent(NULL)
    , m_scene3D(NULL)
    , m_sceneRootEntity(NULL)
    , m_quadEntity(NULL)
    , m_gBuffer(NULL)
    , m_frontDepthTarget(NULL)
    , m_latestGenericID(0)
    , m_renderQuality(HighQuality)
    , m_stageEntity(NULL)
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

    QMetaObject::invokeMethod(m_scene3D, "updateSceneGraph", Q_ARG(QVariant, false));

    QMapIterator<quint32, SceneItem*> it(m_entitiesMap);
    while(it.hasNext())
    {
        it.next();
        SceneItem *e = it.value();
        //if (e->m_headItem)
        //    delete e->m_headItem;
        //if (e->m_armItem)
        //    delete e->m_armItem;
        delete e->m_goboTexture;
        // delete e->m_rootItem; // TODO: with this -> segfault
        delete e->m_selectionBox;
    }

    //const auto end = m_entitiesMap.end();
    //for (auto it = m_entitiesMap.begin(); it != end; ++it)
    //    delete it.value();
    m_entitiesMap.clear();

    QMapIterator<int, SceneItem*> it2(m_genericMap);
    while(it2.hasNext())
    {
        it2.next();
        SceneItem *e = it2.value();
        delete e->m_rootItem;
    }
    m_genericMap.clear();
    m_latestGenericID = 0;
}

QString MainView3D::meshDirectory() const
{
    QDir dir = QDir::cleanPath(QLCFile::systemDirectory(MESHESDIR).path());
    //qDebug() << "Absolute mesh path: " << dir.absolutePath();
    return QString("file:///") + dir.absolutePath() + QDir::separator();
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
    while(it.hasNext())
    {
        it.next();
        quint32 fxID = it.key();
        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == NULL)
            continue;

        SceneItem *meshRef = m_entitiesMap.value(fxID);

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
 * Fixtures
 *********************************************************************/

void MainView3D::initialize3DProperties()
{
    if (m_fixtureComponent == NULL)
    {
        m_fixtureComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/Fixture3DItem.qml"));
        if (m_fixtureComponent->isError())
            qDebug() << m_fixtureComponent->errors();
    }

    if (m_genericComponent == NULL)
    {
        m_genericComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/Generic3DItem.qml"));
        if (m_genericComponent->isError())
            qDebug() << m_genericComponent->errors();
    }

    if (m_selectionComponent == NULL)
    {
        m_selectionComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/SelectionEntity.qml"));
        if (m_selectionComponent->isError())
            qDebug() << m_selectionComponent->errors();
    }

    m_scene3D = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("scene3DItem"));

    qDebug() << Q_FUNC_INFO << m_scene3D;

    if (m_scene3D == NULL)
    {
        qDebug() << "Scene3DItem not found!";
        return;
    }

    m_sceneRootEntity = m_scene3D->findChild<QEntity *>("sceneRootEntity");
    if (m_sceneRootEntity == NULL)
    {
        qDebug() << "sceneRootEntity not found!";
        return;
    }

    m_quadEntity = m_scene3D->findChild<QEntity *>("quadEntity");
    if (m_quadEntity == NULL)
    {
        qDebug() << "quadEntity not found!";
        return;
    }

    m_gBuffer = m_scene3D->findChild<QRenderTarget *>("gBuffer");
    if (m_gBuffer == NULL)
    {
        qDebug() << "gBuffer not found!";
        return;
    }

    m_frontDepthTarget = m_scene3D->findChild<QRenderTarget *>("depthTarget");
    if (m_frontDepthTarget == NULL)
    {
        qDebug() << "frontDepth not found!";
        return;
    }

    qDebug() << m_sceneRootEntity << m_quadEntity << m_gBuffer << m_frontDepthTarget;

    if (m_stageEntity == NULL)
        createStage();

    QMetaObject::invokeMethod(m_scene3D, "updateSceneGraph", Q_ARG(QVariant, true));
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
    if (entity == NULL)
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
    if (fixture == NULL)
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

    if (m_quadEntity == NULL)
        initialize3DProperties();

    qDebug() << "[MainView3D] Creating fixture with ID" << fxID << "pos:" << pos;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    QString meshPath = meshDirectory() + "fixtures" + QDir::separator();
    QString openGobo = goboDirectory() + QDir::separator() + "Others/open.svg";
    quint32 itemID = FixtureUtils::fixtureItemID(fxID, headIndex, linkedIndex);

    SceneItem *mesh = new SceneItem;
    mesh->m_rootItem = NULL;
    mesh->m_rootTransform = NULL;
    mesh->m_armItem = NULL;
    mesh->m_headItem = NULL;
    mesh->m_lightIndex = getNewLightIndex();
    mesh->m_selectionBox = NULL;
    mesh->m_goboTexture = new GoboTextureImage(512, 512, openGobo);

    QEntity *newItem = qobject_cast<QEntity *>(m_fixtureComponent->create());
    newItem->setParent(m_sceneRootEntity);

    if (fixture->type() == QLCFixtureDef::ColorChanger ||
        fixture->type() == QLCFixtureDef::Dimmer)
    {
        meshPath.append("par.dae");
        newItem->setProperty("meshType", FixtureMeshType::ParMeshType);
    } 
    else if (fixture->type() == QLCFixtureDef::MovingHead) 
    {
        meshPath.append("moving_head.dae");
        newItem->setProperty("meshType", FixtureMeshType::MovingHeadMeshType);
    }
     else if (fixture->type() == QLCFixtureDef::Scanner) 
    {
        meshPath.append("scanner.dae");
        newItem->setProperty("meshType", FixtureMeshType::DefaultMeshType);
    } 
    else if (fixture->type() == QLCFixtureDef::Hazer) 
    {
        meshPath.append("hazer.dae");
        newItem->setProperty("meshType", FixtureMeshType::DefaultMeshType);
    } 
    else if (fixture->type() == QLCFixtureDef::Smoke) 
    {
        meshPath.append("smoke.dae");
        newItem->setProperty("meshType", FixtureMeshType::DefaultMeshType);
    } 

    newItem->setProperty("itemID", itemID);
    newItem->setProperty("itemSource", meshPath);
 
    // at last, add the new fixture to the items map
    m_entitiesMap[itemID] = mesh;
}

void MainView3D::setFixtureFlags(quint32 itemID, quint32 flags)
{
    SceneItem *meshRef = m_entitiesMap.value(itemID, NULL);
    if (meshRef == NULL)
        return;

    meshRef->m_rootItem->setProperty("enabled", (flags & MonitorProperties::HiddenFlag) ? false : true);
    meshRef->m_selectionBox->setProperty("enabled", (flags & MonitorProperties::HiddenFlag) ? false : true);
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

    for (SceneItem *mesh : m_entitiesMap.values())
    {
        if (mesh->m_lightIndex >= newIdx)
            newIdx = mesh->m_lightIndex + 1;
    }

    return newIdx;
}

QVector3D MainView3D::lightPosition(quint32 itemID)
{
    SceneItem *meshRef = m_entitiesMap.value(itemID, NULL);
    if (meshRef == NULL)    
        return QVector3D();

    return meshRef->m_rootItem->property("lightPos").value<QVector3D>();
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

void MainView3D::addVolumes(SceneItem *meshRef, QVector3D minCorner, QVector3D maxCorner)
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

QEntity *MainView3D::inspectEntity(QEntity *entity, SceneItem *meshRef,
                                   QLayer *layer, QEffect *effect,
                                   bool calculateVolume, QVector3D translation)
{
    if (entity == NULL)
        return NULL;

    QEntity *baseItem = NULL;
    QGeometryRenderer *geom = NULL;

    for (QComponent *component : entity->components()) // C++11
    {
        //qDebug() << "Class name:" << component->metaObject()->className();

        QMaterial *material = qobject_cast<QMaterial *>(component);
        Qt3DCore::QTransform *transform = qobject_cast<Qt3DCore::QTransform *>(component);

        if (geom == NULL)
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
            }
            else
            {
                material->addParameter(new QParameter("diffuse", QVector3D(0.64f, 0.64f, 0.64f)));
                material->addParameter(new QParameter("specular", QVector3D(0.64f, 0.64f, 0.64f)));
                material->addParameter(new QParameter("shininess", 1.0));
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

void MainView3D::initializeFixture(quint32 itemID, QEntity *fxEntity, QSceneLoader *loader)
{
    if (m_entitiesMap.contains(itemID) == false)
        return;

    quint32 fxID = FixtureUtils::itemFixtureID(itemID);
    quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
    quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    quint32 itemFlags = m_monProps->fixtureFlags(fxID, headIndex, linkedIndex);
    QLCFixtureMode *fxMode = fixture->fixtureMode();
    QVector3D fxSize(0.3, 0.3, 0.3);
    int panDeg = 0;
    int tiltDeg = 0;
    qreal focusMax = 30;
    qreal focusMin = 10;
    bool calculateVolume = false;

    if (fxMode != NULL)
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

    // The QSceneLoader instance is a component of an entity. The loaded scene
    // tree is added under this entity.
    QVector<QEntity *> entities = loader->entities();

    if (entities.isEmpty())
        return;

    // Technically there could be multiple entities referencing the scene loader
    // but sharing is discouraged, and in our case there will be one anyhow.
    QEntity *root = entities[0];

    qDebug() << "There are" << root->children().count() << "components in the loaded fixture";

    QLayer *sceneDeferredLayer = m_sceneRootEntity->property("deferredLayer").value<QLayer *>();
    QEffect *sceneEffect = m_sceneRootEntity->property("geometryPassEffect").value<QEffect *>();
    QEffect *spotlightShadingEffect = m_sceneRootEntity->property("spotlightShadingEffect").value<QEffect *>();
    QEffect *spotlightScatteringEffect = m_sceneRootEntity->property("spotlightScatteringEffect").value<QEffect *>();
    QEffect *outputFrontDepthEffect = m_sceneRootEntity->property("outputFrontDepthEffect").value<QEffect *>();

    qDebug() << sceneDeferredLayer << sceneEffect << spotlightShadingEffect << spotlightScatteringEffect << outputFrontDepthEffect;

    QVector3D translation;
    SceneItem *meshRef = m_entitiesMap.value(itemID);
    meshRef->m_rootItem = fxEntity;
    meshRef->m_rootTransform = getTransform(meshRef->m_rootItem);

    meshRef->m_spotlightShadingLayer = fxEntity->property("spotlightShadingLayer").value<QLayer *>();
    meshRef->m_spotlightScatteringLayer = fxEntity->property("spotlightScatteringLayer").value<QLayer *>();
    meshRef->m_outputDepthLayer = fxEntity->property("outputDepthLayer").value<QLayer *>();

    QTexture2D *tex = fxEntity->property("goboTexture").value<QTexture2D *>();
    //tex->setFormat(Qt3DRender::QAbstractTexture::RGBA8U);
    tex->addTextureImage(meshRef->m_goboTexture); 

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

        meshRef->m_rootItem->setProperty("focusMinDegrees", focusMin);
        meshRef->m_rootItem->setProperty("focusMaxDegrees", focusMax);

        QMetaObject::invokeMethod(meshRef->m_rootItem, "setupScattering",
                                  Q_ARG(QVariant, QVariant::fromValue(meshRef->m_spotlightShadingLayer)),
                                  Q_ARG(QVariant, QVariant::fromValue(meshRef->m_spotlightScatteringLayer)),
                                  Q_ARG(QVariant, QVariant::fromValue(meshRef->m_outputDepthLayer)),
                                  Q_ARG(QVariant, QVariant::fromValue(spotlightShadingEffect)),
                                  Q_ARG(QVariant, QVariant::fromValue(spotlightScatteringEffect)),
                                  Q_ARG(QVariant, QVariant::fromValue(outputFrontDepthEffect)),
                                  Q_ARG(QVariant, QVariant::fromValue(meshRef->m_headItem)),
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

    if (meshRef->m_rootTransform != NULL)
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

    QMetaObject::invokeMethod(m_scene3D, "updateSceneGraph", Q_ARG(QVariant, true));

    // at last, preview the fixture channels
    QByteArray values;
    updateFixture(fixture, values);
}

void MainView3D::updateFixture(Fixture *fixture, QByteArray &previous)
{
    if (m_enabled == false || fixture == NULL)
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
    SceneItem *meshItem = m_entitiesMap.value(itemID, NULL);
    QColor color;

    bool setPosition = false;
    bool goboSet = false;
    int panValue = 0;
    int tiltValue = 0;

    if (meshItem == NULL)
        return;

    QEntity *fixtureItem = meshItem->m_rootItem;
    if (fixtureItem == NULL)
        return;

    // in case of a dimmer pack, headIndex is actually the fixture channel
    // so treat this as a special case and go straight to the point
    if (fixture->type() == QLCFixtureDef::Dimmer)
    {
        qreal value = (qreal)fixture->channelValueAt(headIndex) / 255.0;
        fixtureItem->setProperty("dimmerValue", value);

        QColor gelColor = m_monProps->fixtureGelColor(fixture->id(), headIndex, linkedIndex);
        if (gelColor.isValid() == false)
            gelColor = Qt::white;

        fixtureItem->setProperty("lightColor", gelColor);

        return;
    }

    quint32 headDimmerIndex = fixture->channelNumber(QLCChannel::Intensity, QLCChannel::MSB);
    qreal intensityValue = 1.0;
    if (headDimmerIndex != QLCChannel::invalid())
        intensityValue = (qreal)fixture->channelValueAt(headDimmerIndex) / 255;

    if (previous.isEmpty() || fixture->channelValueAt(headDimmerIndex) != previous.at(headDimmerIndex))
        fixtureItem->setProperty("dimmerValue", intensityValue);

    color = FixtureUtils::headColor(fixture);

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
                    panValue += (value << 8);
                else
                    panValue += (value);

                if (previous.isEmpty() || value != previous.at(i))
                    setPosition = true;
            }
            break;
            case QLCChannel::Tilt:
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    tiltValue += (value << 8);
                else
                    tiltValue += (value);

                if (previous.isEmpty() || value != previous.at(i))
                    setPosition = true;
            }
            break;
            case QLCChannel::Speed:
            {
                if (previous.count() && value == previous.at(i))
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
                // if a fixture has both RGB/CMY and a color wheel
                // this is where RGB wins over the wheel
                if (color.isValid() && color.rgb() != 0xFF000000)
                    break;

                QLCCapability *cap = ch->searchCapability(value);

                if (cap == NULL ||
                   (cap->presetType() != QLCCapability::SingleColor &&
                    cap->presetType() != QLCCapability::DoubleColor))
                    break;

                if (cap->resource(0).isValid())
                    color = cap->resource(0).value<QColor>();
            }
            break;
            case QLCChannel::Beam:
            {
                if (previous.count() && value == previous.at(i))
                    break;

                QMetaObject::invokeMethod(fixtureItem, "setFocus",
                                          Q_ARG(QVariant, value));
            }
            break;
            case QLCChannel::Gobo:
            {
                if (previous.count() && value == previous.at(i))
                    break;

                QLCCapability *cap = ch->searchCapability(value);

                if (cap == NULL)
                    break;

                switch (cap->preset())
                {
                    case QLCCapability::GoboMacro:
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
                if (previous.count() && value == previous.at(i))
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

    fixtureItem->setProperty("lightColor", color);
}

void MainView3D::updateFixtureSelection(QList<quint32> fixtures)
{
    QMapIterator<quint32, SceneItem*> it(m_entitiesMap);
    while(it.hasNext())
    {
        it.next();
        quint32 fxID = it.key();
        SceneItem *meshRef = m_entitiesMap.value(fxID);

        if(fixtures.contains(fxID))
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

    SceneItem *meshRef = m_entitiesMap.value(itemID, NULL);
    if (meshRef)
    {
        meshRef->m_rootItem->setProperty("isSelected", enable);
        meshRef->m_selectionBox->setProperty("isSelected", enable);
    }
}

void MainView3D::updateFixturePosition(quint32 itemID, QVector3D pos)
{
    if (isEnabled() == false)
        return;

    SceneItem *mesh = m_entitiesMap.value(itemID, NULL);
    if (mesh == NULL || mesh->m_rootTransform == NULL)
        return;

    qDebug() << Q_FUNC_INFO << pos;

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

    SceneItem *mesh = m_entitiesMap.value(itemID, NULL);
    if (mesh == NULL || mesh->m_rootTransform == NULL)
        return;

    qDebug() << Q_FUNC_INFO << degrees;

    mesh->m_rootTransform->setRotationX(degrees.x());
    mesh->m_rootTransform->setRotationY(degrees.y());
    mesh->m_rootTransform->setRotationZ(degrees.z());

    updateLightMatrix(mesh);
}

void MainView3D::updateLightMatrix(SceneItem *mesh)
{
    // below, we extract a rotation matrix and position, which we need for properly
    // positioning and rotating the spotlight cone.
    if (mesh->m_headItem) {
        QMatrix4x4 m = (mesh->m_rootTransform->matrix());

        if (mesh->m_armItem)
        {
            QMatrix4x4 armTransform = getTransform(mesh->m_armItem)->matrix();
            m.translate(armTransform.data()[12],armTransform.data()[13],armTransform.data()[14]);
        }
 
        if (mesh->m_headItem) {
            QMatrix4x4 headTransform = getTransform(mesh->m_headItem)->matrix();
            m.translate(headTransform.data()[12],headTransform.data()[13],headTransform.data()[14]);
        }

        QVector4D xb = m * QVector4D(1,0,0,0);
        QVector4D yb = m * QVector4D(0,1,0,0);
        QVector4D zb = m * QVector4D(0,0,1,0);
     
        QVector3D xa = QVector3D(xb.x(), xb.y(), xb.z()).normalized();
        QVector3D ya = QVector3D(yb.x(), yb.y(), yb.z()).normalized();
        QVector3D za = QVector3D(zb.x(), zb.y(), zb.z()).normalized();
     
        QMatrix4x4 lightMatrix = QMatrix4x4(
            xa.x(), xa.y(), xa.z(), 0,
            ya.x(), ya.y(), ya.z(), 0,
            za.x(), za.y(), za.z(), 0,
            0, 0, 0, 1
        ).transposed();

        QVector4D result = m * QVector4D(0,0,0,1); 

        mesh->m_rootItem->setProperty("lightPos", QVector3D(result.x(), result.y(), result.z()) );
        mesh->m_rootItem->setProperty("lightMatrix", lightMatrix);
    }
}

void MainView3D::updateFixtureScale(quint32 itemID, QVector3D origSize)
{
    if (isEnabled() == false)
        return;

    SceneItem *mesh = m_entitiesMap.value(itemID, NULL);
    if (mesh == NULL || mesh->m_rootTransform == NULL)
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

    delete mesh->m_rootItem;
    delete mesh->m_selectionBox;

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

    if (m_quadEntity == NULL)
        initialize3DProperties();

    if (itemID == -1)
    {
        QString resFile = QString(filename);
        if (resFile.startsWith(meshDirectory()))
            resFile = QUrl(filename).toLocalFile();
        else
            resFile.remove("file:///");
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
            meshFile.remove("file:///");
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
                    filename = "file:///" + m_doc->getWorkspacePath() + QDir::separator() + filename;
                }
            }
            else
            {
                filename = meshDirectory() + filename;
            }
        }
        else
        {
            filename = "file:///" + filename;
        }

        m_latestGenericID = itemID;
    }

    SceneItem *mesh = new SceneItem;
    mesh->m_rootItem = NULL;
    mesh->m_rootTransform = NULL;
    mesh->m_selectionBox = NULL;

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
    /*QEntity *baseItem =*/ inspectEntity(root, meshRef, sceneDeferredLayer, sceneEffect, calculateVolume, translation);

    qDebug() << "Calculated volume" << meshRef->m_volume.m_extents << meshRef->m_volume.m_center;

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

    if (meshRef->m_rootTransform != NULL)
    {
        QMetaObject::invokeMethod(meshRef->m_selectionBox, "bindItemTransform",
                Q_ARG(QVariant, itemID),
                Q_ARG(QVariant, QVariant::fromValue(meshRef->m_rootTransform)));
    }

    itemEntity->setProperty("sceneLayer", QVariant::fromValue(sceneDeferredLayer));
    itemEntity->setProperty("effect", QVariant::fromValue(sceneEffect));
}

void MainView3D::setItemSelection(int itemID, bool enable, int keyModifiers)
{
    if (isEnabled() == false)
        return;

    if (enable && keyModifiers == 0)
    {
        for (int id : m_genericSelectedItems)
        {
            SceneItem *meshRef = m_genericMap.value(id, NULL);
            if (meshRef)
            {
                meshRef->m_rootItem->setProperty("isSelected", false);
                meshRef->m_selectionBox->setProperty("isSelected", false);
            }
        }
        m_genericSelectedItems.clear();
        emit genericSelectedCountChanged();
    }

    SceneItem *meshRef = m_genericMap.value(itemID, NULL);
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
    for (int id : m_genericSelectedItems)
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
}

void MainView3D::updateGenericItemPosition(quint32 itemID, QVector3D pos)
{
    if (isEnabled() == false)
        return;

    m_monProps->setItemPosition(itemID, pos);

    SceneItem *item = m_genericMap.value(itemID, NULL);
    if (item == NULL || item->m_rootTransform == NULL)
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
        for (int itemID : m_genericSelectedItems)
        {
            QVector3D newPos = m_monProps->itemPosition(itemID) + pos;
            updateGenericItemPosition(m_genericSelectedItems.first(), newPos);
        }
    }

    emit genericItemsPositionChanged();
}

void MainView3D::updateGenericItemRotation(quint32 itemID, QVector3D rot)
{
    if (isEnabled() == false)
        return;

    m_monProps->setItemRotation(itemID, rot);
    SceneItem *item = m_genericMap.value(itemID, NULL);
    if (item == NULL || item->m_rootTransform == NULL)
        return;

    item->m_rootTransform->setRotationX(rot.x());
    item->m_rootTransform->setRotationY(rot.y());
    item->m_rootTransform->setRotationZ(rot.z());
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
        for (int itemID : m_genericSelectedItems)
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

    m_monProps->setItemScale(itemID, scale);
    SceneItem *item = m_genericMap.value(itemID, NULL);
    if (item == NULL || item->m_rootTransform == NULL)
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
        for (int itemID : m_genericSelectedItems)
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
    update();
}

void GoboTextureImage::paint(QPainter *painter)
{  
    int w = painter->device()->width();
    int h = painter->device()->height();

    QIcon goboFile(m_source);
    painter->fillRect(0, 0, w, h, Qt::black);
    painter->setBrush(QBrush(Qt::white));
    painter->drawEllipse(2, 2, w - 4, h - 4);
    painter->drawPixmap(1, 1, w - 2, h - 2, goboFile.pixmap(QSize(w - 2, h - 2)));
}
