/*
  Q Light Controller Plus
  vcslider.cpp

  Copyright (c) Heikki Junnila
                Stefan Krumm
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
#include <QWidgetAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPaintEvent>
#include <QSettings>
#include <QPainter>
#include <QString>
#include <QSlider>
#include <QTimer>
#include <QDebug>
#include <QLabel>
#include <math.h>
#include <QMenu>
#include <QSize>
#include <QPen>

#include "vcsliderproperties.h"
#include "vcpropertieseditor.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "universe.h"
#include "vcslider.h"
#include "apputil.h"
#include "doc.h"

/** Number of DMXSource cycles to wait to consider a
 *  playback value change stable */
#define PLAYBACK_CHANGE_THRESHOLD   5  // roughly this is 100ms

/** +/- value range to catch the slider for external
 *  controllers with no feedback support */
#define VALUE_CATCHING_THRESHOLD    4

const quint8 VCSlider::sliderInputSourceId = 0;
const quint8 VCSlider::overrideResetInputSourceId = 1;
const quint8 VCSlider::flashButtonInputSourceId = 2;

const QSize VCSlider::defaultSize(QSize(60, 200));

const QString submasterStyleSheet =
    SLIDER_SS_COMMON

    "QSlider::handle:vertical { "
    "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4c4c4c, stop:0.45 #2c2c2c, stop:0.50 #000, stop:0.55 #111111, stop:1 #131313);"
    "border: 1px solid #5c5c5c;"
    "border-radius: 4px; margin: 0 -4px; height: 20px; }"

    "QSlider::handle:vertical:hover {"
    "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6c6c6c, stop:0.45 #4c4c4c, stop:0.50 #ffff00, stop:0.55 #313131, stop:1 #333333);"
    "border: 1px solid #000; }"

    "QSlider::add-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #77DD73, stop: 1 #A5EC98 );"
    "border: 1px solid #5288A7; margin: 0 9px; }";

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCSlider::VCSlider(QWidget *parent, Doc *doc)
    : VCWidget(parent, doc)
    , m_valueDisplayStyle(ExactValue)
    , m_catchValues(false)
    , m_levelLowLimit(0)
    , m_levelHighLimit(UCHAR_MAX)
    , m_levelValueChanged(false)
    , m_levelValue(0)
    , m_monitorEnabled(false)
    , m_monitorValue(0)
    , m_playbackFunction(Function::invalidId())
    , m_playbackValue(0)
    , m_playbackChangeCounter(0)
    , m_playbackFlashEnable(false)
    , m_playbackIsFlashing(false)
    , m_externalMovement(false)
    , m_widgetMode(WSlider)
    , m_cngType(ClickAndGoWidget::None)
    , m_isOverriding(false)
    , m_lastInputValue(-1)
{
    /* Set the class name "VCSlider" as the object name as well */
    setObjectName(VCSlider::staticMetaObject.className());

    m_hbox = NULL;
    m_topLabel = NULL;
    m_slider = NULL;
    m_bottomLabel = NULL;

    setType(VCWidget::SliderWidget);
    setCaption(QString());
    setFrameStyle(KVCFrameStyleSunken);

    /* Main VBox */
    new QVBoxLayout(this);

    /* Top label */
    m_topLabel = new QLabel(this);
    m_topLabel->setAlignment(Qt::AlignHCenter);

    layout()->addWidget(m_topLabel);

    /* Slider's HBox |stretch|slider|stretch| */
    m_hbox = new QHBoxLayout();

    /* Put stretchable space before the slider (to its left side) */
    m_hbox->addStretch();

    /* The slider */
    m_slider = new ClickAndGoSlider(this);

    m_hbox->addWidget(m_slider);
    m_slider->setRange(0, 255);
    m_slider->setPageStep(1);
    m_slider->setInvertedAppearance(false);
    m_slider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_slider->setMinimumWidth(32);
    m_slider->setMaximumWidth(80);
    m_slider->setStyleSheet(CNG_DEFAULT_STYLE);

    connect(m_slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderMoved(int)));
    connect(this, SIGNAL(requestSliderUpdate(int)),
            m_slider, SLOT(setValue(int)));

    /* Put stretchable space after the slider (to its right side) */
    m_hbox->addStretch();

    layout()->addItem(m_hbox);

    /* Click & Go button */
    m_cngButton = new QToolButton(this);
    m_cngButton->setFixedSize(48, 48);
    m_cngButton->setIconSize(QSize(42, 42));
    m_menu = new QMenu(this);
    QWidgetAction* action = new QWidgetAction(this);
    m_cngWidget = new ClickAndGoWidget();
    action->setDefaultWidget(m_cngWidget);
    m_menu->addAction(action);
    m_cngButton->setMenu(m_menu);
    m_cngButton->setPopupMode(QToolButton::InstantPopup);
    layout()->addWidget(m_cngButton);
    layout()->setAlignment(m_cngButton, Qt::AlignHCenter);
    m_cngButton->hide();

    connect(m_cngWidget, SIGNAL(levelChanged(uchar)),
            this, SLOT(slotClickAndGoLevelChanged(uchar)));
    connect(m_cngWidget, SIGNAL(colorChanged(QRgb)),
            this, SLOT(slotClickAndGoColorChanged(QRgb)));
    connect(m_cngWidget, SIGNAL(levelAndPresetChanged(uchar,QImage)),
            this, SLOT(slotClickAndGoLevelAndPresetChanged(uchar, QImage)));
    connect(this, SIGNAL(monitorDMXValueChanged(int)),
            this, SLOT(slotMonitorDMXValueChanged(int)));

    m_resetButton = NULL;
    m_flashButton = NULL;

    /* Bottom label */
    m_bottomLabel = new QLabel(this);
    layout()->addWidget(m_bottomLabel);
    m_bottomLabel->setAlignment(Qt::AlignCenter);
    m_bottomLabel->setWordWrap(true);
    m_bottomLabel->hide();

    setMinimumSize(20, 20);
    QSettings settings;
    QVariant var = settings.value(SETTINGS_SLIDER_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(VCSlider::defaultSize);

    /* Initialize to playback mode by default */
    setInvertedAppearance(false);
    m_sliderMode = SliderMode(-1); // avoid use of uninitialized value
    setSliderMode(Playback);

    /* Update the slider according to current mode */
    slotModeChanged(m_doc->mode());
    setLiveEdit(m_liveEdit);

    /* Listen to fixture removals so that LevelChannels can be removed when
       they no longer point to an existing fixture->channel */
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotFixtureRemoved(quint32)));
}

VCSlider::~VCSlider()
{
    /* When application exits these are already NULL and unregistration
       is no longer necessary. But a normal deletion of a VCSlider in
       design mode must unregister the slider. */
    m_doc->masterTimer()->unregisterDMXSource(this);

    // request to delete all the active faders
    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->requestDelete();
    }
    m_fadersMap.clear();
}

