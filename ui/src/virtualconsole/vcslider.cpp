/*
  Q Light Controller
  vcslider.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QString>
#include <QSlider>
#include <QDebug>
#include <QLabel>
#include <QMenu>
#include <QSize>
#include <QtXml>
#include <QPen>

#include "vcsliderproperties.h"
#include "vcpropertieseditor.h"
#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "qlcinputsource.h"
#include "mastertimer.h"
#include "collection.h"
#include "inputpatch.h"
#include "qlcmacros.h"
#include "universe.h"
#include "vcslider.h"
#include "qlcfile.h"
#include "apputil.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

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

VCSlider::VCSlider(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
{
    /* Set the class name "VCSlider" as the object name as well */
    setObjectName(VCSlider::staticMetaObject.className());

    m_hbox = NULL;
    m_topLabel = NULL;
    m_slider = NULL;
    m_bottomLabel = NULL;

    m_valueDisplayStyle = ExactValue;

    m_levelLowLimit = 0;
    m_levelHighLimit = UCHAR_MAX;

    m_levelValue = 0;
    m_levelValueChanged = false;
    m_monitorEnabled = false;
    m_monitorValue = 0;

    m_playbackFunction = Function::invalidId();
    m_playbackValue = 0;
    m_playbackValueChanged = false;

    m_submasterValue = UCHAR_MAX;

    m_widgetMode = WSlider;

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
    m_externalMovement = false;

    /* Put stretchable space after the slider (to its right side) */
    m_hbox->addStretch();

    layout()->addItem(m_hbox);

    /* Click & Go button */
    m_cngType = ClickAndGoWidget::None;

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
        {
            m_cngButton->show();
        }
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

        if (sliderMode() == Playback)
        {
            /* Follow playback function running/stopped status in case the
               function is started from another control. */
            Function* function = m_doc->function(playbackFunction());
            if (function != NULL)
            {
                connect(function, SIGNAL(running(quint32)),
                        this, SLOT(slotPlaybackFunctionRunning(quint32)));
                connect(function, SIGNAL(stopped(quint32)),
                        this, SLOT(slotPlaybackFunctionStopped(quint32)));
                connect(function, SIGNAL(attributeChanged(int, qreal)),
                        this, SLOT(slotPlaybackFunctionIntensityChanged(int, qreal)));
            }
        }
    }
    else
    {
        enableWidgetUI(false);

        if (sliderMode() == Playback)
        {
            /* Stop following playback function running/stopped status in case
               the function is changed in Design mode to another. */
            Function* function = m_doc->function(playbackFunction());
            if (function != NULL)
            {
                disconnect(function, SIGNAL(running(quint32)),
                        this, SLOT(slotPlaybackFunctionRunning(quint32)));
                disconnect(function, SIGNAL(stopped(quint32)),
                        this, SLOT(slotPlaybackFunctionStopped(quint32)));
                disconnect(function, SIGNAL(attributeChanged(int,qreal)),
                        this, SLOT(slotPlaybackFunctionIntensityChanged(int, qreal)));
            }
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

/*****************************************************************************
 * Slider Mode
 *****************************************************************************/

QString VCSlider::sliderModeToString(SliderMode mode)
{
    switch (mode)
    {
    case Level:
        return QString("Level");
        break;

    case Playback:
        return QString("Playback");
        break;

    case Submaster:
        return QString("Submaster");
        break;

    default:
        return QString("Unknown");
        break;
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

    /* Unregister this as a DMX source if the new mode is not "Level" or "Playback" */
    if ((m_sliderMode == Level && mode != Level) ||
        (m_sliderMode == Playback && mode != Playback))
    {
        m_doc->masterTimer()->unregisterDMXSource(this);
    }

    m_sliderMode = mode;

    if (mode == Level)
    {
        /* Set the slider range */
        uchar level = levelValue();
        if (m_slider)
        {
            m_slider->setRange(levelLowLimit(), levelHighLimit());
            m_slider->setValue(level);
            if (m_widgetMode == WSlider)
                m_slider->setStyleSheet(CNG_DEFAULT_STYLE);
        }
        slotSliderMoved(level);

        m_bottomLabel->show();
        if (m_cngType != ClickAndGoWidget::None)
        {
            setClickAndGoType(m_cngType);
            setupClickAndGoWidget();
            m_cngButton->show();
            if (m_slider)
                setClickAndGoWidgetFromLevel(m_slider->value());
        }

        m_doc->masterTimer()->registerDMXSource(this, "Slider");
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

        m_doc->masterTimer()->registerDMXSource(this, "Slider");
    }
    else if (mode == Submaster)
    {
        m_monitorEnabled = false;
        uchar level = levelValue();
        if (m_slider)
        {
            if (m_widgetMode == WSlider)
                m_slider->setStyleSheet(submasterStyleSheet);
            m_slider->setValue(level);
        }
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
        qSort(m_levelChannels.begin(), m_levelChannels.end());
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
}

uchar VCSlider::levelLowLimit() const
{
    return m_levelLowLimit;
}

void VCSlider::setLevelHighLimit(uchar value)
{
    m_levelHighLimit = value;
}

uchar VCSlider::levelHighLimit() const
{
    return m_levelHighLimit;
}

void VCSlider::setChannelsMonitorEnabled(bool enable)
{
    m_monitorEnabled = enable;
}

bool VCSlider::channelsMonitorEnabled()
{
    return m_monitorEnabled;
}

void VCSlider::setLevelValue(uchar value)
{
    m_levelValueMutex.lock();
    m_levelValue = value;
    if (m_monitorEnabled == true)
        m_monitorValue = m_levelValue;
    m_levelValueChanged = true;
    m_levelValueMutex.unlock();
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
    if (value != sliderValue())
    {
        if (invertedAppearance())
            m_monitorValue = 255 - value;
        else
            m_monitorValue = value;
        m_levelValueMutex.lock();
        m_levelValue = m_monitorValue;
        m_levelValueMutex.unlock();
        if (m_slider)
            m_slider->blockSignals(true);
        setSliderValue(m_monitorValue, true);
        setTopLabelText(sliderValue());
        if (m_slider)
            m_slider->blockSignals(false);
        updateFeedback();
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
    if (m_slider)
        m_slider->setValue(level);

    QColor col = m_cngWidget->getColorAt(level);
    QPixmap px(42, 42);
    px.fill(col);
    m_cngButton->setIcon(px);
}

void VCSlider::slotClickAndGoColorChanged(QRgb color)
{
    QColor col(color);
    m_cngRGBvalue = col;
    QPixmap px(42, 42);
    px.fill(col);
    m_cngButton->setIcon(px);

    // place the slider half way to reach white@255 and black@0
    if (m_slider)
        m_slider->setValue(128);
}

void VCSlider::slotClickAndGoLevelAndPresetChanged(uchar level, QImage img)
{
    if (m_slider)
        m_slider->setValue(level);

    QPixmap px = QPixmap::fromImage(img);
    m_cngButton->setIcon(px);
}

/*****************************************************************************
 * Playback
 *****************************************************************************/

void VCSlider::setPlaybackFunction(quint32 fid)
{
    m_playbackFunction = fid;
}

quint32 VCSlider::playbackFunction() const
{
    return m_playbackFunction;
}

void VCSlider::setPlaybackValue(uchar value)
{
    if (m_externalMovement == true)
        return;

    m_playbackValueMutex.lock();
    m_playbackValue = value;
    m_playbackValueChanged = true;
    m_playbackValueMutex.unlock();
}

uchar VCSlider::playbackValue() const
{
    return m_playbackValue;
}

void VCSlider::notifyFunctionStarting(quint32 fid)
{
    if (mode() == Doc::Design || sliderMode() != Playback)
        return;

    if (fid == m_playbackFunction)
        return;

    if (m_slider != NULL)
        m_slider->setValue(0);
}

void VCSlider::slotPlaybackFunctionRunning(quint32 fid)
{
    Q_UNUSED(fid);
}

void VCSlider::slotPlaybackFunctionStopped(quint32 fid)
{
    m_externalMovement = true;
    if (fid == playbackFunction())
    {
        if (m_slider)
            m_slider->setValue(0);
    }
    m_externalMovement = false;
}

void VCSlider::slotPlaybackFunctionIntensityChanged(int attrIndex, qreal fraction)
{
    if (attrIndex != 0)
        return;

    m_externalMovement = true;
    if (m_slider)
        m_slider->setValue(int(floor((qreal(m_slider->maximum()) * fraction) + 0.5)));
    m_externalMovement = false;
}

/*****************************************************************************
 * DMXSource
 *****************************************************************************/

void VCSlider::writeDMX(MasterTimer* timer, QList<Universe *> universes)
{
    if (sliderMode() == Level)
        writeDMXLevel(timer, universes);
    else if (sliderMode() == Playback)
        writeDMXPlayback(timer, universes);
}

void VCSlider::writeDMXLevel(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);

    QMutexLocker locker(&m_levelValueMutex);

    uchar modLevel = m_levelValue;
    bool mixedDMXlevels = false;
    int monitorSliderValue = -1;

    int r = 0, g = 0, b = 0, c = 0, m = 0, y = 0;

    if (m_cngType == ClickAndGoWidget::RGB)
    {
        float f = 0;
        if (m_slider)
            f = SCALE(float(m_levelValue), float(m_slider->minimum()),
                      float(m_slider->maximum()), float(0), float(200));

        if ((uchar)f != 0)
        {
            QColor modColor = m_cngRGBvalue.lighter((uchar)f);
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
        if ((uchar)f != 0)
        {
            QColor modColor = m_cngRGBvalue.lighter((uchar)f);
            c = modColor.cyan();
            m = modColor.magenta();
            y = modColor.yellow();
        }
    }

    if (m_monitorEnabled == true && m_levelValueChanged == false)
    {
        QListIterator <LevelChannel> it(m_levelChannels);
        while (it.hasNext() == true)
        {
            LevelChannel lch(it.next());
            Fixture* fxi = m_doc->fixture(lch.fixture);
            if (fxi != NULL)
            {
                const QLCChannel* qlcch = fxi->channel(lch.channel);
                if (qlcch == NULL)
                    continue;

                quint32 dmx_ch = fxi->address() + lch.channel;
                int uni = fxi->universe();
                if (uni < universes.count())
                {
                    uchar chValue = universes[uni]->preGMValue(dmx_ch);
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
            }
        }

        // check if all the DMX channels controlled by this slider
        // have the same value. If so, move the widget slider or knob
        // to the detected position
        if (mixedDMXlevels == false &&
            monitorSliderValue != m_monitorValue)
        {
            emit monitorDMXValueChanged(monitorSliderValue);
            // return here. At the next call of this method,
            // the monitor level will kick in
            return;
        }
    }

    QListIterator <LevelChannel> it(m_levelChannels);
    while (it.hasNext() == true)
    {
        LevelChannel lch(it.next());
        Fixture* fxi = m_doc->fixture(lch.fixture);
        if (fxi != NULL)
        {
            const QLCChannel* qlcch = fxi->channel(lch.channel);
            if (qlcch == NULL)
                continue;

            quint32 dmx_ch = fxi->address() + lch.channel;
            int uni = fxi->universe();

            if (qlcch->group() != QLCChannel::Intensity &&
                m_levelValueChanged == false)
            {
                /* Value has not changed and this is not an intensity channel.
                   LTP in effect. */
                continue;
            }
            if (qlcch->group() == QLCChannel::Intensity)
            {
                if (m_cngType == ClickAndGoWidget::RGB)
                {
                    if (qlcch->colour() == QLCChannel::Red)
                        modLevel = (uchar)r;
                    else if (qlcch->colour() == QLCChannel::Green)
                        modLevel = (uchar)g;
                    else if (qlcch->colour() == QLCChannel::Blue)
                        modLevel = (uchar)b;
                }
                else if (m_cngType == ClickAndGoWidget::CMY)
                {
                    if (qlcch->colour() == QLCChannel::Cyan)
                        modLevel = (uchar)c;
                    else if (qlcch->colour() == QLCChannel::Magenta)
                        modLevel = (uchar)m;
                    else if (qlcch->colour() == QLCChannel::Yellow)
                        modLevel = (uchar)y;
                }
            }

            if (uni < universes.count())
                universes[uni]->write(dmx_ch, modLevel * intensity());
        }
    }
    m_levelValueChanged = false;
}

void VCSlider::writeDMXPlayback(MasterTimer* timer, QList<Universe *> ua)
{
    Q_UNUSED(ua);

    Function* function = m_doc->function(m_playbackFunction);
    if (function == NULL || mode() == Doc::Design)
        return;

    /* Grab current values inside a locked mutex */
    m_playbackValueMutex.lock();
    uchar value = m_playbackValue;
    bool changed = m_playbackValueChanged;
    qreal pIntensity = qreal(value) / qreal(UCHAR_MAX);
    m_playbackValueChanged = false;
    m_playbackValueMutex.unlock();

    if (changed == true)
    {
        if (value == 0)
        {
            if (function->stopped() == false)
                function->stop();
        }
        else
        {
            if (function->stopped() == true)
            {
                function->start(timer);
                emit functionStarting(m_playbackFunction);
            }
            function->adjustAttribute(pIntensity * intensity(), Function::Intensity);
        }
    }
}

/*****************************************************************************
 * Top label
 *****************************************************************************/

void VCSlider::setTopLabelText(int value)
{
    QString text;

    if (valueDisplayStyle() == ExactValue)
    {
        text.sprintf("%.3d", value);
    }
    else
    {

        float f = 0;
        if (m_slider)
            f = SCALE(float(value), float(m_slider->minimum()),
                      float(m_slider->maximum()), float(0), float(100));
        text.sprintf("%.3d%%", static_cast<int> (f));
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

void VCSlider::setSliderValue(uchar value, bool noScale)
{
    float val = value;

    if (m_slider)
    {
        /* Scale from input value range to this slider's range */
        if (!noScale)
        {
            val = SCALE((float) value, (float) 0, (float) UCHAR_MAX,
                    (float) m_slider->minimum(),
                    (float) m_slider->maximum());
        }

        if (m_slider->invertedAppearance() == true)
            m_slider->setValue(m_slider->maximum() - (int) val);
        else
            m_slider->setValue((int) val);
    }
}

int VCSlider::sliderValue() const
{
    if (m_slider)
    {
        if (invertedAppearance())
            return 255 - m_slider->value();
        else
            return m_slider->value();
    }

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
        while ( ( item = m_hbox->takeAt( 0 ) ) != NULL )
        {
            delete item->widget();
            delete item;
        }

        m_slider = NULL;

        m_slider = new KnobWidget(this);
        m_slider->setEnabled(false);
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
        while ( ( item = m_hbox->takeAt( 0 ) ) != NULL )
        {
            delete item->widget();
            delete item;
        }

        m_slider = NULL;
        m_hbox->addStretch();
        m_slider = new ClickAndGoSlider(this);
        m_slider->setEnabled(false);
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
            fbv = m_slider->maximum() - m_slider->value();
        else
            fbv = m_slider->value();
        fbv = (int)SCALE(float(fbv), float(m_slider->minimum()),
                         float(m_slider->maximum()), float(0), float(UCHAR_MAX));
    }
    sendFeedback(fbv);
}

void VCSlider::slotSliderMoved(int value)
{
    switch (sliderMode())
    {
    case Level:
    {
        setLevelValue(value);
        setClickAndGoWidgetFromLevel(value);
    }
    break;

    case Playback:
    {
        setPlaybackValue(value);
    }
    break;

    case Submaster:
    {
        float f = 0;
        if (m_slider)
            f = SCALE(float(value), float(m_slider->minimum()),
                      float(m_slider->maximum()), float(0), float(1));
        setLevelValue(value);
        emit submasterValueChanged((qreal)f * intensity());
    }
    break;

    default:
        break;
    }

    /* Set text for the top label */
    setTopLabelText(value);

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

void VCSlider::slotInputValueChanged(quint32 universe, quint32 channel,
                                     uchar value)
{
    /* Don't let input data thru in design mode */
    if (mode() == Doc::Design || isEnabled() == false)
        return;

    if (checkInputSource(universe, (page() << 16) | channel, value, sender()))
    {
        /* Scale from input value range to this slider's range */
        float val;
        if (m_slider)
        {
            val = SCALE((float) value, (float) 0, (float) UCHAR_MAX,
                        (float) m_slider->minimum(),
                        (float) m_slider->maximum());

            if (m_slider->invertedAppearance() == true)
                m_slider->setValue((m_slider->maximum() - (int) val) + m_slider->minimum());
            else
                m_slider->setValue((int) val);
        }
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
        function->adjustAttribute(pIntensity * intensity(), Function::Intensity);
    }
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCSlider::loadXML(const QDomElement* root)
{
    bool visible = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    SliderMode sliderMode = Playback;
    QDomElement tag;
    QDomNode node;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCSlider)
    {
        qWarning() << Q_FUNC_INFO << "Slider node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Widget style */
    if (root->hasAttribute(KXMLQLCVCSliderWidgetStyle))
        setWidgetStyle(stringToWidgetStyle(root->attribute(KXMLQLCVCSliderWidgetStyle)));

    if (root->attribute(KXMLQLCVCSliderInvertedAppearance) == "false")
        setInvertedAppearance(false);
    else
        setInvertedAppearance(true);

    /* Children */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCSliderMode)
        {
            sliderMode = stringToSliderMode(tag.text());

            str = tag.attribute(KXMLQLCVCSliderValueDisplayStyle);
            setValueDisplayStyle(stringToValueDisplayStyle(str));

            if (tag.hasAttribute(KXMLQLCVCSliderClickAndGoType))
            {
                str = tag.attribute(KXMLQLCVCSliderClickAndGoType);
                setClickAndGoType(ClickAndGoWidget::stringToClickAndGoType(str));
            }

            if (tag.hasAttribute(KXMLQLCVCSliderLevelMonitor))
            {
                if (tag.attribute(KXMLQLCVCSliderLevelMonitor) == "false")
                    setChannelsMonitorEnabled(false);
                else
                    setChannelsMonitorEnabled(true);
            }
        }
        else if (tag.tagName() == KXMLQLCVCSliderLevel)
        {
            loadXMLLevel(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetInput)
        {
            loadXMLInput(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCSliderPlayback)
        {
            loadXMLPlayback(&tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    /* Set the mode last, after everything else has been set */
    setSliderMode(sliderMode);

    return true;
}

bool VCSlider::loadXMLLevel(const QDomElement* level_root)
{
    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(level_root != NULL);

    if (level_root->tagName() != KXMLQLCVCSliderLevel)
    {
        qWarning() << Q_FUNC_INFO << "Slider level node not found";
        return false;
    }

    /* Level low limit */
    str = level_root->attribute(KXMLQLCVCSliderLevelLowLimit);
    setLevelLowLimit(str.toInt());

    /* Level high limit */
    str = level_root->attribute(KXMLQLCVCSliderLevelHighLimit);
    setLevelHighLimit(str.toInt());

    /* Level value */
    str = level_root->attribute(KXMLQLCVCSliderLevelValue);
    setLevelValue(str.toInt());

    /* Children */
    node = level_root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCSliderChannel)
        {
            /* Fixture & channel */
            str = tag.attribute(KXMLQLCVCSliderChannelFixture);
            addLevelChannel(
                static_cast<quint32>(str.toInt()),
                static_cast<quint32> (tag.text().toInt()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider level tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCSlider::loadXMLPlayback(const QDomElement* pb_root)
{
    QDomNode node;
    QDomElement tag;

    Q_ASSERT(pb_root != NULL);

    if (pb_root->tagName() != KXMLQLCVCSliderPlayback)
    {
        qWarning() << Q_FUNC_INFO << "Slider playback node not found";
        return false;
    }

    /* Children */
    node = pb_root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCSliderPlaybackFunction)
        {
            /* Function */
            setPlaybackFunction(tag.text().toUInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider playback tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCSlider::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomElement subtag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC Slider entry */
    root = doc->createElement(KXMLQLCVCSlider);
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Widget style */
    root.setAttribute(KXMLQLCVCSliderWidgetStyle, widgetStyleToString(widgetStyle()));

    /* Inverted appearance */
    if (invertedAppearance() == true)
        root.setAttribute(KXMLQLCVCSliderInvertedAppearance, "true");
    else
        root.setAttribute(KXMLQLCVCSliderInvertedAppearance, "false");

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    /* External input */
    saveXMLInput(doc, &root);

    /* Mode */
    tag = doc->createElement(KXMLQLCVCSliderMode);
    root.appendChild(tag);
    text = doc->createTextNode(sliderModeToString(m_sliderMode));
    tag.appendChild(text);

    /* Value display style */
    str = valueDisplayStyleToString(valueDisplayStyle());
    tag.setAttribute(KXMLQLCVCSliderValueDisplayStyle, str);

    /* Click And Go type */
    str = ClickAndGoWidget::clickAndGoTypeToString(m_cngType);
    tag.setAttribute(KXMLQLCVCSliderClickAndGoType, str);

    /* Monitor channels */
    if (sliderMode() == Level)
    {
        if (channelsMonitorEnabled() == true)
            tag.setAttribute(KXMLQLCVCSliderLevelMonitor, "true");
        else
            tag.setAttribute(KXMLQLCVCSliderLevelMonitor, "false");
    }

    /* Level */
    tag = doc->createElement(KXMLQLCVCSliderLevel);
    root.appendChild(tag);

    /* Level low limit */
    str.setNum(levelLowLimit());
    tag.setAttribute(KXMLQLCVCSliderLevelLowLimit, str);

    /* Level high limit */
    str.setNum(levelHighLimit());
    tag.setAttribute(KXMLQLCVCSliderLevelHighLimit, str);

    /* Level value */
    str.setNum(levelValue());
    tag.setAttribute(KXMLQLCVCSliderLevelValue, str);

    /* Level channels */
    QListIterator <LevelChannel> it(m_levelChannels);
    while (it.hasNext() == true)
    {
        LevelChannel lch(it.next());
        lch.saveXML(doc, &tag);
    }

    /* Playback */
    tag = doc->createElement(KXMLQLCVCSliderPlayback);
    root.appendChild(tag);

    /* Playback function */
    subtag = doc->createElement(KXMLQLCVCSliderPlaybackFunction);
    text = doc->createTextNode(QString::number(playbackFunction()));
    subtag.appendChild(text);
    tag.appendChild(subtag);

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
    this->fixture = lc.fixture;
    this->channel = lc.channel;
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

void VCSlider::LevelChannel::saveXML(QDomDocument* doc, QDomElement* root) const
{
    QDomElement chtag = doc->createElement(KXMLQLCVCSliderChannel);
    root->appendChild(chtag);

    chtag.setAttribute(KXMLQLCVCSliderChannelFixture,
                       QString::number(this->fixture));

    QDomText text = doc->createTextNode(QString::number(this->channel));
    chtag.appendChild(text);
}
