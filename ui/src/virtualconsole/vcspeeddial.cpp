/*
  Q Light Controller Plus
  vcspeeddial.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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
#include <QSettings>
#include <QLayout>
#include <QTimer>
#include <QDebug>

#include "vcspeeddialproperties.h"
#include "vcspeeddialfunction.h"
#include "vcpropertieseditor.h"
#include "vcspeeddialpreset.h"
#include "vcspeeddial.h"
#include "flowlayout.h"
#include "speeddial.h"
#include "qlcmacros.h"
#include "function.h"
#include "qlcfile.h"

#define UPDATE_TIMEOUT 50

const quint8 VCSpeedDial::absoluteInputSourceId = 0;
const quint8 VCSpeedDial::tapInputSourceId = 1;
const quint8 VCSpeedDial::multInputSourceId = 2;
const quint8 VCSpeedDial::divInputSourceId = 3;
const quint8 VCSpeedDial::multDivResetInputSourceId = 4;
const quint8 VCSpeedDial::applyInputSourceId = 5;
const QSize VCSpeedDial::defaultSize(QSize(200, 175));

static const QString presetBtnSS =
    "QPushButton { background-color: %1; height: 32px; border: 2px solid #6A6A6A; border-radius: 5px; }"
    "QPushButton:pressed { border: 2px solid #0000FF; }"
    "QPushButton:disabled { border: 2px solid #BBBBBB; color: #8f8f8f }";

static const QString dialSS =
    "QGroupBox { background-color: %1; border: 1px solid gray; border-radius: 5px; margin-top: 0; font-size: %2pt; }"
    "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0px 5px;"
    "                   background-color: transparent; color: %3; }";

/****************************************************************************
 * Initialization
 ****************************************************************************/

VCSpeedDial::VCSpeedDial(QWidget* parent, Doc* doc)
    : VCWidget(parent, doc)
    , m_currentFactor(1)
    , m_resetFactorOnDialChange(false)
    , m_absoluteValueMin(0)
    , m_absoluteValueMax(1000 * 10)
{
    setFrameStyle(KVCFrameStyleSunken);

    QVBoxLayout* vBox = new QVBoxLayout(this);
    vBox->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* speedDialHBox = new QHBoxLayout();
    vBox->addLayout(speedDialHBox);

    m_dial = new SpeedDial(this);
    speedDialHBox->addWidget(m_dial);
    connect(m_dial, SIGNAL(valueChanged(int)), this, SLOT(slotDialValueChanged()));
    connect(m_dial, SIGNAL(tapped()), this, SLOT(slotDialTapped()));
    connect(m_dial, SIGNAL(tapTimeout()), this, SLOT(slotTapTimeout()));

    m_factoredValue = m_dial->value();

    setType(VCWidget::SpeedDialWidget);
    setCaption(tr("Duration"));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPEEDDIAL_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(VCSpeedDial::defaultSize);

    var = settings.value(SETTINGS_SPEEDDIAL_VALUE);
    if (var.isValid() == true)
        m_dial->setValue(var.toUInt());

    // Multiplier, factor, divider and reset box
    QHBoxLayout* multFactorDivHBox = new QHBoxLayout();

    m_divButton = new QToolButton();
    m_divButton->setIconSize(QSize(32, 32));
    m_divButton->setIcon(QIcon(":/back.png"));
    m_divButton->setToolTip(tr("Divide the current time by 2"));
    connect(m_divButton, SIGNAL(clicked()),
            this, SLOT(slotDiv()));
    multFactorDivHBox->addWidget(m_divButton, Qt::AlignVCenter | Qt::AlignLeft);

    QVBoxLayout* labelsVboxBox = new QVBoxLayout();

    m_multDivLabel = new QLabel();
    m_multDivLabel->setAlignment(Qt::AlignCenter);
    m_multDivLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    labelsVboxBox->addWidget(m_multDivLabel, Qt::AlignVCenter | Qt::AlignLeft);

    m_multDivResultLabel = new QLabel();
    m_multDivResultLabel->setAlignment(Qt::AlignCenter);
    m_multDivResultLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_multDivResultLabel->setBackgroundRole(QPalette::BrightText);
    labelsVboxBox->addWidget(m_multDivResultLabel);

    multFactorDivHBox->addLayout(labelsVboxBox);

    m_multButton = new QToolButton();
    m_multButton->setIconSize(QSize(32, 32));
    m_multButton->setIcon(QIcon(":/forward.png"));
    m_multButton->setToolTip(tr("Multiply the current time by 2"));
    connect(m_multButton, SIGNAL(clicked()),
            this, SLOT(slotMult()));
    multFactorDivHBox->addWidget(m_multButton, Qt::AlignVCenter | Qt::AlignLeft);

    m_multDivResetButton = new QToolButton();
    m_multDivResetButton->setIconSize(QSize(32, 32));
    m_multDivResetButton->setIcon(QIcon(":/fileclose.png"));
    m_multDivResetButton->setToolTip(tr("Reset the current factor to 1x"));
    connect(m_multDivResetButton, SIGNAL(clicked()),
            this, SLOT(slotMultDivReset()));
    multFactorDivHBox->addWidget(m_multDivResetButton);

    vBox->addLayout(multFactorDivHBox);

    // Update labels
    slotMultDivChanged();

    // Apply button
    m_applyButton = new QPushButton();
    m_applyButton->setStyleSheet(presetBtnSS.arg("#DDDDDD"));
    m_applyButton->setText(tr("Apply"));
    m_applyButton->setToolTip(tr("Send the current value to the function now"));
    connect(m_applyButton, SIGNAL(clicked()),
            this, SLOT(slotFactoredValueChanged()));
    vBox->addWidget(m_applyButton);

    // Presets
    m_presetsLayout = new FlowLayout(3);
    vBox->addLayout(m_presetsLayout);

    // Don't show Infinite button: it's handled by presets
    setVisibilityMask(SpeedDial::defaultVisibilityMask() & ~SpeedDial::Infinite);

    /* Update timer */
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, SIGNAL(timeout()),
            this, SLOT(slotUpdate()));
    m_updateTimer->setSingleShot(true);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Background color has been moved to Base
    setBackgroundColor(palette().color(QPalette::Base));
