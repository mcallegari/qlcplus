/*
  Q Light Controller Plus
  efxeditor.h

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

#ifndef EFXEDITOR_H
#define EFXEDITOR_H

#include "functioneditor.h"
#include "treemodel.h"

class Doc;
class EFX;
class ListModel;
class FixtureGroup;

class EFXEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QStringList algorithms READ algorithms CONSTANT)
    Q_PROPERTY(int algorithmIndex READ algorithmIndex WRITE setAlgorithmIndex NOTIFY algorithmIndexChanged)

    Q_PROPERTY(bool isRelative READ isRelative WRITE setIsRelative NOTIFY isRelativeChanged)
    Q_PROPERTY(int algorithmWidth READ algorithmWidth WRITE setAlgorithmWidth NOTIFY algorithmWidthChanged)
    Q_PROPERTY(int algorithmHeight READ algorithmHeight WRITE setAlgorithmHeight NOTIFY algorithmHeightChanged)

    Q_PROPERTY(int algorithmXOffset READ algorithmXOffset WRITE setAlgorithmXOffset NOTIFY algorithmXOffsetChanged)
    Q_PROPERTY(int algorithmYOffset READ algorithmYOffset WRITE setAlgorithmYOffset NOTIFY algorithmYOffsetChanged)
    Q_PROPERTY(int algorithmRotation READ algorithmRotation WRITE setAlgorithmRotation NOTIFY algorithmRotationChanged)
    Q_PROPERTY(int algorithmStartOffset READ algorithmStartOffset WRITE setAlgorithmStartOffset NOTIFY algorithmStartOffsetChanged)
    Q_PROPERTY(int algorithmXFrequency READ algorithmXFrequency WRITE setAlgorithmXFrequency NOTIFY algorithmXFrequencyChanged)
    Q_PROPERTY(int algorithmYFrequency READ algorithmYFrequency WRITE setAlgorithmYFrequency NOTIFY algorithmYFrequencyChanged)
    Q_PROPERTY(int algorithmXPhase READ algorithmXPhase WRITE setAlgorithmXPhase NOTIFY algorithmXPhaseChanged)
    Q_PROPERTY(int algorithmYPhase READ algorithmYPhase WRITE setAlgorithmYPhase NOTIFY algorithmYPhaseChanged)

    Q_PROPERTY(QVariant fixtureList READ fixtureList NOTIFY fixtureListChanged)
    Q_PROPERTY(QVariant groupsTreeModel READ groupsTreeModel NOTIFY groupsTreeModelChanged)
    Q_PROPERTY(qreal maxPanDegrees READ maxPanDegrees NOTIFY maxPanDegreesChanged)
    Q_PROPERTY(qreal maxTiltDegrees READ maxTiltDegrees NOTIFY maxTiltDegreesChanged)

    Q_PROPERTY(QVariantList algorithmData READ algorithmData NOTIFY algorithmDataChanged)
    Q_PROPERTY(QVariantList fixturesData READ fixturesData NOTIFY fixturesDataChanged)

public:
    EFXEditor(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~EFXEditor();

    /** Set the ID of the EFX to edit */
    void setFunctionID(quint32 id);

protected slots:
    void slotAttributeChanged(int attrIndex, qreal fraction);

private:
    /** Reference of the EFX currently being edited */
    EFX *m_efx;

    /************************************************************************
     * Algorithm
     ************************************************************************/
