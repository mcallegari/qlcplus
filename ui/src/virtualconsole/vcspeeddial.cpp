/*
  Q Light Controller
  vcspeeddial.cpp

  Copyright (c) Heikki Junnila

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

#include <QDomDocument>
#include <QDomElement>
#include <QLayout>
#include <QDebug>

#include "vcspeeddialproperties.h"
#include "vcpropertieseditor.h"
#include "vcspeeddial.h"
#include "vcspeeddialfunction.h"
#include "vcspeeddialpreset.h"
#include "speeddial.h"
#include "qlcmacros.h"
#include "qlcfile.h"
#include "function.h"
#include "flowlayout.h"

#define UPDATE_TIMEOUT 50

const quint8 VCSpeedDial::absoluteInputSourceId = 0;
const quint8 VCSpeedDial::tapInputSourceId = 1;
const quint8 VCSpeedDial::multInputSourceId = 2;
const quint8 VCSpeedDial::divInputSourceId = 3;
const quint8 VCSpeedDial::multDivResetInputSourceId = 4;
const QSize VCSpeedDial::defaultSize(QSize(200, 175));

static const QString presetBtnSS = "QPushButton { background-color: %1; height: 32px; border: 2px solid #6A6A6A; border-radius: 5px; }"
                                   "QPushButton:pressed { border: 2px solid #0000FF; }"
                                   "QPushButton:disabled { border: 2px solid #BBBBBB; color: #8f8f8f }";

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
    vBox->setMargin(0);

    QHBoxLayout* speedDialHBox = new QHBoxLayout();
    vBox->addLayout(speedDialHBox);

    m_dial = new SpeedDial(this);
    speedDialHBox->addWidget(m_dial);
    connect(m_dial, SIGNAL(valueChanged(int)), this, SLOT(slotDialValueChanged()));
    connect(m_dial, SIGNAL(tapped()), this, SLOT(slotDialTapped()));

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

    // Mult and div
    QVBoxLayout* multDivResetResultVBox = new QVBoxLayout();
    m_multDivTopSpacer = new QLabel();
    multDivResetResultVBox->addWidget(m_multDivTopSpacer);
    QHBoxLayout* multDivResetHBox = new QHBoxLayout();
    QVBoxLayout* multDivVBox = new QVBoxLayout();
    m_multButton = new QToolButton();
    // m_multButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_multButton->setIconSize(QSize(32, 32));
    m_multButton->setIcon(QIcon(":/up.png"));
    connect(m_multButton, SIGNAL(clicked()),
            this, SLOT(slotMult()));
    multDivVBox->addWidget(m_multButton, Qt::AlignVCenter | Qt::AlignLeft);
    m_multDivLabel = new QLabel();
    m_multDivLabel->setAlignment(Qt::AlignCenter);
    m_multDivLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    multDivVBox->addWidget(m_multDivLabel, Qt::AlignVCenter | Qt::AlignLeft);
    m_divButton = new QToolButton();
    // m_divButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_divButton->setIconSize(QSize(32, 32));
    m_divButton->setIcon(QIcon(":/down.png"));
    connect(m_divButton, SIGNAL(clicked()),
            this, SLOT(slotDiv()));
    multDivVBox->addWidget(m_divButton, Qt::AlignVCenter | Qt::AlignLeft);
    multDivResetHBox->addLayout(multDivVBox);
    m_multDivResetButton = new QPushButton(tr("R\nE\nS\nE\nT"));
    // m_multDivResetButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(m_multDivResetButton, SIGNAL(clicked()),
            this, SLOT(slotMultDivReset()));
    multDivResetHBox->addWidget(m_multDivResetButton);
    multDivResetResultVBox->addLayout(multDivResetHBox);
    m_multDivResultLabel = new QLabel();
    m_multDivResultLabel->setAlignment(Qt::AlignCenter);
    m_multDivResultLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_multDivResultLabel->setBackgroundRole(QPalette::BrightText);
    multDivResetResultVBox->addWidget(m_multDivResultLabel);
    speedDialHBox->addLayout(multDivResetResultVBox);

    // Update labels
    slotMultDivChanged();

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

    slotModeChanged(m_doc->mode());
    setLiveEdit(m_liveEdit);
}

VCSpeedDial::~VCSpeedDial()
{
    foreach(VCSpeedDialPreset* preset, m_presets)
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

    resetPresets();
    foreach (VCSpeedDialPreset const* preset, dial->presets())
    {
        addPreset(*preset);
    }

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
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

    updateFeedback();
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

void VCSpeedDial::slotUpdate()
{
    int currentValue = m_dial->value();

    for (QHash<QWidget*, VCSpeedDialPreset*>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        QWidget* widget = it.key();
        VCSpeedDialPreset* preset = it.value();

        {
            QPushButton* button = reinterpret_cast<QPushButton*>(widget);
            button->setDown(preset->m_value == currentValue);
        }
    }
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
        QString btnLabel = preset.m_showName ?
            preset.m_name : Function::speedToString(preset.m_value);
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
    qSort(presetsList.begin(), presetsList.end(), VCSpeedDialPreset::compare);
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
    m_multDivResultLabel->setText(Function::speedToString(m_factoredValue));

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
                    function->setFadeInSpeed(ms * multipliers[speeddialfunction.fadeInMultiplier] / 1000);
                else
                    function->setFadeInSpeed(ms);
            }
            if (speeddialfunction.fadeOutMultiplier != VCSpeedDialFunction::None)
            {
                if ((uint)ms != Function::infiniteSpeed())
                    function->setFadeOutSpeed(ms * multipliers[speeddialfunction.fadeOutMultiplier] / 1000);
                else
                    function->setFadeOutSpeed(ms);
            }
            if (speeddialfunction.durationMultiplier != VCSpeedDialFunction::None)
            {
                if ((uint)ms != Function::infiniteSpeed())
                    function->setDuration(ms * multipliers[speeddialfunction.durationMultiplier] / 1000);
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
    int fbv = (int)SCALE(float(m_dial->value()), float(m_absoluteValueMin),
                     float(m_absoluteValueMax), float(0), float(UCHAR_MAX));

    sendFeedback(fbv, absoluteInputSourceId);

    for (QHash<QWidget*, VCSpeedDialPreset*>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        VCSpeedDialPreset* preset = it.value();
        if (preset->m_inputSource != NULL)
        {
            {
                QPushButton* button = reinterpret_cast<QPushButton*>(it.key());
                sendFeedback(button->isDown() ? 0xff : 0);
            }
        }
    }
}

void VCSpeedDial::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    if (isEnabled() == false)
        return;

    quint32 pagedCh = (page() << 16) | channel;

    if (checkInputSource(universe, pagedCh, value, sender(), tapInputSourceId))
    {
        if (value != 0)
            m_dial->tap();
    }
    if (checkInputSource(universe, pagedCh, value, sender(), absoluteInputSourceId))
    {
        int ms = static_cast<int> (SCALE(qreal(value), qreal(0), qreal(255),
                                         qreal(absoluteValueMin()),
                                         qreal(absoluteValueMax())));
        m_dial->setValue(ms, true);
    }

    if (checkInputSource(universe, pagedCh, value, sender(), multInputSourceId))
        slotMult();
    if (checkInputSource(universe, pagedCh, value, sender(), divInputSourceId))
        slotDiv();
    if (checkInputSource(universe, pagedCh, value, sender(), multDivResetInputSourceId))
        slotMultDivReset();

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

void VCSpeedDial::slotKeyPressed(const QKeySequence& keySequence)
{
    if (isEnabled() == false)
        return;

    if (m_tapKeySequence == keySequence)
        m_dial->tap();

    if (m_multKeySequence == keySequence)
        slotMult();
    if (m_divKeySequence == keySequence)
        slotDiv();
    if (m_multDivResetKeySequence == keySequence)
        slotMultDivReset();

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
        m_multDivTopSpacer->show();
        m_multButton->show();
        m_multDivLabel->show();
        m_divButton->show();
        m_multDivResetButton->show();
        m_multDivResultLabel->show();
    }
    else
    {
        m_multDivTopSpacer->hide();
        m_multButton->hide();
        m_multDivLabel->hide();
        m_divButton->hide();
        m_multDivResetButton->hide();
        m_multDivResultLabel->hide();
    }

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
    infinitePreset->m_showName = true;
    return infinitePreset;
}

bool VCSpeedDial::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCSpeedDial)
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
    if (root->hasAttribute(KXMLQLCVCSpeedDialSpeedTypes))
    {
        SpeedTypes speedTypes = SpeedTypes(root->attribute(KXMLQLCVCSpeedDialSpeedTypes).toInt());
        defaultFadeInMultiplier = speedTypes & FadeIn ? VCSpeedDialFunction::One : VCSpeedDialFunction::None;
        defaultFadeOutMultiplier = speedTypes & FadeOut ? VCSpeedDialFunction::One : VCSpeedDialFunction::None;
        defaultDurationMultiplier = speedTypes & Duration ? VCSpeedDialFunction::One : VCSpeedDialFunction::None;
    }

    /* Children */
    QDomNode node = root->firstChild();
    // Sorted list for new presets
    QList<VCSpeedDialPreset> newPresets;
    // legacy: transform the infinite checkbox into an infinite preset
    QSharedPointer<VCSpeedDialPreset> infinitePreset(NULL);
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCFunction)
        {
            // Function
            VCSpeedDialFunction speeddialfunction;
            if (speeddialfunction.loadXML(tag, defaultFadeInMultiplier, defaultFadeOutMultiplier, defaultDurationMultiplier))
            {
                m_functions.append(speeddialfunction);
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialAbsoluteValue)
        {
            // Value range
            if (tag.hasAttribute(KXMLQLCVCSpeedDialAbsoluteValueMin) &&
                tag.hasAttribute(KXMLQLCVCSpeedDialAbsoluteValueMax))
            {
                uint min = tag.attribute(KXMLQLCVCSpeedDialAbsoluteValueMin).toUInt();
                uint max = tag.attribute(KXMLQLCVCSpeedDialAbsoluteValueMax).toUInt();
                setAbsoluteValueRange(min, max);
            }

            // Input
            QDomNode sub = node.firstChild();
            while (sub.isNull() == false)
            {
                QDomElement subtag = sub.toElement();
                if (subtag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = QLCInputSource::invalidUniverse;
                    quint32 ch = QLCInputSource::invalidChannel;
                    if (loadXMLInput(subtag, &uni, &ch) == true)
                        setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch)), absoluteInputSourceId);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown absolute value tag:" << tag.tagName();
                }

                sub = sub.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialTap)
        {
            // Input
            QDomNode sub = node.firstChild();
            while (sub.isNull() == false)
            {
                QDomElement subtag = sub.toElement();
                if (subtag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = QLCInputSource::invalidUniverse;
                    quint32 ch = QLCInputSource::invalidChannel;
                    if (loadXMLInput(subtag, &uni, &ch) == true)
                        setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch)), tapInputSourceId);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown tap tag:" << tag.tagName();
                }

                sub = sub.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialResetFactorOnDialChange)
        {
            // Reset factor on dial change
            setResetFactorOnDialChange(tag.text() == KXMLQLCTrue);
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialMult)
        {
            // Input
            QDomNode sub = node.firstChild();
            while (sub.isNull() == false)
            {
                QDomElement subtag = sub.toElement();
                if (subtag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = QLCInputSource::invalidUniverse;
                    quint32 ch = QLCInputSource::invalidChannel;
                    if (loadXMLInput(subtag, &uni, &ch) == true)
                        setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch)), multInputSourceId);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown mult tag:" << tag.tagName();
                }

                sub = sub.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialDiv)
        {
            // Input
            QDomNode sub = node.firstChild();
            while (sub.isNull() == false)
            {
                QDomElement subtag = sub.toElement();
                if (subtag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = QLCInputSource::invalidUniverse;
                    quint32 ch = QLCInputSource::invalidChannel;
                    if (loadXMLInput(subtag, &uni, &ch) == true)
                        setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch)), divInputSourceId);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown div tag:" << tag.tagName();
                }

                sub = sub.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialMultDivReset)
        {
            // Input
            QDomNode sub = node.firstChild();
            while (sub.isNull() == false)
            {
                QDomElement subtag = sub.toElement();
                if (subtag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = QLCInputSource::invalidUniverse;
                    quint32 ch = QLCInputSource::invalidChannel;
                    if (loadXMLInput(subtag, &uni, &ch) == true)
                        setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch)), multDivResetInputSourceId);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown multdiv reset tag:" << tag.tagName();
                }

                sub = sub.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialTapKey)
        {
            setTapKeySequence(stripKeySequence(QKeySequence(tag.text())));
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialMultKey)
        {
            setMultKeySequence(stripKeySequence(QKeySequence(tag.text())));
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialDivKey)
        {
            setDivKeySequence(stripKeySequence(QKeySequence(tag.text())));
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialMultDivResetKey)
        {
            setMultDivResetKeySequence(stripKeySequence(QKeySequence(tag.text())));
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialInfinite)
        {
            // Legacy: infinite checkbox input
            if (!infinitePreset)
                infinitePreset = createInfinitePreset();

            QDomNode sub = node.firstChild();
            while (sub.isNull() == false)
            {
                QDomElement subtag = sub.toElement();
                if (subtag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = QLCInputSource::invalidUniverse;
                    quint32 ch = QLCInputSource::invalidChannel;
                    if (loadXMLInput(subtag, &uni, &ch) == true)
                    {
                        infinitePreset->m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch));
                    }
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown infinite tag:" << tag.tagName();
                }

                sub = sub.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialInfiniteKey)
        {
            // Legacy: infinite checkbox key sequence
            if (!infinitePreset)
                infinitePreset = createInfinitePreset();

            infinitePreset->m_keySequence = stripKeySequence(QKeySequence(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = true;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if(tag.tagName() == KXMLQLCVCSpeedDialPreset)
        {
            VCSpeedDialPreset preset(0xff);
            if (preset.loadXML(tag))
                newPresets.insert(qLowerBound(newPresets.begin(), newPresets.end(), preset), preset);
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialVisibilityMask)
        {
            quint32 mask = tag.text().toUInt();

            // legacy: infinite checkbox
            if (mask & SpeedDial::Infinite)
            {
                mask &= ~SpeedDial::Infinite;
                if (!infinitePreset)
                    infinitePreset = createInfinitePreset();
            }

            setVisibilityMask(mask);
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialTime)
        {
            m_dial->setValue(tag.text().toUInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown speed dial tag:" << tag.tagName();
        }

        node = node.nextSibling();
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

bool VCSpeedDial::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCVCSpeedDial);
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    if (m_visibilityMask != SpeedDial::defaultVisibilityMask())
    {
        QDomElement tag = doc->createElement(KXMLQLCVCSpeedDialVisibilityMask);
        root.appendChild(tag);
        QDomText text = doc->createTextNode(QString::number(m_visibilityMask));
        tag.appendChild(text);
    }

    /* Absolute input */
    QDomElement absInput = doc->createElement(KXMLQLCVCSpeedDialAbsoluteValue);
    absInput.setAttribute(KXMLQLCVCSpeedDialAbsoluteValueMin, absoluteValueMin());
    absInput.setAttribute(KXMLQLCVCSpeedDialAbsoluteValueMax, absoluteValueMax());
    saveXMLInput(doc, &absInput, inputSource(absoluteInputSourceId));
    root.appendChild(absInput);

    /* Tap input */
    QDomElement tap = doc->createElement(KXMLQLCVCSpeedDialTap);
    saveXMLInput(doc, &tap, inputSource(tapInputSourceId));
    root.appendChild(tap);

    // MultDiv options
    if (m_resetFactorOnDialChange)
    {
        QDomElement tag = doc->createElement(KXMLQLCVCSpeedDialResetFactorOnDialChange);
        QDomText text = doc->createTextNode(KXMLQLCTrue);
        tag.appendChild(text);
        root.appendChild(tag);
    }

    /* Mult input */
    QDomElement mult = doc->createElement(KXMLQLCVCSpeedDialMult);
    saveXMLInput(doc, &mult, inputSource(multInputSourceId));
    root.appendChild(mult);

    /* Div input */
    QDomElement div = doc->createElement(KXMLQLCVCSpeedDialDiv);
    saveXMLInput(doc, &div, inputSource(divInputSourceId));
    root.appendChild(div);

    /* MultDiv Reset input */
    QDomElement multDivReset = doc->createElement(KXMLQLCVCSpeedDialMultDivReset);
    saveXMLInput(doc, &multDivReset, inputSource(multDivResetInputSourceId));
    root.appendChild(multDivReset);

    /* Save time */
    QDomElement time = doc->createElement(KXMLQLCVCSpeedDialTime);
    root.appendChild(time);
    QDomText text = doc->createTextNode(QString::number(m_dial->value()));
    time.appendChild(text);

    /* Tap key sequence */
    if (m_tapKeySequence.isEmpty() == false)
    {
        QDomElement tag = doc->createElement(KXMLQLCVCSpeedDialTapKey);
        root.appendChild(tag);
        QDomText text = doc->createTextNode(m_tapKeySequence.toString());
        tag.appendChild(text);
    }

    /* Mult key sequence */
    if (m_multKeySequence.isEmpty() == false)
    {
        QDomElement tag = doc->createElement(KXMLQLCVCSpeedDialMultKey);
        root.appendChild(tag);
        QDomText text = doc->createTextNode(m_multKeySequence.toString());
        tag.appendChild(text);
    }

    /* Div key sequence */
    if (m_divKeySequence.isEmpty() == false)
    {
        QDomElement tag = doc->createElement(KXMLQLCVCSpeedDialDivKey);
        root.appendChild(tag);
        QDomText text = doc->createTextNode(m_divKeySequence.toString());
        tag.appendChild(text);
    }

    /* MultDiv Reset key sequence */
    if (m_multDivResetKeySequence.isEmpty() == false)
    {
        QDomElement tag = doc->createElement(KXMLQLCVCSpeedDialMultDivResetKey);
        root.appendChild(tag);
        QDomText text = doc->createTextNode(m_multDivResetKeySequence.toString());
        tag.appendChild(text);
    }

    /* Functions */
    foreach (const VCSpeedDialFunction &speeddialfunction, m_functions)
    {
        speeddialfunction.saveXML(doc, &root);
    }

    // Presets
    foreach(VCSpeedDialPreset *preset, presets())
        preset->saveXML(doc, &root);

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
