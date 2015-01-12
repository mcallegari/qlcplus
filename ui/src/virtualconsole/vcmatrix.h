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
class QDomDocument;
class QDomElement;
class FlowLayout;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCMatrix "Matrix"

#define KXMLQLCVCMatrixFunction "Function"
#define KXMLQLCVCMatrixFunctionID "ID"

#define KXMLQLCVCMatrixInstantApply "InstantApply"

#define KXMLQLCVCMatrixStartColor "StartColor"
#define KXMLQLCVCMatrixEndColor "EndColor"

#define KXMLQLCVCMatrixVisibilityMask "Visibility"

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
        ShowStartColorButton = 1 << 2,
        ShowEndColorButton   = 1 << 3,
        ShowPresetCombo      = 1 << 4,
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
    quint32 m_matrixID;
    ClickAndGoSlider *m_slider;
    QLabel *m_label;
    QToolButton *m_startColorButton;
    ClickAndGoWidget *m_scCnGWidget;
    QToolButton *m_endColorButton;
    ClickAndGoWidget *m_ecCnGWidget;
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

private slots:
    void slotSliderMoved(int value);
    void slotStartColorChanged(QRgb color);
    void slotEndColorChanged(QRgb color);
    void slotAnimationChanged(QString name);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Edit this widget's properties */
    void editProperties();

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

private slots:
    /** Update slider when function stops. */
    void slotFunctionStopped();
    /** Update slider when function starts. */
    void slotFunctionAttributeChanged(int attrIndex, qreal fraction);
    /** Update widget when function changes. */
    void slotFunctionChanged();
    void slotUpdate();

private:
    /** timer for updating the step list */
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

protected slots:
    void slotCustomControlClicked();
    void slotCustomControlValueChanged();

protected:
    QHash<QWidget *, VCMatrixControl *> m_controls;

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
    bool loadXML(const QDomElement* root);
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);

};

/** @} */

#endif // VCMATRIX_H
