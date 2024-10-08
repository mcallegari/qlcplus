/*
  Q Light Controller Plus
  vcmatrix.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QWidgetAction>
#include <QComboBox>
#include <QSettings>
#include <QLayout>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QMenu>
#include <math.h>

#include "vcmatrixproperties.h"
#include "vcpropertieseditor.h"
#include "clickandgoslider.h"
#include "clickandgowidget.h"
#include "knobwidget.h"
#include "qlcmacros.h"
#include "rgbalgorithm.h"
#include "flowlayout.h"
#include "rgbmatrix.h"
#include "vcmatrix.h"
#include "function.h"
#include "rgbtext.h"
#include "doc.h"

#define UPDATE_TIMEOUT 50

static const QString controlBtnSS = "QPushButton { background-color: %1; height: 32px; border: 2px solid #6A6A6A; border-radius: 5px; }"
                                    "QPushButton:pressed { border: 2px solid #0000FF; }"
                                    "QPushButton:disabled { border: 2px solid #BBBBBB; color: #8f8f8f }";

const QSize VCMatrix::defaultSize(QSize(160, 120));

VCMatrix::VCMatrix(QWidget *parent, Doc *doc)
    : VCWidget(parent, doc)
    , m_sliderExternalMovement(false)
    , m_matrixID(Function::invalidId())
    , m_instantApply(true)
    , m_visibilityMask(VCMatrix::defaultVisibilityMask())
{
    /* Set the class name "VCMatrix" as the object name as well */
    setObjectName(VCMatrix::staticMetaObject.className());
    setFrameStyle(KVCFrameStyleSunken);

    QHBoxLayout *hBox = new QHBoxLayout(this);
    //hBox->setContentsMargins(3, 3, 3, 10);
    //hBox->setSpacing(5);

    m_slider = new ClickAndGoSlider();
    m_slider->setSliderStyleSheet(CNG_DEFAULT_STYLE);
    m_slider->setFixedWidth(32);
    m_slider->setRange(0, 255);
    m_slider->setPageStep(1);
    m_slider->setInvertedAppearance(false);
    hBox->addWidget(m_slider);

    connect(m_slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderMoved(int)));

    QVBoxLayout *vbox = new QVBoxLayout();

    /* Color 1 Button */
    m_mtxColor1Button = new QToolButton(this);
    m_mtxColor1Button->setFixedSize(48, 48);
    m_mtxColor1Button->setIconSize(QSize(42, 42));

    QWidgetAction* scAction = new QWidgetAction(this);
    m_mtxColor1CnGWidget = new ClickAndGoWidget();
    m_mtxColor1CnGWidget->setType(ClickAndGoWidget::RGB, NULL);
    scAction->setDefaultWidget(m_mtxColor1CnGWidget);
    QMenu *color1Menu = new QMenu();
    color1Menu->addAction(scAction);
    m_mtxColor1Button->setMenu(color1Menu);
    m_mtxColor1Button->setPopupMode(QToolButton::InstantPopup);

    connect(m_mtxColor1CnGWidget, SIGNAL(colorChanged(QRgb)),
            this, SLOT(slotColor1Changed(QRgb)));

    /* Color 2 Button */
    m_mtxColor2Button = new QToolButton(this);
    m_mtxColor2Button->setFixedSize(48, 48);
    m_mtxColor2Button->setIconSize(QSize(42, 42));

    QWidgetAction* ecAction2 = new QWidgetAction(this);
    m_mtxColor2CnGWidget = new ClickAndGoWidget();
    m_mtxColor2CnGWidget->setType(ClickAndGoWidget::RGB, NULL);
    ecAction2->setDefaultWidget(m_mtxColor2CnGWidget);
    QMenu *color2Menu = new QMenu();
    color2Menu->addAction(ecAction2);
    m_mtxColor2Button->setMenu(color2Menu);
    m_mtxColor2Button->setPopupMode(QToolButton::InstantPopup);

    connect(m_mtxColor2CnGWidget, SIGNAL(colorChanged(QRgb)),
            this, SLOT(slotColor2Changed(QRgb)));

    /* 3rd Color Button */
    m_mtxColor3Button = new QToolButton(this);
    m_mtxColor3Button->setFixedSize(48, 48);
    m_mtxColor3Button->setIconSize(QSize(42, 42));

    QWidgetAction* ecAction3 = new QWidgetAction(this);
    m_mtxColor3CnGWidget = new ClickAndGoWidget();
    m_mtxColor3CnGWidget->setType(ClickAndGoWidget::RGB, NULL);
    ecAction3->setDefaultWidget(m_mtxColor3CnGWidget);
    QMenu *color3Menu = new QMenu();
    color3Menu->addAction(ecAction3);
    m_mtxColor3Button->setMenu(color3Menu);
    m_mtxColor3Button->setPopupMode(QToolButton::InstantPopup);

    connect(m_mtxColor3CnGWidget, SIGNAL(colorChanged(QRgb)),
            this, SLOT(slotColor3Changed(QRgb)));

    /* 4th Color Button */
    m_mtxColor4Button = new QToolButton(this);
    m_mtxColor4Button->setFixedSize(48, 48);
    m_mtxColor4Button->setIconSize(QSize(42, 42));

    QWidgetAction* ecAction4 = new QWidgetAction(this);
    m_mtxColor4CnGWidget = new ClickAndGoWidget();
    m_mtxColor4CnGWidget->setType(ClickAndGoWidget::RGB, NULL);
    ecAction4->setDefaultWidget(m_mtxColor4CnGWidget);
    QMenu *color4Menu = new QMenu();
    color4Menu->addAction(ecAction4);
    m_mtxColor4Button->setMenu(color4Menu);
    m_mtxColor4Button->setPopupMode(QToolButton::InstantPopup);

    connect(m_mtxColor4CnGWidget, SIGNAL(colorChanged(QRgb)),
            this, SLOT(slotColor4Changed(QRgb)));

    /* 5th Color Button */
    m_mtxColor5Button = new QToolButton(this);
    m_mtxColor5Button->setFixedSize(48, 48);
    m_mtxColor5Button->setIconSize(QSize(42, 42));

    QWidgetAction* ecAction5 = new QWidgetAction(this);
    m_mtxColor5CnGWidget = new ClickAndGoWidget();
    m_mtxColor5CnGWidget->setType(ClickAndGoWidget::RGB, NULL);
    ecAction5->setDefaultWidget(m_mtxColor5CnGWidget);
    QMenu *color5Menu = new QMenu();
    color5Menu->addAction(ecAction5);
    m_mtxColor5Button->setMenu(color5Menu);
    m_mtxColor5Button->setPopupMode(QToolButton::InstantPopup);

    connect(m_mtxColor5CnGWidget, SIGNAL(colorChanged(QRgb)),
            this, SLOT(slotColor5Changed(QRgb)));

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    vbox->addWidget(m_label);

    QHBoxLayout *btnHbox = new QHBoxLayout();

    btnHbox->addWidget(m_mtxColor1Button);
    btnHbox->addWidget(m_mtxColor2Button);
    btnHbox->addWidget(m_mtxColor3Button);
    btnHbox->addWidget(m_mtxColor4Button);
    btnHbox->addWidget(m_mtxColor5Button);

    vbox->addLayout(btnHbox);

    m_presetCombo = new QComboBox(this);
    //m_presetCombo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_presetCombo->addItems(RGBAlgorithm::algorithms(m_doc));
    connect(m_presetCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotAnimationChanged(int)));
    vbox->addWidget(m_presetCombo);

    hBox->addLayout(vbox);

    m_controlsLayout = new FlowLayout();
    vbox->addLayout(m_controlsLayout);

    setType(VCWidget::AnimationWidget);
    setCaption(QString());
    /* Initial size */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_RGBMATRIX_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(defaultSize);

    /* Update timer */
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, SIGNAL(timeout()),
            this, SLOT(slotUpdate()));
    m_updateTimer->setSingleShot(true);

    /* Update the slider according to current mode */
    slotModeChanged(m_doc->mode());
    setLiveEdit(m_liveEdit);
}

