/*
  Q Light Controller Plus
  rgbmatrixeditor.h

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

#ifndef RGBMATRIXEDITOR_H
#define RGBMATRIXEDITOR_H

#include "functioneditor.h"

class Doc;
class RGBMatrix;
class RGBMatrixStep;
class FixtureGroup;

class RGBMatrixEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(int fixtureGroup READ fixtureGroup WRITE setFixtureGroup NOTIFY fixtureGroupChanged)

    Q_PROPERTY(QSize previewSize READ previewSize NOTIFY previewSizeChanged)
    Q_PROPERTY(QVariantList previewData READ previewData NOTIFY previewDataChanged)

    Q_PROPERTY(QStringList algorithms READ algorithms CONSTANT)
    Q_PROPERTY(int algorithmIndex READ algorithmIndex WRITE setAlgorithmIndex NOTIFY algorithmIndexChanged)
    Q_PROPERTY(int algoColors READ algoColors NOTIFY algoColorsChanged)
    Q_PROPERTY(QColor color1 READ color1 WRITE setColor1 NOTIFY color1Changed)
    Q_PROPERTY(QColor color2 READ color2 WRITE setColor2 NOTIFY color2Changed)
    Q_PROPERTY(bool hasColor2 READ hasColor2 WRITE setHasColor2 NOTIFY hasColor2Changed)
    Q_PROPERTY(QColor color3 READ color3 WRITE setColor3 NOTIFY color3Changed)
    Q_PROPERTY(bool hasColor3 READ hasColor3 WRITE setHasColor3 NOTIFY hasColor3Changed)
    Q_PROPERTY(QColor color4 READ color4 WRITE setColor4 NOTIFY color4Changed)
    Q_PROPERTY(bool hasColor4 READ hasColor4 WRITE setHasColor4 NOTIFY hasColor4Changed)
    Q_PROPERTY(QColor color5 READ color5 WRITE setColor5 NOTIFY color5Changed)
    Q_PROPERTY(bool hasColor5 READ hasColor5 WRITE setHasColor5 NOTIFY hasColor5Changed)

    Q_PROPERTY(int blendMode READ blendMode WRITE setBlendMode NOTIFY blendModeChanged)
    Q_PROPERTY(int controlMode READ controlMode WRITE setControlMode NOTIFY controlModeChanged)

    // Text Algorithm specific properties
    Q_PROPERTY(QString algoText READ algoText WRITE setAlgoText NOTIFY algoTextChanged)
    Q_PROPERTY(QFont algoTextFont READ algoTextFont WRITE setAlgoTextFont NOTIFY algoTextFontChanged)
    // Image Algorithm specific properties
    Q_PROPERTY(QString algoImagePath READ algoImagePath WRITE setAlgoImagePath NOTIFY algoImagePathChanged)

    Q_PROPERTY(int animationStyle READ animationStyle WRITE setAnimationStyle NOTIFY animationStyleChanged)
    Q_PROPERTY(QSize algoOffset READ algoOffset WRITE setAlgoOffset NOTIFY algoOffsetChanged)

public:
    RGBMatrixEditor(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~RGBMatrixEditor();

    /** Set the ID of the RGBMatrix to edit */
    void setFunctionID(quint32 id);

    int fixtureGroup() const;
    void setFixtureGroup(int fixtureGroup);

signals:
    void fixtureGroupChanged(int fixtureGroup);

private:
    /** Reference of the RGBMatrix currently being edited */
    RGBMatrix *m_matrix;
    /** Reference to the Fixture Group associated to the RGBMatrix */
    FixtureGroup *m_group;

    /************************************************************************
     * Algorithm
     ************************************************************************/
public:
    QStringList algorithms() const;

    /** Get/set the RGBMatrix algorithm index in the algorithms list */
    int algorithmIndex() const;
    void setAlgorithmIndex(int algoIndex);

    /** Return the accepted colors of the current algorithm */
    int algoColors();

    /** Get/set the color 1 of the current algorithm */
    QColor color1() const;
    void setColor1(QColor algoColor);

    /** Get/set the color 2 of the current algorithm */
    QColor color2() const;
    void setColor2(QColor algoColor);

    bool hasColor2() const;
    void setHasColor2(bool hasColor);

    /** Get/set the color 2 of the current algorithm */
    QColor color3() const;
    void setColor3(QColor algoColor);

    bool hasColor3() const;
    void setHasColor3(bool hasColor);

    /** Get/set the color 2 of the current algorithm */
    QColor color4() const;
    void setColor4(QColor algoColor);

    bool hasColor4() const;
    void setHasColor4(bool hasColor);

    /** Get/set the color 2 of the current algorithm */
    QColor color5() const;
    void setColor5(QColor algoColor);

    bool hasColor5() const;
    void setHasColor5(bool hasColor);

    QString algoText() const;
    void setAlgoText(QString text);

    QFont algoTextFont() const;
    void setAlgoTextFont(QFont algoTextFont);

    QString algoImagePath() const;
    void setAlgoImagePath(QString path);

    QSize algoOffset() const;
    void setAlgoOffset(QSize algoOffset);

    int animationStyle() const;
    void setAnimationStyle(int style);

    /** This is an important method called by the QML world
     *  when a RGBScript algorithm is selected.
     *  The steps are:
     *    - QML creates an empty GridLayout. On completed, it invokes this C++ method
     *    - each parameter label is created by the C++ code
     *    - combo boxes are created in QML, and C++ sends the model and properties
     *    - spin boxes are created in QML, and C++ sends the range and properties
     */
    Q_INVOKABLE void createScriptObjects(QQuickItem *parent);

    Q_INVOKABLE void setScriptStringProperty(QString paramName, QString value);
    Q_INVOKABLE void setScriptIntProperty(QString paramName, int value);
    Q_INVOKABLE void setScriptFloatProperty(QString paramName, double value);

signals:
    void algorithmIndexChanged();
    void algoColorsChanged();
    void color1Changed(QColor color);
    void color2Changed(QColor color);
    void hasColor2Changed(bool hasColor);
    void color3Changed(QColor color);
    void hasColor3Changed(bool hasColor);
    void color4Changed(QColor color);
    void hasColor4Changed(bool hasColor);
    void color5Changed(QColor color);
    void hasColor5Changed(bool hasColor);

    void algoTextChanged(QString text);
    void algoTextFontChanged(QFont algoTextFont);

    void algoImagePathChanged(QString path);

    void algoOffsetChanged(QSize algoOffset);
    void animationStyleChanged(int style);

    /************************************************************************
     * Blend mode
     ************************************************************************/
public:
    int blendMode() const;
    void setBlendMode(int mode);

signals:
    void blendModeChanged();

    /************************************************************************
     * Control mode
     ************************************************************************/
public:
    int controlMode() const;
    void setControlMode(int mode);

signals:
    void controlModeChanged();

    /************************************************************************
     * Preview
     ************************************************************************/
public:
    QSize previewSize() const;
    QVariantList previewData() const;

private slots:
    void slotPreviewTimeout();
    void slotBeatReceived();

private:
    void initPreviewData();

signals:
    void previewSizeChanged();
    void previewDataChanged(QVariantList matrixData);

private:
    /** A timer to perform a timed preview of the RGBMatrix pattern */
    QTimer* m_previewTimer;
    uint m_previewElapsed;
    RGBMatrixStep *m_previewStepHandler;
    bool m_gotBeat;
    QMutex m_previewMutex;

    /** exchange variable with the QML world */
    QVariantList m_previewData;
};

#endif // RGBMATRIXEDITOR_H
