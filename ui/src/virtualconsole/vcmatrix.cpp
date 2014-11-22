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

#include <QWidgetAction>
#include <QComboBox>
#include <QLayout>
#include <QLabel>
#include <QMenu>
#include <QtXml>

#include "vcmatrixproperties.h"
#include "vcpropertieseditor.h"
#include "clickandgoslider.h"
#include "clickandgowidget.h"
#include "rgbalgorithm.h"
#include "flowlayout.h"
// #include "knobwidget.h"
#include "vcmatrixknobwidget.h"
#include "rgbmatrix.h"
#include "rgbscriptscache.h"
#include "vcmatrix.h"
#include "function.h"
#include "rgbtext.h"
#include "doc.h"

static const QString controlBtnSS = "QPushButton { background-color: %1; height: 32px; border: 2px solid #6A6A6A; border-radius: 5px; }"
                                    "QPushButton:pressed { border: 2px solid #00E600; }"
                                    "QPushButton:disabled { border: 2px solid #BBBBBB; color: #8f8f8f }";

static const QString controlKnbSS = "QDial { background-color: %1; color: #FFFFFF; height: 32px; border: 2px solid #6A6A6A; border-radius: 5px; }";

const QSize VCMatrix::defaultSize(QSize(160, 120));

VCMatrix::VCMatrix(QWidget *parent, Doc *doc)
    : VCWidget(parent, doc)
    , m_matrixID(Function::invalidId())
    , m_instantApply(true)
{
    /* Set the class name "VCLabel" as the object name as well */
    setObjectName(VCMatrix::staticMetaObject.className());
    setFrameStyle(KVCFrameStyleSunken);

    QHBoxLayout *hBox = new QHBoxLayout(this);
    //hBox->setContentsMargins(3, 3, 3, 10);
    //hBox->setSpacing(5);

    QPushButton* visibleButton = new QPushButton(this);
    // visibleButton->setText(QString("HIDE"));
    visibleButton->setFixedWidth(8);
    visibleButton->setFixedHeight(8);
    hBox->addWidget(visibleButton);

    connect(visibleButton, SIGNAL(clicked()),
                this, SLOT(slotToggleVisible()));

    m_slider = new ClickAndGoSlider();
    m_slider->setStyleSheet(CNG_DEFAULT_STYLE);
    m_slider->setFixedWidth(32);
    m_slider->setRange(0, 255);
    m_slider->setPageStep(1);
    m_slider->setInvertedAppearance(false);
    hBox->addWidget(m_slider);

    connect(m_slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderMoved(int)));

    QVBoxLayout *vbox = new QVBoxLayout(this);

    m_startColorButton = new QToolButton(this);
    m_startColorButton->setFixedSize(48, 48);
    m_startColorButton->setIconSize(QSize(42, 42));

    QWidgetAction* scAction = new QWidgetAction(this);
    m_scCnGWidget = new ClickAndGoWidget();
    m_scCnGWidget->setType(ClickAndGoWidget::RGB, NULL);
    scAction->setDefaultWidget(m_scCnGWidget);
    QMenu *startColorMenu = new QMenu();
    startColorMenu->addAction(scAction);
    m_startColorButton->setMenu(startColorMenu);
    m_startColorButton->setPopupMode(QToolButton::InstantPopup);

    connect(m_scCnGWidget, SIGNAL(colorChanged(QRgb)),
            this, SLOT(slotStartColorChanged(QRgb)));

    m_endColorButton = new QToolButton(this);
    m_endColorButton->setFixedSize(48, 48);
    m_endColorButton->setIconSize(QSize(42, 42));

    QWidgetAction* ecAction = new QWidgetAction(this);
    m_ecCnGWidget = new ClickAndGoWidget();
    m_ecCnGWidget->setType(ClickAndGoWidget::RGB, NULL);
    ecAction->setDefaultWidget(m_ecCnGWidget);
    QMenu *endColorMenu = new QMenu();
    endColorMenu->addAction(ecAction);
    m_endColorButton->setMenu(endColorMenu);
    m_endColorButton->setPopupMode(QToolButton::InstantPopup);

    connect(m_ecCnGWidget, SIGNAL(colorChanged(QRgb)),
            this, SLOT(slotEndColorChanged(QRgb)));

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    vbox->addWidget(m_label);

    QHBoxLayout *btnHbox = new QHBoxLayout(this);

    btnHbox->addWidget(m_startColorButton);
    btnHbox->addWidget(m_endColorButton);

    vbox->addLayout(btnHbox);

    m_presetCombo = new QComboBox(this);
    //m_presetCombo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_presetCombo->addItems(doc->rgbScriptsCache()->names());
    connect(m_presetCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(slotAnimationChanged(QString)));
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

    /* Update the slider according to current mode */
    slotModeChanged(m_doc->mode());
    setLiveEdit(m_liveEdit);
}