VCMatrix::~VCMatrix()
{
    foreach (VCMatrixControl* control, m_controls)
    {
        delete control;
    }
}

void VCMatrix::setID(quint32 id)
{
    VCWidget::setID(id);

    if (caption().isEmpty())
        setCaption(tr("Animation %1").arg(id));
}

/*********************************************************************
 * Clipboard
 *********************************************************************/

VCWidget *VCMatrix::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != NULL);

    VCMatrix* matrix = new VCMatrix(parent, m_doc);
    if (matrix->copyFrom(this) == false)
    {
        delete matrix;
        matrix = NULL;
    }

    return matrix;
}

bool VCMatrix::copyFrom(const VCWidget* widget)
{
    const VCMatrix* matrix = qobject_cast <const VCMatrix*> (widget);
    if (matrix == NULL)
        return false;

    /* Copy vcmatrix-specific stuff */
    setFunction(matrix->function());
    setInstantChanges(matrix->instantChanges());
    setVisibilityMask(matrix->visibilityMask());

    resetCustomControls();
    foreach (VCMatrixControl const* control, matrix->customControls())
    {
        addCustomControl(*control);
    }

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*********************************************************************
 * GUI
 *********************************************************************/

void VCMatrix::setCaption(const QString &text)
{
    VCWidget::setCaption(text);
    m_label->setText(text);
}

void VCMatrix::enableWidgetUI(bool enable)
{
    m_slider->setEnabled(enable);
    m_mtxColor1Button->setEnabled(enable);
    m_mtxColor2Button->setEnabled(enable);
    m_mtxColor3Button->setEnabled(enable);
    m_mtxColor4Button->setEnabled(enable);
    m_mtxColor5Button->setEnabled(enable);
    m_presetCombo->setEnabled(enable);

    foreach (QWidget *ctlBtn, m_controls.keys())
        ctlBtn->setEnabled(enable);

    // Update buttons state
    if (enable)
        slotUpdate();
}

void VCMatrix::slotSetSliderValue(int value)
{
    m_slider->setValue(value);
    slotSliderMoved(value);
}

void VCMatrix::slotSliderMoved(int value)
{
    Function* function = m_doc->function(m_matrixID);
    if (function == NULL || mode() == Doc::Design)
        return;

    if (m_sliderExternalMovement)
        return;

    if (value == 0)
    {
        // Make sure we ignore the fade out time
        adjustFunctionIntensity(function, 0);
        if (function->stopped() == false)
        {
            function->stop(functionParent());
            resetIntensityOverrideAttribute();
        }
    }
    else
    {
        qreal pIntensity = qreal(value) / qreal(UCHAR_MAX);
        emit functionStarting(m_matrixID, pIntensity);
        adjustFunctionIntensity(function, pIntensity * intensity());
        if (function->stopped() == true)
        {
            // TODO once #758 is fixed: function started by a fader -> override fade in time
            function->start(m_doc->masterTimer(), functionParent());
        }
    }

    emit sliderValueChanged(value);
}

int VCMatrix::sliderValue()
{
    return m_slider->value();
}

void VCMatrix::slotSetColor1(QColor color)
{
    RGBMatrix *matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL)
        return;

    if (matrix->getColor(0) != color)
    {
        matrix->setColor(0, color);
        emit mtxColor1Changed();
    }
}

void VCMatrix::slotSetColor2(QColor color)
{
    RGBMatrix *matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL)
        return;

    if (matrix->getColor(1) != color)
    {
        matrix->setColor(1, color);
        emit mtxColor2Changed();
    }
}