void VCSlider::setID(quint32 id)
{
    VCWidget::setID(id);

    if (caption().isEmpty())
        setCaption(tr("Slider %1").arg(id));
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCSlider::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCSlider* slider = new VCSlider(parent, m_doc);
    if (slider->copyFrom(this) == false)
    {
        delete slider;
        slider = NULL;
    }

    return slider;
}

bool VCSlider::copyFrom(const VCWidget* widget)
{
    const VCSlider* slider = qobject_cast<const VCSlider*> (widget);
    if (slider == NULL)
        return false;

    /* Copy widget style */
    setWidgetStyle(slider->widgetStyle());

    /* Copy level stuff */
    setLevelLowLimit(slider->levelLowLimit());
    setLevelHighLimit(slider->levelHighLimit());
    m_levelChannels = slider->m_levelChannels;

    /* Copy playback stuff */
    m_playbackFunction = slider->m_playbackFunction;

    /* Copy slider appearance */
    setValueDisplayStyle(slider->valueDisplayStyle());
    setInvertedAppearance(slider->invertedAppearance());

    /* Copy Click & Go feature */
    setClickAndGoType(slider->clickAndGoType());

    /* Copy mode & current value */
    setSliderMode(slider->sliderMode());
    setSliderValue(slider->sliderValue());

    /* Copy monitor mode */
    setChannelsMonitorEnabled(slider->channelsMonitorEnabled());

    /* Copy flash enabling */
    setPlaybackFlashEnable(slider->playbackFlashEnable());

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * GUI
 *****************************************************************************/

void VCSlider::setCaption(const QString& text)
{
    VCWidget::setCaption(text);

    if (m_bottomLabel != NULL)
        setBottomLabelText(text);
}

void VCSlider::enableWidgetUI(bool enable)
{
    m_topLabel->setEnabled(enable);
    if (m_slider)
        m_slider->setEnabled(enable);
    m_bottomLabel->setEnabled(enable);
    m_cngButton->setEnabled(enable);
    if (m_resetButton)
        m_resetButton->setEnabled(enable);
    if (m_flashButton)
        m_flashButton->setEnabled(enable);
    if (enable == false)
        m_lastInputValue = -1;
}

void VCSlider::hideEvent(QHideEvent *)
{
    m_lastInputValue = -1;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCSlider::editProperties()
{
    VCSliderProperties prop(this, m_doc);
    if (prop.exec() == QDialog::Accepted)
    {
        m_doc->setModified();
        if (m_cngType == ClickAndGoWidget::None)
            m_cngButton->hide();
        else
            m_cngButton->show();
    }
}

/*****************************************************************************
 * QLC Mode
 *****************************************************************************/

void VCSlider::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        enableWidgetUI(true);
        if (m_sliderMode == Level || m_sliderMode == Playback)
        {
            m_doc->masterTimer()->registerDMXSource(this);
            if (m_sliderMode == Level)
                m_levelValueChanged = true;
        }
    }
    else
    {
        enableWidgetUI(false);
        if (m_sliderMode == Level || m_sliderMode == Playback)
        {
            m_doc->masterTimer()->unregisterDMXSource(this);
            // request to delete all the active faders
            foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
            {
                if (!fader.isNull())
                    fader->requestDelete();
            }
            m_fadersMap.clear();
        }
    }

    VCWidget::slotModeChanged(mode);
}

/*****************************************************************************
 * Value display style
 *****************************************************************************/

QString VCSlider::valueDisplayStyleToString(VCSlider::ValueDisplayStyle style)
{
    switch (style)
    {
    case ExactValue:
        return KXMLQLCVCSliderValueDisplayStyleExact;
    case PercentageValue:
        return KXMLQLCVCSliderValueDisplayStylePercentage;
    default:
        return QString("Unknown");
    };
}

VCSlider::ValueDisplayStyle VCSlider::stringToValueDisplayStyle(QString style)
{
    if (style == KXMLQLCVCSliderValueDisplayStyleExact)
        return ExactValue;
    else if (style == KXMLQLCVCSliderValueDisplayStylePercentage)
        return PercentageValue;
    else
        return ExactValue;
}

void VCSlider::setValueDisplayStyle(VCSlider::ValueDisplayStyle style)
{
    m_valueDisplayStyle = style;
    if (m_slider)
        setTopLabelText(m_slider->value());
}

VCSlider::ValueDisplayStyle VCSlider::valueDisplayStyle() const
{
    return m_valueDisplayStyle;
}

/*****************************************************************************
 * Inverted appearance
 *****************************************************************************/

bool VCSlider::invertedAppearance() const
{
    if (m_slider)
        return m_slider->invertedAppearance();

    return false;
}

void VCSlider::setInvertedAppearance(bool invert)
{
    if (m_slider)
    {
        m_slider->setInvertedAppearance(invert);
        m_slider->setInvertedControls(invert);
    }
}

/*********************************************************************
 * Value catching feature
 *********************************************************************/

bool VCSlider::catchValues() const
{
    return m_catchValues;
}

void VCSlider::setCatchValues(bool enable)
{
    if (enable == m_catchValues)
        return;

    m_catchValues = enable;
}

/*****************************************************************************
 * Slider Mode
 *****************************************************************************/

QString VCSlider::sliderModeToString(SliderMode mode)
{
    switch (mode)
    {
        case Level: return QString("Level"); break;
        case Playback: return QString("Playback"); break;
        case Submaster: return QString("Submaster"); break;
        default: return QString("Unknown"); break;
    }
}

VCSlider::SliderMode VCSlider::stringToSliderMode(const QString& mode)
{
    if (mode == QString("Level"))
        return Level;
    else  if (mode == QString("Playback"))
       return Playback;
    else //if (mode == QString("Submaster"))
        return Submaster;
}

VCSlider::SliderMode VCSlider::sliderMode() const
{
    return m_sliderMode;
}

void VCSlider::setSliderMode(SliderMode mode)
{
    Q_ASSERT(mode >= Level && mode <= Submaster);

    m_sliderMode = mode;

    if (mode == Level)
    {
        /* Set the slider range */
        if (m_slider)
        {
            m_slider->setRange(levelLowLimit(), levelHighLimit());
            m_slider->setValue(levelValue());
            if (m_widgetMode == WSlider)
                m_slider->setStyleSheet(CNG_DEFAULT_STYLE);
        }

        m_bottomLabel->show();
        if (m_cngType != ClickAndGoWidget::None)
        {
            setClickAndGoType(m_cngType);
            setupClickAndGoWidget();
            m_cngButton->show();
            if (m_slider)
                setClickAndGoWidgetFromLevel(m_slider->value());
        }

        if (m_doc->mode() == Doc::Operate)
            m_doc->masterTimer()->registerDMXSource(this);
    }
    else if (mode == Playback)
    {
        m_bottomLabel->show();
        m_cngButton->hide();
        m_monitorEnabled = false;

        uchar level = playbackValue();
        if (m_slider)
        {
            m_slider->setRange(0, UCHAR_MAX);
            m_slider->setValue(level);
            if (m_widgetMode == WSlider)
                m_slider->setStyleSheet(CNG_DEFAULT_STYLE);
        }
        slotSliderMoved(level);

        if (m_doc->mode() == Doc::Operate)
            m_doc->masterTimer()->registerDMXSource(this);
        setPlaybackFunction(this->m_playbackFunction);
    }
    else if (mode == Submaster)
    {
        m_monitorEnabled = false;
        setPlaybackFunction(Function::invalidId());

        if (m_slider)
        {
            m_slider->setRange(0, UCHAR_MAX);
            m_slider->setValue(levelValue());
            if (m_widgetMode == WSlider)
                m_slider->setStyleSheet(submasterStyleSheet);
        }
        if (m_doc->mode() == Doc::Operate)
            m_doc->masterTimer()->unregisterDMXSource(this);
    }
}

