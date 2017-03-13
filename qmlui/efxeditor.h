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

class Doc;
class EFX;
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

    Q_PROPERTY(QVariantList algorithmData READ algorithmData NOTIFY algorithmDataChanged)

    Q_PROPERTY(int runOrder READ runOrder WRITE setRunOrder NOTIFY runOrderChanged)
    Q_PROPERTY(int direction READ direction WRITE setDirection NOTIFY directionChanged)

public:
    EFXEditor(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~EFXEditor();

    /** Set the ID of the EFX to edit */
    void setFunctionID(quint32 id);

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
    void setIsRelative(bool val);

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
     * Run order and direction
     ************************************************************************/
public:
    /** EFX run order getter/setter */
    int runOrder() const;
    void setRunOrder(int runOrder);

    /** EFX direction getter/setter */
    int direction() const;
    void setDirection(int direction);

signals:
    void runOrderChanged(int runOrder);
    void directionChanged(int direction);

    /************************************************************************
     * Preview
     ************************************************************************/
public:
    QVariantList algorithmData();

private:
    void updateAlgorithmData();

signals:
    void algorithmDataChanged();

private:
    // exchange variable with the QML world
    QVariantList m_algorithmData;
};

#endif // EFXEDITOR_H
