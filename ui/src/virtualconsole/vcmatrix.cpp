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
#include "clickandgoslider.h"
#include "clickandgowidget.h"
#include "rgbalgorithm.h"
#include "flowlayout.h"
#include "rgbmatrix.h"
#include "vcmatrix.h"
#include "function.h"
#include "rgbtext.h"
#include "doc.h"

const QString controlBtnSS = "QPushButton { background-color: %1; height: 32px; border: 2px solid #6A6A6A; border-radius: 5px; }"
                             "QPushButton:pressed { border: 2px solid #00E600; }"
                             "QPushButton:disabled { border: 2px solid #BBBBBB; }";

VCMatrix::VCMatrix(QWidget *parent, Doc *doc)
    : VCWidget(parent, doc)
    , m_matrixID(Function::invalidId())
    , m_instantApply(true)
{
    /* Set the class name "VCLabel" as the object name as well */
    setObjectName(VCMatrix::staticMetaObject.className());
    setFrameStyle(KVCFrameStyleSunken);

    QHBoxLayout *hBox = new QHBoxLayout(this);
    hBox->setContentsMargins(3, 3, 3, 10);
    hBox->setSpacing(5);

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
    m_presetCombo->addItems(RGBScript::scriptNames(m_doc));
    connect(m_presetCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(slotAnimationChanged(QString)));
    vbox->addWidget(m_presetCombo);

    hBox->addLayout(vbox);

    m_controlsLayout = new FlowLayout();
    vbox->addLayout(m_controlsLayout);

    setType(VCWidget::MatrixWidget);
    setCaption(QString());
    resize(QSize(160, 120));

    /* Update the slider according to current mode */
    slotModeChanged(mode());
}

VCMatrix::~VCMatrix()
{
}

void VCMatrix::setID(quint32 id)
{
    VCWidget::setID(id);

    if (caption().isEmpty())
        setCaption(tr("Matrix %1").arg(id));
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

    foreach(QPushButton *ctlBtn, m_controls.keys())
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

bool VCMatrix::instantChanges()
{
    return m_instantApply;
}

/*********************************************************************
 * Custom controls
 *********************************************************************/

void VCMatrix::addCustomControl(VCMatrixControl *control)
{
    if (control == NULL)
        return;

    QPushButton *controlButton = new QPushButton(this);

    if (control->m_type == VCMatrixControl::StartColor)
    {
        controlButton->setStyleSheet(controlBtnSS.arg(control->m_color.name()));
        controlButton->setFixedWidth(36);
        controlButton->setText("S");
    }
    else if (control->m_type == VCMatrixControl::EndColor)
    {
        controlButton->setStyleSheet(controlBtnSS.arg(control->m_color.name()));
        controlButton->setFixedWidth(36);
        controlButton->setText("E");
    }
    else if (control->m_type == VCMatrixControl::Animation ||
             control->m_type == VCMatrixControl::Text)
    {
        controlButton->setStyleSheet(controlBtnSS.arg("#BBBBBB"));
        controlButton->setMaximumWidth(80);
        controlButton->setToolTip(control->m_resource);
        controlButton->setText(fontMetrics().elidedText(control->m_resource, Qt::ElideRight, 72));
    }
    if (mode() == Doc::Design)
        controlButton->setEnabled(false);

    connect(controlButton, SIGNAL(clicked()),
            this, SLOT(slotCustomControlClicked()));

    m_controls[controlButton] = new VCMatrixControl(control);
    m_controlsLayout->addWidget(controlButton);
}

void VCMatrix::resetCustomControls()
{
    QHashIterator<QPushButton *, VCMatrixControl *> it(m_controls);
    while(it.hasNext())
    {
        it.next();
        m_controlsLayout->removeWidget(it.key());
        delete it.key();
        delete it.value();
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
            matrix->calculateColorDelta();
        }
        else if (control->m_type == VCMatrixControl::EndColor)
        {
            QPixmap px(42, 42);
            px.fill(control->m_color);

            m_endColorButton->setIcon(px);
            matrix->setEndColor(control->m_color);
            matrix->calculateColorDelta();
        }
        else if (control->m_type == VCMatrixControl::Animation)
        {
            RGBAlgorithm* algo = RGBAlgorithm::algorithm(m_doc, control->m_resource);
            matrix->setAlgorithm(algo);
            if (instantChanges() == true)
                matrix->calculateColorDelta();
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

    QHashIterator<QPushButton *, VCMatrixControl *> it(m_controls);
    while(it.hasNext())
    {
        it.next();
        VCMatrixControl *control = it.value();
        if (control->m_keySequence == keySequence)
        {
            QPushButton *button = it.key();
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
    else
    {
        QHashIterator<QPushButton *, VCMatrixControl *> it(m_controls);
        while(it.hasNext())
        {
            it.next();
            VCMatrixControl *control = it.value();
            if (control->m_inputSource != NULL &&
                control->m_inputSource->universe() == universe &&
                control->m_inputSource->channel() == pagedCh)
            {
                QPushButton *button = it.key();
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
            VCMatrixControl *control = new VCMatrixControl(0xFF);
            if (control->loadXML(tag) == false)
                delete control;
            else
                addCustomControl(control);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCMatrix tag:" << tag.tagName();
        }

        node = node.nextSibling();
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

    /* Slider External input */
    saveXMLInput(doc, &root);

    foreach(VCMatrixControl *control, customControls())
        control->saveXML(doc, &root);

    return true;
}