#endif
    m_foregroundColor = palette().color(QPalette::WindowText);
    m_dial->setStyleSheet(dialSS.arg(palette().color(QPalette::Window).name())
                              .arg(font().pointSize())
                              .arg(m_foregroundColor.name()));

    slotModeChanged(m_doc->mode());
    setLiveEdit(m_liveEdit);
}

VCSpeedDial::~VCSpeedDial()
{
    foreach (VCSpeedDialPreset* preset, m_presets)
    {
        delete preset;
    }
}

void VCSpeedDial::enableWidgetUI(bool enable)
{
    m_dial->setEnabled(enable);

    // Mult and div
    m_multButton->setEnabled(enable);
    m_divButton->setEnabled(enable);
    m_multDivResetButton->setEnabled(enable);

    // Apply
    m_applyButton->setEnabled(enable);

    // Presets enable
    foreach (QWidget *presetWidget, m_presets.keys())
        presetWidget->setEnabled(enable);

    // Presets: update state
    if (enable)
        slotUpdate();
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCSpeedDial::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCSpeedDial* dial = new VCSpeedDial(parent, m_doc);
    if (dial->copyFrom(this) == false)
    {
        delete dial;
        dial = NULL;
    }

    return dial;
}

bool VCSpeedDial::copyFrom(const VCWidget* widget)
{
    const VCSpeedDial* dial = qobject_cast<const VCSpeedDial*> (widget);
    if (dial == NULL)
        return false;

    setFunctions(dial->functions());
    setAbsoluteValueRange(dial->absoluteValueMin(), dial->absoluteValueMax());
    setVisibilityMask(dial->visibilityMask());
    setResetFactorOnDialChange(dial->resetFactorOnDialChange());

    setTapKeySequence(dial->tapKeySequence());
    setMultKeySequence(dial->multKeySequence());
    setDivKeySequence(dial->divKeySequence());
    setMultDivResetKeySequence(dial->multDivResetKeySequence());
    setApplyKeySequence(dial->applyKeySequence());

    resetPresets();
    foreach (VCSpeedDialPreset const* preset, dial->presets())
    {
        addPreset(*preset);
    }

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

void VCSpeedDial::setFont(const QFont &font)
{
    VCWidget::setFont(font);
    m_dial->setStyleSheet(dialSS.arg(palette().color(QPalette::Window).name())
                              .arg(font.pointSize())
                              .arg(m_foregroundColor.name()));
}

/*********************************************************************
 * Background/Foreground color
 *********************************************************************/

void VCSpeedDial::setBackgroundColor(const QColor &color)
{
    VCWidget::setBackgroundColor(color);
    m_dial->setStyleSheet(dialSS.arg(palette().color(QPalette::Window).name())
                              .arg(font().pointSize())
                              .arg(m_foregroundColor.name()));
}

void VCSpeedDial::setForegroundColor(const QColor &color)
{
    m_foregroundColor = color;
    m_hasCustomForegroundColor = true;

    m_dial->setStyleSheet(dialSS.arg(palette().color(QPalette::Window).name())
                              .arg(font().pointSize())
                              .arg(color.name()));
    VCWidget::setForegroundColor(color);
}

QColor VCSpeedDial::foregroundColor() const
{
    return m_foregroundColor;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCSpeedDial::editProperties()
{
    VCSpeedDialProperties sdp(this, m_doc);
    sdp.exec();
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCSpeedDial::setCaption(const QString& text)
{
    VCWidget::setCaption(text);

    Q_ASSERT(m_dial != NULL);
    m_dial->setTitle(text);
}

/*****************************************************************************
 * QLC Mode
 *****************************************************************************/

void VCSpeedDial::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate && isDisabled() == false)
    {
        enableWidgetUI(true);
        updateFeedback();
    }
    else
    {
        m_dial->stopTimers();
        enableWidgetUI(false);
    }
    VCWidget::slotModeChanged(mode);
}

/****************************************************************************
 * Functions
 ****************************************************************************/

void VCSpeedDial::setFunctions(const QList <VCSpeedDialFunction> & functions)
{
    m_functions = functions;
}

QList <VCSpeedDialFunction> VCSpeedDial::functions() const
{
    return m_functions;
}

void VCSpeedDial::tap()
{
    m_dial->tap();
}

void VCSpeedDial::slotDialValueChanged()
{
    // The (m_currentFactor != 1) test ensures that we don't call
    // slotMultDivChanged() 2 times
    if (m_resetFactorOnDialChange && m_currentFactor != 1)
        slotMultDivReset();
    else
        slotMultDivChanged();

    m_updateTimer->start(UPDATE_TIMEOUT);
}

void VCSpeedDial::slotDialTapped()
{
    foreach (const VCSpeedDialFunction &speeddialfunction, m_functions)
    {
        Function* function = m_doc->function(speeddialfunction.functionId);
        if (function != NULL)
        {
            if (speeddialfunction.durationMultiplier != VCSpeedDialFunction::None)
                function->tap();
        }
    }
}

void VCSpeedDial::slotTapTimeout()
{
    updateFeedback();
}

void VCSpeedDial::slotUpdate()
{
    int currentValue = m_dial->value();

    for (QHash<QWidget*, VCSpeedDialPreset*>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        QWidget* widget = it.key();
        VCSpeedDialPreset* preset = it.value();

        QPushButton* button = reinterpret_cast<QPushButton*>(widget);
        button->setDown(preset->m_value == currentValue);
    }
    updateFeedback();
}

/*********************************************************************
 * Presets
 *********************************************************************/

void VCSpeedDial::addPreset(VCSpeedDialPreset const& preset)
{
    QWidget *presetWidget = NULL;

    {
        QPushButton *presetButton = new QPushButton(this);
        presetWidget = presetButton;
        presetButton->setStyleSheet(presetBtnSS.arg("#BBBBBB"));
        presetButton->setMinimumWidth(36);
        presetButton->setMaximumWidth(80);
        presetButton->setFocusPolicy(Qt::TabFocus);
        QString btnLabel = preset.m_name;
        presetButton->setToolTip(btnLabel);
        presetButton->setText(fontMetrics().elidedText(btnLabel, Qt::ElideRight, 72));
    }

    Q_ASSERT(presetWidget != NULL);

    connect(reinterpret_cast<QPushButton*>(presetWidget), SIGNAL(clicked()),
            this, SLOT(slotPresetClicked()));

    if (mode() == Doc::Design)
        presetWidget->setEnabled(false);

    m_presets[presetWidget] = new VCSpeedDialPreset(preset);
    m_presetsLayout->addWidget(presetWidget);

    if (m_presets[presetWidget]->m_inputSource != NULL)
    {
        setInputSource(m_presets[presetWidget]->m_inputSource,
                       m_presets[presetWidget]->m_id);
    }

    m_updateTimer->start(UPDATE_TIMEOUT);
}

void VCSpeedDial::resetPresets()
{
    for (QHash<QWidget*, VCSpeedDialPreset*>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        QWidget* widget = it.key();
        m_presetsLayout->removeWidget(widget);
        delete widget;

        VCSpeedDialPreset* preset = it.value();
        if (!preset->m_inputSource.isNull())
            setInputSource(QSharedPointer<QLCInputSource>(), preset->m_id);
        delete preset;
    }
    m_presets.clear();
}

QList<VCSpeedDialPreset*> VCSpeedDial::presets() const
{
    QList<VCSpeedDialPreset*> presetsList = m_presets.values();
    std::sort(presetsList.begin(), presetsList.end(), VCSpeedDialPreset::compare);
    return presetsList;
}

void VCSpeedDial::slotPresetClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    VCSpeedDialPreset *preset = m_presets[btn];

    Q_ASSERT(preset != NULL);

    {
        // Special case: infinite buttons should act as checkboxes
        // so the previous value can be restore when clicking 2 times on them
        if (preset->m_value == (int)Function::infiniteSpeed())
        {
            m_dial->toggleInfinite();
        }
        else // Normal case
        {
            m_dial->setValue(preset->m_value, true);
        }
    }
}

