/*
  Q Light Controller Plus
  efxeditor.cpp

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

#include <QTimer>
#include <QDebug>
#include <QQmlEngine>

#include "qlcfixturemode.h"
#include "efxeditor.h"
#include "listmodel.h"
#include "tardis.h"
#include "efx.h"
#include "doc.h"
#include "app.h"

EFXEditor::EFXEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_efx(NULL)
    , m_fixtureTree(NULL)
    , m_maxPanDegrees(360.0)
    , m_maxTiltDegrees(270.0)
{
    m_view->rootContext()->setContextProperty("efxEditor", this);

    m_fixtureList = new ListModel(this);
    QStringList listRoles;
    listRoles << "name" << "fxID" << "head" << "isSelected" << "reverse" << "offset";
    m_fixtureList->setRoleNames(listRoles);
}

EFXEditor::~EFXEditor()
{
    m_view->rootContext()->setContextProperty("efxEditor", NULL);
}

void EFXEditor::setFunctionID(quint32 id)
{
    if (id == Function::invalidId())
    {
        disconnect(m_efx, &EFX::attributeChanged, this, &EFXEditor::slotAttributeChanged);
        m_efx = NULL;
        return;
    }

    m_efx = qobject_cast<EFX *>(m_doc->function(id));
    if (m_efx != NULL)
    {
        connect(m_efx, &EFX::attributeChanged, this, &EFXEditor::slotAttributeChanged);
        updateAlgorithmData();
        emit algorithmIndexChanged();
    }

    FunctionEditor::setFunctionID(id);
    updateFixtureList();
}

void EFXEditor::slotAttributeChanged(int attrIndex, qreal fraction)
{
    Q_UNUSED(fraction)

    switch (attrIndex)
    {
        case EFX::Width: emit algorithmWidthChanged(); break;
        case EFX::Height: emit algorithmHeightChanged(); break;
        case EFX::Rotation: emit algorithmRotationChanged(); break;
        case EFX::XOffset: emit algorithmXOffsetChanged(); break;
        case EFX::YOffset: emit algorithmYOffsetChanged(); break;
        case EFX::StartOffset: emit algorithmStartOffsetChanged(); break;
        default: break;
    }

    updateAlgorithmData();
}

/************************************************************************
 * Algorithm
 ************************************************************************/

QStringList EFXEditor::algorithms() const
{
    return EFX::algorithmList();
}

int EFXEditor::algorithmIndex() const
{
    if (m_efx == NULL)
        return 0;

    QStringList algoList = algorithms();
    return algoList.indexOf(EFX::algorithmToString(m_efx->algorithm()));
}

void EFXEditor::setAlgorithmIndex(int algoIndex)
{
    if (m_efx == NULL || algoIndex == m_efx->algorithm())
        return;

    Tardis::instance()->enqueueAction(EFXSetAlgorithmIndex, m_efx->id(), m_efx->algorithm(), algoIndex);
    m_efx->setAlgorithm(EFX::Algorithm(algoIndex));
    emit algorithmIndexChanged();
    updateAlgorithmData();
}

bool EFXEditor::isRelative() const
{
    if (m_efx == NULL)
        return false;

    return m_efx->isRelative();
}

void EFXEditor::setIsRelative(bool val)
{
    if (m_efx == NULL || val == m_efx->isRelative())
        return;

    Tardis::instance()->enqueueAction(EFXSetRelative, m_efx->id(), m_efx->isRelative(), val);
    m_efx->setIsRelative(val);
    emit isRelativeChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmWidth() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->width();
}

void EFXEditor::setAlgorithmWidth(int algorithmWidth)
{
    if (m_efx == NULL || algorithmWidth == m_efx->width())
        return;

    Tardis::instance()->enqueueAction(EFXSetWidth, m_efx->id(), m_efx->width(), algorithmWidth);
    m_efx->setWidth(algorithmWidth);
    emit algorithmWidthChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmHeight() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->height();
}