void VCMatrix::slotSetColor3(QColor color)
{
    RGBMatrix *matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL)
        return;

    if (matrix->getColor(2) != color)
    {
        matrix->setColor(2, color);
        emit mtxColor3Changed();
    }
}

void VCMatrix::slotSetColor4(QColor color)
{
    RGBMatrix *matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL)
        return;

    if (matrix->getColor(3) != color)
    {
        matrix->setColor(3, color);
        emit mtxColor4Changed();
    }
}

void VCMatrix::slotSetColor5(QColor color)
{
    RGBMatrix *matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL)
        return;

    if (matrix->getColor(4) != color)
    {
        matrix->setColor(4, color);
        emit mtxColor5Changed();
    }
}

QColor VCMatrix::mtxColor(int id)
{
    RGBMatrix *matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL)
        return QColor();

    return matrix->getColor(id);
}

void VCMatrix::slotColor1Changed(QRgb color)
{
    QColor col(color);
    slotSetColor1(col);
    QPixmap px(42, 42);
    px.fill(col);
    m_mtxColor1Button->setIcon(px);

    RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL || mode() == Doc::Design)
        return;

    matrix->setColor(0, col);
    if (instantChanges() == true)
        matrix->updateColorDelta();
}

void VCMatrix::slotColor2Changed(QRgb color)
{
    QColor col(color);
    slotSetColor2(col);
    QPixmap px(42, 42);
    px.fill(col);
    m_mtxColor2Button->setIcon(px);

    RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL || mode() == Doc::Design)
        return;

    matrix->setColor(1, col);
    if (instantChanges() == true)
        matrix->updateColorDelta();
}

void VCMatrix::slotColor3Changed(QRgb color)
{
    QColor col(color);
    slotSetColor3(col);
    QPixmap px(42, 42);
    px.fill(col);
    m_mtxColor3Button->setIcon(px);

    RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL || mode() == Doc::Design)
        return;

    matrix->setColor(2, col);
}

void VCMatrix::slotColor4Changed(QRgb color)
{
    QColor col(color);
    slotSetColor4(col);
    QPixmap px(42, 42);
    px.fill(col);
    m_mtxColor4Button->setIcon(px);

    RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL || mode() == Doc::Design)
        return;

    matrix->setColor(3, col);
}

void VCMatrix::slotColor5Changed(QRgb color)
{
    QColor col(color);
    slotSetColor5(col);
    QPixmap px(42, 42);
    px.fill(col);
    m_mtxColor5Button->setIcon(px);

    RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL || mode() == Doc::Design)
        return;

    matrix->setColor(4, col);
}

void VCMatrix::slotSetAnimationValue(QString name)
{
    for (int i = 0; i < m_presetCombo->count(); i++)
    {
        if (name == m_presetCombo->itemText(i))
        {
            m_presetCombo->setCurrentIndex(i);
            slotAnimationChanged(i);
            return;
        }
    }
}

void VCMatrix::slotAnimationChanged(int index)
{
    RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL || mode() == Doc::Design)
        return;

    QString pValue = m_presetCombo->itemText(index);
    RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, pValue);
    matrix->setAlgorithm(algo);
    if (instantChanges() == true)
        matrix->updateColorDelta();

    emit animationValueChanged(pValue);
}

QString VCMatrix::animationValue()
{
    return m_presetCombo->currentText();
}

void VCMatrix::setVisibilityMask(quint32 mask)
{
    if (mask & ShowSlider) m_slider->show();
    else m_slider->hide();

    if (mask & ShowLabel) m_label->show();
    else m_label->hide();

    if (mask & ShowColor1Button) m_mtxColor1Button->show();
    else m_mtxColor1Button->hide();

    if (mask & ShowColor2Button) m_mtxColor2Button->show();
    else m_mtxColor2Button->hide();

    if (mask & ShowColor3Button) m_mtxColor3Button->show();
    else m_mtxColor3Button->hide();

    if (mask & ShowColor4Button) m_mtxColor4Button->show();
    else m_mtxColor4Button->hide();

    if (mask & ShowColor5Button) m_mtxColor5Button->show();
    else m_mtxColor5Button->hide();

    if (mask & ShowPresetCombo) m_presetCombo->show();
    else m_presetCombo->hide();

    m_visibilityMask = mask;
}

quint32 VCMatrix::visibilityMask() const
{
    return m_visibilityMask;
}

quint32 VCMatrix::defaultVisibilityMask()
{
    return ShowSlider
        | ShowLabel
        | ShowPresetCombo
        | ShowColor1Button
        | ShowColor2Button
        | ShowColor3Button
        | ShowColor4Button
        | ShowColor5Button
        ;
}

/*********************************************************************
 * Properties
 *********************************************************************/

void VCMatrix::editProperties()
{
    VCMatrixProperties prop(this, m_doc);
    if (prop.exec() == QDialog::Accepted)
        m_doc->setModified();
}

/*************************************************************************
 * VCWidget-inherited
 *************************************************************************/

void VCMatrix::adjustIntensity(qreal val)
{
    VCWidget::adjustIntensity(val);
    this->slotSliderMoved(this->m_slider->value());
}

/*********************************************************************
 * Function attachment
 *********************************************************************/

void VCMatrix::setFunction(quint32 id)
{
    Function *old = m_doc->function(m_matrixID);
    if (old != NULL)
    {
        disconnect(old, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped()));
        disconnect(old, SIGNAL(changed(quint32)),
                this, SLOT(slotFunctionChanged()));
    }

    RGBMatrix* matrix = qobject_cast<RGBMatrix*> (m_doc->function(id));

    if (matrix == NULL)
        m_matrixID = Function::invalidId();
    else
    {
        m_matrixID = id;
        connect(matrix, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped()));
        connect(matrix, SIGNAL(changed(quint32)),
                this, SLOT(slotFunctionChanged()));
    }

    slotUpdate();
}