/*****************************************************************************
 * Level
 *****************************************************************************/

void VCSlider::addLevelChannel(quint32 fixture, quint32 channel)
{
    LevelChannel lch(fixture, channel);

    if (m_levelChannels.contains(lch) == false)
    {
        m_levelChannels.append(lch);
        std::sort(m_levelChannels.begin(), m_levelChannels.end());
    }
}

void VCSlider::removeLevelChannel(quint32 fixture, quint32 channel)
{
    LevelChannel lch(fixture, channel);
    m_levelChannels.removeAll(lch);
}

void VCSlider::clearLevelChannels()
{
    m_levelChannels.clear();
}

QList <VCSlider::LevelChannel> VCSlider::levelChannels()
{
    return m_levelChannels;
}

void VCSlider::setLevelLowLimit(uchar value)
{
    m_levelLowLimit = value;
    if (m_cngWidget != NULL)
        m_cngWidget->setLevelLowLimit(value);
}

uchar VCSlider::levelLowLimit() const
{
    return m_levelLowLimit;
}

void VCSlider::setLevelHighLimit(uchar value)
{
    m_levelHighLimit = value;
    if (m_cngWidget != NULL)
        m_cngWidget->setLevelHighLimit(value);
}

uchar VCSlider::levelHighLimit() const
{
    return m_levelHighLimit;
}

void VCSlider::setChannelsMonitorEnabled(bool enable)
{
    m_monitorEnabled = enable;

    if (m_resetButton != NULL)
    {
        disconnect(m_resetButton, SIGNAL(clicked(bool)),
                this, SLOT(slotResetButtonClicked()));
        delete m_resetButton;
        m_resetButton = NULL;
    }

    if (enable)
    {
        m_resetButton = new QToolButton(this);
        m_cngButton->setFixedSize(32, 32);
        m_resetButton->setIconSize(QSize(32, 32));
        m_resetButton->setStyle(AppUtil::saneStyle());
        m_resetButton->setIcon(QIcon(":/fileclose.png"));
        m_resetButton->setToolTip(tr("Reset channels override"));
        layout()->addWidget(m_resetButton);
        layout()->setAlignment(m_resetButton, Qt::AlignHCenter);

        connect(m_resetButton, SIGNAL(clicked(bool)),
                this, SLOT(slotResetButtonClicked()));
        m_resetButton->show();
        setSliderShadowValue(m_monitorValue);
    }
    else
    {
        setSliderShadowValue(-1);
    }
}

bool VCSlider::channelsMonitorEnabled() const
{
    return m_monitorEnabled;
}

void VCSlider::setLevelValue(uchar value, bool external)
{
    QMutexLocker locker(&m_levelValueMutex);
    m_levelValue = CLAMP(value, levelLowLimit(), levelHighLimit());
    if (m_monitorEnabled == true)
        m_monitorValue = m_levelValue;
    if (m_slider->isSliderDown() || external)
        m_levelValueChanged = true;
}

uchar VCSlider::levelValue() const
{
    return m_levelValue;
}

void VCSlider::slotFixtureRemoved(quint32 fxi_id)
{
    QMutableListIterator <LevelChannel> it(m_levelChannels);
    while (it.hasNext() == true)
    {
        it.next();
        if (it.value().fixture == fxi_id)
            it.remove();
    }
}

void VCSlider::slotMonitorDMXValueChanged(int value)
{
    if (value == sliderValue())
        return;

    m_monitorValue = value;

    if (m_isOverriding == false)
    {
        {
            QMutexLocker locker(&m_levelValueMutex);
            m_levelValue = m_monitorValue;
        }

        if (m_slider)
            m_slider->blockSignals(true);
        setSliderValue(value, false);
        setTopLabelText(sliderValue());
        if (m_slider)
            m_slider->blockSignals(false);
    }
    setSliderShadowValue(value);
    updateFeedback();
}

void VCSlider::slotUniverseWritten(quint32 idx, const QByteArray &universeData)
{
    if (m_levelValueChanged)
        return;

    bool mixedDMXlevels = false;
    int monitorSliderValue = -1;
    QListIterator <LevelChannel> it(m_levelChannels);

    while (it.hasNext() == true)
    {
        LevelChannel lch(it.next());
        Fixture* fxi = m_doc->fixture(lch.fixture);
        if (fxi == NULL || fxi->universe() != idx)
            continue;

        if (lch.channel >= fxi->channels() ||
            fxi->address() + lch.channel >= (quint32)universeData.length())
            continue;

        quint32 dmx_ch = fxi->address() + lch.channel;
        uchar chValue = universeData.at(dmx_ch);
        if (monitorSliderValue == -1)
        {
            monitorSliderValue = chValue;
            //qDebug() << "Monitor DMX value:" << monitorSliderValue << "level value:" << m_levelValue;
        }
        else
        {
            if (chValue != (uchar)monitorSliderValue)
            {
                mixedDMXlevels = true;
                // no need to proceed further as mixed values cannot
                // be represented by one single slider
                break;
            }
        }
    }

    // check if all the DMX channels controlled by this slider
    // have the same value. If so, move the widget slider or knob
    // to the detected position
    if (mixedDMXlevels == false &&
        monitorSliderValue != m_monitorValue)
    {
        emit monitorDMXValueChanged(monitorSliderValue);

        if (m_isOverriding == false)
        {
            // return here. At the next call of this method,
            // the monitor level will kick in
            return;
        }
    }
}

/*********************************************************************
 * Click & Go
 *********************************************************************/

void VCSlider::setClickAndGoType(ClickAndGoWidget::ClickAndGo type)
{
    m_cngType = type;
}

ClickAndGoWidget::ClickAndGo VCSlider::clickAndGoType() const
{
    return m_cngType;
}

void VCSlider::setupClickAndGoWidget()
{
    if (m_cngWidget != NULL)
    {
        qDebug() << Q_FUNC_INFO << "Level channel: " << m_levelChannels.size() << "type: " << m_cngType;
        if (m_cngType == ClickAndGoWidget::Preset && m_levelChannels.size() > 0)
        {
            LevelChannel lChan = m_levelChannels.first();
            Fixture *fxi = m_doc->fixture(lChan.fixture);
            if (fxi != NULL)
            {
                const QLCChannel *chan = fxi->channel(lChan.channel);
                m_cngWidget->setType(m_cngType, chan);
                m_cngWidget->setLevelLowLimit(this->levelLowLimit());
                m_cngWidget->setLevelHighLimit(this->levelHighLimit());
            }
        }
        else
            m_cngWidget->setType(m_cngType, NULL);
    }
}