void EFXEditor::setAlgorithmHeight(int algorithmHeight)
{
    if (m_efx == NULL || algorithmHeight == m_efx->height())
        return;

    Tardis::instance()->enqueueAction(EFXSetHeight, m_efx->id(), m_efx->height(), algorithmHeight);
    m_efx->setHeight(algorithmHeight);
    emit algorithmHeightChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmXOffset() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->xOffset();
}

void EFXEditor::setAlgorithmXOffset(int algorithmXOffset)
{
    if (m_efx == NULL || algorithmXOffset == m_efx->xOffset())
        return;

    Tardis::instance()->enqueueAction(EFXSetXOffset, m_efx->id(), m_efx->xOffset(), algorithmXOffset);
    m_efx->setXOffset(algorithmXOffset);
    emit algorithmXOffsetChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmYOffset() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->yOffset();
}

void EFXEditor::setAlgorithmYOffset(int algorithmYOffset)
{
    if (m_efx == NULL || algorithmYOffset == m_efx->yOffset())
        return;

    Tardis::instance()->enqueueAction(EFXSetYOffset, m_efx->id(), m_efx->yOffset(), algorithmYOffset);
    m_efx->setYOffset(algorithmYOffset);
    emit algorithmYOffsetChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmRotation() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->rotation();
}

void EFXEditor::setAlgorithmRotation(int algorithmRotation)
{
    if (m_efx == NULL || algorithmRotation == m_efx->rotation())
        return;

    Tardis::instance()->enqueueAction(EFXSetRotation, m_efx->id(), m_efx->rotation(), algorithmRotation);
    m_efx->setRotation(algorithmRotation);
    emit algorithmRotationChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmStartOffset() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->startOffset();
}

void EFXEditor::setAlgorithmStartOffset(int algorithmStartOffset)
{
    if (m_efx == NULL || algorithmStartOffset == m_efx->startOffset())
        return;

    Tardis::instance()->enqueueAction(EFXSetStartOffset, m_efx->id(), m_efx->startOffset(), algorithmStartOffset);
    m_efx->setStartOffset(algorithmStartOffset);
    emit algorithmStartOffsetChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmXFrequency() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->xFrequency();
}

void EFXEditor::setAlgorithmXFrequency(int algorithmXFrequency)
{
    if (m_efx == NULL || algorithmXFrequency == m_efx->xFrequency())
        return;

    Tardis::instance()->enqueueAction(EFXSetXFrequency, m_efx->id(), m_efx->xFrequency(), algorithmXFrequency);
    m_efx->setXFrequency(algorithmXFrequency);
    emit algorithmXFrequencyChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmYFrequency() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->yFrequency();
}

void EFXEditor::setAlgorithmYFrequency(int algorithmYFrequency)
{
    if (m_efx == NULL || algorithmYFrequency == m_efx->yFrequency())
        return;

    Tardis::instance()->enqueueAction(EFXSetYFrequency, m_efx->id(), m_efx->yFrequency(), algorithmYFrequency);
    m_efx->setYFrequency(algorithmYFrequency);
    emit algorithmYFrequencyChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmXPhase() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->xPhase();
}

void EFXEditor::setAlgorithmXPhase(int algorithmXPhase)
{
    if (m_efx == NULL || algorithmXPhase == m_efx->xPhase())
        return;

    Tardis::instance()->enqueueAction(EFXSetXPhase, m_efx->id(), m_efx->xPhase(), algorithmXPhase);
    m_efx->setXPhase(algorithmXPhase);
    emit algorithmXPhaseChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmYPhase() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->yPhase();
}

void EFXEditor::setAlgorithmYPhase(int algorithmYPhase)
{
    if (m_efx == NULL || algorithmYPhase == m_efx->yPhase())
        return;

    Tardis::instance()->enqueueAction(EFXSetYPhase, m_efx->id(), m_efx->yPhase(), algorithmYPhase);
    m_efx->setYPhase(algorithmYPhase);
    emit algorithmYPhaseChanged();
    updateAlgorithmData();
}

/************************************************************************
 * Fixtures
 ************************************************************************/

QVariant EFXEditor::fixtureList() const
{
    return QVariant::fromValue(m_fixtureList);
}