quint32 VCMatrix::function() const
{
    return m_matrixID;
}

void VCMatrix::notifyFunctionStarting(quint32 fid, qreal functionIntensity)
{
    if (mode() == Doc::Design)
        return;

    if (fid == m_matrixID)
        return;

    int value = SCALE(1.0 - functionIntensity,
            0, 1.0,
            m_slider->minimum(), m_slider->maximum());
    if (m_slider->value() > value)
    {
        m_sliderExternalMovement = true;
        m_slider->setValue(value);
        m_sliderExternalMovement = false;

        Function* function = m_doc->function(m_matrixID);
        if (function != NULL)
        {
            qreal pIntensity = qreal(value) / qreal(UCHAR_MAX);
            adjustFunctionIntensity(function, pIntensity * intensity());
            if (value == 0 && !function->stopped())
            {
                function->stop(functionParent());
                resetIntensityOverrideAttribute();
            }
        }
    }
}

void VCMatrix::slotFunctionStopped()
{
    m_slider->blockSignals(true);
    m_slider->setValue(0);
    resetIntensityOverrideAttribute();
    m_slider->blockSignals(false);
}

void VCMatrix::slotFunctionChanged()
{
    m_updateTimer->start(UPDATE_TIMEOUT);
}

void VCMatrix::slotUpdate()
{
    if (m_matrixID == Function::invalidId())
        return;

    RGBMatrix *matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL)
        return;

    QString algorithmName;
    RGBAlgorithm::Type algorithmType = RGBAlgorithm::Plain;
    QHash<QString, QString> algorithmProperties;
    QString algorithmText;

    {
        QMutexLocker locker(&matrix->algorithmMutex());

        RGBAlgorithm *algo = matrix->algorithm();
        if (algo != NULL)
        {
            algorithmName = algo->name();
            algorithmType = algo->type();
            if (algorithmType == RGBAlgorithm::Script)
            {
                algorithmProperties = reinterpret_cast<RGBScript*>(algo)->propertiesAsStrings();
            }
            else if (algorithmType == RGBAlgorithm::Text)
            {
                algorithmText = reinterpret_cast<RGBText*>(algo)->text();
            }
        }
    }

    // Color buttons
    QPixmap px(42, 42);
    px.fill(matrix->getColor(0));
    m_mtxColor1Button->setIcon(px);

    if (matrix->getColor(1) == QColor())
        px.fill(Qt::transparent);
    else
        px.fill(matrix->getColor(1));
    m_mtxColor2Button->setIcon(px);

    if (matrix->getColor(2) == QColor())
        px.fill(Qt::transparent);
    else
        px.fill(matrix->getColor(2));
    m_mtxColor3Button->setIcon(px);

    if (matrix->getColor(3) == QColor())
        px.fill(Qt::transparent);
    else
        px.fill(matrix->getColor(3));
    m_mtxColor4Button->setIcon(px);

    if (matrix->getColor(4) == QColor())
        px.fill(Qt::transparent);
    else
        px.fill(matrix->getColor(4));
    m_mtxColor5Button->setIcon(px);

    // Algo combo box
    if (algorithmName != QString())
    {
        m_presetCombo->blockSignals(true);
        m_presetCombo->setCurrentText(algorithmName);
        m_presetCombo->blockSignals(false);
    }

    // Custom Buttons
    for (QHash<QWidget *, VCMatrixControl *>::iterator it = m_controls.begin();
            it != m_controls.end(); ++it)
    {
        QWidget *widget = it.key();
        VCMatrixControl *control = it.value();

        if (control->m_type == VCMatrixControl::Color1Knob)
        {
            KnobWidget *knob = reinterpret_cast<KnobWidget*>(widget);
            int val = control->rgbToValue(matrix->getColor(0).rgb());
            if (knob->value() != val)
            {
                knob->blockSignals(true);
                knob->setValue(val);
                emit matrixControlKnobValueChanged(control->m_id, val);
                knob->blockSignals(false);
            }
        }
        else if (control->m_type == VCMatrixControl::Color2Knob)
        {
            KnobWidget *knob = reinterpret_cast<KnobWidget*>(widget);
            int val = control->rgbToValue(matrix->getColor(1).rgb());
            if (knob->value() != val)
            {
                knob->blockSignals(true);
                knob->setValue(val);
                emit matrixControlKnobValueChanged(control->m_id, val);
                knob->blockSignals(false);
            }
        }
        else if (control->m_type == VCMatrixControl::Color3Knob)
        {
            KnobWidget *knob = reinterpret_cast<KnobWidget*>(widget);
            int val = control->rgbToValue(matrix->getColor(2).rgb());
            if (knob->value() != val)
            {
                knob->blockSignals(true);
                knob->setValue(val);
                emit matrixControlKnobValueChanged(control->m_id, val);
                knob->blockSignals(false);
            }
        }
        else if (control->m_type == VCMatrixControl::Color4Knob)
        {
            KnobWidget *knob = reinterpret_cast<KnobWidget*>(widget);
            int val = control->rgbToValue(matrix->getColor(3).rgb());
            if (knob->value() != val)
            {
                knob->blockSignals(true);
                knob->setValue(val);
                emit matrixControlKnobValueChanged(control->m_id, val);
                knob->blockSignals(false);
            }
        }
        else if (control->m_type == VCMatrixControl::Color5Knob)
        {
            KnobWidget *knob = reinterpret_cast<KnobWidget*>(widget);
            int val = control->rgbToValue(matrix->getColor(4).rgb());
            if (knob->value() != val)
            {
                knob->blockSignals(true);
                knob->setValue(val);
                emit matrixControlKnobValueChanged(control->m_id, val);
                knob->blockSignals(false);
            }
        }
        else if (control->m_type == VCMatrixControl::Color1)
        {
            QPushButton* button = reinterpret_cast<QPushButton*>(it.key());
            button->setDown(matrix->getColor(0) == control->m_color);
        }
        else if (control->m_type == VCMatrixControl::Color2)
        {
            QPushButton* button = reinterpret_cast<QPushButton*>(it.key());
            button->setDown(matrix->getColor(1) == control->m_color);
        }
        else if (control->m_type == VCMatrixControl::Color3)
        {
            QPushButton* button = reinterpret_cast<QPushButton*>(it.key());
            button->setDown(matrix->getColor(2) == control->m_color);
        }
        else if (control->m_type == VCMatrixControl::Color4)
        {
            QPushButton* button = reinterpret_cast<QPushButton*>(it.key());
            button->setDown(matrix->getColor(3) == control->m_color);
        }
        else if (control->m_type == VCMatrixControl::Color5)
        {
            QPushButton* button = reinterpret_cast<QPushButton*>(it.key());
            button->setDown(matrix->getColor(4) == control->m_color);
        }
        else if (control->m_type == VCMatrixControl::Animation)
        {
            bool on = false;
            if (algorithmType == RGBAlgorithm::Script &&
                algorithmName == control->m_resource)
            {
                on = true;
                for (QHash<QString, QString>::const_iterator it = control->m_properties.begin();
                        it != control->m_properties.end(); ++it)
                {
                    if (algorithmProperties.value(it.key(), QString()) != it.value())
                        on = false;
                }
            }

            QPushButton *button = reinterpret_cast<QPushButton*>(it.key());
            button->setDown(on);
        }
        else if (control->m_type == VCMatrixControl::Text)
        {
            bool on = false;
            if (algorithmType == RGBAlgorithm::Text &&
                algorithmText == control->m_resource)
            {
                on = true;
            }

            QPushButton *button = reinterpret_cast<QPushButton*>(it.key());
            button->setDown(on);
        }
    }

    updateFeedback();
}

