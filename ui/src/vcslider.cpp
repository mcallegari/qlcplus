/*
  Q Light Controller
  vcslider.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QWidgetAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QString>
#include <QSlider>
#include <QDebug>
#include <QLabel>
#include <QMenu>
#include <QTime>
#include <QSize>
#include <QtXml>
#include <QPen>

#include "vcsliderproperties.h"
#include "vcpropertieseditor.h"
#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "qlcinputsource.h"
#include "universearray.h"
#include "mastertimer.h"
#include "collection.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "vcslider.h"
#include "qlcmacros.h"
#include "qlcfile.h"
#include "apputil.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

const QSize VCSlider::defaultSize(QSize(60, 200));

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
    m_tapButton = NULL;

    m_valueDisplayStyle = ExactValue;

    m_levelLowLimit = 0;
    m_levelHighLimit = UCHAR_MAX;

    m_levelValue = 0;
    m_levelValueChanged = false;

    m_playbackFunction = Function::invalidId();
    m_playbackValue = 0;
    m_playbackValueChanged = false;

    m_time = NULL;

    setType(VCWidget::SliderWidget);
    setCaption(QString());
    setFrameStyle(KVCFrameStyleSunken);

    /* Main VBox */
    new QVBoxLayout(this);

    /* Top label */
    m_topLabel = new QLabel(this);
    layout()->addWidget(m_topLabel);
    m_topLabel->setAlignment(Qt::AlignHCenter);

    /* Slider's HBox |stretch|slider|stretch| */
    m_hbox = new QHBoxLayout();
    layout()->addItem(m_hbox);

    /* Put stretchable space before the slider (to its left side) */
    m_hbox->addStretch();

    /* The slider */
    m_slider = new ClickAndGoSlider(this);

    m_hbox->addWidget(m_slider);
    m_slider->setRange(0, 255);
    m_slider->setPageStep(1);
    m_slider->setInvertedAppearance(false);
#if 1
    m_slider->setStyle(AppUtil::saneStyle());
#else
    m_slider->setStyleSheet("QSlider::handle:vertical {"
                          "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #a4a4a4, stop:1 #4f4f4f);"
                          "border: 1px solid #5c5c5c;"
                          "border-radius: 3px;"
                          "margin: 0 -20px;"
                          "}");
#endif
    connect(m_slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderMoved(int)));
    m_externalMovement = false;

    /* Put stretchable space after the slider (to its right side) */
    m_hbox->addStretch();

    /* Tap button */
    m_tapButton = new QPushButton(this);
    layout()->addWidget(m_tapButton);
    connect(m_tapButton, SIGNAL(clicked()),
            this, SLOT(slotTapButtonClicked()));
    m_time = new QTime();

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
    slotModeChanged(mode());

    /* Listen to fixture removals so that LevelChannels can be removed when
       they no longer point to an existing fixture->channel */
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotFixtureRemoved(quint32)));
}