ClickAndGoWidget *VCSlider::getClickAndGoWidget()
{
    return m_cngWidget;
}

void VCSlider::setClickAndGoWidgetFromLevel(uchar level)
{
    if (m_cngType == ClickAndGoWidget::None || m_cngWidget == NULL)
        return;

    if (m_cngType == ClickAndGoWidget::RGB || m_cngType == ClickAndGoWidget::CMY)
    {
        QPixmap px(42, 42);
        float f = 0;
        if (m_slider)
            f = SCALE(float(level), float(m_slider->minimum()),
                      float(m_slider->maximum()), float(0), float(200));

        if ((uchar)f == 0)
        {
            px.fill(Qt::black);
        }
        else
        {
            QColor modColor = m_cngRGBvalue.lighter((uchar)f);
            px.fill(modColor);
        }
        m_cngButton->setIcon(px);
    }
    else
        m_cngButton->setIcon(QPixmap::fromImage(m_cngWidget->getImageFromValue(level)));
}

void VCSlider::slotClickAndGoLevelChanged(uchar level)
{
    setSliderValue(level, false, false);
    updateFeedback();

    QColor col = m_cngWidget->getColorAt(level);
    QPixmap px(42, 42);
    px.fill(col);
    m_cngButton->setIcon(px);
    m_levelValueChanged = true;
}

void VCSlider::slotClickAndGoColorChanged(QRgb color)
{
    QColor col(color);
    m_cngRGBvalue = col;
    QPixmap px(42, 42);
    px.fill(col);
    m_cngButton->setIcon(px);

    // place the slider half way to reach white@255 and black@0
    setSliderValue(128);
    updateFeedback();

    // let's force a value change to cover all the HTP/LTP cases
    m_levelValueChanged = true;
}

void VCSlider::slotClickAndGoLevelAndPresetChanged(uchar level, QImage img)
{
    setSliderValue(level, false, false);
    updateFeedback();

    QPixmap px = QPixmap::fromImage(img);
    m_cngButton->setIcon(px);
    m_levelValueChanged = true;
}

/*********************************************************************
 * Override reset button
 *********************************************************************/

void VCSlider::setOverrideResetKeySequence(const QKeySequence &keySequence)
{
    m_overrideResetKeySequence = QKeySequence(keySequence);
}

QKeySequence VCSlider::overrideResetKeySequence() const
{
    return m_overrideResetKeySequence;
}

void VCSlider::slotResetButtonClicked()
{
    m_isOverriding = false;
    m_resetButton->setStyleSheet(QString("QToolButton{ background: %1; }")
                                 .arg(m_slider->palette().window().color().name()));

    // request to delete all the active fader channels
    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->removeAll();
    }
    updateOverrideFeedback(false);

    emit monitorDMXValueChanged(m_monitorValue);
}

void VCSlider::slotKeyPressed(const QKeySequence &keySequence)
{
    if (isEnabled() == false)
        return;

    if (m_overrideResetKeySequence == keySequence)
        slotResetButtonClicked();
    else if (m_playbackFlashKeySequence == keySequence)
        flashPlayback(true);
}

void VCSlider::slotKeyReleased(const QKeySequence &keySequence)
{
    if (m_playbackFlashKeySequence == keySequence && m_playbackIsFlashing)
        flashPlayback(false);
}

/*********************************************************************
 * Flash button
 *********************************************************************/

QKeySequence VCSlider::playbackFlashKeySequence() const
{
    return m_playbackFlashKeySequence;
}

void VCSlider::setPlaybackFlashKeySequence(const QKeySequence &keySequence)
{
    m_playbackFlashKeySequence = QKeySequence(keySequence);
}

void VCSlider::mousePressEvent(QMouseEvent *e)
{
    VCWidget::mousePressEvent(e);

    if (mode() != Doc::Design && e->button() == Qt::LeftButton &&
        m_flashButton && m_flashButton->isDown())
    {
        flashPlayback(true);
    }
}

void VCSlider::mouseReleaseEvent(QMouseEvent *e)
{
    if (mode() == Doc::Design)
    {
        VCWidget::mouseReleaseEvent(e);
    }
    else if (m_playbackIsFlashing)
    {
        flashPlayback(false);
    }
}

/*****************************************************************************
 * Playback
 *****************************************************************************/

void VCSlider::setPlaybackFunction(quint32 fid)
{
    Function* old = m_doc->function(m_playbackFunction);
    if (old != NULL)
    {
        /* Get rid of old function connections */
        disconnect(old, SIGNAL(running(quint32)),
                this, SLOT(slotPlaybackFunctionRunning(quint32)));
        disconnect(old, SIGNAL(stopped(quint32)),
                this, SLOT(slotPlaybackFunctionStopped(quint32)));
        disconnect(old, SIGNAL(attributeChanged(int, qreal)),
                this, SLOT(slotPlaybackFunctionIntensityChanged(int, qreal)));
        if (old->type() == Function::SceneType)
        {
            disconnect(old, SIGNAL(flashing(quint32,bool)),
                       this, SLOT(slotPlaybackFunctionFlashing(quint32,bool)));
        }
    }

    Function* function = m_doc->function(fid);
    if (function != NULL)
    {
        /* Connect to the new function */
        connect(function, SIGNAL(running(quint32)),
                this, SLOT(slotPlaybackFunctionRunning(quint32)));
        connect(function, SIGNAL(stopped(quint32)),
                this, SLOT(slotPlaybackFunctionStopped(quint32)));
        connect(function, SIGNAL(attributeChanged(int, qreal)),
                this, SLOT(slotPlaybackFunctionIntensityChanged(int, qreal)));
        if (function->type() == Function::SceneType)
        {
            connect(function, SIGNAL(flashing(quint32,bool)),
                    this, SLOT(slotPlaybackFunctionFlashing(quint32,bool)));
        }

        m_playbackFunction = fid;
    }
    else
    {
        /* No function attachment */
        m_playbackFunction = Function::invalidId();
    }
}

quint32 VCSlider::playbackFunction() const
{
    return m_playbackFunction;
}

void VCSlider::setPlaybackValue(uchar value)
{
    if (m_externalMovement == true || value == m_playbackValue)
        return;

    QMutexLocker locker(&m_playbackValueMutex);
    m_playbackValue = value;
    m_playbackChangeCounter = 5;
}

uchar VCSlider::playbackValue() const
{
    return m_playbackValue;
}