void VCSpeedDial::slotMult()
{
    if (m_currentFactor == -2)
    {
        m_currentFactor = 1;
        slotMultDivChanged();
    }
    else if (m_currentFactor > 0)
    {
        if (m_currentFactor < 2048)
        {
            m_currentFactor *= 2;
            slotMultDivChanged();
        }
    }
    else
    {
        m_currentFactor /= 2;
        slotMultDivChanged();
    }
}

void VCSpeedDial::slotDiv()
{
    if (m_currentFactor == 1)
    {
        m_currentFactor = -2;
        slotMultDivChanged();
    }
    else if (m_currentFactor > 0)
    {
        m_currentFactor /= 2;
        slotMultDivChanged();
    }
    else
    {
        if (m_currentFactor > -2048)
        {
            m_currentFactor *= 2;
            slotMultDivChanged();
        }
    }
}

void VCSpeedDial::slotMultDivReset()
{
    if (m_currentFactor != 1)
    {
        m_currentFactor = 1;
        slotMultDivChanged();
    }
}

void VCSpeedDial::slotMultDivChanged()
{
    if (m_currentFactor > 0)
    {
        m_factoredValue = m_dial->value() * m_currentFactor;
        m_multDivLabel->setText(QString("%1x").arg(m_currentFactor));
    }
    else
    {
        m_factoredValue = m_dial->value() / qAbs(m_currentFactor);
        m_multDivLabel->setText(QString("1/%1x").arg(qAbs(m_currentFactor)));
    }
    m_multDivResultLabel->setText("(" + Function::speedToString(m_factoredValue) + ")");

    slotFactoredValueChanged();
}