VCSlider::~VCSlider()
{
    if (m_time != NULL)
        delete m_time;
    m_time = NULL;

    /* When application exits these are already NULL and unregistration
       is no longer necessary. But a normal deletion of a VCSlider in
       design mode must unregister the slider. */
    m_doc->masterTimer()->unregisterDMXSource(this);
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

bool VCSlider::copyFrom(VCWidget* widget)
{
    VCSlider* slider = qobject_cast<VCSlider*> (widget);
    if (slider == NULL)
        return false;

    /* Copy level stuff */
    setLevelLowLimit(slider->levelLowLimit());
    setLevelHighLimit(slider->levelHighLimit());
    m_levelChannels = slider->m_levelChannels;

    /* Copy playback stuff */
    m_playbackFunction = slider->m_playbackFunction;

    /* Copy slider appearance */
    setValueDisplayStyle(slider->valueDisplayStyle());
    setInvertedAppearance(slider->invertedAppearance());

    /* Copy mode & current value */
    setSliderMode(slider->sliderMode());
    m_slider->setValue(slider->sliderValue());

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCSlider::setCaption(const QString& text)
{
    VCWidget::setCaption(text);

    if (m_bottomLabel != NULL)
        setBottomLabelText(text);

    if (m_tapButton != NULL)
        setTapButtonText(text);
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
        m_topLabel->setEnabled(true);
        m_slider->setEnabled(true);
        m_bottomLabel->setEnabled(true);
        m_tapButton->setEnabled(true);
        m_cngButton->setEnabled(true);

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
                connect(function, SIGNAL(intensityChanged(qreal)),
                        this, SLOT(slotPlaybackFunctionIntensityChanged(qreal)));
            }
        }
    }
    else
    {
        m_topLabel->setEnabled(false);
        m_slider->setEnabled(false);
        m_bottomLabel->setEnabled(false);
        m_tapButton->setEnabled(false);
        m_cngButton->setEnabled(false);

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
                disconnect(function, SIGNAL(intensityChanged(qreal)),
                        this, SLOT(slotPlaybackFunctionIntensityChanged(qreal)));
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
}

VCSlider::ValueDisplayStyle VCSlider::valueDisplayStyle()
{
    return m_valueDisplayStyle;
}

/*****************************************************************************
 * Inverted appearance
 *****************************************************************************/

bool VCSlider::invertedAppearance() const
{
    Q_ASSERT(m_slider != NULL);
    return m_slider->invertedAppearance();
}

void VCSlider::setInvertedAppearance(bool invert)
{
    Q_ASSERT(m_slider != NULL);
    m_slider->setInvertedAppearance(invert);
    m_slider->setInvertedControls(invert);
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

    default:
        return QString("Unknown");
        break;
    }
}

VCSlider::SliderMode VCSlider::stringToSliderMode(const QString& mode)
{
    if (mode == QString("Level"))
        return Level;
    else // if (mode == QString("Playback"))
        return Playback;
}

VCSlider::SliderMode VCSlider::sliderMode()
{
    return m_sliderMode;
}