FunctionParent VCMatrix::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

/*********************************************************************
 * Instant changes apply
 *********************************************************************/

void VCMatrix::setInstantChanges(bool instantly)
{
    m_instantApply = instantly;
}

bool VCMatrix::instantChanges() const
{
    return m_instantApply;
}

/*********************************************************************
 * Custom controls
 *********************************************************************/

void VCMatrix::addCustomControl(VCMatrixControl const& control)
{
    QWidget *controlWidget = NULL;

    if (control.m_type == VCMatrixControl::Color1)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg(control.m_color.name()));
        controlButton->setFixedWidth(36);
        controlButton->setFocusPolicy(Qt::TabFocus);
        controlButton->setText("1");
    }
    else if (control.m_type == VCMatrixControl::Color2)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg(control.m_color.name()));
        controlButton->setFixedWidth(36);
        controlButton->setFocusPolicy(Qt::TabFocus);
        controlButton->setText("2");
    }
    else if (control.m_type == VCMatrixControl::Color3)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg(control.m_color.name()));
        controlButton->setFixedWidth(36);
        controlButton->setFocusPolicy(Qt::TabFocus);
        controlButton->setText("3");
    }
    else if (control.m_type == VCMatrixControl::Color4)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg(control.m_color.name()));
        controlButton->setFixedWidth(36);
        controlButton->setFocusPolicy(Qt::TabFocus);
        controlButton->setText("4");
    }
    else if (control.m_type == VCMatrixControl::Color5)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg(control.m_color.name()));
        controlButton->setFixedWidth(36);
        controlButton->setFocusPolicy(Qt::TabFocus);
        controlButton->setText("5");
    }
    else if (control.m_type == VCMatrixControl::Color2Reset)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg("#BBBBBB"));
        controlButton->setMinimumWidth(36);
        controlButton->setMaximumWidth(80);
        controlButton->setFocusPolicy(Qt::TabFocus);
        QString btnLabel = tr("Color 2 Reset");
        controlButton->setToolTip(btnLabel);
        controlButton->setText(fontMetrics().elidedText(btnLabel, Qt::ElideRight, 72));
    }
    else if (control.m_type == VCMatrixControl::Color3Reset)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg("#BBBBBB"));
        controlButton->setMinimumWidth(36);
        controlButton->setMaximumWidth(80);
        controlButton->setFocusPolicy(Qt::TabFocus);
        QString btnLabel = tr("Color 3 Reset");
        controlButton->setToolTip(btnLabel);
        controlButton->setText(fontMetrics().elidedText(btnLabel, Qt::ElideRight, 72));
    }
    else if (control.m_type == VCMatrixControl::Color4Reset)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg("#BBBBBB"));
        controlButton->setMinimumWidth(36);
        controlButton->setMaximumWidth(80);
        controlButton->setFocusPolicy(Qt::TabFocus);
        QString btnLabel = tr("Color 4 Reset");
        controlButton->setToolTip(btnLabel);
        controlButton->setText(fontMetrics().elidedText(btnLabel, Qt::ElideRight, 72));
    }
    else if (control.m_type == VCMatrixControl::Color5Reset)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg("#BBBBBB"));
        controlButton->setMinimumWidth(36);
        controlButton->setMaximumWidth(80);
        controlButton->setFocusPolicy(Qt::TabFocus);
        QString btnLabel = tr("Color 5 Reset");
        controlButton->setToolTip(btnLabel);
        controlButton->setText(fontMetrics().elidedText(btnLabel, Qt::ElideRight, 72));
    }
    else if (control.m_type == VCMatrixControl::Animation ||
             control.m_type == VCMatrixControl::Text)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg("#BBBBBB"));
        controlButton->setMinimumWidth(36);
        controlButton->setMaximumWidth(80);
        controlButton->setFocusPolicy(Qt::TabFocus);
        QString btnLabel = control.m_resource;
        if (!control.m_properties.isEmpty())
        {
            btnLabel += " (";
            QHashIterator<QString, QString> it(control.m_properties);
            while (it.hasNext())
            {
                it.next();
                btnLabel += it.value();
                if (it.hasNext())
                    btnLabel += ",";
            }
            btnLabel += ")";
        }
        controlButton->setToolTip(btnLabel);
        controlButton->setText(fontMetrics().elidedText(btnLabel, Qt::ElideRight, 72));
    }
    else if (control.m_type == VCMatrixControl::Color1Knob)
    {
        KnobWidget *controlKnob = new KnobWidget(this);
        controlWidget = controlKnob;
        controlKnob->setColor(control.m_color);
        controlKnob->setFixedWidth(36);
        controlKnob->setFixedHeight(36);
        QString knobLabel;
        if (control.m_color == Qt::red)
            knobLabel = tr("Color 1 Red component");
        else if (control.m_color == Qt::green)
            knobLabel = tr("Color 1 Green component");
        else if (control.m_color == Qt::blue)
            knobLabel = tr("Color 1 Blue component");
        controlKnob->setToolTip(knobLabel);
    }
    else if (control.m_type == VCMatrixControl::Color2Knob)
    {
        KnobWidget *controlKnob = new KnobWidget(this);
        controlWidget = controlKnob;
        controlKnob->setColor(control.m_color.darker(250));
        controlKnob->setFixedWidth(36);
        controlKnob->setFixedHeight(36);
        QString knobLabel;
        if (control.m_color == Qt::red)
            knobLabel = tr("Color 2 Red component");
        else if (control.m_color == Qt::green)
            knobLabel = tr("Color 2 Green component");
        else if (control.m_color == Qt::blue)
            knobLabel = tr("Color 2 Blue component");
        controlKnob->setToolTip(knobLabel);
    }
    else if (control.m_type == VCMatrixControl::Color3Knob)
    {
        KnobWidget *controlKnob = new KnobWidget(this);
        controlWidget = controlKnob;
        controlKnob->setColor(control.m_color.darker(250));
        controlKnob->setFixedWidth(36);
        controlKnob->setFixedHeight(36);
        QString knobLabel;
        if (control.m_color == Qt::red)
            knobLabel = tr("Color 3 Red component");
        else if (control.m_color == Qt::green)
            knobLabel = tr("Color 3 Green component");
        else if (control.m_color == Qt::blue)
            knobLabel = tr("Color 3 Blue component");
        controlKnob->setToolTip(knobLabel);
    }
    else if (control.m_type == VCMatrixControl::Color4Knob)
    {
        KnobWidget *controlKnob = new KnobWidget(this);
        controlWidget = controlKnob;
        controlKnob->setColor(control.m_color.darker(250));
        controlKnob->setFixedWidth(36);
        controlKnob->setFixedHeight(36);
        QString knobLabel;
        if (control.m_color == Qt::red)
            knobLabel = tr("Color 4 Red component");
        else if (control.m_color == Qt::green)
            knobLabel = tr("Color 4 Green component");
        else if (control.m_color == Qt::blue)
            knobLabel = tr("Color 4 Blue component");
        controlKnob->setToolTip(knobLabel);
    }
    else if (control.m_type == VCMatrixControl::Color5Knob)
    {
        KnobWidget *controlKnob = new KnobWidget(this);
        controlWidget = controlKnob;
        controlKnob->setColor(control.m_color.darker(250));
        controlKnob->setFixedWidth(36);
        controlKnob->setFixedHeight(36);
        QString knobLabel;
        if (control.m_color == Qt::red)
            knobLabel = tr("Color 5 Red component");
        else if (control.m_color == Qt::green)
            knobLabel = tr("Color 5 Green component");
        else if (control.m_color == Qt::blue)
            knobLabel = tr("Color 5 Blue component");
        controlKnob->setToolTip(knobLabel);
    }

    Q_ASSERT(controlWidget != NULL);

    if (control.widgetType() == VCMatrixControl::Knob)
    {
        connect(reinterpret_cast<KnobWidget*>(controlWidget), SIGNAL(valueChanged(int)),
                this, SLOT(slotCustomControlValueChanged()));
    }
    else
    {
        connect(reinterpret_cast<QPushButton*>(controlWidget), SIGNAL(clicked()),
                this, SLOT(slotCustomControlClicked()));
    }


    if (mode() == Doc::Design)
        controlWidget->setEnabled(false);

    VCMatrixControl *c_control = new VCMatrixControl(control);
    m_controls[controlWidget] = c_control;
    m_widgets[c_control] = controlWidget;

    m_controlsLayout->addWidget(controlWidget);

    if (m_controls[controlWidget]->m_inputSource != NULL)
    {
        setInputSource(m_controls[controlWidget]->m_inputSource, m_controls[controlWidget]->m_id);
    }

    slotFunctionChanged(); // Start update timer
}