VCMatrix::~VCMatrix()
{
    foreach(VCMatrixControl* control, m_controls)
    {
        if (control->m_inputSource != NULL)
            setInputSource(NULL, control->m_id + 1);
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

    /* Copy button-specific stuff */
    setFunction(matrix->function());
    setInstantChanges(matrix->instantChanges());
    if (!matrix->m_slider->isVisible())
        slotToggleVisible();

    {
        resetCustomControls();
        QList<VCMatrixControl const*> newControls;
        foreach(VCMatrixControl const* control, matrix->customControls())
        {
            // add sorted by ID
            QList<VCMatrixControl const*>::iterator it = newControls.begin();
            while (it != newControls.end() && (*it)->m_id < control->m_id)
                ++it;
            newControls.insert(it, control);
        }
        foreach (VCMatrixControl const* control, newControls)
        {
            addCustomControl(*control);
        }
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
    m_startColorButton->setEnabled(enable);
    m_endColorButton->setEnabled(enable);
    m_presetCombo->setEnabled(enable);

    foreach(QWidget *ctlBtn, m_controls.keys())
        ctlBtn->setEnabled(enable);
}

void VCMatrix::slotSliderMoved(int value)
{
    Function* function = m_doc->function(m_matrixID);
    if (function == NULL || mode() == Doc::Design)
        return;

    if (value == 0)
    {
        if (function->stopped() == false)
            function->stop();
    }
    else
    {
        qreal pIntensity = qreal(value) / qreal(UCHAR_MAX);
        function->adjustAttribute(pIntensity * intensity(), Function::Intensity);

        if (function->stopped() == true)
            function->start(m_doc->masterTimer());
    }
}

void VCMatrix::slotStartColorChanged(QRgb color)
{
    QColor col(color);
    QPixmap px(42, 42);
    px.fill(col);
    m_startColorButton->setIcon(px);

    for (QHash<QWidget *, VCMatrixControl *>::iterator controlIt = m_controls.begin();
            controlIt != m_controls.end(); ++controlIt)
    {
        QWidget* widget = controlIt.key();
        VCMatrixControl* control = controlIt.value();

        if (control->m_type == VCMatrixControl::StartColorKnob)
        {
            VCMatrixKnobWidget* knob = reinterpret_cast<VCMatrixKnobWidget*>(widget);
            knob->blockSignals(true);
            knob->setValue(control->colorToValue(color));
            knob->blockSignals(false);
        }
    }

    RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL || mode() == Doc::Design)
        return;

    matrix->setStartColor(col);
    if (instantChanges() == true)
        matrix->calculateColorDelta();
}

void VCMatrix::slotEndColorChanged(QRgb color)
{
    QColor col(color);
    QPixmap px(42, 42);
    px.fill(col);
    m_endColorButton->setIcon(px);

    for (QHash<QWidget *, VCMatrixControl *>::iterator controlIt = m_controls.begin();
            controlIt != m_controls.end(); ++controlIt)
    {
        QWidget* widget = controlIt.key();
        VCMatrixControl* control = controlIt.value();

        if (control->m_type == VCMatrixControl::EndColorKnob)
        {
            VCMatrixKnobWidget* knob = reinterpret_cast<VCMatrixKnobWidget*>(widget);
            knob->blockSignals(true);
            knob->setValue(control->colorToValue(color));
            knob->blockSignals(false);
        }
    }

    RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL || mode() == Doc::Design)
        return;

    matrix->setEndColor(col);
    if (instantChanges() == true)
        matrix->calculateColorDelta();
}