QVariant EFXEditor::groupsTreeModel()
{
    if (m_fixtureTree == NULL)
    {
        m_fixtureTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "type" << "id" << "subid" << "head";
        m_fixtureTree->setColumnNames(treeColumns);
        m_fixtureTree->enableSorting(false);
        updateFixtureTree(m_doc, m_fixtureTree);
    }

    return QVariant::fromValue(m_fixtureTree);
}

qreal EFXEditor::maxPanDegrees() const
{
    return m_maxPanDegrees;
}

qreal EFXEditor::maxTiltDegrees() const
{
    return m_maxTiltDegrees;
}

void EFXEditor::addGroup(QVariant reference)
{
    //qDebug() << "[EFXEditor::addGroup]" << reference;
    bool listChanged = false;

    if (m_efx == NULL)
        return;

    if (reference.canConvert<Universe *>())
    {
        Universe *uni = reference.value<Universe *>();

        if (uni != NULL)
        {
            qDebug() << "Adding a universe";
            for (Fixture *fixture : m_doc->fixtures()) // C++11
            {
                if (fixture->universe() != uni->id())
                    continue;

                for (int headIdx = 0; headIdx < fixture->heads(); headIdx++)
                {
                    quint32 panCh = fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB, headIdx);
                    quint32 tiltCh = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, headIdx);

                    if (panCh != QLCChannel::invalid() || tiltCh != QLCChannel::invalid())
                    {
                        EFXFixture* ef = new EFXFixture(m_efx);
                        GroupHead head(fixture->id(), headIdx);
                        ef->setHead(head);

                        if (m_efx->addFixture(ef) == false)
                            delete ef;
                        else
                            listChanged = true;
                    }
                }
            }
        }
    }
    else if (reference.canConvert<FixtureGroup *>())
    {
        FixtureGroup *group = reference.value<FixtureGroup *>();

        if (group != NULL)
        {
            qDebug() << "Adding a fixture group";
            for (GroupHead head : group->headList())
            {
                Fixture* fixture = m_doc->fixture(head.fxi);
                if (fixture == NULL)
                    continue;

                quint32 panCh = fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB, head.head);
                quint32 tiltCh = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, head.head);

                if (panCh != QLCChannel::invalid() || tiltCh != QLCChannel::invalid())
                {
                    EFXFixture* ef = new EFXFixture(m_efx);
                    ef->setHead(head);
                    if (m_efx->addFixture(ef) == false)
                    {
                        delete ef;
                    }
                    else
                    {
                        Tardis::instance()->enqueueAction(EFXAddFixture, m_efx->id(), QVariant(),
                                                          Tardis::instance()->actionToByteArray(EFXAddFixture, m_efx->id(),
                                                                                                qVariantFromValue((void *)ef)));
                        listChanged = true;
                    }
                }
            }
        }
    }

    if (listChanged)
        updateFixtureList();
}

void EFXEditor::addFixture(QVariant reference)
{
    if (m_efx == NULL)
        return;

    if (reference.canConvert<Fixture *>() == false)
        return;

    Fixture *fixture = reference.value<Fixture *>();

    for (int headIdx = 0; headIdx < fixture->heads(); headIdx++)
    {
        quint32 panCh = fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB, headIdx);
        quint32 tiltCh = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, headIdx);

        if (panCh != QLCChannel::invalid() || tiltCh != QLCChannel::invalid())
        {
            EFXFixture* ef = new EFXFixture(m_efx);
            GroupHead head(fixture->id(), headIdx);
            ef->setHead(head);

            if (m_efx->addFixture(ef) == false)
            {
                delete ef;
            }
            else
            {
                Tardis::instance()->enqueueAction(EFXAddFixture, m_efx->id(), QVariant(),
                                                  Tardis::instance()->actionToByteArray(EFXAddFixture, m_efx->id(),
                                                                                        qVariantFromValue((void *)ef)));
                updateFixtureList();
            }
        }
    }
}