public:
    QStringList algorithms() const;

    /** Get/set the EFX selected algorithm index*/
    int algorithmIndex() const;
    void setAlgorithmIndex(int algoIndex);

    /** Get/set if the EFX has relative movement */
    bool isRelative() const;
    void setIsRelative(bool relative);

    /** Get/set the current algorithm width */
    int algorithmWidth() const;
    void setAlgorithmWidth(int algorithmWidth);

    /** Get/set the current algorithm height */
    int algorithmHeight() const;
    void setAlgorithmHeight(int algorithmHeight);

    /** Get/set the current algorithm X offset */
    int algorithmXOffset() const;
    void setAlgorithmXOffset(int algorithmXOffset);

    /** Get/set the current algorithm Y offset */
    int algorithmYOffset() const;
    void setAlgorithmYOffset(int algorithmYOffset);

    /** Get/set the current algorithm rotation */
    int algorithmRotation() const;
    void setAlgorithmRotation(int algorithmRotation);

    /** Get/set the current algorithm start offset */
    int algorithmStartOffset() const;
    void setAlgorithmStartOffset(int algorithmStartOffset);

    /** Get/set the current algorithm X frequency */
    int algorithmXFrequency() const;
    void setAlgorithmXFrequency(int algorithmXFrequency);

    /** Get/set the current algorithm Y frequency */
    int algorithmYFrequency() const;
    void setAlgorithmYFrequency(int algorithmYFrequency);

    /** Get/set the current algorithm X phase */
    int algorithmXPhase() const;
    void setAlgorithmXPhase(int algorithmXPhase);

    /** Get/set the current algorithm Y phase */
    int algorithmYPhase() const;
    void setAlgorithmYPhase(int algorithmYPhase);

signals:
    void algorithmIndexChanged();
    void isRelativeChanged();
    void algorithmWidthChanged();
    void algorithmHeightChanged();
    void algorithmXOffsetChanged();
    void algorithmYOffsetChanged();
    void algorithmRotationChanged();
    void algorithmStartOffsetChanged();
    void algorithmXFrequencyChanged();
    void algorithmYFrequencyChanged();
    void algorithmXPhaseChanged();
    void algorithmYPhaseChanged();

    /************************************************************************
     * Fixtures
     ************************************************************************/
public:
    QVariant fixtureList() const;

    /** Returns the data model to display a tree of FixtureGroups/Fixtures */
    QVariant groupsTreeModel();

    /** Return the maximum Pan/Tilt degrees gotten from a fixture list update */
    qreal maxPanDegrees() const;
    qreal maxTiltDegrees() const;

    Q_INVOKABLE void addGroup(QVariant reference);

    Q_INVOKABLE void addFixture(QVariant reference);

    Q_INVOKABLE void addHead(int fixtureID, int headIndex);

    Q_INVOKABLE void removeHeads(QVariantList heads);

    Q_INVOKABLE void setFixtureMode(quint32 fixtureID, int headIndex, int modeIndex);

    Q_INVOKABLE void setFixtureReversed(quint32 fixtureID, int headIndex, bool reversed);

    Q_INVOKABLE void setFixtureOffset(quint32 fixtureID, int headIndex, int offset);

protected:
    void updateFixtureList();

signals:
    /** Notify the listeners that the fixture list model has changed */
    void fixtureListChanged();

    /** Notify the listeners that the fixture tree model has changed */
    void groupsTreeModelChanged();

    /** Notify the listeners that the Pan/Tilt degrees have changed */
    void maxPanDegreesChanged();
    void maxTiltDegreesChanged();

private:
    /** Reference to a ListModel representing the fixtures list for the QML UI */
    ListModel *m_fixtureList;
    /** Data model used by the QML UI to represent groups/fixtures/heads */
    TreeModel *m_fixtureTree;

    qreal m_maxPanDegrees, m_maxTiltDegrees;

    /************************************************************************
     * Algorithm preview
     ************************************************************************/
public:
    QVariantList algorithmData();
    QVariantList fixturesData();

private:
    void updateAlgorithmData();

signals:
    void algorithmDataChanged();
    void fixturesDataChanged();

private:
    /** EFX algorithm data exchanged with the QML world */
    QVariantList m_algorithmData;
    /** Start index and direction of each fixture of the EFX */
    QVariantList m_fixturesData;
};

#endif // EFXEDITOR_H
