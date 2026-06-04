/*
  Q Light Controller Plus
  mainview3drhi.cpp

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

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QModelIndex>
#include <QPointer>
#include <QQuaternion>
#include <QQuickItem>
#include <QSet>
#include <QtMath>
#include <QQmlComponent>
#include <QQmlContext>
#include <QUrl>
#include <utility>

#include "contextmanager.h"
#include "doc.h"
#include "fixtureutils.h"
#include "listmodel.h"
#include "mainview3drhi.h"
#include "monitorproperties.h"
#include "qlccapability.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "qlcfixturemode.h"
#include "qml/BeamBarItem.h"
#include "qml/CameraItem.h"
#include "qml/CubeItem.h"
#include "qml/HazerItem.h"
#include "qml/LightItem.h"
#include "qml/ModelItem.h"
#include "qml/MovingHeadItem.h"
#include "qml/PixelBarItem.h"
#include "qml/RhiQmlItem.h"
#include "qml/SphereItem.h"
#include "qml/StaticLightItem.h"
#include "qml/VideoItem.h"
#include "scene/AssimpLoader.h"
#include "tardis.h"

namespace
{
struct RhiItemState
{
    QObject *item = nullptr;
    QVector3D lightPos;
    QMatrix4x4 lightMatrix;
    QVector3D placementExtents = QVector3D(0.0f, 0.0f, 0.0f);
    bool placementExtentsValid = false;
};

struct GenericModelBounds
{
    QVector3D extents = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D center = QVector3D(0.0f, 0.0f, 0.0f);
    bool valid = false;
};

struct RhiGenericItemState
{
    QObject *item = nullptr;
    GenericModelBounds bounds;
};

struct RhiViewState
{
    QPointer<QQuickItem> renderer;
    QMap<quint32, RhiItemState> items;
    QMap<quint32, RhiGenericItemState> genericItems;
    QHash<QString, GenericModelBounds> genericBoundsCache;
};

static QHash<MainView3D *, RhiViewState> s_rhiViewStates;
static QSet<quint32> s_missingRhiItemsLogged;
static QSet<MainView3D *> s_rendererResolvedLogged;
struct ZoomLensDebugState
{
    float lensMin = 0.0f;
    float lensMax = 0.0f;
    float focusMin = 0.0f;
    float focusMax = 0.0f;
    float zoom = 0.0f;
    bool zoomSet = false;
    bool hasZoomChannel = false;
    bool zoomMsbSet = false;
    bool zoomLsbSet = false;
    quint16 zoomWord = 0;
    bool beamMode = false;
};
static QHash<quint32, ZoomLensDebugState> s_zoomLensDebugState;

static RhiViewState &rhiState(MainView3D *self)
{
    return s_rhiViewStates[self];
}

static bool hasProperty(const QObject *obj, const char *name)
{
    if (obj == nullptr || name == nullptr)
        return false;
    return obj->metaObject()->indexOfProperty(name) >= 0;
}

static QVector3D colorToVec3(const QColor &color)
{
    return QVector3D(color.redF(), color.greenF(), color.blueF());
}

static QString localPathForFileCheck(const QString &path)
{
    const QUrl url(path);
    if (url.isValid() && url.isLocalFile())
        return url.toLocalFile();
    return path;
}

static QString genericBoundsCacheKey(const QString &path)
{
    const QString localPath = localPathForFileCheck(path);
    return QDir::cleanPath(localPath);
}

static GenericModelBounds computeGenericModelBounds(const QString &path)
{
    GenericModelBounds bounds;
    if (path.isEmpty())
        return bounds;

    AssimpLoader loader;
    RhiScene scene;
    if (!loader.loadModel(path, scene, false))
        return bounds;

    bool haveBounds = false;
    QVector3D minV;
    QVector3D maxV;

    for (const Mesh &mesh : scene.meshes())
    {
        if (!mesh.boundsValid)
            continue;

        const QVector3D bmin = mesh.boundsMin;
        const QVector3D bmax = mesh.boundsMax;
        const QVector3D corners[8] = {
            QVector3D(bmin.x(), bmin.y(), bmin.z()),
            QVector3D(bmax.x(), bmin.y(), bmin.z()),
            QVector3D(bmin.x(), bmax.y(), bmin.z()),
            QVector3D(bmax.x(), bmax.y(), bmin.z()),
            QVector3D(bmin.x(), bmin.y(), bmax.z()),
            QVector3D(bmax.x(), bmin.y(), bmax.z()),
            QVector3D(bmin.x(), bmax.y(), bmax.z()),
            QVector3D(bmax.x(), bmax.y(), bmax.z())
        };

        for (const QVector3D &corner : corners)
        {
            const QVector3D p = (mesh.modelMatrix * QVector4D(corner, 1.0f)).toVector3D();
            if (!haveBounds)
            {
                minV = p;
                maxV = p;
                haveBounds = true;
            }
            else
            {
                minV.setX(qMin(minV.x(), p.x()));
                minV.setY(qMin(minV.y(), p.y()));
                minV.setZ(qMin(minV.z(), p.z()));
                maxV.setX(qMax(maxV.x(), p.x()));
                maxV.setY(qMax(maxV.y(), p.y()));
                maxV.setZ(qMax(maxV.z(), p.z()));
            }
        }
    }

    if (!haveBounds)
        return bounds;

    bounds.extents = maxV - minV;
    bounds.center = (minV + maxV) * 0.5f;
    bounds.valid = bounds.extents.x() > 1e-4f
            && bounds.extents.y() > 1e-4f
            && bounds.extents.z() > 1e-4f;
    return bounds;
}

static const char *fixtureTypeName(QLCFixtureDef::FixtureType type)
{
    switch (type)
    {
        case QLCFixtureDef::Dimmer:
            return "Dimmer";
        case QLCFixtureDef::ColorChanger:
            return "ColorChanger";
        case QLCFixtureDef::MovingHead:
            return "MovingHead";
        case QLCFixtureDef::Scanner:
            return "Scanner";
        case QLCFixtureDef::LEDBarPixels:
            return "LEDBarPixels";
        case QLCFixtureDef::LEDBarBeams:
            return "LEDBarBeams";
        case QLCFixtureDef::Strobe:
            return "Strobe";
        case QLCFixtureDef::Hazer:
            return "Hazer";
        case QLCFixtureDef::Smoke:
            return "Smoke";
        default:
            return "Unknown";
    }
}

static float dmxToUnit(uchar value)
{
    return qBound(0.0f, float(value) / 255.0f, 1.0f);
}

static float dmx16ToUnit(quint16 value)
{
    return qBound(0.0f, float(value) / 65535.0f, 1.0f);
}

static bool shouldUseBeamShape(float zoomDegrees, float lensMinDegrees, float lensMaxDegrees)
{
    if (zoomDegrees > 0.0f && zoomDegrees <= 5.0f)
        return true;

    bool haveLensValue = false;
    float minLensDegrees = 0.0f;
    if (lensMinDegrees > 0.0f)
    {
        minLensDegrees = lensMinDegrees;
        haveLensValue = true;
    }
    if (lensMaxDegrees > 0.0f)
    {
        minLensDegrees = haveLensValue ? qMin(minLensDegrees, lensMaxDegrees) : lensMaxDegrees;
        haveLensValue = true;
    }

    if (!haveLensValue)
        return false;

    return minLensDegrees < 5.0f;
}

static QQuaternion rotationFromAxesOrderXYZ(const QVector3D &rotationDegrees)
{
    // Match legacy Qt3D behavior from QTransform::fromAxesAndAngles(X, Y, Z).
    return QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, rotationDegrees.z())
            * QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, rotationDegrees.y())
            * QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, rotationDegrees.x());
}

static QString encodeDoubleColorGoboPath(const QColor &colorA, const QColor &colorB)
{
    auto toHexRgb = [](const QColor &color) -> QString
    {
        return QStringLiteral("%1%2%3")
                .arg(color.red(), 2, 16, QChar('0'))
                .arg(color.green(), 2, 16, QChar('0'))
                .arg(color.blue(), 2, 16, QChar('0'))
                .toLower();
    };

    return QStringLiteral("colorwheel://double/%1/%2").arg(toHexRgb(colorA), toHexRgb(colorB));
}

static bool decodeDoubleColorGoboPath(const QString &path, QColor &colorA, QColor &colorB, QString *goboPath = nullptr)
{
    static const QString prefix = QStringLiteral("colorwheel://double/");
    if (!path.startsWith(prefix, Qt::CaseInsensitive))
        return false;

    const QString tail = path.mid(prefix.length());
    const int queryPos = tail.indexOf(QLatin1Char('?'));
    const QString wheelPart = queryPos >= 0 ? tail.left(queryPos) : tail;
    const QString queryPart = queryPos >= 0 ? tail.mid(queryPos + 1) : QString();

    const QStringList parts = wheelPart.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (parts.size() != 2)
        return false;

    const QColor c1(QStringLiteral("#") + parts[0]);
    const QColor c2(QStringLiteral("#") + parts[1]);
    if (!c1.isValid() || !c2.isValid())
        return false;

    colorA = c1;
    colorB = c2;

    if (goboPath != nullptr)
    {
        goboPath->clear();
        if (queryPart.startsWith(QLatin1String("gobo="), Qt::CaseInsensitive))
        {
            const QString encoded = queryPart.mid(5);
            *goboPath = QUrl::fromPercentEncoding(encoded.toUtf8());
        }
    }
    return true;
}

static QString encodeDoubleColorCompositePath(const QColor &colorA, const QColor &colorB, const QString &goboPath)
{
    QString path = encodeDoubleColorGoboPath(colorA, colorB);
    if (!goboPath.isEmpty())
        path += QStringLiteral("?gobo=%1").arg(QString::fromLatin1(QUrl::toPercentEncoding(goboPath)));
    return path;
}

static float shutterIntensityMultiplier(int shutterPreset, int highTimeMs, int lowTimeMs, qint64 nowMs)
{
    switch (shutterPreset)
    {
        case QLCCapability::ShutterClose:
            return 0.0f;
        case QLCCapability::ShutterOpen:
            return 1.0f;
        default:
        break;
    }

    const int highMs = qMax(1, highTimeMs);
    const int lowMs = qMax(0, lowTimeMs);
    const int periodMs = qMax(1, highMs + lowMs);
    const int phaseMs = int(nowMs % qint64(periodMs));

    switch (shutterPreset)
    {
        case QLCCapability::StrobeSlowToFast:
        case QLCCapability::StrobeFastToSlow:
        case QLCCapability::StrobeRandom:
        case QLCCapability::StrobeRandomSlowToFast:
        case QLCCapability::StrobeRandomFastToSlow:
        case QLCCapability::StrobeFrequency:
        case QLCCapability::StrobeFreqRange:
            return phaseMs < highMs ? 1.0f : 0.0f;
        case QLCCapability::PulseSlowToFast:
        case QLCCapability::PulseFastToSlow:
        case QLCCapability::PulseFrequency:
        case QLCCapability::PulseFreqRange:
        {
            if (phaseMs >= highMs)
                return 0.0f;
            const float phase = float(phaseMs) / float(highMs);
            return phase < 0.5f ? (phase * 2.0f) : ((1.0f - phase) * 2.0f);
        }
        case QLCCapability::RampUpSlowToFast:
        case QLCCapability::RampUpFastToSlow:
        case QLCCapability::RampUpFrequency:
        case QLCCapability::RampUpFreqRange:
            if (phaseMs >= highMs)
                return 0.0f;
            return float(phaseMs) / float(highMs);
        case QLCCapability::RampDownSlowToFast:
        case QLCCapability::RampDownFastToSlow:
        case QLCCapability::RampDownFrequency:
        case QLCCapability::RampDownFreqRange:
            if (phaseMs >= highMs)
                return 0.0f;
            return 1.0f - (float(phaseMs) / float(highMs));
        default:
            return 1.0f;
    }
}
}

MainView3D::MainView3D(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "3D", parent)
    , m_monProps(doc->monitorProperties())
    , m_frameCountEnabled(false)
    , m_frameCount(0)
    , m_minFrameCount(0)
    , m_maxFrameCount(0)
    , m_avgFrameCount(1.0f)
    , m_ambientIntensity(0.6f)
    , m_smokeAmount(0.8f)
    , m_latestGenericID(0)
    , m_genericItemsList(nullptr)
{
    static bool qmlTypesRegistered = false;
    if (!qmlTypesRegistered)
    {
        qmlTypesRegistered = true;
        qmlRegisterType<RhiQmlItem>("RhiQmlItem", 1, 0, "RhiQmlItem");
        qmlRegisterUncreatableType<RhiQmlItem>("RhiQmlItem", 1, 0, "Scene", "Scene enums only");
        qmlRegisterType<LightItem>("RhiQmlItem", 1, 0, "Light");
        qmlRegisterType<HazerItem>("RhiQmlItem", 1, 0, "Hazer");
        qmlRegisterType<CameraItem>("RhiQmlItem", 1, 0, "Camera");
        qmlRegisterType<ModelItem>("RhiQmlItem", 1, 0, "Model");
        qmlRegisterType<MovingHeadItem>("RhiQmlItem", 1, 0, "MovingHeadLight");
        qmlRegisterType<CubeItem>("RhiQmlItem", 1, 0, "Cube");
        qmlRegisterType<SphereItem>("RhiQmlItem", 1, 0, "Sphere");
        qmlRegisterType<StaticLightItem>("RhiQmlItem", 1, 0, "StaticLight");
        qmlRegisterType<PixelBarItem>("RhiQmlItem", 1, 0, "PixelBar");
        qmlRegisterType<BeamBarItem>("RhiQmlItem", 1, 0, "BeamBar");
        qmlRegisterType<VideoItem>("RhiQmlItem", 1, 0, "VideoItem");
        qmlRegisterUncreatableType<MainView3D>("org.qlcplus.classes", 1, 0, "MainView3D",
                                               "Can't create an MainView3D!");
    }

    setContextResource("qrc:/3DViewRhi/3DView.qml");
    setContextTitle(tr("3D View"));

    m_stagesList << tr("Simple ground") << tr("Simple box") << tr("Rock stage") << tr("Theatre stage");

    m_genericItemsList = new ListModel(this);
    QStringList listRoles;
    listRoles << "itemID" << "name" << "isSelected";
    m_genericItemsList->setRoleNames(listRoles);

    resetCameraPosition();
}

MainView3D::~MainView3D()
{
    resetItems();
    s_rhiViewStates.remove(this);
    s_rendererResolvedLogged.remove(this);
}

void MainView3D::enableContext(bool enable)
{
    qDebug() << "Enable 3D context..." << enable;

    PreviewContext::enableContext(enable);
    if (!enable)
    {
        resetItems();
        rhiState(this).renderer = nullptr;
        return;
    }

    QMetaObject::invokeMethod(this, "slotRefreshView", Qt::QueuedConnection);
}

void MainView3D::slotRefreshView()
{
    if (!isEnabled())
        return;

    resetItems();
    initialize3DProperties();

    qDebug() << "Refreshing 3D view (RHI)..."
             << "fixtures in doc:" << m_doc->fixtures().size()
             << "monitor stage:" << m_monProps->stageType();

    for (Fixture *fixture : m_doc->fixtures())
    {
        if (m_monProps->containsFixture(fixture->id()))
        {
            for (quint32 &subID : m_monProps->fixtureIDList(fixture->id()))
            {
                const quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
                const quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
                createFixtureItem(fixture->id(), headIndex, linkedIndex,
                                  m_monProps->fixturePosition(fixture->id(), headIndex, linkedIndex), true);
            }
        }
        else
        {
            createFixtureItems(fixture->id(), QVector3D(0.0f, 0.0f, 0.0f), false);
        }
    }

    for (quint32 &itemID : m_monProps->genericItemsID())
    {
        const QString path = m_monProps->itemResource(itemID);
        createGenericItem(path, int(itemID));
    }

    qDebug() << "MainView3D(RHI): refresh complete."
             << "created fixture items:" << rhiState(this).items.size()
             << "created generic items:" << rhiState(this).genericItems.size();
}

void MainView3D::resetItems()
{
    qDebug() << "Resetting 3D items...";

    RhiViewState &state = rhiState(this);
    for (auto it = state.items.begin(); it != state.items.end(); ++it)
    {
        if (it.value().item)
            it.value().item->deleteLater();
    }
    state.items.clear();
    for (auto it = state.genericItems.begin(); it != state.genericItems.end(); ++it)
    {
        if (it.value().item)
            it.value().item->deleteLater();
    }
    state.genericItems.clear();
    state.genericBoundsCache.clear();

    m_genericSelectedItems.clear();
    m_latestGenericID = 0;
    if (m_genericItemsList)
        m_genericItemsList->clear();
    emit genericSelectedCountChanged();
    emit genericItemsListChanged();

    m_frameCount = 0;
    m_minFrameCount = 0;
    m_maxFrameCount = 0;
    m_avgFrameCount = 1.0f;
    setFrameCountEnabled(false);

    s_zoomLensDebugState.clear();
}

void MainView3D::resetCameraPosition()
{
    setCameraPosition(QVector3D(0.0f, 3.0f, 7.5f));
    setCameraViewCenter(QVector3D(0.0f, 1.0f, 0.0f));
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
    if (!isEnabled())
        return;

    RhiViewState &state = rhiState(this);
    for (auto it = state.items.begin(); it != state.items.end(); ++it)
    {
        QObject *item = it.value().item;
        if (item == nullptr)
            continue;

        const quint32 fixtureID = FixtureUtils::itemFixtureID(it.key());
        Fixture *fixture = m_doc->fixture(fixtureID);
        if (fixture == nullptr)
            continue;

        const bool visible = (universeFilter == Universe::invalid() || fixture->universe() == universeFilter);
        item->setProperty("visible", visible);
    }
}

bool MainView3D::frameCountEnabled() const
{
    return m_frameCountEnabled;
}

void MainView3D::setFrameCountEnabled(bool enable)
{
    if (m_frameCountEnabled == enable)
        return;

    m_frameCountEnabled = enable;
    if (!enable)
    {
        m_frameCount = 0;
        m_minFrameCount = 0;
        m_maxFrameCount = 0;
        m_avgFrameCount = 1.0f;
    }
    emit frameCountEnabledChanged();
}

void MainView3D::initialize3DProperties()
{
    RhiViewState &state = rhiState(this);
    if (state.renderer != nullptr || !m_view || !m_view->rootObject())
        return;

    state.renderer = qobject_cast<QQuickItem *>(m_view->rootObject()->findChild<QObject *>("sceneRenderer"));
    if (state.renderer == nullptr)
    {
        qWarning() << "RHI scene renderer not found!";
        return;
    }

    if (!s_rendererResolvedLogged.contains(this))
    {
        s_rendererResolvedLogged.insert(this);
        qDebug() << "MainView3D(RHI): scene renderer resolved"
                 << state.renderer.data()
                 << "class" << state.renderer->metaObject()->className()
                 << "objectName" << state.renderer->objectName();
    }

    if (RhiQmlItem *rhiRenderer = qobject_cast<RhiQmlItem *>(state.renderer.data()))
    {
        connect(rhiRenderer, &RhiQmlItem::objectPositionEdited,
                this, &MainView3D::syncSceneItemPosition, Qt::UniqueConnection);
        connect(rhiRenderer, &RhiQmlItem::objectRotationEdited,
                this, &MainView3D::syncSceneItemRotation, Qt::UniqueConnection);
    }
}

void MainView3D::syncSceneItemPosition(QObject *item, const QVector3D &scenePos)
{
    if (!isEnabled() || item == nullptr)
        return;

    RhiViewState &state = rhiState(this);
    ContextManager *ctxManager = nullptr;
    if (m_view && m_view->rootContext())
    {
        QObject *ctxObj = m_view->rootContext()->contextProperty("contextManager").value<QObject *>();
        ctxManager = qobject_cast<ContextManager *>(ctxObj);
    }

    const float unitScale = m_monProps->gridUnits() == MonitorProperties::Meters ? 1.0f : 0.3048f;
    const QVector3D gridMeters = m_monProps->gridSize() * unitScale;

    for (auto it = state.items.cbegin(); it != state.items.cend(); ++it)
    {
        if (it.value().item != item)
            continue;

        const QVector3D extents = it.value().placementExtentsValid
                ? it.value().placementExtents : QVector3D(0.0f, 0.0f, 0.0f);
        const QVector3D monitorPos((scenePos.x() + (gridMeters.x() * 0.5f) - (extents.x() * 0.5f)) * 1000.0f,
                                   (scenePos.y() - (extents.y() * 0.5f)) * 1000.0f,
                                   (scenePos.z() + (gridMeters.z() * 0.5f) - (extents.z() * 0.5f)) * 1000.0f);

        const quint32 itemID = it.key();
        const quint32 fxID = FixtureUtils::itemFixtureID(itemID);
        const quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        const quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
        const QVector3D currPos = m_monProps->fixturePosition(fxID, headIndex, linkedIndex);
        if ((currPos - monitorPos).lengthSquared() < 0.25f)
            return;

        if (ctxManager)
        {
            ctxManager->setFixturePosition(itemID,
                                           qreal(monitorPos.x()),
                                           qreal(monitorPos.y()),
                                           qreal(monitorPos.z()));
        }
        else
        {
            m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, monitorPos);
            updateFixturePosition(itemID, monitorPos);
        }
        return;
    }

    for (auto it = state.genericItems.cbegin(); it != state.genericItems.cend(); ++it)
    {
        if (it.value().item != item)
            continue;

        const QVector3D extents = it.value().bounds.valid
                ? it.value().bounds.extents : QVector3D(0.0f, 0.0f, 0.0f);
        const QVector3D monitorPos((scenePos.x() + (gridMeters.x() * 0.5f) - (extents.x() * 0.5f)) * 1000.0f,
                                   (scenePos.y() - (extents.y() * 0.5f)) * 1000.0f,
                                   (scenePos.z() + (gridMeters.z() * 0.5f) - (extents.z() * 0.5f)) * 1000.0f);
        const quint32 itemID = it.key();
        const QVector3D currPos = m_monProps->itemPosition(itemID);
        if ((currPos - monitorPos).lengthSquared() < 0.25f)
            return;

        updateGenericItemPosition(itemID, monitorPos);
        emit genericItemsPositionChanged();
        return;
    }
}

void MainView3D::syncSceneItemRotation(QObject *item, const QVector3D &sceneRot)
{
    if (!isEnabled() || item == nullptr)
        return;

    RhiViewState &state = rhiState(this);
    ContextManager *ctxManager = nullptr;
    if (m_view && m_view->rootContext())
    {
        QObject *ctxObj = m_view->rootContext()->contextProperty("contextManager").value<QObject *>();
        ctxManager = qobject_cast<ContextManager *>(ctxObj);
    }

    for (auto it = state.items.cbegin(); it != state.items.cend(); ++it)
    {
        if (it.value().item != item)
            continue;

        const quint32 itemID = it.key();
        const quint32 fxID = FixtureUtils::itemFixtureID(itemID);
        const quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        const quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
        const QVector3D monitorRot(-sceneRot.x(), -sceneRot.y(), -sceneRot.z());
        const QVector3D currRot = m_monProps->fixtureRotation(fxID, headIndex, linkedIndex);
        if ((currRot - monitorRot).lengthSquared() < 0.01f)
            return;

        if (ctxManager)
        {
            ctxManager->setFixtureRotation(itemID, monitorRot);
        }
        else
        {
            m_monProps->setFixtureRotation(fxID, headIndex, linkedIndex, monitorRot);
            updateFixtureRotation(itemID, monitorRot);
        }
        return;
    }

    for (auto it = state.genericItems.cbegin(); it != state.genericItems.cend(); ++it)
    {
        if (it.value().item != item)
            continue;

        const quint32 itemID = it.key();
        const QVector3D currRot = m_monProps->itemRotation(itemID);
        if ((currRot - sceneRot).lengthSquared() < 0.01f)
            return;

        updateGenericItemRotation(itemID, sceneRot);
        emit genericItemsRotationChanged();
        return;
    }
}

void MainView3D::createFixtureItems(quint32 fxID, QVector3D pos, bool mmCoords)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
    {
        qWarning() << "MainView3D(RHI): fixture not found while creating items for id" << fxID;
        return;
    }

    qDebug() << "MainView3D(RHI): createFixtureItems"
             << "fixture" << fxID
             << "type" << fixtureTypeName(fixture->type())
             << "channels" << fixture->channels()
             << "heads" << fixture->heads()
             << "mmCoords" << mmCoords
             << "pos(mm)" << pos;

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
    if (!isEnabled())
        return;

    initialize3DProperties();
    RhiViewState &state = rhiState(this);
    if (state.renderer == nullptr)
    {
        qWarning() << "MainView3D(RHI): renderer not ready while creating fixture item"
                   << "fixture" << fxID << "head" << headIndex << "linked" << linkedIndex;
        return;
    }

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
    {
        qWarning() << "MainView3D(RHI): fixture not found while creating item"
                   << "fixture" << fxID << "head" << headIndex << "linked" << linkedIndex;
        return;
    }

    const quint32 itemID = FixtureUtils::fixtureItemID(fxID, headIndex, linkedIndex);
    if (state.items.contains(itemID))
    {
        if (state.items[itemID].item)
            state.items[itemID].item->deleteLater();
        state.items.remove(itemID);
    }

    QVector3D physicalSize(0.3f, 0.3f, 0.3f);
    if (QLCFixtureMode *fxMode = fixture->fixtureMode())
    {
        const QLCPhysical phy = fxMode->physical();
        if (phy.width() > 0)
            physicalSize.setX(float(phy.width()) / 1000.0f);
        if (phy.height() > 0)
            physicalSize.setY(float(phy.height()) / 1000.0f);
        if (phy.depth() > 0)
            physicalSize.setZ(float(phy.depth()) / 1000.0f);
    }

    QString typeName = QStringLiteral("StaticLight");
    QString meshName = QStringLiteral("par.glb");
    switch (fixture->type())
    {
        case QLCFixtureDef::ColorChanger:
        case QLCFixtureDef::Dimmer:
            typeName = QStringLiteral("StaticLight");
            meshName = QStringLiteral("par.glb");
        break;
        case QLCFixtureDef::MovingHead:
        case QLCFixtureDef::Scanner:
            typeName = QStringLiteral("MovingHeadLight");
            meshName = QStringLiteral("moving_head.glb");
        break;
        case QLCFixtureDef::LEDBarPixels:
        case QLCFixtureDef::Strobe:
            typeName = QStringLiteral("PixelBar");
            meshName.clear();
        break;
        case QLCFixtureDef::LEDBarBeams:
            typeName = QStringLiteral("BeamBar");
            meshName.clear();
        break;
        case QLCFixtureDef::Hazer:
            meshName = QStringLiteral("hazer.glb");
        break;
        case QLCFixtureDef::Smoke:
            meshName = QStringLiteral("smoke.glb");
        break;
        default:
            typeName = QStringLiteral("StaticLight");
            meshName = QStringLiteral("par.glb");
        break;
    }

    QQmlComponent component(m_view->engine());
    bool usingFixtureWrapper = false;
    if (typeName == QLatin1String("StaticLight"))
    {
        usingFixtureWrapper = true;
        component.loadUrl(QUrl(QStringLiteral("qrc:/3DViewRhi/StaticLightFixture.qml")));
    }
    else if (typeName == QLatin1String("MovingHeadLight"))
    {
        usingFixtureWrapper = true;
        component.loadUrl(QUrl(QStringLiteral("qrc:/3DViewRhi/MovingHeadFixture.qml")));
    }
    else
    {
        const QByteArray qmlData = QStringLiteral("import RhiQmlItem 1.0\n%1 {}").arg(typeName).toUtf8();
        component.setData(qmlData, QUrl());
    }

    auto createInlineFixture = [this](const QString &inlineTypeName) -> QObject *
    {
        QQmlComponent fallbackComp(m_view->engine());
        fallbackComp.setData(QStringLiteral("import RhiQmlItem 1.0\n%1 {}").arg(inlineTypeName).toUtf8(), QUrl());
        if (fallbackComp.isError())
        {
            qWarning() << fallbackComp.errors();
            return nullptr;
        }
        return fallbackComp.create();
    };

    if (component.isError())
    {
        qWarning() << "MainView3D(RHI): component error while creating" << typeName
                   << "for fixture" << fxID << "head" << headIndex << "linked" << linkedIndex
                   << component.errors();
        if (!usingFixtureWrapper)
            return;
    }

    QObject *newItem = component.create();
    if (newItem == nullptr && usingFixtureWrapper)
    {
        qWarning() << typeName + " fixture wrapper creation failed, using inline fallback";
        newItem = createInlineFixture(typeName);
    }
    if (newItem == nullptr)
    {
        qWarning() << "MainView3D(RHI): fixture item creation failed for" << typeName
                   << "fixture" << fxID << "head" << headIndex << "linked" << linkedIndex;
        return;
    }
    newItem->setParent(state.renderer);

    QString resolvedMeshPath;
    if (hasProperty(newItem, "path") && !meshName.isEmpty())
    {
        const QString meshPath = meshDirectory() + "fixtures" + QDir::separator() + meshName;
        resolvedMeshPath = meshPath;
        newItem->setProperty("path", meshPath);
        const QFileInfo meshInfo(localPathForFileCheck(meshPath));
        if (!meshInfo.exists())
        {
            qWarning() << "MainView3D(RHI): fixture mesh path does not exist"
                       << meshPath
                       << "for fixture" << fxID << "type" << typeName;
        }
        else if (!meshInfo.isFile())
        {
            qWarning() << "MainView3D(RHI): fixture mesh path is not a file"
                       << meshPath
                       << "for fixture" << fxID << "type" << typeName;
        }
    }
    else if (!meshName.isEmpty())
    {
        qWarning() << "MainView3D(RHI): created item missing 'path' property"
                   << newItem->metaObject()->className()
                   << "for fixture" << fxID << "type" << typeName;
    }

    if (hasProperty(newItem, "emitterCount"))
        newItem->setProperty("emitterCount", fixture->heads());
    if (fixture->type() == QLCFixtureDef::LEDBarBeams && hasProperty(newItem, "baseColor"))
        newItem->setProperty("baseColor", QVector3D(0.64f, 0.64f, 0.64f));
    if (fixture->type() == QLCFixtureDef::LEDBarBeams && hasProperty(newItem, "headsLayout"))
    {
        QSize layout(1, 1);
        if (QLCFixtureMode *fxMode = fixture->fixtureMode())
        {
            const QSize phyLayout = fxMode->physical().layoutSize();
            if (phyLayout.isValid() && phyLayout.width() > 0 && phyLayout.height() > 0)
                layout = phyLayout;
        }
        if (layout.width() * layout.height() < int(qMax(1, fixture->heads())))
            layout = QSize(qMax(1, fixture->heads()), 1);
        newItem->setProperty("headsLayout", layout);
    }

    bool isSelected = false;
    if (m_view && m_view->rootContext())
    {
        QObject *ctxObj = m_view->rootContext()->contextProperty("contextManager").value<QObject *>();
        if (ctxObj)
        {
            bool selected = false;
            if (QMetaObject::invokeMethod(ctxObj, "isFixtureSelected",
                                          Q_RETURN_ARG(bool, selected),
                                          Q_ARG(quint32, itemID)))
            {
                isSelected = selected;
            }
        }
    }

    newItem->setProperty("itemID", itemID);
    newItem->setProperty("monitorItemID", itemID);
    newItem->setProperty("selectable", true);
    newItem->setProperty("visible", true);
    newItem->setProperty("isSelected", isSelected);
    newItem->setProperty("genericItem", false);
    newItem->setProperty("physicalSize", physicalSize);

    RhiItemState itemState;
    itemState.item = newItem;
    QVector3D placementExtents = physicalSize;
    if (!resolvedMeshPath.isEmpty())
    {
        const QString boundsKey = genericBoundsCacheKey(resolvedMeshPath);
        GenericModelBounds bounds;
        if (state.genericBoundsCache.contains(boundsKey))
        {
            bounds = state.genericBoundsCache.value(boundsKey);
        }
        else
        {
            bounds = computeGenericModelBounds(resolvedMeshPath);
            state.genericBoundsCache.insert(boundsKey, bounds);
        }

        if (bounds.valid)
        {
            const float ex = qMax(bounds.extents.x(), 1e-6f);
            const float ey = qMax(bounds.extents.y(), 1e-6f);
            const float ez = qMax(bounds.extents.z(), 1e-6f);
            const float xScale = physicalSize.x() / ex;
            const float yScale = physicalSize.y() / ey;
            const float zScale = physicalSize.z() / ez;
            const float minScale = qMin(xScale, qMin(yScale, zScale));
            if (qIsFinite(minScale) && minScale > 0.0f)
                placementExtents = bounds.extents * minScale;
            else
                placementExtents = bounds.extents;
        }
    }
    itemState.placementExtents = placementExtents;
    itemState.placementExtentsValid = placementExtents.x() > 1e-6f
            && placementExtents.y() > 1e-6f
            && placementExtents.z() > 1e-6f;
    state.items[itemID] = itemState;
    s_missingRhiItemsLogged.remove(itemID);

    auto defaultPixelBarRotationForPointOfView = [this]() -> QVector3D
    {
        switch (m_monProps->pointOfView())
        {
            case MonitorProperties::FrontView:
                return QVector3D(90.0f, 0.0f, 0.0f);
            case MonitorProperties::LeftSideView:
                return QVector3D(0.0f, -90.0f, 0.0f);
            case MonitorProperties::RightSideView:
                return QVector3D(0.0f, 90.0f, 0.0f);
            default:
                break;
        }

        return QVector3D(0.0f, 0.0f, 0.0f);
    };

    const bool hasMonItem = m_monProps->containsItem(fxID, headIndex, linkedIndex);
    QVector3D fixturePos = pos;
    if (!hasMonItem)
    {
        if (mmCoords)
        {
            m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, fixturePos);
            m_monProps->setFixtureFlags(fxID, headIndex, linkedIndex, 0);
        }
        else
        {
            QLCFixtureMode *fxMode = fixture->fixtureMode();
            const QSizeF size = FixtureUtils::item2DDimension(fixture->type() == QLCFixtureDef::Dimmer ? nullptr : fxMode,
                                                              MonitorProperties::TopView);
            const QPointF itemPos = FixtureUtils::available2DPosition(m_doc, MonitorProperties::TopView,
                                                                       QRectF(0, 0, size.width(), size.height()));
            fixturePos = QVector3D(itemPos.x(), 1000.0f, itemPos.y());
            m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, fixturePos);
            m_monProps->setFixtureFlags(fxID, headIndex, linkedIndex, 0);
        }

        if (fixture->type() == QLCFixtureDef::LEDBarPixels)
        {
            const QVector3D defaultRot = defaultPixelBarRotationForPointOfView();
            if (!qFuzzyIsNull(defaultRot.x()) || !qFuzzyIsNull(defaultRot.y()) || !qFuzzyIsNull(defaultRot.z()))
                m_monProps->setFixtureRotation(fxID, headIndex, linkedIndex, defaultRot);
        }
    }
    else if (!mmCoords)
    {
        fixturePos = m_monProps->fixturePosition(fxID, headIndex, linkedIndex);
    }

    if (fixture->type() == QLCFixtureDef::LEDBarPixels)
    {
        const QVector3D existingRot = m_monProps->fixtureRotation(fxID, headIndex, linkedIndex);
        if (qFuzzyIsNull(existingRot.x()) && qFuzzyIsNull(existingRot.y()) && qFuzzyIsNull(existingRot.z()))
        {
            const QVector3D defaultRot = defaultPixelBarRotationForPointOfView();
            if (!qFuzzyIsNull(defaultRot.x()) || !qFuzzyIsNull(defaultRot.y()) || !qFuzzyIsNull(defaultRot.z()))
                m_monProps->setFixtureRotation(fxID, headIndex, linkedIndex, defaultRot);
        }
    }

    updateFixturePosition(itemID, fixturePos);
    updateFixtureRotation(itemID, m_monProps->fixtureRotation(fxID, headIndex, linkedIndex));
    setFixtureFlags(itemID, m_monProps->fixtureFlags(fxID, headIndex, linkedIndex));
    updateFixtureItem(fixture, headIndex, linkedIndex, QByteArray());
}

void MainView3D::setFixtureFlags(quint32 itemID, quint32 flags)
{
    RhiViewState &state = rhiState(this);
    if (!state.items.contains(itemID) || state.items[itemID].item == nullptr)
        return;

    QObject *item = state.items[itemID].item;
    item->setProperty("visible", (flags & MonitorProperties::HiddenFlag) ? false : true);
    item->setProperty("invertedPan", (flags & MonitorProperties::InvertedPanFlag) ? true : false);
    item->setProperty("invertedTilt", (flags & MonitorProperties::InvertedTiltFlag) ? true : false);
}

QVector3D MainView3D::lightPosition(quint32 itemID) const
{
    MainView3D *self = const_cast<MainView3D *>(this);
    RhiViewState &state = rhiState(self);
    if (state.items.contains(itemID))
        return state.items[itemID].lightPos;
    return QVector3D();
}

QMatrix4x4 MainView3D::lightMatrix(quint32 itemID) const
{
    MainView3D *self = const_cast<MainView3D *>(this);
    RhiViewState &state = rhiState(self);
    if (state.items.contains(itemID))
        return state.items[itemID].lightMatrix;
    return QMatrix4x4();
}

void MainView3D::updateFixture(Fixture *fixture, QByteArray &previous)
{
    if (m_enabled == false || fixture == nullptr)
        return;

    for (quint32 &subID : m_monProps->fixtureIDList(fixture->id()))
    {
        const quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
        const quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
        updateFixtureItem(fixture, headIndex, linkedIndex, previous);
    }
}

void MainView3D::updateFixtureItem(Fixture *fixture, quint16 headIndex, quint16 linkedIndex,
                                   const QByteArray &previous)
{
    if (fixture == nullptr)
        return;

    const quint32 itemID = FixtureUtils::fixtureItemID(fixture->id(), headIndex, linkedIndex);
    RhiViewState &state = rhiState(this);
    if (!state.items.contains(itemID) || state.items[itemID].item == nullptr)
    {
        if (!s_missingRhiItemsLogged.contains(itemID))
        {
            s_missingRhiItemsLogged.insert(itemID);
            qWarning() << "MainView3D(RHI): update requested for missing fixture item"
                       << "fixture" << fixture->id()
                       << "head" << headIndex
                       << "linked" << linkedIndex
                       << "itemID" << itemID;
        }
        return;
    }
    s_missingRhiItemsLogged.remove(itemID);

    QObject *fixtureItem = state.items[itemID].item;

    QColor color = FixtureUtils::headColor(fixture, headIndex);
    if (!color.isValid())
        color = Qt::white;
    if (fixture->type() == QLCFixtureDef::Dimmer)
    {
        QColor gelColor = m_monProps->fixtureGelColor(fixture->id(), headIndex, linkedIndex);
        if (gelColor.isValid())
            color = gelColor;
    }

    const quint32 masterDimmerChannel = fixture->masterIntensityChannel();
    const float masterDimmerValue = masterDimmerChannel != QLCChannel::invalid()
            ? dmxToUnit(fixture->channelValueAt(int(masterDimmerChannel)))
            : 1.0f;
    const bool hasEmitterCount = hasProperty(fixtureItem, "emitterCount");
    const bool hasEmitterColors = hasProperty(fixtureItem, "emitterColors");
    const bool hasEmitterIntensities = hasProperty(fixtureItem, "emitterIntensities");
    QVector<QVector3D> emitterColors;
    QVector<float> emitterIntensities;
    if (hasEmitterCount || hasEmitterColors || hasEmitterIntensities)
    {
        const int fixtureHeadCount = qMax(0, fixture->heads());
        const int emitterCount = qMax(1, fixtureHeadCount);
        emitterColors.reserve(emitterCount);
        emitterIntensities.reserve(emitterCount);

        for (int emitterIndex = 0; emitterIndex < emitterCount; ++emitterIndex)
        {
            const int head = fixtureHeadCount > 0 ? qMin(emitterIndex, fixtureHeadCount - 1) : 0;
            quint32 emitterDimmerChannel = fixture->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, head);
            if (emitterDimmerChannel == QLCChannel::invalid())
                emitterDimmerChannel = masterDimmerChannel;

            float emitterIntensity = 1.0f;
            if (emitterDimmerChannel != QLCChannel::invalid())
                emitterIntensity = dmxToUnit(fixture->channelValueAt(int(emitterDimmerChannel)));
            if (emitterDimmerChannel != masterDimmerChannel && masterDimmerChannel != QLCChannel::invalid())
                emitterIntensity *= masterDimmerValue;
            emitterIntensities.push_back(emitterIntensity);

            QColor emitterColor = FixtureUtils::headColor(fixture, head);
            if (!emitterColor.isValid())
                emitterColor = Qt::white;
            emitterColors.push_back(colorToVec3(emitterColor));
        }
    }

    float intensityValue = 1.0f;
    if (fixture->type() == QLCFixtureDef::Dimmer)
    {
        intensityValue = dmxToUnit(fixture->channelValueAt(headIndex));
    }
    else
    {
        quint32 headDimmerChannel = fixture->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, headIndex);
        if (headDimmerChannel == QLCChannel::invalid())
            headDimmerChannel = masterDimmerChannel;

        if (headDimmerChannel != QLCChannel::invalid())
            intensityValue = dmxToUnit(fixture->channelValueAt(int(headDimmerChannel)));

        if (headDimmerChannel != masterDimmerChannel && masterDimmerChannel != QLCChannel::invalid())
            intensityValue *= masterDimmerValue;
    }

    int panValue = 0;
    int tiltValue = 0;
    bool hasPan = false;
    bool hasTilt = false;
    bool panTiltChanged = false;
    float panMaxDegrees = 360.0f;
    float tiltMaxDegrees = 270.0f;
    float focusMinDegrees = 10.0f;
    float focusMaxDegrees = 30.0f;
    float lensMinDegrees = 0.0f;
    float lensMaxDegrees = 0.0f;
    float zoomDegrees = 25.0f;
    bool zoomSet = false;
    bool hasZoomChannel = false;
    bool zoomMsbSet = false;
    bool zoomLsbSet = false;
    quint16 zoomWord = 0;
    bool colorSet = true;
    int shutterPreset = QLCCapability::ShutterOpen;
    int shutterHighMs = 200;
    int shutterLowMs = 800;
    bool shutterSet = false;
    QColor colorWheelA;
    QColor colorWheelB;
    bool colorWheelActive = false;
    QString goboMacroPath;
    bool goboMacroUpdated = false;
    const quint32 itemFlags = m_monProps->fixtureFlags(fixture->id(), headIndex, linkedIndex);
    const bool invertedPan = (itemFlags & MonitorProperties::InvertedPanFlag);
    const bool invertedTilt = (itemFlags & MonitorProperties::InvertedTiltFlag);
    const bool isScanner = (fixture->type() == QLCFixtureDef::Scanner);

    if (QLCFixtureMode *fxMode = fixture->fixtureMode())
    {
        const QLCPhysical phy = fxMode->physical();
        lensMinDegrees = (phy.lensDegreesMin() > 0) ? float(phy.lensDegreesMin()) : 0.0f;
        lensMaxDegrees = (phy.lensDegreesMax() > 0) ? float(phy.lensDegreesMax()) : 0.0f;
        if (phy.focusPanMax() > 0)
            panMaxDegrees = float(phy.focusPanMax());
        if (phy.focusTiltMax() > 0)
            tiltMaxDegrees = float(phy.focusTiltMax());
        if (lensMinDegrees > 0.0f)
            focusMinDegrees = lensMinDegrees;
        if (lensMaxDegrees > 0.0f)
            focusMaxDegrees = lensMaxDegrees;
    }

    if (fixture->type() == QLCFixtureDef::Dimmer)
    {
        const int fixedZoom = m_monProps->fixtureFixedZoom(fixture->id(), headIndex, linkedIndex);
        if (fixedZoom > 0)
        {
            zoomDegrees = float(fixedZoom);
            zoomSet = true;
        }
    }

    for (int i = 0; i < int(fixture->channels()); i++)
    {
        const QLCChannel *ch = fixture->channel(quint32(i));
        if (ch == nullptr)
            continue;

        const uchar value = fixture->channelValueAt(i);
        switch (ch->group())
        {
            case QLCChannel::Pan:
            {
                hasPan = true;
                panValue += (ch->controlByte() == QLCChannel::MSB) ? (value << 8) : value;
                if (previous.isEmpty() || value != uchar(previous.at(i)))
                    panTiltChanged = true;
            }
            break;
            case QLCChannel::Tilt:
            {
                hasTilt = true;
                tiltValue += (ch->controlByte() == QLCChannel::MSB) ? (value << 8) : value;
                if (previous.isEmpty() || value != uchar(previous.at(i)))
                    panTiltChanged = true;
            }
            break;
            case QLCChannel::Beam:
            {
                if (ch->preset() == QLCChannel::BeamZoomSmallBig
                        || ch->preset() == QLCChannel::BeamZoomBigSmall
                        || ch->preset() == QLCChannel::BeamZoomFine)
                {
                    hasZoomChannel = true;
                    const bool isFine = (ch->controlByte() == QLCChannel::LSB || ch->preset() == QLCChannel::BeamZoomFine);
                    if (isFine)
                    {
                        zoomWord = (zoomWord & 0xFF00) | quint16(value);
                        zoomLsbSet = true;
                    }
                    else
                    {
                        const uchar coarse = (ch->preset() == QLCChannel::BeamZoomBigSmall) ? uchar(0xFF - value) : value;
                        zoomWord = (zoomWord & 0x00FF) | (quint16(coarse) << 8);
                        zoomMsbSet = true;
                    }
                }
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
                {
                    break;
                }

                if (cap->presetType() == QLCCapability::DoubleColor)
                {
                    const QColor colorA = cap->resource(0).value<QColor>();
                    const QColor colorB = cap->resource(1).value<QColor>();
                    if (colorA.isValid() && colorB.isValid())
                    {
                        colorWheelA = colorA;
                        colorWheelB = colorB;
                        colorWheelActive = true;
                        colorSet = true;
                    }
                }
                else
                {
                    const QColor wheelColor = cap->resource(0).value<QColor>();
                    if (wheelColor.isValid())
                    {
                        color = FixtureUtils::applyColorFilter(color, wheelColor);
                        if (hasEmitterColors)
                        {
                            const QVector3D wheelFilter = colorToVec3(wheelColor);
                            for (QVector3D &emitterColor : emitterColors)
                            {
                                emitterColor.setX(emitterColor.x() * wheelFilter.x());
                                emitterColor.setY(emitterColor.y() * wheelFilter.y());
                                emitterColor.setZ(emitterColor.z() * wheelFilter.z());
                            }
                        }
                        colorSet = true;
                    }
                }
            }
            break;
            case QLCChannel::Gobo:
            {
                if (!hasProperty(fixtureItem, "goboPath"))
                    break;
                if (previous.length() && value == uchar(previous.at(i)))
                    break;

                QLCCapability *cap = ch->searchCapability(value);
                if (cap == nullptr)
                    break;

                if (cap->preset() == QLCCapability::GoboMacro ||
                    cap->preset() == QLCCapability::GoboShakeMacro)
                {
                    QString goboPath = cap->resource(0).toString();
                    if (QFileInfo(goboPath).isRelative())
                        goboPath = QDir::cleanPath(goboDirectory() + QDir::separator() + goboPath);
                    goboMacroPath = goboPath;
                    goboMacroUpdated = true;
                }
            }
            break;
            case QLCChannel::Shutter:
            {
                if (previous.length() && value == uchar(previous.at(i)))
                    break;

                int high = shutterHighMs;
                int low = shutterLowMs;
                shutterPreset = FixtureUtils::shutterTimings(ch, value, high, low);
                shutterHighMs = high;
                shutterLowMs = low;
                shutterSet = true;
            }
            break;
            case QLCChannel::Speed:
            {
                if (previous.length() && value == uchar(previous.at(i)))
                    break;

                int panSpeed = -1;
                int tiltSpeed = -1;
                FixtureUtils::positionTimings(ch, value, panSpeed, tiltSpeed);
                QMetaObject::invokeMethod(fixtureItem, "setPositionSpeed",
                                          Q_ARG(QVariant, panSpeed),
                                          Q_ARG(QVariant, tiltSpeed));
            }
            break;
            default:
            break;
        }
    }

    if (hasZoomChannel && (zoomMsbSet || zoomLsbSet))
    {
        float zoomNorm = 0.0f;
        if (zoomMsbSet && zoomLsbSet)
            zoomNorm = dmx16ToUnit(zoomWord);
        else if (zoomMsbSet)
            zoomNorm = dmxToUnit(uchar(zoomWord >> 8));
        else
            zoomNorm = dmxToUnit(uchar(zoomWord & 0x00FF));

        zoomDegrees = focusMinDegrees + zoomNorm * (focusMaxDegrees - focusMinDegrees);
        zoomSet = true;
    }
    else if (!hasZoomChannel && !zoomSet && (lensMinDegrees > 0.0f || lensMaxDegrees > 0.0f))
    {
        // No zoom channel fallback rule:
        // 1) if min != 0 and max != 0 -> use min
        // 2) if min == 0 and max != 0 -> use max
        // 3) if only min != 0 -> use min
        if (lensMinDegrees > 0.0f && lensMaxDegrees > 0.0f)
            zoomDegrees = lensMinDegrees;
        else if (lensMaxDegrees > 0.0f)
            zoomDegrees = lensMaxDegrees;
        else
            zoomDegrees = lensMinDegrees;
        zoomSet = true;
    }

    bool shutterHandledByItem = false;
    if (shutterSet)
    {
        shutterHandledByItem = QMetaObject::invokeMethod(fixtureItem, "setShutter",
                                                         Q_ARG(QVariant, shutterPreset),
                                                         Q_ARG(QVariant, shutterLowMs),
                                                         Q_ARG(QVariant, shutterHighMs));
    }
    if (shutterSet && !shutterHandledByItem)
    {
        const float shutterMultiplier = shutterIntensityMultiplier(shutterPreset, shutterHighMs, shutterLowMs,
                                                                   QDateTime::currentMSecsSinceEpoch());
        intensityValue *= shutterMultiplier;
        for (float &emitterIntensity : emitterIntensities)
            emitterIntensity *= shutterMultiplier;
    }

    if (hasProperty(fixtureItem, "baseIntensity"))
        fixtureItem->setProperty("baseIntensity", intensityValue);
    else if (hasProperty(fixtureItem, "intensity"))
        fixtureItem->setProperty("intensity", intensityValue);
    if (hasProperty(fixtureItem, "color"))
        fixtureItem->setProperty("color", colorToVec3(color));
    if (hasEmitterCount && !emitterIntensities.isEmpty())
        fixtureItem->setProperty("emitterCount", emitterIntensities.size());
    if (hasEmitterColors)
    {
        QVariantList emitterColorsList;
        emitterColorsList.reserve(emitterColors.size());
        for (const QVector3D &emitterColor : std::as_const(emitterColors))
            emitterColorsList.push_back(QVariant::fromValue(emitterColor));
        fixtureItem->setProperty("emitterColors", emitterColorsList);
    }
    if (hasEmitterIntensities)
    {
        QVariantList emitterIntensitiesList;
        emitterIntensitiesList.reserve(emitterIntensities.size());
        for (float emitterIntensity : std::as_const(emitterIntensities))
            emitterIntensitiesList.push_back(emitterIntensity);
        fixtureItem->setProperty("emitterIntensities", emitterIntensitiesList);
    }
    if (hasProperty(fixtureItem, "goboPath"))
    {
        const QString currentPath = fixtureItem->property("goboPath").toString();
        QString explicitGoboPath = goboMacroUpdated ? goboMacroPath : QString();

        if (!goboMacroUpdated)
        {
            QColor currentColorA;
            QColor currentColorB;
            QString embeddedGoboPath;
            if (decodeDoubleColorGoboPath(currentPath, currentColorA, currentColorB, &embeddedGoboPath))
                explicitGoboPath = embeddedGoboPath;
            else if (!currentPath.isEmpty())
                explicitGoboPath = currentPath;
        }

        if (colorWheelActive)
        {
            fixtureItem->setProperty("goboPath",
                                     encodeDoubleColorCompositePath(colorWheelA, colorWheelB, explicitGoboPath));
        }
        else if (goboMacroUpdated)
        {
            fixtureItem->setProperty("goboPath", goboMacroPath);
        }
        else
        {
            QColor currentColorA;
            QColor currentColorB;
            QString embeddedGoboPath;
            if (decodeDoubleColorGoboPath(currentPath, currentColorA, currentColorB, &embeddedGoboPath))
                fixtureItem->setProperty("goboPath", embeddedGoboPath);
        }
    }

    if (zoomSet && hasProperty(fixtureItem, "zoom"))
        fixtureItem->setProperty("zoom", zoomDegrees);
    const bool isMovingFixture = (fixture->type() == QLCFixtureDef::MovingHead
                                  || fixture->type() == QLCFixtureDef::Scanner);
    const bool autoBeamMode = shouldUseBeamShape(zoomDegrees, focusMinDegrees, focusMaxDegrees);
    bool beamMode = isMovingFixture
            ? autoBeamMode
            : (zoomSet && autoBeamMode);
    if (isMovingFixture && hasProperty(fixtureItem, "beamMode"))
    {
        // Preserve explicit/manual beam mode when no zoom/lens signal is available.
        const bool hasBeamModeSignal = zoomSet
                || hasZoomChannel
                || lensMinDegrees > 0.0f
                || lensMaxDegrees > 0.0f;
        if (!hasBeamModeSignal)
            beamMode = fixtureItem->property("beamMode").toBool();
    }
    if (hasProperty(fixtureItem, "beamMode"))
    {
        fixtureItem->setProperty("beamMode", beamMode);
    }
    if (isMovingFixture)
    {
        ZoomLensDebugState next;
        next.lensMin = lensMinDegrees;
        next.lensMax = lensMaxDegrees;
        next.focusMin = focusMinDegrees;
        next.focusMax = focusMaxDegrees;
        next.zoom = zoomDegrees;
        next.zoomSet = zoomSet;
        next.hasZoomChannel = hasZoomChannel;
        next.zoomMsbSet = zoomMsbSet;
        next.zoomLsbSet = zoomLsbSet;
        next.zoomWord = zoomWord;
        next.beamMode = beamMode;

        const ZoomLensDebugState prev = s_zoomLensDebugState.value(itemID);
        const bool changed = !qFuzzyCompare(prev.lensMin + 1.0f, next.lensMin + 1.0f)
                || !qFuzzyCompare(prev.lensMax + 1.0f, next.lensMax + 1.0f)
                || !qFuzzyCompare(prev.focusMin + 1.0f, next.focusMin + 1.0f)
                || !qFuzzyCompare(prev.focusMax + 1.0f, next.focusMax + 1.0f)
                || !qFuzzyCompare(prev.zoom + 1.0f, next.zoom + 1.0f)
                || prev.zoomSet != next.zoomSet
                || prev.hasZoomChannel != next.hasZoomChannel
                || prev.zoomMsbSet != next.zoomMsbSet
                || prev.zoomLsbSet != next.zoomLsbSet
                || prev.zoomWord != next.zoomWord
                || prev.beamMode != next.beamMode;

        if (changed)
        {
            s_zoomLensDebugState.insert(itemID, next);
            qDebug() << "MainView3D(RHI): zoom/lens debug"
                     << "fixture" << fixture->id()
                     << "itemID" << itemID
                     << "lensMin" << lensMinDegrees
                     << "lensMax" << lensMaxDegrees
                     << "focusMin" << focusMinDegrees
                     << "focusMax" << focusMaxDegrees
                     << "zoom" << zoomDegrees
                     << "zoomSet" << zoomSet
                     << "hasZoomChannel" << hasZoomChannel
                     << "zoomMsbSet" << zoomMsbSet
                     << "zoomLsbSet" << zoomLsbSet
                     << "zoomWord" << zoomWord
                     << "beamMode" << beamMode;
        }
    }
    if (hasProperty(fixtureItem, "beamRadius"))
    {
        const QVector3D physicalSize = fixtureItem->property("physicalSize").value<QVector3D>();
        float fixtureDiameter = qMax(physicalSize.x(), physicalSize.z());
        if (physicalSize.x() > 0.0f && physicalSize.z() > 0.0f)
            fixtureDiameter = qMin(physicalSize.x(), physicalSize.z());
        const float beamRadius = qBound(0.01f, fixtureDiameter * 0.18f, 0.12f);
        fixtureItem->setProperty("beamRadius", beamRadius);
    }
    if (hasProperty(fixtureItem, "panRangeDegrees"))
        fixtureItem->setProperty("panRangeDegrees", panMaxDegrees);
    if (hasProperty(fixtureItem, "tiltRangeDegrees"))
        fixtureItem->setProperty("tiltRangeDegrees", tiltMaxDegrees);

    float panDegrees = 0.0f;
    if (hasPan)
    {
        const float panDeg = (panMaxDegrees / 65535.0f) * float(panValue);
        if (isScanner)
        {
            const float basePan = 180.0f - (panMaxDegrees / 2.0f);
            panDegrees = invertedPan ? (basePan + panMaxDegrees - panDeg)
                                     : (basePan + panDeg);
        }
        else
        {
            panDegrees = invertedPan ? (panMaxDegrees - panDeg)
                                     : panDeg;
        }
    }

    float tiltDegrees = 0.0f;
    if (hasTilt)
    {
        const float tiltDeg = (tiltMaxDegrees / 65535.0f) * float(tiltValue);
        if (isScanner)
        {
            const float baseTilt = 90.0f - (tiltMaxDegrees / 2.0f);
            tiltDegrees = invertedTilt ? (baseTilt + tiltMaxDegrees - tiltDeg)
                                       : (baseTilt + tiltDeg);
        }
        else
        {
            const float baseTilt = tiltMaxDegrees / 2.0f;
            tiltDegrees = invertedTilt ? (-baseTilt + tiltDeg)
                                       : (baseTilt - tiltDeg);
        }
    }

    if (panTiltChanged && (hasPan || hasTilt))
    {
        const float currentPan = hasProperty(fixtureItem, "pan")
                ? fixtureItem->property("pan").toFloat() : 0.0f;
        const float currentTilt = hasProperty(fixtureItem, "tilt")
                ? fixtureItem->property("tilt").toFloat() : 0.0f;

        const float targetPan = hasPan ? panDegrees : currentPan;
        const float targetTilt = hasTilt ? tiltDegrees : currentTilt;

        const bool animatedPanTilt = QMetaObject::invokeMethod(fixtureItem, "setPanTilt",
                                                                Q_ARG(QVariant, targetPan),
                                                                Q_ARG(QVariant, targetTilt));
        if (!animatedPanTilt)
        {
            if (hasPan && hasProperty(fixtureItem, "pan"))
                fixtureItem->setProperty("pan", targetPan);
            if (hasTilt && hasProperty(fixtureItem, "tilt"))
                fixtureItem->setProperty("tilt", targetTilt);
        }
    }

}

void MainView3D::updateFixtureSelection(QList<quint32> fixtures)
{
    RhiViewState &state = rhiState(this);
    for (auto it = state.items.begin(); it != state.items.end(); ++it)
    {
        if (it.value().item)
            it.value().item->setProperty("isSelected", fixtures.contains(it.key()));
    }
}

void MainView3D::updateFixtureSelection(quint32 itemID, bool enable)
{
    RhiViewState &state = rhiState(this);
    qDebug() << "MainView3D(RHI): updateFixtureSelection itemID" << itemID
             << "enable" << enable
             << "exists" << state.items.contains(itemID);
    if (state.items.contains(itemID) && state.items[itemID].item)
        state.items[itemID].item->setProperty("isSelected", enable);
}

void MainView3D::updateFixturePosition(quint32 itemID, QVector3D pos)
{
    if (!isEnabled())
        return;

    RhiViewState &state = rhiState(this);
    if (!state.items.contains(itemID) || state.items[itemID].item == nullptr)
        return;

    QObject *item = state.items[itemID].item;
    const float unitScale = m_monProps->gridUnits() == MonitorProperties::Meters ? 1.0f : 0.3048f;
    const QVector3D gridMeters = m_monProps->gridSize() * unitScale;
    const QVector3D extents = state.items[itemID].placementExtentsValid
            ? state.items[itemID].placementExtents : QVector3D(0.0f, 0.0f, 0.0f);
    const QVector3D newPos((pos.x() / 1000.0f) - (gridMeters.x() / 2.0f) + (extents.x() / 2.0f),
                           (pos.y() / 1000.0f) + (extents.y() / 2.0f),
                           (pos.z() / 1000.0f) - (gridMeters.z() / 2.0f) + (extents.z() / 2.0f));

    item->setProperty("position", newPos);
    state.items[itemID].lightPos = newPos;
}

void MainView3D::updateFixtureRotation(quint32 itemID, QVector3D degrees)
{
    if (!isEnabled())
        return;

    RhiViewState &state = rhiState(this);
    if (!state.items.contains(itemID) || state.items[itemID].item == nullptr)
        return;

    QObject *item = state.items[itemID].item;
    const QVector3D sceneRot(-degrees.x(), -degrees.y(), -degrees.z());
    item->setProperty("rotationDegrees", sceneRot);

    QMatrix4x4 matrix;
    matrix.rotate(rotationFromAxesOrderXYZ(sceneRot));
    state.items[itemID].lightMatrix = matrix;
}

void MainView3D::removeFixtureItem(quint32 itemID)
{
    RhiViewState &state = rhiState(this);
    if (!state.items.contains(itemID))
        return;

    if (state.items[itemID].item)
        state.items[itemID].item->deleteLater();
    state.items.remove(itemID);
}

void MainView3D::createGenericItem(QString filename, int itemID)
{
    if (!isEnabled())
        return;

    initialize3DProperties();
    RhiViewState &state = rhiState(this);
    if (state.renderer == nullptr || m_view == nullptr || m_view->engine() == nullptr)
    {
        qWarning() << "MainView3D(RHI): renderer not ready while creating generic item" << filename;
        return;
    }

    QString modelPath = filename;
    quint32 targetID = 0;
    auto preferGlbPath = [&](const QString &path) -> QString
    {
        if (!path.endsWith(QStringLiteral(".obj"), Qt::CaseInsensitive))
            return path;

        const QString urlPrefix = QLCFile::fileUrlPrefix();
        const bool isUrl = path.startsWith(urlPrefix);
        const QString localPath = localPathForFileCheck(path);
        const QFileInfo localInfo(localPath);

        if (localInfo.exists())
        {
            const QString glbLocal = localInfo.path() + QDir::separator()
                    + localInfo.completeBaseName() + QStringLiteral(".glb");
            if (QFileInfo::exists(glbLocal))
                return isUrl ? urlPrefix + glbLocal : glbLocal;
        }

        if (!QDir::isAbsolutePath(path) && !isUrl)
        {
            QString meshLocal = meshDirectory() + path;
            meshLocal.remove(urlPrefix);
            const QFileInfo meshInfo(meshLocal);
            if (meshInfo.exists())
            {
                const QString glbLocal = meshInfo.path() + QDir::separator()
                        + meshInfo.completeBaseName() + QStringLiteral(".glb");
                if (QFileInfo::exists(glbLocal))
                {
                    const QString relGlb = path.left(path.length() - 4) + QStringLiteral(".glb");
                    return meshDirectory() + relGlb;
                }
            }
        }

        return path;
    };

    if (itemID == -1)
    {
        while (m_monProps->containsItem(quint32(m_latestGenericID)))
            ++m_latestGenericID;

        targetID = quint32(m_latestGenericID);
        ++m_latestGenericID;

        QString resFile = filename;
        if (resFile.startsWith(meshDirectory()))
            resFile = QUrl(filename).toLocalFile();
        else
            resFile.remove(QLCFile::fileUrlPrefix());

        if (resFile.endsWith(QStringLiteral(".obj"), Qt::CaseInsensitive))
        {
            QFileInfo resInfo(resFile);
            if (!resInfo.exists() && !QDir::isAbsolutePath(resFile))
            {
                QString meshLocal = meshDirectory() + resFile;
                meshLocal.remove(QLCFile::fileUrlPrefix());
                resInfo.setFile(meshLocal);
            }
            if (resInfo.exists())
            {
                const QString glbLocal = resInfo.path() + QDir::separator()
                        + resInfo.completeBaseName() + QStringLiteral(".glb");
                if (QFileInfo::exists(glbLocal))
                    resFile = glbLocal;
            }
        }

        const QVector3D envSize = m_monProps->gridSize();
        const QVector3D newPos((envSize.x() / 2.0f) * 1000.0f,
                               1000.0f,
                               (envSize.z() / 2.0f) * 1000.0f);
        m_monProps->setItemPosition(targetID, newPos);
        m_monProps->setItemScale(targetID, QVector3D(1.0f, 1.0f, 1.0f));
        m_monProps->setItemRotation(targetID, QVector3D(0.0f, 0.0f, 0.0f));
        m_monProps->setItemResource(targetID, resFile);
    }
    else
    {
        targetID = quint32(itemID);
        m_latestGenericID = qMax(m_latestGenericID, itemID + 1);

        QFileInfo fInfo(localPathForFileCheck(filename));
        if (!fInfo.exists())
        {
            QString meshFile = meshDirectory() + filename;
            meshFile.remove(QLCFile::fileUrlPrefix());
            fInfo.setFile(meshFile);
            if (!fInfo.exists())
            {
                const QString projectFile = m_doc->denormalizeComponentPath(filename);
                fInfo.setFile(projectFile);
                if (!fInfo.exists())
                {
                    qWarning() << "MainView3D(RHI): generic mesh file doesn't exist:" << filename;
                    return;
                }
                modelPath = QLCFile::fileUrlPrefix() + m_doc->workspacePath()
                        + QDir::separator() + filename;
            }
            else
            {
                modelPath = meshDirectory() + filename;
            }
        }
        else
        {
            modelPath = QLCFile::fileUrlPrefix() + fInfo.absoluteFilePath();
        }
    }

    modelPath = preferGlbPath(modelPath);

    if (state.genericItems.contains(targetID))
    {
        if (state.genericItems[targetID].item)
            state.genericItems[targetID].item->deleteLater();
        state.genericItems.remove(targetID);
    }

    QQmlComponent component(m_view->engine());
    component.setData(QByteArrayLiteral("import RhiQmlItem 1.0\nModel {}"), QUrl());
    if (component.isError())
    {
        qWarning() << "MainView3D(RHI): generic model component error:" << component.errors();
        return;
    }

    QObject *newItem = component.create();
    if (newItem == nullptr)
    {
        qWarning() << "MainView3D(RHI): failed to create generic model item for" << modelPath;
        return;
    }

    newItem->setParent(state.renderer);
    newItem->setProperty("path", modelPath);
    newItem->setProperty("itemID", targetID);
    newItem->setProperty("monitorItemID", targetID);
    newItem->setProperty("selectable", true);
    newItem->setProperty("visible", true);
    newItem->setProperty("isSelected", m_genericSelectedItems.contains(int(targetID)));
    newItem->setProperty("genericItem", true);

    const QString boundsKey = genericBoundsCacheKey(modelPath);
    GenericModelBounds bounds;
    if (state.genericBoundsCache.contains(boundsKey))
    {
        bounds = state.genericBoundsCache.value(boundsKey);
    }
    else
    {
        bounds = computeGenericModelBounds(modelPath);
        state.genericBoundsCache.insert(boundsKey, bounds);
    }

    RhiGenericItemState genericState;
    genericState.item = newItem;
    genericState.bounds = bounds;
    state.genericItems[targetID] = genericState;

    updateGenericItemScale(targetID, m_monProps->itemScale(targetID));
    updateGenericItemPosition(targetID, m_monProps->itemPosition(targetID));
    updateGenericItemRotation(targetID, m_monProps->itemRotation(targetID));
    updateGenericItemsList();
}

void MainView3D::setItemSelection(int itemID, bool enable, int keyModifiers)
{
    if (!isEnabled())
        return;

    RhiViewState &state = rhiState(this);
    if ((enable && keyModifiers == 0) || itemID < 0)
    {
        for (int selectedID : std::as_const(m_genericSelectedItems))
        {
            auto it = state.genericItems.find(quint32(selectedID));
            if (it != state.genericItems.end() && it.value().item)
                it.value().item->setProperty("isSelected", false);
        }
        m_genericSelectedItems.clear();
        emit genericSelectedCountChanged();

        if (itemID < 0)
        {
            updateGenericItemsList();
            return;
        }
    }

    auto it = state.genericItems.find(quint32(itemID));
    if (it == state.genericItems.end())
    {
        updateGenericItemsList();
        return;
    }

    if (it.value().item)
        it.value().item->setProperty("isSelected", enable);

    if (enable)
    {
        if (!m_genericSelectedItems.contains(itemID))
            m_genericSelectedItems.append(itemID);
    }
    else
    {
        m_genericSelectedItems.removeAll(itemID);
    }

    emit genericSelectedCountChanged();
    updateGenericItemsList();
}

int MainView3D::sceneItemMonitorID(QObject *item) const
{
    if (item == nullptr)
        return -1;

    MainView3D *self = const_cast<MainView3D *>(this);
    RhiViewState &state = rhiState(self);

    for (auto it = state.genericItems.cbegin(); it != state.genericItems.cend(); ++it)
    {
        if (it.value().item == item)
            return int(it.key());
    }

    for (auto it = state.items.cbegin(); it != state.items.cend(); ++it)
    {
        if (it.value().item == item)
            return int(it.key());
    }

    return -1;
}

bool MainView3D::isGenericSceneItem(QObject *item) const
{
    if (item == nullptr)
        return false;

    MainView3D *self = const_cast<MainView3D *>(this);
    RhiViewState &state = rhiState(self);
    for (auto it = state.genericItems.cbegin(); it != state.genericItems.cend(); ++it)
    {
        if (it.value().item == item)
            return true;
    }

    return false;
}

bool MainView3D::isFixtureSceneItem(QObject *item) const
{
    if (item == nullptr)
        return false;

    MainView3D *self = const_cast<MainView3D *>(this);
    RhiViewState &state = rhiState(self);
    for (auto it = state.items.cbegin(); it != state.items.cend(); ++it)
    {
        if (it.value().item == item)
            return true;
    }

    return false;
}

int MainView3D::genericSelectedCount() const
{
    return m_genericSelectedItems.count();
}

void MainView3D::removeSelectedGenericItems()
{
    RhiViewState &state = rhiState(this);
    const QList<int> selected = m_genericSelectedItems;
    for (int id : selected)
    {
        auto it = state.genericItems.find(quint32(id));
        if (it != state.genericItems.end())
        {
            if (it.value().item)
                it.value().item->deleteLater();
            state.genericItems.erase(it);
        }

        m_monProps->removeItem(quint32(id));
    }

    m_genericSelectedItems.clear();
    emit genericSelectedCountChanged();
    emit genericItemsPositionChanged();
    emit genericItemsRotationChanged();
    emit genericItemsScaleChanged();
    updateGenericItemsList();
}

void MainView3D::normalizeSelectedGenericItems()
{
    RhiViewState &state = rhiState(this);

    for (int id : std::as_const(m_genericSelectedItems))
    {
        auto it = state.genericItems.find(quint32(id));
        if (it == state.genericItems.end())
            continue;

        const GenericModelBounds &bounds = it.value().bounds;
        if (!bounds.valid)
            continue;

        const float targetSize = 2.0f;
        float factor = 1.0f;
        if (bounds.extents.x() > 1e-6f)
            factor = qMin(factor, targetSize / bounds.extents.x());
        if (bounds.extents.y() > 1e-6f)
            factor = qMin(factor, targetSize / bounds.extents.y());
        if (bounds.extents.z() > 1e-6f)
            factor = qMin(factor, targetSize / bounds.extents.z());

        if (!qIsFinite(factor) || factor <= 0.0f)
            continue;

        updateGenericItemScale(quint32(id), QVector3D(factor, factor, factor));
        updateGenericItemPosition(quint32(id), QVector3D(bounds.center.x() * -1000.0f,
                                                         bounds.center.y() * -1000.0f,
                                                         bounds.center.z() * -1000.0f));
    }

    emit genericItemsPositionChanged();
    emit genericItemsScaleChanged();
}

void MainView3D::updateGenericItemsList()
{
    if (m_genericItemsList == nullptr)
        return;

    m_genericItemsList->clear();

    for (quint32 &itemID : m_monProps->genericItemsID())
    {
        QVariantMap itemMap;
        itemMap.insert("itemID", itemID);
        itemMap.insert("name", m_monProps->itemName(itemID));
        itemMap.insert("isSelected", m_genericSelectedItems.contains(int(itemID)));
        m_genericItemsList->addDataMap(itemMap);
    }

    emit genericItemsListChanged();
}

QVariant MainView3D::genericItemsList() const
{
    return QVariant::fromValue(m_genericItemsList);
}

void MainView3D::updateGenericItemPosition(quint32 itemID, QVector3D pos) const
{
    if (!isEnabled())
        return;

    const QVector3D currPos = m_monProps->itemPosition(itemID);
    Tardis::instance()->enqueueAction(Tardis::GenericItemSetPosition, itemID,
                                      QVariant(currPos), QVariant(pos));
    m_monProps->setItemPosition(itemID, pos);

    MainView3D *self = const_cast<MainView3D *>(this);
    RhiViewState &state = rhiState(self);
    auto it = state.genericItems.find(itemID);
    if (it == state.genericItems.end() || it.value().item == nullptr)
        return;

    const float unitScale = m_monProps->gridUnits() == MonitorProperties::Meters ? 1.0f : 0.3048f;
    const QVector3D gridMeters = m_monProps->gridSize() * unitScale;
    const QVector3D extents = it.value().bounds.valid
            ? it.value().bounds.extents : QVector3D(0.0f, 0.0f, 0.0f);
    const QVector3D newPos((pos.x() / 1000.0f) - (gridMeters.x() / 2.0f) + (extents.x() / 2.0f),
                           (pos.y() / 1000.0f) + (extents.y() / 2.0f),
                           (pos.z() / 1000.0f) - (gridMeters.z() / 2.0f) + (extents.z() / 2.0f));
    it.value().item->setProperty("position", newPos);
}

QVector3D MainView3D::genericItemsPosition() const
{
    if (m_genericSelectedItems.count() == 1)
        return m_monProps->itemPosition(quint32(m_genericSelectedItems.first()));

    return QVector3D(0.0f, 0.0f, 0.0f);
}

void MainView3D::setGenericItemsPosition(QVector3D pos)
{
    if (m_genericSelectedItems.isEmpty())
        return;

    if (m_genericSelectedItems.count() == 1)
    {
        updateGenericItemPosition(quint32(m_genericSelectedItems.first()), pos);
    }
    else
    {
        for (int itemID : std::as_const(m_genericSelectedItems))
        {
            const QVector3D newPos = m_monProps->itemPosition(quint32(itemID)) + pos;
            updateGenericItemPosition(quint32(itemID), newPos);
        }
    }

    emit genericItemsPositionChanged();
}

void MainView3D::updateGenericItemRotation(quint32 itemID, QVector3D rot) const
{
    if (!isEnabled())
        return;

    const QVector3D currRot = m_monProps->itemRotation(itemID);
    Tardis::instance()->enqueueAction(Tardis::GenericItemSetRotation, itemID,
                                      QVariant(currRot), QVariant(rot));
    m_monProps->setItemRotation(itemID, rot);

    MainView3D *self = const_cast<MainView3D *>(this);
    RhiViewState &state = rhiState(self);
    auto it = state.genericItems.find(itemID);
    if (it == state.genericItems.end() || it.value().item == nullptr)
        return;

    it.value().item->setProperty("rotationDegrees", rot);
}

QVector3D MainView3D::genericItemsRotation() const
{
    if (m_genericSelectedItems.count() == 1)
        return m_monProps->itemRotation(quint32(m_genericSelectedItems.first()));

    return QVector3D(0.0f, 0.0f, 0.0f);
}

void MainView3D::setGenericItemsRotation(QVector3D rot)
{
    if (m_genericSelectedItems.isEmpty())
        return;

    if (m_genericSelectedItems.count() == 1)
    {
        updateGenericItemRotation(quint32(m_genericSelectedItems.first()), rot);
    }
    else
    {
        for (int itemID : std::as_const(m_genericSelectedItems))
        {
            QVector3D newRot = m_monProps->itemRotation(quint32(itemID)) + rot;

            if (newRot.x() < 0.0f) newRot.setX(newRot.x() + 360.0f);
            else if (newRot.x() >= 360.0f) newRot.setX(newRot.x() - 360.0f);

            if (newRot.y() < 0.0f) newRot.setY(newRot.y() + 360.0f);
            else if (newRot.y() >= 360.0f) newRot.setY(newRot.y() - 360.0f);

            if (newRot.z() < 0.0f) newRot.setZ(newRot.z() + 360.0f);
            else if (newRot.z() >= 360.0f) newRot.setZ(newRot.z() - 360.0f);

            updateGenericItemRotation(quint32(itemID), newRot);
        }
    }

    emit genericItemsRotationChanged();
}

void MainView3D::updateGenericItemScale(quint32 itemID, QVector3D scale) const
{
    if (!isEnabled())
        return;

    const QVector3D currScale = m_monProps->itemScale(itemID);
    Tardis::instance()->enqueueAction(Tardis::GenericItemSetScale, itemID,
                                      QVariant(currScale), QVariant(scale));
    m_monProps->setItemScale(itemID, scale);

    MainView3D *self = const_cast<MainView3D *>(this);
    RhiViewState &state = rhiState(self);
    auto it = state.genericItems.find(itemID);
    if (it == state.genericItems.end() || it.value().item == nullptr)
        return;

    it.value().item->setProperty("scale", scale);
}

QVector3D MainView3D::genericItemsScale() const
{
    if (m_genericSelectedItems.count() == 1)
    {
        const QVector3D scale = m_monProps->itemScale(quint32(m_genericSelectedItems.first()));
        return QVector3D(scale.x() * 100.0f, scale.y() * 100.0f, scale.z() * 100.0f);
    }

    return QVector3D(100.0f, 100.0f, 100.0f);
}

void MainView3D::setGenericItemsScale(QVector3D scale)
{
    if (m_genericSelectedItems.isEmpty())
        return;

    const QVector3D normScale(scale.x() / 100.0f, scale.y() / 100.0f, scale.z() / 100.0f);
    if (m_genericSelectedItems.count() == 1)
    {
        updateGenericItemScale(quint32(m_genericSelectedItems.first()), normScale);
    }
    else
    {
        for (int itemID : std::as_const(m_genericSelectedItems))
        {
            const QVector3D newScale = m_monProps->itemScale(quint32(itemID)) + normScale;
            updateGenericItemScale(quint32(itemID), newScale);
        }
    }

    emit genericItemsScaleChanged();
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
    emit stageIndexChanged(stageIndex);
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