void VCSlider::notifyFunctionStarting(quint32 fid, qreal functionIntensity)
{
    if (mode() == Doc::Design || sliderMode() != Playback)
        return;

    if (fid == playbackFunction())
        return;

    if (m_slider != NULL)
    {
        int value = SCALE(1.0 - functionIntensity, 0, 1.0,
                          m_slider->minimum(), m_slider->maximum());
        if (m_slider->value() > value)
        {
            m_externalMovement = true;
            m_slider->setValue(value);
            m_externalMovement = false;

            Function* function = m_doc->function(m_playbackFunction);
            if (function != NULL)
            {
                qreal pIntensity = qreal(value) / qreal(UCHAR_MAX);
                adjustFunctionIntensity(function, pIntensity * intensity());
                if (value == 0 && !function->stopped())
                    function->stop(functionParent());
            }
        }
    }
}

bool VCSlider::playbackFlashEnable() const
{
    return m_playbackFlashEnable;
}

void VCSlider::setPlaybackFlashEnable(bool enable)
{
    m_playbackFlashEnable = enable;

    if (enable == false && m_flashButton != NULL)
    {
        delete m_flashButton;
        m_flashButton = NULL;
    }
    else if (enable == true && m_flashButton == NULL)
    {
        m_flashButton = new FlashButton(this);
        m_flashButton->setIconSize(QSize(32, 32));
        m_flashButton->setStyle(AppUtil::saneStyle());
        m_flashButton->setIcon(QIcon(":/flash.png"));
        m_flashButton->setToolTip(tr("Flash Function"));
        layout()->addWidget(m_flashButton);
        layout()->setAlignment(m_flashButton, Qt::AlignHCenter);

        m_flashButton->show();
    }
}

void VCSlider::flashPlayback(bool on)
{
    if (on)
        m_playbackFlashPreviousValue = m_playbackValue;
    m_playbackIsFlashing = on;

    setPlaybackValue(on ? UCHAR_MAX : m_playbackFlashPreviousValue);
}

void VCSlider::slotPlaybackFunctionRunning(quint32 fid)
{
    Q_UNUSED(fid);
}

void VCSlider::slotPlaybackFunctionStopped(quint32 fid)
{
    if (fid != playbackFunction())
        return;

    m_externalMovement = true;
    if (m_slider)
        m_slider->setValue(0);
    resetIntensityOverrideAttribute();
    updateFeedback();
    m_externalMovement = false;
}

void VCSlider::slotPlaybackFunctionIntensityChanged(int attrIndex, qreal fraction)
{
    //qDebug() << "Function intensity changed" << attrIndex << fraction << m_playbackChangeCounter;

    if (attrIndex != Function::Intensity || m_playbackChangeCounter)
        return;

    m_externalMovement = true;
    if (m_slider)
        m_slider->setValue(int(floor((qreal(m_slider->maximum()) * fraction) + 0.5)));
    updateFeedback();
    m_externalMovement = false;
}

void VCSlider::slotPlaybackFunctionFlashing(quint32 fid, bool flashing)
{
    if (fid != playbackFunction())
        return;

    m_externalMovement = true;
    if (m_slider)
        m_slider->setValue(flashing ? m_slider->maximum() : m_slider->minimum());
    updateFeedback();
    m_externalMovement = false;
}

FunctionParent VCSlider::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

/*********************************************************************
 * Submaster
 *********************************************************************/

void VCSlider::emitSubmasterValue()
{
    Q_ASSERT(sliderMode() == Submaster);

    emit submasterValueChanged(SCALE(float(m_levelValue), float(0),
                float(UCHAR_MAX), float(0), float(1)) * intensity());
}

/*****************************************************************************
 * DMXSource
 *****************************************************************************/

void VCSlider::writeDMX(MasterTimer *timer, QList<Universe *> universes)
{
    if (sliderMode() == Level)
        writeDMXLevel(timer, universes);
    else if (sliderMode() == Playback)
        writeDMXPlayback(timer, universes);
}

void VCSlider::writeDMXLevel(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);

    QMutexLocker locker(&m_levelValueMutex);

    uchar modLevel = m_levelValue;
    int r = 0, g = 0, b = 0, c = 0, m = 0, y = 0;

    if (m_cngType == ClickAndGoWidget::RGB)
    {
        float f = 0;
        if (m_slider)
            f = SCALE(float(m_levelValue), float(m_slider->minimum()),
                      float(m_slider->maximum()), float(0), float(200));

        if (uchar(f) != 0)
        {
            QColor modColor = m_cngRGBvalue.lighter(uchar(f));
            r = modColor.red();
            g = modColor.green();
            b = modColor.blue();
        }
    }
    else if (m_cngType == ClickAndGoWidget::CMY)
    {
        float f = 0;
        if (m_slider)
            f = SCALE(float(m_levelValue), float(m_slider->minimum()),
                      float(m_slider->maximum()), float(0), float(200));
        if (uchar(f) != 0)
        {
            QColor modColor = m_cngRGBvalue.lighter(uchar(f));
            c = modColor.cyan();
            m = modColor.magenta();
            y = modColor.yellow();
        }
    }

    if (m_levelValueChanged)
    {
        QListIterator <LevelChannel> it(m_levelChannels);
        while (it.hasNext() == true)
        {
            LevelChannel lch(it.next());
            Fixture *fxi = m_doc->fixture(lch.fixture);
            if (fxi == NULL)
                continue;

            quint32 universe = fxi->universe();

            QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
            if (fader.isNull())
            {
                fader = universes[universe]->requestFader(m_monitorEnabled ? Universe::Override : Universe::Auto);
                fader->adjustIntensity(intensity());
                m_fadersMap[universe] = fader;
                if (m_monitorEnabled)
                {
                    qDebug() << "VC slider monitor enabled";
                    fader->setMonitoring(true);
                    connect(fader.data(), SIGNAL(preWriteData(quint32,QByteArray)),
                            this, SLOT(slotUniverseWritten(quint32,QByteArray)));
                }
            }

            FadeChannel *fc = fader->getChannelFader(m_doc, universes[universe], lch.fixture, lch.channel);
            if (fc->universe() == Universe::invalid())
            {
                fader->remove(fc);
                continue;
            }

            int chType = fc->flags();
            const QLCChannel *qlcch = fxi->channel(lch.channel);
            if (qlcch == NULL)
                continue;

            // set override flag if needed
            if (m_isOverriding)
                fc->addFlag(FadeChannel::Override);

            // request to autoremove LTP channels when set
            if (qlcch->group() != QLCChannel::Intensity)
                fc->addFlag(FadeChannel::AutoRemove);

            if (chType & FadeChannel::Intensity)
            {
                if (m_cngType == ClickAndGoWidget::RGB)
                {
                    if (qlcch->colour() == QLCChannel::Red)
                        modLevel = uchar(r);
                    else if (qlcch->colour() == QLCChannel::Green)
                        modLevel = uchar(g);
                    else if (qlcch->colour() == QLCChannel::Blue)
                        modLevel = uchar(b);
                }
                else if (m_cngType == ClickAndGoWidget::CMY)
                {
                    if (qlcch->colour() == QLCChannel::Cyan)
                        modLevel = uchar(c);
                    else if (qlcch->colour() == QLCChannel::Magenta)
                        modLevel = uchar(m);
                    else if (qlcch->colour() == QLCChannel::Yellow)
                        modLevel = uchar(y);
                }
            }

            fc->setStart(fc->current());
            fc->setTarget(modLevel);
            fc->setReady(false);
            fc->setElapsed(0);

            //qDebug() << "VC Slider write channel" << fc->target();
        }
    }
    m_levelValueChanged = false;
}

