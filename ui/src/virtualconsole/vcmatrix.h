/*
  Q Light Controller Plus
  vcmatrix.h

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

#ifndef VCMATRIX_H
#define VCMATRIX_H

#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QHash>

#include "vcwidget.h"
#include "vcmatrixcontrol.h"

class ClickAndGoSlider;
class ClickAndGoWidget;
class QXmlStreamReader;
class QXmlStreamWriter;
class FlowLayout;
class RGBMatrix;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCMatrix             QString("Matrix")

#define KXMLQLCVCMatrixFunction     QString("Function")
#define KXMLQLCVCMatrixFunctionID   QString("ID")

#define KXMLQLCVCMatrixInstantApply QString("InstantApply")

#define KXMLQLCVCMatrixColor1       QString("Color 1")
#define KXMLQLCVCMatrixColor2       QString("Color 2")
#define KXMLQLCVCMatrixColor3       QString("Color 3")
#define KXMLQLCVCMatrixColor4       QString("Color 4")
#define KXMLQLCVCMatrixColor5       QString("Color 5")

#define KXMLQLCVCMatrixVisibilityMask QString("Visibility")

class VCMatrix : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCMatrix)

public:
    enum Visibility
    {
        None                 = 0,
        ShowSlider           = 1 << 0,
        ShowLabel            = 1 << 1,
        ShowPresetCombo      = 1 << 2,
        ShowColor1Button     = 1 << 3,
        ShowColor2Button     = 1 << 4,
        ShowColor3Button     = 1 << 5,
        ShowColor4Button     = 1 << 6,
        ShowColor5Button     = 1 << 7
    };

public:
    /** Default size for newly-created widget */
    static const QSize defaultSize;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCMatrix(QWidget* parent, Doc* doc);
    ~VCMatrix();

    /** @reimp */
    void setID(quint32 id);

private:
    ClickAndGoSlider *m_slider;
    bool m_sliderExternalMovement;
    QLabel *m_label;
    QToolButton *m_mtxColor1Button;
    ClickAndGoWidget *m_mtxColor1CnGWidget;
    QToolButton *m_mtxColor2Button;
    ClickAndGoWidget *m_mtxColor2CnGWidget;
    QToolButton *m_mtxColor3Button;
    ClickAndGoWidget *m_mtxColor3CnGWidget;
    QToolButton *m_mtxColor4Button;
    ClickAndGoWidget *m_mtxColor4CnGWidget;
    QToolButton *m_mtxColor5Button;
    ClickAndGoWidget *m_mtxColor5CnGWidget;
    QComboBox *m_presetCombo;
    FlowLayout *m_controlsLayout;

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    VCWidget* createCopy(VCWidget* parent);

protected:
    bool copyFrom(const VCWidget* widget);

    /*********************************************************************
     * GUI
     *********************************************************************/
public:
    /** @reimp */
    void setCaption(const QString& text);

    /** @reimp */
    void enableWidgetUI(bool enable);

    /** @reimp */
    int sliderValue();
    QString animationValue();
    QColor mtxColor(int id);

signals:
    void sliderValueChanged(int value);
    void mtxColor1Changed();
    void mtxColor2Changed();
    void mtxColor3Changed();
    void mtxColor4Changed();
    void mtxColor5Changed();
    void animationValueChanged(QString name);
    void matrixControlKnobValueChanged(int controlID, int value);

public slots:
    void slotSetSliderValue(int value);
    void slotSliderMoved(int value);
    void slotSetColor1(QColor color);
    void slotColor1Changed(QRgb color);
    void slotSetColor2(QColor color);
    void slotColor2Changed(QRgb color);
    void slotSetColor3(QColor color);
    void slotColor3Changed(QRgb color);
    void slotSetColor4(QColor color);
    void slotColor4Changed(QRgb color);
    void slotSetColor5(QColor color);
    void slotColor5Changed(QRgb color);
    void slotSetAnimationValue(QString name);
    void slotAnimationChanged(int index);
    void slotMatrixControlKnobValueChanged(int controlID, int value);
    void slotMatrixControlPushButtonClicked(int controlID);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Edit this widget's properties */
    void editProperties();

    /*************************************************************************
     * VCWidget-inherited
     *************************************************************************/
public:
    /** @reimp */
    void adjustIntensity(qreal val);

    /*********************************************************************
     * Function attachment
     *********************************************************************/
public:
    /**
     * Attach a function to a VCMatrix. This function is started when the
     * slider is not zero.
     *
     * @param function An ID of a function to attach
     */
    void setFunction(quint32 function);

    /**
     * Get the ID of the function attached to a VCMatrix
     *
     * @return The ID of the attached function or Function::invalidId()
     *         if there isn't one
     */
    quint32 function() const;

    /** @reimp */
    virtual void notifyFunctionStarting(quint32 fid, qreal intensity);

private slots:
    /** Update slider when function stops. */
    void slotFunctionStopped();
    /** Update widget when function changes. */
    void slotFunctionChanged();
    void slotUpdate();

private:
    FunctionParent functionParent() const;

private:
    /** ID of the RGB Matrix that this widget is controlling */
    quint32 m_matrixID;
    /** timer for updating the controls */
    QTimer* m_updateTimer;

    /*********************************************************************
     * Instant changes apply
     *********************************************************************/
public:
    /**
     * Set a flag to immediately apply color and preset changes
     * instead of waiting for the next loop
     *
     * @param instantly true = instant changes, false = loop changes
     */
    void setInstantChanges(bool instantly);

    /**
     * Returns if changes should be applied immediately or
     * at the next loop
     */
    bool instantChanges() const;

private:
    bool m_instantApply;

    /*********************************************************************
     * Base items visibility
     *********************************************************************/
public:
    void setVisibilityMask(quint32 mask);
    quint32 visibilityMask() const;
    static quint32 defaultVisibilityMask();

private:
    quint32 m_visibilityMask;

    /*********************************************************************
     * Custom controls
     *********************************************************************/
public:
    void addCustomControl(VCMatrixControl const& control);
    void resetCustomControls();
    QList<VCMatrixControl *> customControls() const;
    QMap<quint32,QString> customControlsMap() const;
    QWidget *getWidget(VCMatrixControl* control) const;

protected slots:
    void slotCustomControlClicked();
    void slotCustomControlValueChanged();

protected:
    QHash<QWidget *, VCMatrixControl *> m_controls;
    QHash<VCMatrixControl *, QWidget *> m_widgets;

    /*********************************************************************
     * QLC+ Mode
     *********************************************************************/
public slots:
    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * External input / key binding
     *********************************************************************/
public:
    /** @reimp */
    void slotKeyPressed(const QKeySequence& keySequence);

    /** @reimp */
    void updateFeedback();

protected slots:
    /** Called when an external input device produces input data */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);

};

/** @} */

#endif // VCMATRIX_H