void VCSpeedDial::slotFactoredValueChanged()
{
    const QVector<quint32> multipliers = VCSpeedDialFunction::speedMultiplierValuesTimes1000();

    int ms = m_factoredValue;

    foreach (const VCSpeedDialFunction &speeddialfunction, m_functions)
    {
        Function* function = m_doc->function(speeddialfunction.functionId);
        if (function != NULL)
        {
            if (speeddialfunction.fadeInMultiplier != VCSpeedDialFunction::None)
            {
                if ((uint)ms != Function::infiniteSpeed())
                    function->setFadeInSpeed(ms * ((float)multipliers[speeddialfunction.fadeInMultiplier] / 1000.0));
                else
                    function->setFadeInSpeed(ms);
            }
            if (speeddialfunction.fadeOutMultiplier != VCSpeedDialFunction::None)
            {
                if ((uint)ms != Function::infiniteSpeed())
                    function->setFadeOutSpeed(ms * ((float)multipliers[speeddialfunction.fadeOutMultiplier] / 1000.0));
                else
                    function->setFadeOutSpeed(ms);
            }
            if (speeddialfunction.durationMultiplier != VCSpeedDialFunction::None)
            {
                if ((uint)ms != Function::infiniteSpeed())
                    function->setDuration(ms * ((float)multipliers[speeddialfunction.durationMultiplier] / 1000.0));
                else
                    function->setDuration(ms);
            }
        }
    }
}


void VCSpeedDial::setResetFactorOnDialChange(bool value)
{
    m_resetFactorOnDialChange = value;
}