void VCSlider::writeDMXPlayback(MasterTimer* timer, QList<Universe *> ua)
{
    Q_UNUSED(ua);

    QMutexLocker locker(&m_playbackValueMutex);

    if (m_playbackChangeCounter == 0)
        return;

    Function* function = m_doc->function(m_playbackFunction);
    if (function == NULL || mode() == Doc::Design)
        return;

    uchar value = m_playbackValue;
    qreal pIntensity = qreal(value) / qreal(UCHAR_MAX);

    if (value == 0)
    {
        // Make sure we ignore the fade out time
        if (function->stopped() == false)
        {
            function->stop(functionParent());
            resetIntensityOverrideAttribute();
        }
    }
    else
    {
        if (function->stopped() == true)
        {
#if 0 // temporarily revert #699 until a better solution is found
            // Since this function is started by a fader, its fade in time
            // is decided by the fader movement.
            function->start(timer, functionParent(),
                            0, 0, Function::defaultSpeed(), Function::defaultSpeed());
#endif
            function->start(timer, functionParent());
        }
        adjustFunctionIntensity(function, pIntensity * intensity());
        emit functionStarting(m_playbackFunction, pIntensity);
    }
    m_playbackChangeCounter--;
}

/*****************************************************************************
 * Top label
 *****************************************************************************/

void VCSlider::setTopLabelText(int value)
{
    QString text;

    if (valueDisplayStyle() == ExactValue)
    {
        text = text.asprintf("%.3d", value);
    }
    else
    {

        float f = 0;
        if (m_slider)
            f = SCALE(float(value), float(m_slider->minimum()),
                      float(m_slider->maximum()), float(0), float(100));
        text = text.asprintf("%.3d%%", static_cast<int> (f));
    }
    m_topLabel->setText(text);
    emit valueChanged(text);
}

QString VCSlider::topLabelText()
{
    return m_topLabel->text();
}

/*****************************************************************************
 * Slider
 *****************************************************************************/

void VCSlider::setSliderValue(uchar value, bool scale, bool external)
{
    if (m_slider == NULL)
        return;

    float val = value;

    /* Scale from input value range to this slider's range */
    if (scale)
    {
        val = SCALE(float(value), float(0), float(UCHAR_MAX),
                float(m_slider->minimum()),
                float(m_slider->maximum()));
    }

    /* Request the UI to update */
    if (m_slider->isSliderDown() == false && val != m_slider->value())
       emit requestSliderUpdate(val);

    switch (sliderMode())
    {
        case Level:
        {
            if (m_monitorEnabled == true && m_isOverriding == false && m_slider->isSliderDown())
            {
                m_resetButton->setStyleSheet(QString("QToolButton{ background: red; }"));
                m_isOverriding = true;
                updateOverrideFeedback(true);
            }
            setLevelValue(val, external);
            setClickAndGoWidgetFromLevel(val);
        }
        break;

        case Playback:
        {
            setPlaybackValue(value);
        }
        break;

        case Submaster:
        {
            setLevelValue(val);
            emitSubmasterValue();
        }
        break;
    }
}

void VCSlider::setSliderShadowValue(int value)
{
    if (m_widgetMode == WSlider)
    {
        ClickAndGoSlider *sl = qobject_cast<ClickAndGoSlider*> (m_slider);
        sl->setShadowLevel(value);
    }
}

int VCSlider::sliderValue() const
{
    if (m_slider)
        return m_slider->value();

    return 0;
}

void VCSlider::setWidgetStyle(SliderWidgetStyle mode)
{
    if (mode == m_widgetMode)
        return;

    if (mode == WKnob)
    {
        qDebug() << "Switching to knob widget";
        disconnect(m_slider, SIGNAL(valueChanged(int)),
                this, SLOT(slotSliderMoved(int)));

        QLayoutItem* item;
        while ((item = m_hbox->takeAt(0)) != NULL)
        {
            delete item->widget();
            delete item;
        }

        m_slider = NULL;

        m_slider = new KnobWidget(this);
        m_slider->setEnabled(false);
        m_slider->setRange(levelLowLimit(), levelHighLimit());
        m_hbox->addWidget(m_slider);
        m_slider->show();
        connect(m_slider, SIGNAL(valueChanged(int)),
                this, SLOT(slotSliderMoved(int)));
    }
    else if (mode == WSlider)
    {
        qDebug() << "Switching to slider widget";
        disconnect(m_slider, SIGNAL(valueChanged(int)),
                this, SLOT(slotSliderMoved(int)));

        QLayoutItem* item;
        while ((item = m_hbox->takeAt(0)) != NULL)
        {
            delete item->widget();
            delete item;
        }

        m_slider = NULL;
        m_hbox->addStretch();
        m_slider = new ClickAndGoSlider(this);
        m_slider->setEnabled(false);
        m_slider->setRange(levelLowLimit(), levelHighLimit());
        m_hbox->addWidget(m_slider);
        m_slider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        m_slider->setMinimumWidth(32);
        m_slider->setMaximumWidth(80);
        m_slider->setStyleSheet(CNG_DEFAULT_STYLE);
        m_hbox->addStretch();
        m_slider->show();
        connect(m_slider, SIGNAL(valueChanged(int)),
                this, SLOT(slotSliderMoved(int)));
    }
    connect(this, SIGNAL(requestSliderUpdate(int)),
            m_slider, SLOT(setValue(int)));
    m_widgetMode = mode;
    update();
}

VCSlider::SliderWidgetStyle VCSlider::widgetStyle() const
{
    return m_widgetMode;
}

QString VCSlider::widgetStyleToString(VCSlider::SliderWidgetStyle style)
{
    if (style == VCSlider::WSlider)
        return QString("Slider");
    else if (style == VCSlider::WKnob)
        return QString("Knob");

    return QString();
}

VCSlider::SliderWidgetStyle VCSlider::stringToWidgetStyle(QString style)
{
    if (style == "Slider")
        return VCSlider::WSlider;
    else if (style == "Knob")
        return VCSlider::WKnob;

    return VCSlider::WSlider;
}

void VCSlider::updateFeedback()
{
    int fbv = 0;
    if (m_slider)
    {
        if (invertedAppearance() == true)
            fbv = m_slider->maximum() - m_slider->value() + m_slider->minimum();
        else
            fbv = m_slider->value();
        fbv = int(SCALE(float(fbv), float(m_slider->minimum()),
                        float(m_slider->maximum()), float(0), float(UCHAR_MAX)));
    }
    sendFeedback(fbv);
}