void EFXEditor::addHead(int fixtureID, int headIndex)
{
    if (m_efx == NULL)
        return;

    GroupHead head(fixtureID, headIndex);
    EFXFixture* ef = new EFXFixture(m_efx);
    ef->setHead(head);

    if (m_efx->addFixture(ef) == false)
    {
        delete ef;
    }
    else
    {
        Tardis::instance()->enqueueAction(EFXAddFixture, m_efx->id(), QVariant(),
                                          Tardis::instance()->actionToByteArray(EFXAddFixture, m_efx->id(),
                                                                                qVariantFromValue((void *)ef)));
        updateFixtureList();
    }
}

void EFXEditor::setFixtureReversed(quint32 fixtureID, int headIndex, bool reversed)
{
    if (m_efx == NULL)
        return;

    EFXFixture *ef = m_efx->fixture(fixtureID, headIndex);

    if (ef != NULL)
    {
        ef->setDirection(reversed ? Function::Backward : Function::Forward);
        updateAlgorithmData();
    }
}

void EFXEditor::setFixtureOffset(quint32 fixtureID, int headIndex, int offset)
{
    if (m_efx == NULL)
        return;

    EFXFixture *ef = m_efx->fixture(fixtureID, headIndex);

    if (ef != NULL)
    {
        ef->setStartOffset(offset);
        updateAlgorithmData();
    }
}

void EFXEditor::updateFixtureList()
{
    m_fixtureList->clear();

    if (m_efx == NULL)
        return;

    qreal oldPanDegrees = m_maxPanDegrees;
    qreal oldTiltDegrees = m_maxTiltDegrees;

    // listRoles << "name" << "fxID" << "head" << "isSelected" << "reverse" << "offset";

    for (EFXFixture *ef : m_efx->fixtures()) // C++11
    {
        QVariantMap fxMap;
        GroupHead head = ef->head();
        Fixture *fixture = m_doc->fixture(head.fxi);

        if (fixture == NULL || head.isValid() == false)
            continue;

        if (fixture->fixtureMode() != NULL && fixture->fixtureMode()->physical().focusPanMax() > m_maxPanDegrees)
            m_maxPanDegrees = fixture->fixtureMode()->physical().focusPanMax();

        if (fixture->fixtureMode() != NULL && fixture->fixtureMode()->physical().focusTiltMax() > m_maxTiltDegrees)
            m_maxTiltDegrees = fixture->fixtureMode()->physical().focusTiltMax();

        if (fixture->heads() > 1)
            fxMap.insert("name", QString("%1 [%2]").arg(fixture->name()).arg(ef->head().head));
        else
            fxMap.insert("name", fixture->name());
        fxMap.insert("fxID", head.fxi);
        fxMap.insert("head", head.head);
        fxMap.insert("isSelected", false);
        fxMap.insert("reverse", ef->direction() == Function::Backward ? true : false);
        fxMap.insert("offset", ef->startOffset());

        m_fixtureList->addDataMap(fxMap);
    }

    emit fixtureListChanged();
    if (oldPanDegrees != m_maxPanDegrees)
        emit maxPanDegreesChanged();
    if (oldTiltDegrees != m_maxTiltDegrees)
        emit maxTiltDegreesChanged();
    updateAlgorithmData();
}