bool VCSpeedDial::resetFactorOnDialChange() const
{
    return m_resetFactorOnDialChange;
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCSpeedDial::updateFeedback()
{
    // Feedback to absolute input source
    int fbv = (int)SCALE(float(m_dial->value()), float(m_absoluteValueMin),
                     float(m_absoluteValueMax), float(0), float(UCHAR_MAX));

    sendFeedback(fbv, absoluteInputSourceId);

    // Feedback to tap button
    sendFeedback(m_dial->isTapTick() ? 255 : 0, tapInputSourceId);

    // Feedback to preset buttons
    for (QHash<QWidget*, VCSpeedDialPreset*>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        VCSpeedDialPreset* preset = it.value();
        if (preset->m_inputSource != NULL)
        {
            QPushButton* button = reinterpret_cast<QPushButton*>(it.key());
            if (preset->m_inputSource.isNull() == false)
                sendFeedback(button->isDown() ?
                             preset->m_inputSource->feedbackValue(QLCInputFeedback::UpperValue) :
                             preset->m_inputSource->feedbackValue(QLCInputFeedback::LowerValue),
                             preset->m_inputSource);
        }
    }
}

void VCSpeedDial::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    /* Don't let input data through in design mode or if disabled */
    if (acceptsInput() == false)
        return;

    quint32 pagedCh = (page() << 16) | channel;

    if (checkInputSource(universe, pagedCh, value, sender(), tapInputSourceId))
    {
        if (value != 0)
            m_dial->tap();
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), absoluteInputSourceId))
    {
        int ms = static_cast<int> (SCALE(qreal(value), qreal(0), qreal(255),
                                         qreal(absoluteValueMin()),
                                         qreal(absoluteValueMax())));
        m_dial->setValue(ms, true);
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), multInputSourceId))
    {
        if (value != 0)
            slotMult();
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), divInputSourceId))
    {
        if (value != 0)
            slotDiv();
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), multDivResetInputSourceId))
    {
        if (value != 0)
            slotMultDivReset();
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), applyInputSourceId))
    {
        if (value != 0)
            slotFactoredValueChanged();
    }
    else
    {
        for (QHash<QWidget*, VCSpeedDialPreset*>::iterator it = m_presets.begin();
                it != m_presets.end(); ++it)
        {
            VCSpeedDialPreset *preset = it.value();
            if (preset->m_inputSource != NULL &&
                    preset->m_inputSource->universe() == universe &&
                    preset->m_inputSource->channel() == pagedCh)
            {
                {
                    QPushButton *button = reinterpret_cast<QPushButton*>(it.key());
                    button->click();
                }
            }
        }
    }
}

/*********************************************************************
 * Tap key sequence handler
 *********************************************************************/

void VCSpeedDial::setTapKeySequence(const QKeySequence& keySequence)
{
    m_tapKeySequence = QKeySequence(keySequence);
}

QKeySequence VCSpeedDial::tapKeySequence() const
{
    return m_tapKeySequence;
}

void VCSpeedDial::setMultKeySequence(const QKeySequence& keySequence)
{
    m_multKeySequence = QKeySequence(keySequence);
}

QKeySequence VCSpeedDial::multKeySequence() const
{
    return m_multKeySequence;
}

void VCSpeedDial::setDivKeySequence(const QKeySequence& keySequence)
{
    m_divKeySequence = QKeySequence(keySequence);
}

QKeySequence VCSpeedDial::divKeySequence() const
{
    return m_divKeySequence;
}

void VCSpeedDial::setMultDivResetKeySequence(const QKeySequence& keySequence)
{
    m_multDivResetKeySequence = QKeySequence(keySequence);
}

QKeySequence VCSpeedDial::multDivResetKeySequence() const
{
    return m_multDivResetKeySequence;
}

void VCSpeedDial::setApplyKeySequence(const QKeySequence& keySequence)
{
    m_applyKeySequence = QKeySequence(keySequence);
}

QKeySequence VCSpeedDial::applyKeySequence() const
{
    return m_applyKeySequence;
}