void VCMatrix::resetCustomControls()
{
    for (QHash<QWidget *, VCMatrixControl *>::iterator it = m_controls.begin();
            it != m_controls.end(); ++it)
    {
        QWidget* widget = it.key();
        m_controlsLayout->removeWidget(widget);
        delete widget;

        VCMatrixControl* control = it.value();
        if (!control->m_inputSource.isNull())
            setInputSource(QSharedPointer<QLCInputSource>(), control->m_id);
        delete control;
    }
    m_controls.clear();
    m_widgets.clear();
}

QList<VCMatrixControl *> VCMatrix::customControls() const
{
    QList<VCMatrixControl*> controls = m_controls.values();
    std::sort(controls.begin(), controls.end(), VCMatrixControl::compare);
    return controls;
}

QMap<quint32,QString> VCMatrix::customControlsMap() const
{
    QMap<quint32,QString> map;

    foreach (VCMatrixControl *control, m_controls.values())
        map.insert(control->m_id, VCMatrixControl::typeToString(control->m_type));

    return map;
}

QWidget *VCMatrix::getWidget(VCMatrixControl* control) const
{
    return m_widgets[control];
}

void VCMatrix::slotCustomControlClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    VCMatrixControl *control = m_controls[btn];
    if (control != NULL)
    {
        RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
        if (matrix == NULL || mode() == Doc::Design)
            return;

        if (control->m_type == VCMatrixControl::Color1)
        {
            matrix->setColor(0, control->m_color);
            if (instantChanges() == true)
                matrix->updateColorDelta();
            btn->setDown(true);
            emit mtxColor1Changed();
        }
        else if (control->m_type == VCMatrixControl::Color2)
        {
            matrix->setColor(1, control->m_color);
            if (instantChanges() == true)
                matrix->updateColorDelta();
            btn->setDown(true);
            emit mtxColor2Changed();
        }
        else if (control->m_type == VCMatrixControl::Color3)
        {
            matrix->setColor(2, control->m_color);
            btn->setDown(true);
            emit mtxColor3Changed();
        }
        else if (control->m_type == VCMatrixControl::Color4)
        {
            matrix->setColor(3, control->m_color);
            btn->setDown(true);
            emit mtxColor4Changed();
        }
        else if (control->m_type == VCMatrixControl::Color5)
        {
            matrix->setColor(4, control->m_color);
            btn->setDown(true);
            emit mtxColor5Changed();
        }
        else if (control->m_type == VCMatrixControl::Color2Reset)
        {
            matrix->setColor(1, QColor());
            if (instantChanges() == true)
                matrix->updateColorDelta();
            emit mtxColor2Changed();
        }
        else if (control->m_type == VCMatrixControl::Color3Reset)
        {
            matrix->setColor(2, QColor());
            emit mtxColor3Changed();
        }
        else if (control->m_type == VCMatrixControl::Color4Reset)
        {
            matrix->setColor(3, QColor());
            emit mtxColor4Changed();
        }
        else if (control->m_type == VCMatrixControl::Color5Reset)
        {
            matrix->setColor(4, QColor());
            emit mtxColor5Changed();
        }
        else if (control->m_type == VCMatrixControl::Animation)
        {
            RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, control->m_resource);
            if (!control->m_properties.isEmpty())
            {
                RGBScript *script = static_cast<RGBScript*> (algo);
                QHashIterator<QString, QString> it(control->m_properties);
                while (it.hasNext())
                {
                    it.next();
                    script->setProperty(it.key(), it.value());
                    matrix->setProperty(it.key(), it.value());
                }
            }
            matrix->setAlgorithm(algo);
            if (instantChanges() == true)
                matrix->updateColorDelta();
            btn->setDown(true);
        }
        else if (control->m_type == VCMatrixControl::Text)
        {
            RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, "Text");
            RGBText* text = static_cast<RGBText*> (algo);
            text->setText(control->m_resource);
            matrix->setAlgorithm(algo);
            if (instantChanges() == true)
                matrix->updateColorDelta();
            btn->setDown(true);
        }
    }
}