void EFXEditor::updateFixtureTree(Doc *doc, TreeModel *treeModel)
{
    if (doc == NULL || treeModel == NULL)
        return;

    treeModel->clear();

    QStringList uniNames = doc->inputOutputMap()->universeNames();

    // add Fixture Groups first
    for (FixtureGroup* grp : doc->fixtureGroups()) // C++11
    {
        foreach(quint32 fxID, grp->fixtureList())
        {
            Fixture *fixture = doc->fixture(fxID);
            if (fixture == NULL)
                continue;

            QLCFixtureMode *mode = fixture->fixtureMode();
            if (mode == NULL)
                continue;

            QString fxPath = QString("%1/%2").arg(grp->name()).arg(fixture->name());

            for (int i = 0; i < fixture->heads(); i++)
            {
                QVariantList headParams;
                headParams.append(QVariant::fromValue(NULL)); // classRef
                headParams.append(App::HeadDragItem); // type
                headParams.append(fixture->id()); // id
                headParams.append(grp->id()); // subid
                headParams.append(i); // head
                treeModel->addItem(QString("%1 %2").arg(tr("Head")).arg(i + 1, 3, 10, QChar('0')), headParams, fxPath);
            }

            // when all the head 'leaves' have been added, set the parent node data
            QVariantList fxParams;
            fxParams.append(QVariant::fromValue(fixture)); // classRef
            fxParams.append(App::FixtureDragItem); // type
            fxParams.append(fixture->id()); // id
            fxParams.append(grp->id()); // subid
            fxParams.append(0); // head

            treeModel->setPathData(fxPath, fxParams);
        }
        // add also the fixture group data
        QVariantList grpParams;
        grpParams.append(QVariant::fromValue(grp)); // classRef
        grpParams.append(App::FixtureGroupDragItem); // type
        grpParams.append(grp->id()); // id
        grpParams.append(0); // subid
        grpParams.append(0); // head

        treeModel->setPathData(grp->name(), grpParams);
    }

    // add the current universes as groups
    for (Fixture *fixture : doc->fixtures()) // C++11
    {
        if (fixture->universe() >= (quint32)uniNames.count())
            continue;

        QString fxPath = QString("%1/%2").arg(uniNames.at(fixture->universe())).arg(fixture->name());

        for (int i = 0; i < fixture->heads(); i++)
        {
            QVariantList headParams;
            headParams.append(QVariant::fromValue(NULL)); // classRef
            headParams.append(App::HeadDragItem); // type
            headParams.append(fixture->id()); // id
            headParams.append(fixture->universe()); // subid
            headParams.append(i); // head
            treeModel->addItem(QString("%1 %2").arg(tr("Head")).arg(i + 1, 3, 10, QChar('0')), headParams, fxPath);
        }

        // when all the channel 'leaves' have been added, set the parent node data
        QVariantList params;
        params.append(QVariant::fromValue(fixture)); // classRef
        params.append(App::FixtureDragItem); // type
        params.append(fixture->id()); // id
        params.append(fixture->universe()); // subid
        params.append(0); // head

        treeModel->setPathData(fxPath, params);
    }

    for (Universe *universe : m_doc->inputOutputMap()->universes())
    {
        // add also the fixture group data
        QVariantList uniParams;
        uniParams.append(QVariant::fromValue(universe)); // classRef
        uniParams.append(App::UniverseDragItem); // type
        uniParams.append(universe->id()); // id
        uniParams.append(0); // subid
        uniParams.append(0); // chIdx

        treeModel->setPathData(universe->name(), uniParams);
    }
}

/************************************************************************
 * Speed
 ************************************************************************/

int EFXEditor::fadeInSpeed() const
{
    if (m_efx == NULL)
        return Function::defaultSpeed();

    return m_efx->fadeInSpeed();
}

void EFXEditor::setFadeInSpeed(int fadeInSpeed)
{
    if (m_efx == NULL || m_efx->fadeInSpeed() == (uint)fadeInSpeed)
        return;

    Tardis::instance()->enqueueAction(FunctionSetFadeIn, m_efx->id(), m_efx->fadeInSpeed(), fadeInSpeed);
    m_efx->setFadeInSpeed(fadeInSpeed);
    emit fadeInSpeedChanged(fadeInSpeed);
}

int EFXEditor::holdSpeed() const
{
    if (m_efx == NULL)
        return Function::defaultSpeed();

    return m_efx->duration();
}

void EFXEditor::setHoldSpeed(int holdSpeed)
{
    if (m_efx == NULL)
        return;

    if (m_efx->duration() - m_efx->fadeInSpeed() == (uint)holdSpeed)
        return;

    uint duration = Function::speedAdd(m_efx->fadeInSpeed(), holdSpeed);
    Tardis::instance()->enqueueAction(FunctionSetDuration, m_efx->id(), m_efx->duration(), duration);
    m_efx->setDuration(duration);

    emit holdSpeedChanged(holdSpeed);
    emit durationChanged(duration);
}

int EFXEditor::fadeOutSpeed() const
{
    if (m_efx == NULL)
        return Function::defaultSpeed();

    return m_efx->fadeOutSpeed();
}