void VCMatrix::slotAnimationChanged(QString name)
{
    RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
    if (matrix == NULL || mode() == Doc::Design)
        return;

    RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, name);
    matrix->setAlgorithm(algo);
    if (instantChanges() == true)
        matrix->calculateColorDelta();
}

void VCMatrix::slotToggleVisible()
{
    m_slider->setVisible(!m_slider->isVisible());
    // m_label->setVisible(!m_label->isVisible());
    m_startColorButton->setVisible(!m_startColorButton->isVisible());
    m_endColorButton->setVisible(!m_endColorButton->isVisible());
    m_presetCombo->setVisible(!m_presetCombo->isVisible());
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

/*********************************************************************
 * Function attachment
 *********************************************************************/

void VCMatrix::setFunction(quint32 function)
{
    m_matrixID = function;
}

quint32 VCMatrix::function() const
{
    return m_matrixID;
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

    if (control.m_type == VCMatrixControl::StartColor)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg(control.m_color.name()));
        controlButton->setFixedWidth(36);
        controlButton->setText("S");
    }
    else if (control.m_type == VCMatrixControl::EndColor)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg(control.m_color.name()));
        controlButton->setFixedWidth(36);
        controlButton->setText("E");
    }
    else if (control.m_type == VCMatrixControl::ResetEndColor)
    {
        QPushButton *controlButton = new QPushButton(this);
        controlWidget = controlButton;
        controlButton->setStyleSheet(controlBtnSS.arg("#BBBBBB"));
        controlButton->setMinimumWidth(36);
        controlButton->setMaximumWidth(80);
        QString btnLabel = tr("End Color Reset");
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
        QString btnLabel = control.m_resource;
        if (!control.m_properties.isEmpty())
        {
            btnLabel += " (";
            QHashIterator<QString, QString> it(control.m_properties);
            while(it.hasNext())
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
    else if (control.m_type == VCMatrixControl::StartColorKnob)
    {
        VCMatrixKnobWidget *controlKnob = new VCMatrixKnobWidget(this);
        controlWidget = controlKnob;
        controlKnob->setStyleSheet(controlKnbSS.arg(control.m_color.name()));
        controlKnob->setFixedWidth(36);
        controlKnob->setToolTip("Start");
    }
    else if (control.m_type == VCMatrixControl::EndColorKnob)
    {
        VCMatrixKnobWidget *controlKnob = new VCMatrixKnobWidget(this);
        controlWidget = controlKnob;
        controlKnob->setStyleSheet(controlKnbSS.arg(control.m_color.name()));
        controlKnob->setFixedWidth(36);
        controlKnob->setToolTip("End");
    }

    Q_ASSERT(controlWidget != NULL);

    if (control.widgetType() == VCMatrixControl::Knob)
    {
        connect(reinterpret_cast<VCMatrixKnobWidget*>(controlWidget), SIGNAL(valueChanged(int)),
                this, SLOT(slotCustomControlValueChanged()));
    }
    else
    {
        connect(reinterpret_cast<QPushButton*>(controlWidget), SIGNAL(clicked()),
                this, SLOT(slotCustomControlClicked()));
    }


    if (mode() == Doc::Design)
        controlWidget->setEnabled(false);

    m_controls[controlWidget] = new VCMatrixControl(control);
    m_controlsLayout->addWidget(controlWidget);

    setInputSource(m_controls[controlWidget]->m_inputSource, m_controls[controlWidget]->m_id + 1);
}

void VCMatrix::resetCustomControls()
{
    for (QHash<QWidget *, VCMatrixControl *>::iterator controlIt = m_controls.begin();
            controlIt != m_controls.end(); ++controlIt)
    {
        m_controlsLayout->removeWidget(controlIt.key());
        delete controlIt.key();
        delete controlIt.value();
    }
    m_controls.clear();
}

QList<VCMatrixControl *> VCMatrix::customControls() const
{
    return m_controls.values();
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

        if (control->m_type == VCMatrixControl::StartColor)
        {
            QPixmap px(42, 42);
            px.fill(control->m_color);

            m_startColorButton->setIcon(px);
            matrix->setStartColor(control->m_color);
            if (instantChanges() == true)
                matrix->calculateColorDelta();

            for (QHash<QWidget *, VCMatrixControl *>::iterator controlIt = m_controls.begin();
                    controlIt != m_controls.end(); ++controlIt)
            {
                QWidget* updatedWidget = controlIt.key();
                VCMatrixControl* updatedControl = controlIt.value();

                if (updatedControl->m_type == VCMatrixControl::StartColorKnob)
                {
                    VCMatrixKnobWidget* updatedKnob = reinterpret_cast<VCMatrixKnobWidget*>(updatedWidget);
                    updatedKnob->blockSignals(true);
                    updatedKnob->setValue(updatedControl->colorToValue(control->m_color.rgb()));
                    updatedKnob->blockSignals(false);
                }
            }
        }
        else if (control->m_type == VCMatrixControl::EndColor)
        {
            QPixmap px(42, 42);
            px.fill(control->m_color);

            m_endColorButton->setIcon(px);
            matrix->setEndColor(control->m_color);
            if (instantChanges() == true)
                matrix->calculateColorDelta();

            for (QHash<QWidget *, VCMatrixControl *>::iterator controlIt = m_controls.begin();
                    controlIt != m_controls.end(); ++controlIt)
            {
                QWidget* updatedWidget = controlIt.key();
                VCMatrixControl* updatedControl = controlIt.value();

                if (updatedControl->m_type == VCMatrixControl::EndColorKnob)
                {
                    VCMatrixKnobWidget* updatedKnob = reinterpret_cast<VCMatrixKnobWidget*>(updatedWidget);
                    updatedKnob->blockSignals(true);
                    updatedKnob->setValue(updatedControl->colorToValue(control->m_color.rgb()));
                    updatedKnob->blockSignals(false);
                }
            }
        }
        else if (control->m_type == VCMatrixControl::ResetEndColor)
        {
            QPixmap px(42, 42);
            px.fill(Qt::transparent);

            m_endColorButton->setIcon(px);
            matrix->setEndColor(QColor());
            if (instantChanges() == true)
                matrix->calculateColorDelta();
        }
        else if (control->m_type == VCMatrixControl::Animation)
        {
            RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, control->m_resource);
            if (!control->m_properties.isEmpty())
            {
                RGBScript *script = static_cast<RGBScript*> (algo);
                QHashIterator<QString, QString> it(control->m_properties);
                while(it.hasNext())
                {
                    it.next();
                    script->setProperty(it.key(), it.value());
                }
            }
            matrix->setAlgorithm(algo);
            if (instantChanges() == true)
                matrix->calculateColorDelta();
            m_presetCombo->blockSignals(true);
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
            m_presetCombo->setCurrentText(control->m_resource);
#else
            m_presetCombo->setCurrentIndex(m_presetCombo->findText(control->m_resource));
#endif
            m_presetCombo->blockSignals(false);
        }
        else if (control->m_type == VCMatrixControl::Text)
        {
            RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, "Text");
            RGBText* text = static_cast<RGBText*> (algo);
            text->setText(control->m_resource);
            matrix->setAlgorithm(algo);
            if (instantChanges() == true)
                matrix->calculateColorDelta();
        }
    }
}