void VCSlider::setSliderMode(SliderMode mode)
{
    Q_ASSERT(mode >= Level && mode <= Playback);

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
        m_slider->setRange(levelLowLimit(), levelHighLimit());
        m_slider->setValue(level);
        slotSliderMoved(level);

        m_bottomLabel->show();
        m_tapButton->hide();
        if (m_cngType != ClickAndGoWidget::None)
        {
            setClickAndGoType(m_cngType);
            setupClickAndGoWidegt();
            m_cngButton->show();
            setClickAndGoWidgetFromLevel(m_slider->value());
        }

        m_doc->masterTimer()->registerDMXSource(this);
    }
    else if (mode == Playback)
    {
        m_bottomLabel->show();
        m_tapButton->hide();
        m_cngButton->hide();

        uchar level = playbackValue();
        m_slider->setRange(0, UCHAR_MAX);
        m_slider->setValue(level);
        slotSliderMoved(level);

        m_doc->masterTimer()->registerDMXSource(this);
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

uchar VCSlider::levelLowLimit()
{
    return m_levelLowLimit;
}

void VCSlider::setLevelHighLimit(uchar value)
{
    m_levelHighLimit = value;
}

uchar VCSlider::levelHighLimit()
{
    return m_levelHighLimit;
}

void VCSlider::setLevelValue(uchar value)
{
    m_levelValueMutex.lock();
    m_levelValue = value;
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

/*********************************************************************
 * Click & Go
 *********************************************************************/

void VCSlider::setClickAndGoType(ClickAndGoWidget::ClickAndGo type)
{
    m_cngType = type;
}

ClickAndGoWidget::ClickAndGo VCSlider::getClickAndGoType()
{
    return m_cngType;
}

void VCSlider::setupClickAndGoWidegt()
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
        float f = SCALE(float(level),
                        float(m_slider->minimum()),
                        float(m_slider->maximum()),
                        float(0), float(200));
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
    m_slider->setValue(128);
}

void VCSlider::slotClickAndGoLevelAndPresetChanged(uchar level, QImage img)
{
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

void VCSlider::slotPlaybackFunctionRunning(quint32 fid)
{
    Q_UNUSED(fid);
}

void VCSlider::slotPlaybackFunctionStopped(quint32 fid)
{
    m_externalMovement = true;
    if (fid == playbackFunction())
        m_slider->setValue(0);
    m_externalMovement = false;
}

void VCSlider::slotPlaybackFunctionIntensityChanged(qreal fraction)
{
    m_externalMovement = true;
    m_slider->setValue(int(floor((qreal(m_slider->maximum()) * fraction) + 0.5)));
    m_externalMovement = false;
}

/*****************************************************************************
 * DMXSource
 *****************************************************************************/

void VCSlider::writeDMX(MasterTimer* timer, UniverseArray* universes)
{
    if (sliderMode() == Level)
        writeDMXLevel(timer, universes);
    else if (sliderMode() == Playback)
        writeDMXPlayback(timer, universes);
}

void VCSlider::writeDMXLevel(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);

    m_levelValueMutex.lock();

    uchar modLevel = m_levelValue;

    int r = 0, g = 0, b = 0, c = 0, m = 0, y = 0;
    if (m_cngType == ClickAndGoWidget::RGB)
    {
        float f = SCALE(float(m_levelValue),
                        float(m_slider->minimum()),
                        float(m_slider->maximum()),
                        float(0), float(200));
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
        float f = SCALE(float(m_levelValue),
                        float(m_slider->minimum()),
                        float(m_slider->maximum()),
                        float(0), float(200));
        if ((uchar)f != 0)
        {
            QColor modColor = m_cngRGBvalue.lighter((uchar)f);
            c = modColor.cyan();
            m = modColor.magenta();
            y = modColor.yellow();
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

            quint32 dmx_ch = fxi->channelAddress(lch.channel);
            universes->write(dmx_ch, modLevel, qlcch->group());
        }
    }
    m_levelValueChanged = false;
    m_levelValueMutex.unlock();
}

void VCSlider::writeDMXPlayback(MasterTimer* timer, UniverseArray* ua)
{
    Q_UNUSED(ua);

    Function* function = m_doc->function(m_playbackFunction);
    if (function == NULL || mode() == Doc::Design)
        return;

    /* Grab current values inside a locked mutex */
    m_playbackValueMutex.lock();
    uchar value = m_playbackValue;
    bool changed = m_playbackValueChanged;
    qreal intensity = qreal(value) / qreal(UCHAR_MAX);
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
                function->start(timer);
            function->adjustIntensity(intensity);
        }
    }
}

/*****************************************************************************
 * Top label
 *****************************************************************************/

void VCSlider::setTopLabelText(const QString& text)
{
    m_topLabel->setText(text);
}

QString VCSlider::topLabelText()
{
    return m_topLabel->text();
}

/*****************************************************************************
 * Slider
 *****************************************************************************/

void VCSlider::setSliderValue(uchar value)
{
    Q_ASSERT(m_slider != NULL);

    /* Scale from input value range to this slider's range */
    float val;
    val = SCALE((float) value, (float) 0, (float) UCHAR_MAX,
                (float) m_slider->minimum(),
                (float) m_slider->maximum());

    if (m_slider->invertedAppearance() == true)
        m_slider->setValue(m_slider->maximum() - (int) val);
    else
        m_slider->setValue((int) val);
}

int VCSlider::sliderValue() const
{
    Q_ASSERT(m_slider != NULL);
    return m_slider->value();
}