void VCSlider::updateOverrideFeedback(bool on)
{
    QSharedPointer<QLCInputSource> src = inputSource(overrideResetInputSourceId);
    if (!src.isNull() && src->isValid() == true)
        sendFeedback(src->feedbackValue(on ? QLCInputFeedback::UpperValue : QLCInputFeedback::LowerValue), overrideResetInputSourceId);
}

void VCSlider::slotSliderMoved(int value)
{
    /* Set text for the top label */
    setTopLabelText(value);

    /* Do the rest only if the slider is being moved by the user */
    if (m_slider->isSliderDown() == false)
        return;

    setSliderValue(value, false);

    updateFeedback();
}

/*****************************************************************************
 * Bottom label
 *****************************************************************************/
void VCSlider::setBottomLabelText(const QString& text)
{
    m_bottomLabel->setText(text);
}

QString VCSlider::bottomLabelText()
{
    return m_bottomLabel->text();
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCSlider::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    /* Don't let input data through in design mode or if disabled */
    if (acceptsInput() == false)
        return;

    quint32 pagedCh = (page() << 16) | channel;

    if (checkInputSource(universe, pagedCh, value, sender(), sliderInputSourceId))
    {
        if (m_slider)
        {
            /* When 'values catching" is enabled, controllers that do not have motorized faders
             * can catch up with the current slider value by entering a certain threshold
             * or by 'surpassing' the current value */
            if (catchValues())
            {
                uchar currentValue = sliderValue();

                // filter 'out of threshold' cases
                if (m_lastInputValue == -1 ||
                    (m_lastInputValue < currentValue - VALUE_CATCHING_THRESHOLD && value < currentValue - VALUE_CATCHING_THRESHOLD) ||
                    (m_lastInputValue > currentValue + VALUE_CATCHING_THRESHOLD && value > currentValue + VALUE_CATCHING_THRESHOLD))
                {
                    m_lastInputValue = value;
                    return;
                }
            }

            if (m_monitorEnabled == true && m_isOverriding == false)
            {
                m_resetButton->setStyleSheet(QString("QToolButton{ background: red; }"));
                m_isOverriding = true;
                updateOverrideFeedback(true);
            }

            if (invertedAppearance())
                value = UCHAR_MAX - value;

            setSliderValue(value, true, true);
            m_lastInputValue = value;
        }
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), overrideResetInputSourceId))
    {
        if (value > 0)
            slotResetButtonClicked();
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), flashButtonInputSourceId))
    {
        flashPlayback(value ? true : false);
    }
}