void VCMatrix::slotCustomControlValueChanged()
{
    KnobWidget *knob = qobject_cast<KnobWidget*>(sender());
    VCMatrixControl *control = m_controls[knob];
    if (control != NULL)
    {
        RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
        if (matrix == NULL || mode() == Doc::Design)
            return;

        if (control->m_type == VCMatrixControl::Color1Knob)
        {
            QRgb color = matrix->getColor(0).rgb();
            QRgb knobValueColor = control->valueToRgb(knob->value());
            color = (color & ~control->m_color.rgb()) | (knobValueColor & control->m_color.rgb());

            matrix->setColor(0, color);
            if (instantChanges() == true)
                matrix->updateColorDelta();
            emit mtxColor1Changed();
        }
        else if (control->m_type == VCMatrixControl::Color2Knob)
        {
            QRgb color = matrix->getColor(1).rgb();
            QRgb knobValueColor = control->valueToRgb(knob->value());
            color = (color & ~control->m_color.rgb()) | (knobValueColor & control->m_color.rgb());

            matrix->setColor(1, color);
            if (instantChanges() == true)
                matrix->updateColorDelta();
            emit mtxColor2Changed();
        }
        else if (control->m_type == VCMatrixControl::Color3Knob)
        {
            QRgb color = matrix->getColor(2).rgb();
            QRgb knobValueColor = control->valueToRgb(knob->value());
            color = (color & ~control->m_color.rgb()) | (knobValueColor & control->m_color.rgb());

            matrix->setColor(2, color);
            emit mtxColor3Changed();
        }
        else if (control->m_type == VCMatrixControl::Color4Knob)
        {
            QRgb color = matrix->getColor(3).rgb();
            QRgb knobValueColor = control->valueToRgb(knob->value());
            color = (color & ~control->m_color.rgb()) | (knobValueColor & control->m_color.rgb());

            matrix->setColor(3, color);
            emit mtxColor4Changed();
        }
        else if (control->m_type == VCMatrixControl::Color5Knob)
        {
            QRgb color = matrix->getColor(4).rgb();
            QRgb knobValueColor = control->valueToRgb(knob->value());
            color = (color & ~control->m_color.rgb()) | (knobValueColor & control->m_color.rgb());

            matrix->setColor(4, color);
            emit mtxColor5Changed();
        }
        else
        {
            // We are not supposed to be here
            Q_ASSERT(false);
        }
        emit matrixControlKnobValueChanged(control->m_id, knob->value());
    }
}

