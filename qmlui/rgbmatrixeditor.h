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
    Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor NOTIFY startColorChanged)
    Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor NOTIFY endColorChanged)
    Q_PROPERTY(bool hasEndColor READ hasEndColor WRITE setHasEndColor NOTIFY hasEndColorChanged)

    Q_PROPERTY(int fadeInSpeed READ fadeInSpeed WRITE setFadeInSpeed NOTIFY fadeInSpeedChanged)
    Q_PROPERTY(int holdSpeed READ holdSpeed WRITE setHoldSpeed NOTIFY holdSpeedChanged)
    Q_PROPERTY(int fadeOutSpeed READ fadeOutSpeed WRITE setFadeOutSpeed NOTIFY fadeOutSpeedChanged)

    // Text Algorithm specific properties
    Q_PROPERTY(QString algoText READ algoText WRITE setAlgoText NOTIFY algoTextChanged)
    Q_PROPERTY(QFont algoTextFont READ algoTextFont WRITE setAlgoTextFont NOTIFY algoTextFontChanged)
    // Image Algorithm specific properties
    Q_PROPERTY(QString algoImagePath READ algoImagePath WRITE setAlgoImagePath NOTIFY algoImagePathChanged)

    Q_PROPERTY(int animationStyle READ animationStyle WRITE setAnimationStyle NOTIFY animationStyleChanged)
    Q_PROPERTY(QSize algoOffset READ algoOffset WRITE setAlgoOffset NOTIFY algoOffsetChanged)

    Q_PROPERTY(int runOrder READ runOrder WRITE setRunOrder NOTIFY runOrderChanged)
    Q_PROPERTY(int direction READ direction WRITE setDirection NOTIFY directionChanged)
    
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

    /** Get/set the start color of the current algorithm */
    QColor startColor() const;
    void setStartColor(QColor startColor);

    /** Get/set the end color of the current algorithm */
    QColor endColor() const;
    void setEndColor(QColor algoEndColor);

    bool hasEndColor() const;
    void setHasEndColor(bool hasEndCol);

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

signals:
    void algorithmIndexChanged();
    void algoColorsChanged();
    void startColorChanged(QColor startColor);
    void endColorChanged(QColor endColor);
    void hasEndColorChanged(bool hasEndColor);

    void algoTextChanged(QString text);
    void algoTextFontChanged(QFont algoTextFont);

    void algoImagePathChanged(QString path);

    void algoOffsetChanged(QSize algoOffset);
    void animationStyleChanged(int style);

    /************************************************************************
     * Speed
     ************************************************************************/
public:
    int fadeInSpeed() const;
    void setFadeInSpeed(int fadeInSpeed);

    int holdSpeed() const;
    void setHoldSpeed(int holdSpeed);

    int fadeOutSpeed() const;
    void setFadeOutSpeed(int fadeOutSpeed);

signals:
    void fadeInSpeedChanged(int fadeInSpeed);
    void holdSpeedChanged(int holdSpeed);
    void fadeOutSpeedChanged(int fadeOutSpeed);

    /************************************************************************
     * Run order and direction
     ************************************************************************/
public:
    /** RGB Matrix run order getter/setter */
    int runOrder() const;
    void setRunOrder(int runOrder);

    /** RGB Matrix direction getter/setter */
    int direction() const;
    void setDirection(int direction);

signals:
    void runOrderChanged(int runOrder);
    void directionChanged(int direction);

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

    /** exchange variable with the QML world */
    QVariantList m_previewData;
};

#endif // RGBMATRIXEDITOR_H