void VCSpeedDial::slotKeyPressed(const QKeySequence& keySequence)
{
    if (acceptsInput() == false)
        return;

    if (m_tapKeySequence == keySequence)
        m_dial->tap();

    if (m_multKeySequence == keySequence)
        slotMult();
    if (m_divKeySequence == keySequence)
        slotDiv();
    if (m_multDivResetKeySequence == keySequence)
        slotMultDivReset();
    if (m_applyKeySequence == keySequence)
        slotFactoredValueChanged();

    for (QHash<QWidget*, VCSpeedDialPreset*>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        VCSpeedDialPreset *preset = it.value();
        if (preset->m_keySequence == keySequence)
        {
            QPushButton *button = reinterpret_cast<QPushButton*>(it.key());
            button->click();
        }
    }
}


/****************************************************************************
 * Absolute value range
 ****************************************************************************/

void VCSpeedDial::setAbsoluteValueRange(uint min, uint max)
{
    m_absoluteValueMin = min;
    m_absoluteValueMax = max;
}

uint VCSpeedDial::absoluteValueMin() const
{
    return m_absoluteValueMin;
}

uint VCSpeedDial::absoluteValueMax() const
{
    return m_absoluteValueMax;
}

quint32 VCSpeedDial::visibilityMask() const
{
    return m_visibilityMask;
}

