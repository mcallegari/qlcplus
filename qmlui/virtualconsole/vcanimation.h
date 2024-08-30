/*
  Q Light Controller Plus
  vcanimation.h

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

#ifndef VCANIMATION_H
#define VCANIMATION_H

#include "vcwidget.h"

#define KXMLQLCVCAnimation             QString("Matrix")
#define KXMLQLCVCAnimationFunction     QString("Function")
#define KXMLQLCVCAnimationFunctionID   QString("ID")
#define KXMLQLCVCAnimationInstantApply QString("InstantApply")
#define KXMLQLCVCAnimationStartColor   QString("StartColor")
#define KXMLQLCVCAnimationEndColor     QString("EndColor")
#define KXMLQLCVCAnimationVisibilityMask QString("Visibility")

class RGBMatrix;

class VCAnimation : public VCWidget
{
    Q_OBJECT

    Q_PROPERTY(quint32 visibilityMask READ visibilityMask WRITE setVisibilityMask NOTIFY visibilityMaskChanged)
    Q_PROPERTY(quint32 functionID READ functionID WRITE setFunctionID NOTIFY functionIDChanged)
    Q_PROPERTY(int faderLevel READ faderLevel WRITE setFaderLevel NOTIFY faderLevelChanged FINAL)
    Q_PROPERTY(bool instantChanges READ instantChanges WRITE setInstantChanges NOTIFY instantChangesChanged FINAL)

    Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor NOTIFY startColorChanged)
    Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor NOTIFY endColorChanged)
    Q_PROPERTY(QStringList algorithms READ algorithms CONSTANT)
    Q_PROPERTY(int algorithmIndex READ algorithmIndex WRITE setAlgorithmIndex NOTIFY algorithmIndexChanged FINAL)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCAnimation(Doc* doc = nullptr, QObject *parent = nullptr);
    virtual ~VCAnimation();

    /** @reimp */
    QString defaultCaption();

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page);

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent);

    enum Visibility
    {
        Nothing = 0,
        Fader           = 1 << 0,
        Label           = 1 << 1,
        StartColor      = 1 << 2,
        EndColor        = 1 << 3,
        PresetCombo     = 1 << 4
    };
    Q_ENUM(Visibility)

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget);

private:
    FunctionParent functionParent() const;

    /*********************************************************************
     * UI elements visibility
     *********************************************************************/
public:
    quint32 defaultVisibilityMask();

    /** Get/Set the widget's elements visibility bitmask */
    quint32 visibilityMask() const;
    void setVisibilityMask(quint32 mask);

signals:
    void visibilityMaskChanged();

private:
    quint32 m_visibilityMask;

    /*********************************************************************
     * Function control
     *********************************************************************/
public:
    /** Get/Set the controlled RGBMatrix Function */
    quint32 functionID() const;
    void setFunctionID(quint32 newFunctionID);

    /** Get/Set the fader level */
    int faderLevel() const;
    void setFaderLevel(int level);

    /** Get/Set a flag to instantly apply changes on property change */
    bool instantChanges() const;
    void setInstantChanges(bool newInstantChanges);

signals:
    void functionIDChanged();
    void faderLevelChanged();
    void instantChangesChanged();

private:
    quint32 m_functionID;
    RGBMatrix *m_matrix;
    int m_faderLevel;
    bool m_instantChanges;

    /*********************************************************************
     * Colors and presets
     *********************************************************************/
public:
    /** Get/set the start color of the current algorithm */
    QColor startColor() const;
    void setStartColor(QColor color);

    /** Get/set the end color of the current algorithm */
    QColor endColor() const;
    void setEndColor(QColor color);

    /** Returns the list of available algorithms */
    QStringList algorithms() const;

    /** Get/Set the algorithm index to run */
    int algorithmIndex() const;
    void setAlgorithmIndex(int index);

signals:
    void startColorChanged();
    void endColorChanged();
    void algorithmIndexChanged();

    /*********************************************************************
     * External input
     *********************************************************************/

public:
    /** @reimp */
    void updateFeedback();

public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value);

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);
};

#endif