void VCMatrix::slotMatrixControlKnobValueChanged(int controlID, int value)
{
    QList<VCMatrixControl *> customControls = this->customControls();
    for (int i = 0; i < customControls.length(); i++)
    {
        if (customControls[i]->m_id == controlID)
        {
            if (customControls[i]->m_type == VCMatrixControl::Color1Knob
                    || customControls[i]->m_type == VCMatrixControl::Color2Knob
                    || customControls[i]->m_type == VCMatrixControl::Color3Knob
                    || customControls[i]->m_type == VCMatrixControl::Color4Knob
                    || customControls[i]->m_type == VCMatrixControl::Color5Knob)
            {
                KnobWidget *knob = qobject_cast<KnobWidget*>(this->getWidget(customControls[i]));
                knob->setValue(value);
                break;
            }
        }
    }
}

void VCMatrix::slotMatrixControlPushButtonClicked(int controlID)
{
    QList<VCMatrixControl *> customControls = this->customControls();
    for (int i = 0; i < customControls.length(); i++)
    {
        if (customControls[i]->m_id == controlID)
        {
            QPushButton *btn = qobject_cast<QPushButton*>(this->getWidget(customControls[i]));
            btn->click();
            break;
        }
    }
}

void VCMatrix::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
        enableWidgetUI(true);
    else
        enableWidgetUI(false);

    VCWidget::slotModeChanged(mode);
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCMatrix::slotKeyPressed(const QKeySequence &keySequence)
{
    if (acceptsInput() == false)
        return;

    for (QHash<QWidget *, VCMatrixControl *>::iterator it = m_controls.begin();
            it != m_controls.end(); ++it)
    {
        VCMatrixControl *control = it.value();
        if (control->m_keySequence == keySequence &&
                control->widgetType() == VCMatrixControl::Button) // Only for buttons
        {
            QPushButton *button = reinterpret_cast<QPushButton*>(it.key());
            button->click();
        }
    }
}

void VCMatrix::updateFeedback()
{
    sendFeedback(m_slider->value());

    for (QHash<QWidget *, VCMatrixControl *>::iterator it = m_controls.begin();
            it != m_controls.end(); ++it)
    {
        VCMatrixControl *control = it.value();
        if (control->m_inputSource != NULL)
        {
            if (control->widgetType() == VCMatrixControl::Knob)
            {
                KnobWidget* knob = reinterpret_cast<KnobWidget*>(it.key());
                sendFeedback(knob->value(), control->m_inputSource);
            }
            else // if (control->widgetType() == VCMatrixControl::Button)
            {
                QPushButton* button = reinterpret_cast<QPushButton*>(it.key());
                sendFeedback(button->isDown() ?
                                 control->m_inputSource->feedbackValue(QLCInputFeedback::UpperValue) :
                                 control->m_inputSource->feedbackValue(QLCInputFeedback::LowerValue),
                                 control->m_inputSource);
            }
        }
    }
}

void VCMatrix::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    /* Don't let input data through in design mode or if disabled */
    if (acceptsInput() == false)
        return;

    quint32 pagedCh = (page() << 16) | channel;

    if (checkInputSource(universe, pagedCh, value, sender()))
    {
        m_slider->setValue((int) value);
        return;
    }

    for (QHash<QWidget *, VCMatrixControl *>::iterator it = m_controls.begin();
            it != m_controls.end(); ++it)
    {
        VCMatrixControl *control = it.value();
        if (control->m_inputSource != NULL &&
                control->m_inputSource->universe() == universe &&
                control->m_inputSource->channel() == pagedCh)
        {
            if (control->widgetType() == VCMatrixControl::Knob)
            {
                KnobWidget* knob = reinterpret_cast<KnobWidget*>(it.key());
                knob->setValue(value);
            }
            else
            {
                QPushButton *button = reinterpret_cast<QPushButton*>(it.key());
                button->click();
            }
        }
    }
}

bool VCMatrix::loadXML(QXmlStreamReader &root)
{
    QString str;

    if (root.name() != KXMLQLCVCMatrix)
    {
        qWarning() << Q_FUNC_INFO << "Matrix node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    // Sorted list for new controls
    QList<VCMatrixControl> newControls;

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCMatrixFunction)
        {
            QXmlStreamAttributes attrs = root.attributes();
            str = attrs.value(KXMLQLCVCMatrixFunctionID).toString();
            setFunction(str.toUInt());
            if (attrs.hasAttribute(KXMLQLCVCMatrixInstantApply))
                setInstantChanges(true);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInput(root);
        }
        else if (root.name() == KXMLQLCVCMatrixControl)
        {
            VCMatrixControl control(0xff);
            if (control.loadXML(root))
                newControls.insert(std::lower_bound(newControls.begin(), newControls.end(), control), control);
        }
        else if (root.name() == KXMLQLCVCMatrixVisibilityMask)
        {
            setVisibilityMask(root.readElementText().toUInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCMatrix tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    foreach (VCMatrixControl const& control, newControls)
        addCustomControl(control);

    return true;
}

bool VCMatrix::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* VC button entry */
    doc->writeStartElement(KXMLQLCVCMatrix);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Function */
    doc->writeStartElement(KXMLQLCVCMatrixFunction);
    doc->writeAttribute(KXMLQLCVCMatrixFunctionID, QString::number(function()));

    if (instantChanges() == true)
        doc->writeAttribute(KXMLQLCVCMatrixInstantApply, "true");
    doc->writeEndElement();

    /* Default controls visibility  */
    if (m_visibilityMask != VCMatrix::defaultVisibilityMask())
        doc->writeTextElement(KXMLQLCVCMatrixVisibilityMask, QString::number(m_visibilityMask));

    /* Slider External input */
    saveXMLInput(doc);

    foreach (VCMatrixControl *control, customControls())
        control->saveXML(doc);

    /* End the <Matrix> tag */
    doc->writeEndElement();

    return true;
}