void VCSpeedDial::setVisibilityMask(quint32 mask)
{
    if (m_dial != NULL)
        m_dial->setVisibilityMask(mask);

    if (mask & MultDiv)
    {
        m_multButton->show();
        m_multDivLabel->show();
        m_divButton->show();
        m_multDivResetButton->show();
        m_multDivResultLabel->show();
    }
    else
    {
        m_multButton->hide();
        m_multDivLabel->hide();
        m_divButton->hide();
        m_multDivResetButton->hide();
        m_multDivResultLabel->hide();
    }

    if (mask & Apply)
        m_applyButton->show();
    else
        m_applyButton->hide();

    m_visibilityMask = mask;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

static QSharedPointer<VCSpeedDialPreset> createInfinitePreset()
{
    QSharedPointer<VCSpeedDialPreset> infinitePreset(new VCSpeedDialPreset(16));
    infinitePreset->m_value = Function::infiniteSpeed();
    infinitePreset->m_name = Function::speedToString(Function::infiniteSpeed());
    return infinitePreset;
}

bool VCSpeedDial::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCSpeedDial)
    {
        qWarning() << Q_FUNC_INFO << "SpeedDial node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    // Compatibility with old workspace files:
    // Get old style speedtype selection
    VCSpeedDialFunction::SpeedMultiplier defaultFadeInMultiplier = VCSpeedDialFunction::None;
    VCSpeedDialFunction::SpeedMultiplier defaultFadeOutMultiplier = VCSpeedDialFunction::None;
    VCSpeedDialFunction::SpeedMultiplier defaultDurationMultiplier = VCSpeedDialFunction::One;
    if (root.attributes().hasAttribute(KXMLQLCVCSpeedDialSpeedTypes))
    {
        SpeedTypes speedTypes = SpeedTypes(root.attributes().value(KXMLQLCVCSpeedDialSpeedTypes).toString().toInt());
        defaultFadeInMultiplier = (speedTypes & FadeIn) ? VCSpeedDialFunction::One : VCSpeedDialFunction::None;
        defaultFadeOutMultiplier = (speedTypes & FadeOut) ? VCSpeedDialFunction::One : VCSpeedDialFunction::None;
        defaultDurationMultiplier = (speedTypes & Duration) ? VCSpeedDialFunction::One : VCSpeedDialFunction::None;
    }

    // Sorted list for new presets
    QList<VCSpeedDialPreset> newPresets;
    // legacy: transform the infinite checkbox into an infinite preset
    QSharedPointer<VCSpeedDialPreset> infinitePreset(NULL);

    /* Children */
    while (root.readNextStartElement())
    {
        //qDebug() << "VC Speed Dial tag:" << root.name();
        if (root.name() == KXMLQLCFunction)
        {
            // Function
            VCSpeedDialFunction speeddialfunction;
            if (speeddialfunction.loadXML(root, defaultFadeInMultiplier,
                                          defaultFadeOutMultiplier,
                                          defaultDurationMultiplier))
            {
                m_functions.append(speeddialfunction);
            }
        }
        else if (root.name() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = true;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCSpeedDialAbsoluteValue)
        {
            // Value range
            QXmlStreamAttributes vAttrs = root.attributes();
            if (vAttrs.hasAttribute(KXMLQLCVCSpeedDialAbsoluteValueMin) &&
                vAttrs.hasAttribute(KXMLQLCVCSpeedDialAbsoluteValueMax))
            {
                uint min = vAttrs.value(KXMLQLCVCSpeedDialAbsoluteValueMin).toString().toUInt();
                uint max = vAttrs.value(KXMLQLCVCSpeedDialAbsoluteValueMax).toString().toUInt();
                setAbsoluteValueRange(min, max);
            }
            loadXMLSources(root, absoluteInputSourceId);
        }
        else if (root.name() == KXMLQLCVCSpeedDialTap)
        {
            loadXMLSources(root, tapInputSourceId);
        }
        else if (root.name() == KXMLQLCVCSpeedDialResetFactorOnDialChange)
        {
            // Reset factor on dial change
            setResetFactorOnDialChange(root.readElementText() == KXMLQLCTrue);
        }
        else if (root.name() == KXMLQLCVCSpeedDialMult)
        {
            loadXMLSources(root, multInputSourceId);
        }
        else if (root.name() == KXMLQLCVCSpeedDialDiv)
        {
            loadXMLSources(root, divInputSourceId);
        }
        else if (root.name() == KXMLQLCVCSpeedDialMultDivReset)
        {
            loadXMLSources(root, multDivResetInputSourceId);
        }
        else if (root.name() == KXMLQLCVCSpeedDialApply)
        {
            loadXMLSources(root, applyInputSourceId);
        }
        else if (root.name() == KXMLQLCVCSpeedDialTapKey)
        {
            setTapKeySequence(stripKeySequence(QKeySequence(root.readElementText())));
        }
        else if (root.name() == KXMLQLCVCSpeedDialMultKey)
        {
            setMultKeySequence(stripKeySequence(QKeySequence(root.readElementText())));
        }
        else if (root.name() == KXMLQLCVCSpeedDialDivKey)
        {
            setDivKeySequence(stripKeySequence(QKeySequence(root.readElementText())));
        }
        else if (root.name() == KXMLQLCVCSpeedDialMultDivResetKey)
        {
            setMultDivResetKeySequence(stripKeySequence(QKeySequence(root.readElementText())));
        }
        else if (root.name() == KXMLQLCVCSpeedDialApplyKey)
        {
            setApplyKeySequence(stripKeySequence(QKeySequence(root.readElementText())));
        }
        else if (root.name() == KXMLQLCVCSpeedDialInfinite)
        {
            // Legacy: infinite checkbox input
            if (!infinitePreset)
                infinitePreset = createInfinitePreset();

            loadXMLInfiniteLegacy(root, infinitePreset);
        }
        else if (root.name() == KXMLQLCVCSpeedDialInfiniteKey)
        {
            // Legacy: infinite checkbox key sequence
            if (!infinitePreset)
                infinitePreset = createInfinitePreset();

            infinitePreset->m_keySequence = stripKeySequence(QKeySequence(root.readElementText()));
        }
        else if (root.name() == KXMLQLCVCSpeedDialPreset)
        {
            VCSpeedDialPreset preset(0xff);
            if (preset.loadXML(root))
                newPresets.insert(std::lower_bound(newPresets.begin(), newPresets.end(), preset), preset);
        }
        else if (root.name() == KXMLQLCVCSpeedDialVisibilityMask)
        {
            quint32 mask = root.readElementText().toUInt();

            // legacy: infinite checkbox
            if (mask & SpeedDial::Infinite)
            {
                mask &= ~SpeedDial::Infinite;
                if (!infinitePreset)
                    infinitePreset = createInfinitePreset();
            }

            setVisibilityMask(mask);
        }
        else if (root.name() == KXMLQLCVCSpeedDialTime)
        {
            m_dial->setValue(root.readElementText().toUInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown speed dial tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    if (infinitePreset && newPresets.size() > 0)
    {
        qWarning() << Q_FUNC_INFO << "Can't have an infinite checkbox + presets";
        return false;
    }

    if (infinitePreset)
        addPreset(*infinitePreset);
    else
    {
        foreach (VCSpeedDialPreset const& preset, newPresets)
            addPreset(preset);
    }

    return true;
}

bool VCSpeedDial::loadXMLInfiniteLegacy(QXmlStreamReader &root, QSharedPointer<VCSpeedDialPreset> preset)
{
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCWidgetInput)
        {
            quint32 uni = QLCInputSource::invalidUniverse;
            quint32 ch = QLCInputSource::invalidChannel;
            if (loadXMLInput(root, &uni, &ch) == true)
            {
                preset->m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch));
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Frame Source tag" << root.name().toString();
            root.skipCurrentElement();
        }
    }
    return true;
}