void VCSlider::adjustIntensity(qreal val)
{
    VCWidget::adjustIntensity(val);

    if (sliderMode() == Playback)
    {
        Function* function = m_doc->function(m_playbackFunction);
        if (function == NULL || mode() == Doc::Design)
            return;

        qreal pIntensity = qreal(m_playbackValue) / qreal(UCHAR_MAX);
        adjustFunctionIntensity(function, pIntensity * intensity());
    }
    else if (sliderMode() == Level)
    {
        foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
        {
            if (!fader.isNull())
                fader->adjustIntensity(val);
        }
    }
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCSlider::loadXML(QXmlStreamReader &root)
{
    bool visible = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    SliderMode sliderMode = Playback;
    QString str;

    if (root.name() != KXMLQLCVCSlider)
    {
        qWarning() << Q_FUNC_INFO << "Slider node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    QXmlStreamAttributes attrs = root.attributes();

    /* Widget style */
    if (attrs.hasAttribute(KXMLQLCVCSliderWidgetStyle))
        setWidgetStyle(stringToWidgetStyle(attrs.value(KXMLQLCVCSliderWidgetStyle).toString()));

    if (attrs.value(KXMLQLCVCSliderInvertedAppearance).toString() == "false")
        setInvertedAppearance(false);
    else
        setInvertedAppearance(true);

    /* Values catching */
    if (attrs.hasAttribute(KXMLQLCVCSliderCatchValues))
        setCatchValues(true);

    /* Children */
    while (root.readNextStartElement())
    {
        //qDebug() << "VC Slider tag:" << root.name();
        if (root.name() == KXMLQLCWindowState)
        {
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCSliderMode)
        {
            QXmlStreamAttributes mAttrs = root.attributes();
            sliderMode = stringToSliderMode(root.readElementText());

            str = mAttrs.value(KXMLQLCVCSliderValueDisplayStyle).toString();
            setValueDisplayStyle(stringToValueDisplayStyle(str));

            if (mAttrs.hasAttribute(KXMLQLCVCSliderClickAndGoType))
            {
                str = mAttrs.value(KXMLQLCVCSliderClickAndGoType).toString();
                setClickAndGoType(ClickAndGoWidget::stringToClickAndGoType(str));
            }

            if (mAttrs.hasAttribute(KXMLQLCVCSliderLevelMonitor))
            {
                if (mAttrs.value(KXMLQLCVCSliderLevelMonitor).toString() == "false")
                    setChannelsMonitorEnabled(false);
                else
                    setChannelsMonitorEnabled(true);
            }
        }
        else if (root.name() == KXMLQLCVCSliderOverrideReset)
        {
            QString str = loadXMLSources(root, overrideResetInputSourceId);
            if (str.isEmpty() == false)
                m_overrideResetKeySequence = stripKeySequence(QKeySequence(str));
        }
        else if (root.name() == KXMLQLCVCSliderLevel)
        {
            loadXMLLevel(root);
        }
        else if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInput(root);
        }
        else if (root.name() == KXMLQLCVCSliderPlayback)
        {
            loadXMLPlayback(root);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    /* Set the mode last, after everything else has been set */
    setSliderMode(sliderMode);

    return true;
}

bool VCSlider::loadXMLLevel(QXmlStreamReader &level_root)
{
    QString str;

    if (level_root.name() != KXMLQLCVCSliderLevel)
    {
        qWarning() << Q_FUNC_INFO << "Slider level node not found";
        return false;
    }

    QXmlStreamAttributes attrs = level_root.attributes();

    /* Level low limit */
    str = attrs.value(KXMLQLCVCSliderLevelLowLimit).toString();
    setLevelLowLimit(str.toInt());

    /* Level high limit */
    str = attrs.value(KXMLQLCVCSliderLevelHighLimit).toString();
    setLevelHighLimit(str.toInt());

    /* Level value */
    str = attrs.value(KXMLQLCVCSliderLevelValue).toString();
    setLevelValue(str.toInt());

    QXmlStreamReader::TokenType tType = level_root.readNext();

    if (tType == QXmlStreamReader::EndElement)
    {
        level_root.readNext();
        return true;
    }

    if (tType == QXmlStreamReader::Characters)
        tType = level_root.readNext();

    // check if there is a Channel tag defined
    if (tType == QXmlStreamReader::StartElement)
    {
        /* Children */
        do
        {
            if (level_root.name() == KXMLQLCVCSliderChannel)
            {
                /* Fixture & channel */
                str = level_root.attributes().value(KXMLQLCVCSliderChannelFixture).toString();
                addLevelChannel(
                    static_cast<quint32>(str.toInt()),
                    static_cast<quint32> (level_root.readElementText().toInt()));
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Unknown slider level tag:" << level_root.name().toString();
                level_root.skipCurrentElement();
            }
        } while (level_root.readNextStartElement());
    }

    return true;
}

bool VCSlider::loadXMLPlayback(QXmlStreamReader &pb_root)
{
    if (pb_root.name() != KXMLQLCVCSliderPlayback)
    {
        qWarning() << Q_FUNC_INFO << "Slider playback node not found";
        return false;
    }

    /* Children */
    while (pb_root.readNextStartElement())
    {
        if (pb_root.name() == KXMLQLCVCSliderPlaybackFunction)
        {
            /* Function */
            setPlaybackFunction(pb_root.readElementText().toUInt());
        }
        else if (pb_root.name() == KXMLQLCVCSliderPlaybackFlash)
        {
            setPlaybackFlashEnable(true);
            QString str = loadXMLSources(pb_root, flashButtonInputSourceId);
            if (str.isEmpty() == false)
                m_playbackFlashKeySequence = stripKeySequence(QKeySequence(str));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider playback tag:" << pb_root.name().toString();
            pb_root.skipCurrentElement();
        }
    }

    return true;
}

bool VCSlider::saveXML(QXmlStreamWriter *doc)
{
    QString str;

    Q_ASSERT(doc != NULL);

    /* VC Slider entry */
    doc->writeStartElement(KXMLQLCVCSlider);

    saveXMLCommon(doc);

    /* Widget style */
    doc->writeAttribute(KXMLQLCVCSliderWidgetStyle, widgetStyleToString(widgetStyle()));

    /* Inverted appearance */
    if (invertedAppearance() == true)
        doc->writeAttribute(KXMLQLCVCSliderInvertedAppearance, "true");
    else
        doc->writeAttribute(KXMLQLCVCSliderInvertedAppearance, "false");

    /* Values catching */
    if (catchValues() == true)
        doc->writeAttribute(KXMLQLCVCSliderCatchValues, "true");

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* External input */
    saveXMLInput(doc, inputSource(sliderInputSourceId));

    /* SliderMode */
    doc->writeStartElement(KXMLQLCVCSliderMode);

    /* Value display style */
    str = valueDisplayStyleToString(valueDisplayStyle());
    doc->writeAttribute(KXMLQLCVCSliderValueDisplayStyle, str);

    /* Click And Go type */
    str = ClickAndGoWidget::clickAndGoTypeToString(m_cngType);
    doc->writeAttribute(KXMLQLCVCSliderClickAndGoType, str);

    /* Monitor channels */
    if (sliderMode() == Level)
    {
        if (channelsMonitorEnabled() == true)
            doc->writeAttribute(KXMLQLCVCSliderLevelMonitor, "true");
        else
            doc->writeAttribute(KXMLQLCVCSliderLevelMonitor, "false");
    }

    doc->writeCharacters(sliderModeToString(m_sliderMode));

    /* End the <SliderMode> tag */
    doc->writeEndElement();

    if (sliderMode() == Level && channelsMonitorEnabled() == true)
    {
        doc->writeStartElement(KXMLQLCVCSliderOverrideReset);
        if (m_overrideResetKeySequence.toString().isEmpty() == false)
            doc->writeTextElement(KXMLQLCVCWidgetKey, m_overrideResetKeySequence.toString());
        saveXMLInput(doc, inputSource(overrideResetInputSourceId));
        doc->writeEndElement();
    }

    /* Level */
    doc->writeStartElement(KXMLQLCVCSliderLevel);
    /* Level low limit */
    doc->writeAttribute(KXMLQLCVCSliderLevelLowLimit, QString::number(levelLowLimit()));
    /* Level high limit */
    doc->writeAttribute(KXMLQLCVCSliderLevelHighLimit, QString::number(levelHighLimit()));
    /* Level value */
    doc->writeAttribute(KXMLQLCVCSliderLevelValue, QString::number(levelValue()));

    /* Level channels */
    QListIterator <LevelChannel> it(m_levelChannels);
    while (it.hasNext() == true)
    {
        LevelChannel lch(it.next());
        lch.saveXML(doc);
    }

    /* End the <Level> tag */
    doc->writeEndElement();

    /* Playback */
    doc->writeStartElement(KXMLQLCVCSliderPlayback);
    /* Playback function */
    doc->writeTextElement(KXMLQLCVCSliderPlaybackFunction, QString::number(playbackFunction()));

    if (sliderMode() == Playback && playbackFlashEnable() == true)
    {
        doc->writeStartElement(KXMLQLCVCSliderPlaybackFlash);
        if (m_playbackFlashKeySequence.toString().isEmpty() == false)
            doc->writeTextElement(KXMLQLCVCWidgetKey, m_playbackFlashKeySequence.toString());
        saveXMLInput(doc, inputSource(flashButtonInputSourceId));
        doc->writeEndElement();
    }

    /* End the <Playback> tag */
    doc->writeEndElement();

    /* End the <Slider> tag */
    doc->writeEndElement();

    return true;
}

/****************************************************************************
 * LevelChannel implementation
 ****************************************************************************/

VCSlider::LevelChannel::LevelChannel(quint32 fid, quint32 ch)
{
    this->fixture = fid;
    this->channel = ch;
}

VCSlider::LevelChannel::LevelChannel(const LevelChannel& lc)
{
    *this = lc;
}

VCSlider::LevelChannel &VCSlider::LevelChannel::operator=(const VCSlider::LevelChannel &lc)
{
    if (this != &lc)
    {
        this->fixture = lc.fixture;
        this->channel = lc.channel;
    }

    return *this;
}

bool VCSlider::LevelChannel::operator==(const LevelChannel& lc) const
{
    return (this->fixture == lc.fixture && this->channel == lc.channel);
}

bool VCSlider::LevelChannel::operator<(const LevelChannel& lc) const
{
    if (this->fixture < lc.fixture)
        return true;
    else if (this->fixture == lc.fixture && this->channel < lc.channel)
        return true;
    else
        return false;
}

void VCSlider::LevelChannel::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCVCSliderChannel);

    doc->writeAttribute(KXMLQLCVCSliderChannelFixture,
                       QString::number(this->fixture));

    doc->writeCharacters(QString::number(this->channel));
    doc->writeEndElement();
}

void VCSlider::FlashButton::mousePressEvent(QMouseEvent *e)
{
    QToolButton::mousePressEvent(e);
    // ignore event so that it can be
    // forwarded to parent widget
    e->ignore();
}

void VCSlider::FlashButton::mouseReleaseEvent(QMouseEvent *e)
{
    QToolButton::mouseReleaseEvent(e);
    // ignore event so that it can be
    // forwarded to parent widget
    e->ignore();
}
