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

#include "efxeditor.h"
#include "listmodel.h"

#include "efx.h"
#include "doc.h"

EFXEditor::EFXEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_efx(NULL)
    , m_fixtureTree(NULL)
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
        m_efx = NULL;
        return;
    }

    m_efx = qobject_cast<EFX *>(m_doc->function(id));
    if (m_efx != NULL)
    {
        updateAlgorithmData();
        emit algorithmIndexChanged();
    }

    FunctionEditor::setFunctionID(id);
    updateFixtureList();
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
    if (m_efx == NULL)
        return;

    if (algoIndex == m_efx->algorithm())
        return;

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

void EFXEditor::updateFixtureList()
{
    m_fixtureList->clear();

    if (m_efx == NULL)
        return;

    // listRoles << "name" << "fxID" << "head" << "isSelected" << "reverse" << "offset";

    for (EFXFixture *ef : m_efx->fixtures()) // C++11
    {
        QVariantMap fxMap;
        GroupHead head = ef->head();
        Fixture *fixture = m_doc->fixture(head.fxi);

        if (fixture == NULL || head.isValid() == false)
            continue;

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
                headParams.append("FHG"); // type (FHG = Fixture Head from Group)
                headParams.append(fixture->id()); // id
                headParams.append(grp->id()); // subid
                headParams.append(i); // head
                treeModel->addItem(QString("%1 %2").arg(tr("Head")).arg(i + 1, 3, 10, QChar('0')), headParams, fxPath);
            }

            // when all the head 'leaves' have been added, set the parent node data
            QVariantList params;
            params.append(QVariant::fromValue(fixture)); // classRef
            params.append("FXG"); // type
            params.append(fixture->id()); // id
            params.append(grp->id()); // subid
            params.append(0); // head

            treeModel->setPathData(fxPath, params);
        }
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
            headParams.append("FHU"); // type (FHU = Fixture Head from Universe)
            headParams.append(fixture->id()); // id
            headParams.append(fixture->universe()); // subid
            headParams.append(i); // head
            treeModel->addItem(QString("%1 %2").arg(tr("Head")).arg(i + 1, 3, 10, QChar('0')), headParams, fxPath);
        }

        // when all the channel 'leaves' have been added, set the parent node data
        QVariantList params;
        params.append(QVariant::fromValue(fixture)); // classRef
        params.append("FXU"); // type
        params.append(fixture->id()); // id
        params.append(fixture->universe()); // subid
        params.append(0); // head

        treeModel->setPathData(fxPath, params);
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
    if (m_efx == NULL)
        return;

    if (m_efx->fadeInSpeed() == (uint)fadeInSpeed)
        return;

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
    m_efx->setDuration(duration);

    emit holdSpeedChanged(holdSpeed);
}

int EFXEditor::fadeOutSpeed() const
{
    if (m_efx == NULL)
        return Function::defaultSpeed();

    return m_efx->fadeOutSpeed();
}

void EFXEditor::setFadeOutSpeed(int fadeOutSpeed)
{
    if (m_efx == NULL)
        return;

    if (m_efx->fadeOutSpeed() == (uint)fadeOutSpeed)
        return;

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
    for (EFXFixture *fixture : m_efx->fixtures())
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

            for (int i = 0; i < polygon.count(); i++)
            {
                float tmpDist = qAbs(x - polygon.at(i).x()) + qAbs(y - polygon.at(i).y());
                if (tmpDist < distance)
                    pathIdx = i;
            }
            m_fixturesData.prepend(pathIdx);
        }
    }

    emit algorithmDataChanged();
    emit fixturesDataChanged();
}