void EFXEditor::setFadeOutSpeed(int fadeOutSpeed)
{
    if (m_efx == NULL || m_efx->fadeOutSpeed() == (uint)fadeOutSpeed)
        return;

    Tardis::instance()->enqueueAction(FunctionSetFadeOut, m_efx->id(), m_efx->fadeOutSpeed(), fadeOutSpeed);
    m_efx->setFadeOutSpeed(fadeOutSpeed);
    emit fadeOutSpeedChanged(fadeOutSpeed);
}

int EFXEditor::duration() const
{
    if (m_efx == NULL)
        return Function::defaultSpeed();

    return m_efx->duration();
}

/************************************************************************
 * Run order and direction
 ************************************************************************/

int EFXEditor::runOrder() const
{
    if (m_efx == NULL)
        return Function::Loop;

    return m_efx->runOrder();
}

void EFXEditor::setRunOrder(int runOrder)
{
    if (m_efx == NULL || m_efx->runOrder() == Function::RunOrder(runOrder))
        return;

    Tardis::instance()->enqueueAction(FunctionSetRunOrder, m_efx->id(), m_efx->runOrder(), runOrder);

    m_efx->setRunOrder(Function::RunOrder(runOrder));
    emit runOrderChanged(runOrder);
}

int EFXEditor::direction() const
{
    if (m_efx == NULL)
        return Function::Forward;

    return m_efx->direction();
}

void EFXEditor::setDirection(int direction)
{
    if (m_efx == NULL || m_efx->direction() == Function::Direction(direction))
        return;

    Tardis::instance()->enqueueAction(FunctionSetDirection, m_efx->id(), m_efx->direction(), direction);

    m_efx->setDirection(Function::Direction(direction));
    emit directionChanged(direction);
}

/************************************************************************
 * Preview
 ************************************************************************/

QVariantList EFXEditor::algorithmData()
{
    return m_algorithmData;
}

QVariantList EFXEditor::fixturesData()
{
    return m_fixturesData;
}

void EFXEditor::updateAlgorithmData()
{
    if (m_efx == NULL)
        return;

    QPolygonF polygon;
    m_efx->preview(polygon);

    m_algorithmData.clear();
    m_fixturesData.clear();

    /** 1- fill a QVariantList or XY coordinates representing
     *  the EFX algorithm */
    for (int i = 0; i < polygon.size(); i++)
    {
        QPointF pt = polygon.at(i);
        m_algorithmData.append(pt.x());
        m_algorithmData.append(pt.y());
    }

    /** 2- for each fixture, find the closest XY coordinate
     *  along the EFX algorithm, and store the start index and
     *  the direction of the animation that will happen in the UI
     *  NOTE: the data array is filled backward to display first fixtures on top */
    for (EFXFixture *fixture : m_efx->fixtures()) // C++11
    {
        float distance = 1000.0;
        float x = 0, y = 0;
        int pathIdx = 0;

        /** Append the delta to apply on each animation step */
        if (fixture->direction() == Function::Forward)
            m_fixturesData.prepend(1);
        else
            m_fixturesData.prepend(-1);

        /** Pre-determined case. Easy to solve */
        if (fixture->startOffset() == 0)
        {
            if (fixture->direction() == Function::Forward)
                m_fixturesData.prepend(0);
            else
                m_fixturesData.prepend(polygon.count() - 1);
        }
        else
        {
            /** With a start offset, we need scan the algorithm points
             *  to find the index of the closest one */
            m_efx->calculatePoint(fixture->direction(), fixture->startOffset(), 0, &x, &y);
            qDebug() << "Got position:" << x << y << fixture->startOffset();

            for (int i = 0; i < polygon.count(); i++)
            {
                QPointF delta = QPointF(x, y) - polygon.at(i);
                qreal pointsDist = delta.manhattanLength();

                if (pointsDist < distance)
                {
                    pathIdx = i;
                    distance = pointsDist;
                }
            }
            //qDebug() << "Closest point found at index" << pathIdx;
            m_fixturesData.prepend(pathIdx);
        }
    }

    emit algorithmDataChanged();
    emit fixturesDataChanged();
}