bool VCSpeedDial::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCVCSpeedDial);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    if (m_visibilityMask != SpeedDial::defaultVisibilityMask())
    {
        doc->writeTextElement(KXMLQLCVCSpeedDialVisibilityMask, QString::number(m_visibilityMask));
    }

    /* Absolute input */
    doc->writeStartElement(KXMLQLCVCSpeedDialAbsoluteValue);
    doc->writeAttribute(KXMLQLCVCSpeedDialAbsoluteValueMin, QString::number(absoluteValueMin()));
    doc->writeAttribute(KXMLQLCVCSpeedDialAbsoluteValueMax, QString::number(absoluteValueMax()));
    saveXMLInput(doc, inputSource(absoluteInputSourceId));
    doc->writeEndElement();

    /* Tap input */
    QSharedPointer<QLCInputSource> tapSrc = inputSource(tapInputSourceId);
    if (!tapSrc.isNull() && tapSrc->isValid())
    {
        doc->writeStartElement(KXMLQLCVCSpeedDialTap);
        saveXMLInput(doc, tapSrc);
        doc->writeEndElement();
    }

    // MultDiv options
    if (m_resetFactorOnDialChange)
        doc->writeTextElement(KXMLQLCVCSpeedDialResetFactorOnDialChange, KXMLQLCTrue);

    /* Mult input */
    QSharedPointer<QLCInputSource> multSrc = inputSource(multInputSourceId);
    if (!multSrc.isNull() && multSrc->isValid())
    {
        doc->writeStartElement(KXMLQLCVCSpeedDialMult);
        saveXMLInput(doc, multSrc);
        doc->writeEndElement();
    }

    /* Div input */
    QSharedPointer<QLCInputSource> divSrc = inputSource(divInputSourceId);
    if (!divSrc.isNull() && divSrc->isValid())
    {
        doc->writeStartElement(KXMLQLCVCSpeedDialDiv);
        saveXMLInput(doc, divSrc);
        doc->writeEndElement();
    }

    /* MultDiv Reset input */
    QSharedPointer<QLCInputSource> resetSrc = inputSource(multDivResetInputSourceId);
    if (!resetSrc.isNull() && resetSrc->isValid())
    {
        doc->writeStartElement(KXMLQLCVCSpeedDialMultDivReset);
        saveXMLInput(doc, resetSrc);
        doc->writeEndElement();
    }

    /* Apply input */
    QSharedPointer<QLCInputSource> applySrc = inputSource(applyInputSourceId);
    if (!applySrc.isNull() && applySrc->isValid())
    {
        doc->writeStartElement(KXMLQLCVCSpeedDialApply);
        saveXMLInput(doc, applySrc);
        doc->writeEndElement();
    }

    /* Save time */
    doc->writeTextElement(KXMLQLCVCSpeedDialTime, QString::number(m_dial->value()));

    /* Tap key sequence */
    if (m_tapKeySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCSpeedDialTapKey, m_tapKeySequence.toString());

    /* Mult key sequence */
    if (m_multKeySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCSpeedDialMultKey, m_multKeySequence.toString());

    /* Div key sequence */
    if (m_divKeySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCSpeedDialDivKey, m_divKeySequence.toString());

    /* MultDiv Reset key sequence */
    if (m_multDivResetKeySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCSpeedDialMultDivResetKey, m_multDivResetKeySequence.toString());

    /* MultDiv Reset key sequence */
    if (m_applyKeySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCSpeedDialApplyKey, m_applyKeySequence.toString());

    /* Functions */
    foreach (const VCSpeedDialFunction &speeddialfunction, m_functions)
        speeddialfunction.saveXML(doc);

    // Presets
    foreach (VCSpeedDialPreset *preset, presets())
        preset->saveXML(doc);

    /* End the <SpeedDial> tag */
    doc->writeEndElement();

    return true;
}

void VCSpeedDial::postLoad()
{
    /* Remove such function IDs that don't exist */
    QMutableListIterator <VCSpeedDialFunction> it(m_functions);
    while (it.hasNext() == true)
    {
        it.next();
        Function* function = m_doc->function(it.value().functionId);
        if (function == NULL)
            it.remove();
    }
}