void VCMatrix::slotCustomControlValueChanged()
{
    VCMatrixKnobWidget *knob = qobject_cast<VCMatrixKnobWidget*>(sender());
    VCMatrixControl *control = m_controls[knob];
    if (control != NULL)
    {
        RGBMatrix* matrix = qobject_cast<RGBMatrix*>(m_doc->function(m_matrixID));
        if (matrix == NULL || mode() == Doc::Design)
            return;

        if (control->m_type == VCMatrixControl::StartColorKnob)
        {
            QRgb color = matrix->startColor().rgb();
            QRgb knobValueColor = control->valueToColor(knob->value());
            color = (color & ~control->m_color.rgb()) | (knobValueColor & control->m_color.rgb());

            QPixmap px(42, 42);
            px.fill(color);
            m_startColorButton->setIcon(px);
            matrix->setStartColor(color);
            if (instantChanges() == true)
                matrix->calculateColorDelta();
        }
        else if (control->m_type == VCMatrixControl::EndColorKnob)
        {
            QRgb color = matrix->endColor().rgb();
            QRgb knobValueColor = control->valueToColor(knob->value());
            color = (color & ~control->m_color.rgb()) | (knobValueColor & control->m_color.rgb());

            QPixmap px(42, 42);
            px.fill(color);
            m_endColorButton->setIcon(px);
            matrix->setEndColor(color);
            if (instantChanges() == true)
                matrix->calculateColorDelta();
        }
        else
        {
            // We are not supposed to be here
            Q_ASSERT(false);
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
    if (isEnabled() == false)
        return;

    QHashIterator<QWidget *, VCMatrixControl *> it(m_controls);
    while(it.hasNext())
    {
        it.next();
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
}

void VCMatrix::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    /* Don't let input data thru in design mode */
    if (mode() == Doc::Design || isEnabled() == false)
        return;

    quint32 pagedCh = (page() << 16) | channel;

    if (checkInputSource(universe, pagedCh, value, sender()))
    {
        m_slider->setValue((int) value);
    }

    QHashIterator<QWidget *, VCMatrixControl *> it(m_controls);
    while (it.hasNext())
    {
        it.next();
        VCMatrixControl *control = it.value();
        if (control->m_inputSource != NULL &&
                control->m_inputSource->universe() == universe &&
                control->m_inputSource->channel() == pagedCh)
        {
            if (control->widgetType() == VCMatrixControl::Knob)
            {
                VCMatrixKnobWidget* knob = reinterpret_cast<VCMatrixKnobWidget*>(it.key());
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

bool VCMatrix::loadXML(const QDomElement *root)
{
    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCMatrix)
    {
        qWarning() << Q_FUNC_INFO << "Matrix node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    node = root->firstChild();
    QList<VCMatrixControl> newControls;
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCMatrixFunction)
        {
            str = tag.attribute(KXMLQLCVCMatrixFunctionID);
            setFunction(str.toUInt());
            if (tag.hasAttribute(KXMLQLCVCMatrixInstantApply))
                setInstantChanges(true);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetInput)
        {
            loadXMLInput(&tag);
        }
        else if(tag.tagName() == KXMLQLCVCMatrixControl)
        {
            VCMatrixControl control(0xff);
            if (control.loadXML(tag))
            {
                // add sorted by ID
                QList<VCMatrixControl>::iterator it = newControls.begin();
                while (it != newControls.end() && it->m_id < control.m_id)
                    ++it;
                newControls.insert(it, control);
            }
        }
        else if (tag.tagName() == KXMLQLCVCMatrixHideBasicControls)
        {
            slotToggleVisible();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCMatrix tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    foreach (VCMatrixControl const& control, newControls)
    {
        addCustomControl(control);
    }

    return true;
}

bool VCMatrix::saveXML(QDomDocument *doc, QDomElement *vc_root)
{
    QDomElement root;
    QDomElement tag;
    //QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC button entry */
    root = doc->createElement(KXMLQLCVCMatrix);
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    /* Function */
    tag = doc->createElement(KXMLQLCVCMatrixFunction);
    root.appendChild(tag);
    str.setNum(function());
    tag.setAttribute(KXMLQLCVCMatrixFunctionID, str);

    if (instantChanges() == true)
        tag.setAttribute(KXMLQLCVCMatrixInstantApply, "true");

    if (!m_slider->isVisible())
        tag.setAttribute(KXMLQLCVCMatrixHideBasicControls, "true");

    /* Slider External input */
    saveXMLInput(doc, &root);

    foreach(VCMatrixControl *control, customControls())
        control->saveXML(doc, &root);

    return true;
}