void VCSlider::slotSliderMoved(int value)
{
    QString num;

    switch (sliderMode())
    {
    case Level:
    {
        setLevelValue(value);
        setClickAndGoWidgetFromLevel(value);

        /* Set text for the top label */
        if (valueDisplayStyle() == ExactValue)
        {
            num.sprintf("%.3d", value);
        }
        else
        {
            float f = SCALE(float(value),
                            float(m_slider->minimum()),
                            float(m_slider->maximum()),
                            float(0), float(100));
            num.sprintf("%.3d%%", static_cast<int> (f));
        }
        setTopLabelText(num);
    }
    break;

    case Playback:
    {
        setPlaybackValue(value);

        /* Set text for the top label */
        if (valueDisplayStyle() == ExactValue)
        {
            num.sprintf("%.3d", value);
        }
        else
        {
            float f = SCALE(float(value),
                            float(m_slider->minimum()),
                            float(m_slider->maximum()),
                            float(0), float(100));
            num.sprintf("%.3d%%", static_cast<int> (f));
        }
        setTopLabelText(num);
    }
    break;

    default:
        break;
    }

    //if (m_slider->isSliderDown() == true)
    sendFeedBack(value);
}

void VCSlider::sendFeedBack(int value)
{
    /* Send input feedback */
    QLCInputSource src = inputSource();
    if (src.isValid() == true)
    {
        if (invertedAppearance() == true)
            value = m_slider->maximum() - value;

        float fb = SCALE(float(value), float(m_slider->minimum()),
                         float(m_slider->maximum()), float(0),
                         float(UCHAR_MAX));

        QString chName = QString();

        InputPatch* pat = m_doc->inputMap()->patch(src.universe());
        if (pat != NULL)
        {
            QLCInputProfile* profile = pat->profile();
            if (profile != NULL)
            {
                QLCInputChannel* ich = profile->channel(src.channel());
                if (ich != NULL)
                    chName = ich->name();
            }
        }

        m_doc->outputMap()->feedBack(src.universe(), src.channel(), int(fb), chName);
    }
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
 * Tap button
 *****************************************************************************/

void VCSlider::setTapButtonText(const QString& text)
{
    m_tapButton->setText(QString(text).replace(" ", "\n"));
}

QString VCSlider::tapButtonText()
{
    return m_tapButton->text();
}

void VCSlider::slotTapButtonClicked()
{
    int t = m_time->elapsed();
    qDebug() << "TODO!" << t;
    m_time->restart();
}

/*****************************************************************************
 * External input
 *****************************************************************************/

bool VCSlider::isButton(quint32 universe, quint32 channel)
{
    InputPatch* patch = NULL;
    QLCInputProfile* profile = NULL;
    QLCInputChannel* ch = NULL;

    patch = m_doc->inputMap()->patch(universe);
    if (patch != NULL)
    {
        profile = patch->profile();
        if (profile != NULL)
        {
            ch = profile->channels()[channel];
            if (ch != NULL)
            {
                return (ch->type() == QLCInputChannel::Button);
            }
        }
    }

    return false;
}

void VCSlider::slotInputValueChanged(quint32 universe, quint32 channel,
                                     uchar value)
{
    /* Don't let input data thru in design mode */
    if (mode() == Doc::Design)
        return;

    if (inputSource() == QLCInputSource(universe, channel))
    {
        if (isButton(universe, channel) == true)
        {
            // Check value here so that value == 0 won't end up in the else branch
            if (value > 0)
                slotTapButtonClicked();
        }
        else
        {
            /* Scale from input value range to this slider's range */
            float val;
            val = SCALE((float) value, (float) 0, (float) UCHAR_MAX,
                        (float) m_slider->minimum(),
                        (float) m_slider->maximum());

            if (m_slider->invertedAppearance() == true)
                m_slider->setValue(m_slider->maximum() - (int) val);
            else
                m_slider->setValue((int) val);
        }
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
    QString caption;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCSlider)
    {
        qWarning() << Q_FUNC_INFO << "Slider node not found";
        return false;
    }

    /* Caption */
    caption = root->attribute(KXMLQLCVCCaption);
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
    setCaption(caption);

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

    /* Caption */
    root.setAttribute(KXMLQLCVCCaption, caption());

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
