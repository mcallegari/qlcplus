/*
  Q Light Controller Plus
  vcwidget.cpp

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

#include <QStyleOptionFrame>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QApplication>
#include <QInputDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMetaObject>
#include <QPainter>
#include <QPalette>
#include <QCursor>
#include <QPixmap>
#include <QDebug>
#include <QBrush>
#include <QPoint>
#include <QStyle>
#include <QSize>
#include <QMenu>
#include <QList>

#include "qlcinputsource.h"
#include "qlcfile.h"

#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "inputpatch.h"
#include "vcwidget.h"
#include "doc.h"

#define GRID_RESOLUTION 5

VCWidget::VCWidget(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_id(invalidId())
    , m_disableState(false)
    , m_page(0)
    , m_allowChildren(false)
    , m_allowResize(true)
    , m_intensityOverrideId(Function::invalidAttributeId())
    , m_intensity(1.0)
    , m_liveEdit(VirtualConsole::instance()->liveEdit())
{
    Q_ASSERT(parent != NULL);
    Q_ASSERT(doc != NULL);

    /* Set the class name "VCWidget" as the object name as well */
    setObjectName(VCWidget::staticMetaObject.className());

    setMinimumSize(QSize(20, 20));

    m_type = UnknownWidget;
    m_hasCustomBackgroundColor = false;
    m_hasCustomForegroundColor = false;
    m_backgroundImage = QString();
    m_hasCustomFont = false;
    m_frameStyle = KVCFrameStyleNone;

    m_resizeMode = false;

    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setEnabled(true);

    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));

    /* Listen to the virtual console key signals */
    connect(VirtualConsole::instance(), SIGNAL(keyPressed(const QKeySequence&)),
            this, SLOT(slotKeyPressed(const QKeySequence&)));
    connect(VirtualConsole::instance(), SIGNAL(keyReleased(const QKeySequence&)),
            this, SLOT(slotKeyReleased(const QKeySequence&)));
}

VCWidget::~VCWidget()
{
}

/*****************************************************************************
 * ID
 *****************************************************************************/

void VCWidget::setID(quint32 id)
{
    /* Don't set doc modified status or emit changed signal, because this
       function is called only once during widget creation. */
    m_id = id;
}

quint32 VCWidget::id() const
{
    return m_id;
}

quint32 VCWidget::invalidId()
{
    return UINT_MAX;
}

/*********************************************************************
 * Type
 *********************************************************************/

void VCWidget::setType(int type)
{
    m_type = type;
}

int VCWidget::type()
{
    return m_type;
}

QString VCWidget::typeToString(int type)
{
    switch (type)
    {

        case ButtonWidget: return QString(tr("Button"));
        case SliderWidget: return QString(tr("Slider"));
        case XYPadWidget: return QString(tr("XYPad"));
        case FrameWidget: return QString(tr("Frame"));
        case SoloFrameWidget: return QString(tr("Solo frame"));
        case SpeedDialWidget: return QString(tr("Speed dial"));
        case CueListWidget: return QString(tr("Cue list"));
        case LabelWidget: return QString(tr("Label"));
        case AudioTriggersWidget: return QString(tr("Audio Triggers"));
        case AnimationWidget: return QString(tr("Animation"));
        case ClockWidget: return QString(tr("Clock"));
        case UnknownWidget:
        default:
             return QString(tr("Unknown"));
    }
    return QString(tr("Unknown"));
}

QIcon VCWidget::typeToIcon(int type)
{
    switch (type)
    {
        case ButtonWidget: return QIcon(":/button.png");
        case SliderWidget: return QIcon(":/slider.png");
        case XYPadWidget: return QIcon(":/xypad.png");
        case FrameWidget: return QIcon(":/frame.png");
        case SoloFrameWidget: return QIcon(":/soloframe.png");
        case SpeedDialWidget: return QIcon(":/speed.png");
        case CueListWidget: return QIcon(":/cuelist.png");
        case LabelWidget: return QIcon(":/label.png");
        case AudioTriggersWidget: return QIcon(":/audioinput.png");
        case AnimationWidget: return QIcon(":/rgbmatrix.png");
        case ClockWidget: return QIcon(":/clock.png");
        case UnknownWidget:
        default:
             return QIcon(":/virtualconsole.png");
    }
    return QIcon(":/virtualconsole.png");
}

/*********************************************************************
 * Disable state
 *********************************************************************/

void VCWidget::setDisableState(bool disable)
{
    m_disableState = disable;
    if (mode() == Doc::Operate)
    {
        setEnabled(!disable);
        enableWidgetUI(!disable);
    }

    emit disableStateChanged(m_disableState);
}

void VCWidget::enableWidgetUI(bool enable)
{
    Q_UNUSED(enable)
}

bool VCWidget::isDisabled()
{
    return m_disableState;
}

/*********************************************************************
 * Page
 *********************************************************************/

void VCWidget::setPage(int pNum)
{
    m_page = pNum;
}

int VCWidget::page()
{
    return m_page;
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

bool VCWidget::copyFrom(const VCWidget* widget)
{
    if (widget == NULL)
        return false;

    setBackgroundImage(widget->m_backgroundImage);

    m_hasCustomBackgroundColor = widget->m_hasCustomBackgroundColor;
    if (m_hasCustomBackgroundColor == true)
        setBackgroundColor(widget->backgroundColor());

    m_hasCustomForegroundColor = widget->m_hasCustomForegroundColor;
    if (m_hasCustomForegroundColor == true)
        setForegroundColor(widget->foregroundColor());

    m_hasCustomFont = widget->m_hasCustomFont;
    if (m_hasCustomFont == true)
        setFont(widget->font());

    m_frameStyle = widget->m_frameStyle;

    setGeometry(widget->geometry());
    setCaption(widget->caption());

    m_allowChildren = widget->m_allowChildren;
    m_allowResize = widget->m_allowResize;

    QHashIterator <quint8, QSharedPointer<QLCInputSource> > it(widget->m_inputs);
    while (it.hasNext() == true)
    {
        it.next();
        quint8 id = it.key();
        QSharedPointer<QLCInputSource> src(new QLCInputSource(it.value()->universe(), it.value()->channel()));
        src->setFeedbackValue(QLCInputFeedback::LowerValue, it.value()->feedbackValue(QLCInputFeedback::LowerValue));
        src->setFeedbackValue(QLCInputFeedback::UpperValue, it.value()->feedbackValue(QLCInputFeedback::UpperValue));
        src->setFeedbackValue(QLCInputFeedback::MonitorValue, it.value()->feedbackValue(QLCInputFeedback::MonitorValue));
        src->setFeedbackExtraParams(QLCInputFeedback::LowerValue, it.value()->feedbackExtraParams(QLCInputFeedback::LowerValue));
        src->setFeedbackExtraParams(QLCInputFeedback::UpperValue, it.value()->feedbackExtraParams(QLCInputFeedback::UpperValue));
        src->setFeedbackExtraParams(QLCInputFeedback::MonitorValue, it.value()->feedbackExtraParams(QLCInputFeedback::MonitorValue));
        setInputSource(src, id);
    }

    m_page = widget->m_page;

    return true;
}

/*****************************************************************************
 * Background image
 *****************************************************************************/

void VCWidget::setBackgroundImage(const QString& path)
{
    QPalette pal = palette();

    m_hasCustomBackgroundColor = false;
    m_backgroundImage = path;

    pal.setBrush(QPalette::Window, QBrush(QPixmap(path)));
    setPalette(pal);

    m_doc->setModified();
}

QString VCWidget::backgroundImage() const
{
    return m_backgroundImage;
}


/*****************************************************************************
 * Background color
 *****************************************************************************/

void VCWidget::setBackgroundColor(const QColor& color)
{
    QPalette pal = palette();

    m_hasCustomBackgroundColor = true;
    m_backgroundImage = QString();

    pal.setColor(QPalette::Window, color);
    setPalette(pal);

    m_doc->setModified();
}

QColor VCWidget::backgroundColor() const
{
    return palette().color(QPalette::Window);
}

bool VCWidget::hasCustomBackgroundColor() const
{
    return m_hasCustomBackgroundColor;
}

void VCWidget::resetBackgroundColor()
{
    QColor fg;

    m_hasCustomBackgroundColor = false;
    m_backgroundImage = QString();

    /* Store foreground color */
    if (m_hasCustomForegroundColor == true)
        fg = palette().color(QPalette::WindowText);

    /* Reset the whole palette to application palette */
    setPalette(QApplication::palette());
    /* setAutoFillBackground(false); */

    /* Restore foreground color */
    if (fg.isValid() == true)
    {
        QPalette pal = palette();
        pal.setColor(QPalette::WindowText, fg);
        setPalette(pal);
    }

    m_doc->setModified();
}

/*****************************************************************************
 * Foreground color
 *****************************************************************************/

void VCWidget::setForegroundColor(const QColor& color)
{
    QPalette pal = palette();

    m_hasCustomForegroundColor = true;

    pal.setColor(QPalette::WindowText, color);
    setPalette(pal);

    m_doc->setModified();
}

QColor VCWidget::foregroundColor() const
{
    return palette().color(QPalette::WindowText);
}

bool VCWidget::hasCustomForegroundColor() const
{
    return m_hasCustomForegroundColor;
}

void VCWidget::resetForegroundColor()
{
    QColor bg;

    m_hasCustomForegroundColor = false;

    /* Store background color */
    if (hasCustomBackgroundColor() == true)
        bg = palette().color(QPalette::Window);

    /* Reset the whole palette to application palette */
    setPalette(QApplication::palette());

    /* Restore foreground color (the first two emit Doc::modified() signal) */
    if (bg.isValid() == true)
        setBackgroundColor(bg);
    else if (backgroundImage().isEmpty() == false)
        setBackgroundImage(backgroundImage());
    else
        m_doc->setModified();
}

/*****************************************************************************
 * Font
 *****************************************************************************/

void VCWidget::setFont(const QFont& font)
{
    m_hasCustomFont = true;
    QWidget::setFont(font);
    m_doc->setModified();
}

QFont VCWidget::font() const
{
    return QWidget::font();
}

bool VCWidget::hasCustomFont() const
{
    return m_hasCustomFont;
}

void VCWidget::resetFont()
{
    QWidget::setFont(QApplication::font());
    m_hasCustomFont = false;
    m_doc->setModified();
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCWidget::setCaption(const QString& text)
{
    setWindowTitle(text);
    update();
    m_doc->setModified();
}

QString VCWidget::caption() const
{
    return windowTitle();
}

/*****************************************************************************
 * Frame style
 *****************************************************************************/

void VCWidget::setFrameStyle(int style)
{
    m_frameStyle = style;
    update();
    m_doc->setModified();
}

int VCWidget::frameStyle() const
{
    return m_frameStyle;
}

void VCWidget::resetFrameStyle()
{
    setFrameStyle(KVCFrameStyleNone);
}

QString VCWidget::frameStyleToString(int style)
{
    if (style == KVCFrameStyleSunken)
        return "Sunken";
    else if (style == KVCFrameStyleRaised)
        return "Raised";
    else
        return "None";
}

int VCWidget::stringToFrameStyle(const QString& style)
{
    if (style == "Sunken")
        return KVCFrameStyleSunken;
    else if (style == "Raised")
        return KVCFrameStyleRaised;
    else
        return KVCFrameStyleNone;
}

/*****************************************************************************
 * Allow adding children
 *****************************************************************************/

void VCWidget::setAllowChildren(bool allow)
{
    m_allowChildren = allow;
}

bool VCWidget::allowChildren() const
{
    return m_allowChildren;
}

/*********************************************************************
 * Allow resizing
 *********************************************************************/

void VCWidget::setAllowResize(bool allow)
{
    m_allowResize = allow;
}

bool VCWidget::allowResize() const
{
    return m_allowResize;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCWidget::editProperties()
{
    QMessageBox::information(this, staticMetaObject.className(),
                             tr("This widget has no properties"));
}

/*********************************************************************
 * Intensity
 *********************************************************************/

void VCWidget::adjustFunctionIntensity(Function *f, qreal value)
{
    if (f == NULL)
        return;

    //qDebug() << "adjustFunctionIntensity" << caption() << "value" << value;

    if (m_intensityOverrideId == Function::invalidAttributeId())
        m_intensityOverrideId = f->requestAttributeOverride(Function::Intensity, value);
    else
        f->adjustAttribute(value, m_intensityOverrideId);
}

void VCWidget::resetIntensityOverrideAttribute()
{
    m_intensityOverrideId = Function::invalidAttributeId();
}

void VCWidget::adjustIntensity(qreal val)
{
    m_intensity = val;
}

qreal VCWidget::intensity() const
{
    return m_intensity;
}

/*****************************************************************************
 * External input
 *****************************************************************************/

bool VCWidget::acceptsInput()
{
    if (mode() == Doc::Design || isEnabled() == false || isDisabled())
        return false;

    return true;
}

bool VCWidget::checkInputSource(quint32 universe, quint32 channel,
                                uchar value, QObject *sender, quint32 id)
{
    QSharedPointer<QLCInputSource> const& src = m_inputs.value(id);
    if (src.isNull())
        return false;

    if (src->isValid() && src->universe() == universe && src->channel() == channel)
    {
        // if the event has been fired by an external controller
        // and this channel is set to relative mode, inform the input source
        // and don't allow the event to pass through. A synthetic event
        // will be generated by the input source itself
        if (src != sender && src->needsUpdate())
        {
            src->updateInputValue(value);
            return false;
        }
        return true;
    }

    return false;
}

void VCWidget::setInputSource(QSharedPointer<QLCInputSource> const& source, quint8 id)
{
    // Connect when the first valid input source is set
    if (m_inputs.isEmpty() == true && !source.isNull() && source->isValid() == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
        connect(m_doc->inputOutputMap(), SIGNAL(profileChanged(quint32,QString)),
                this, SLOT(slotInputProfileChanged(quint32,QString)));
    }

    // Clear previous source
    if (m_inputs.contains(id))
    {
        disconnect(m_inputs.value(id).data(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
        m_inputs.remove(id);
    }

    // Assign
    if (!source.isNull() && source->isValid() == true)
    {
        m_inputs.insert(id, source);
        // now check if the source is defined in the associated universe
        // profile and if it has specific settings
        InputPatch *ip = m_doc->inputOutputMap()->inputPatch(source->universe());
        if (ip != NULL)
        {
            QLCInputProfile *profile = ip->profile();
            if (profile != NULL)
            {
                // Do not care about the page since input profiles don't do either
                QLCInputChannel *ich = profile->channel(source->channel() & 0xFFFF);
                if (ich != NULL)
                {
                    // retrieve plugin specific params for feedback
                    if (source->feedbackExtraParams(QLCInputFeedback::LowerValue).toInt() == -1)
                        source->setFeedbackExtraParams(QLCInputFeedback::LowerValue, profile->channelExtraParams(ich));
                    if (source->feedbackExtraParams(QLCInputFeedback::UpperValue).toInt() == -1 ||
                        !source->feedbackExtraParams(QLCInputFeedback::UpperValue).isValid())
                        source->setFeedbackExtraParams(QLCInputFeedback::UpperValue, profile->channelExtraParams(ich));
                    if (source->feedbackExtraParams(QLCInputFeedback::MonitorValue).toInt() == -1)
                        source->setFeedbackExtraParams(QLCInputFeedback::MonitorValue, profile->channelExtraParams(ich));

                    if (ich->movementType() == QLCInputChannel::Relative)
                    {
                        source->setWorkingMode(QLCInputSource::Relative);
                        source->setSensitivity(ich->movementSensitivity());
                        connect(source.data(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
                    }
                    else if (ich->type() == QLCInputChannel::Encoder)
                    {
                        source->setWorkingMode(QLCInputSource::Encoder);
                        source->setSensitivity(ich->movementSensitivity());
                        connect(source.data(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
                    }
                    else if (ich->type() == QLCInputChannel::Button)
                    {
                        if (ich->sendExtraPress() == true)
                        {
                            source->setSendExtraPressRelease(true);
                            connect(source.data(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                                    this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
                        }

                        // user custom feedback have precedence over input profile custom feedback
                        uchar lower = source->feedbackValue(QLCInputFeedback::LowerValue) != 0 ?
                                      source->feedbackValue(QLCInputFeedback::LowerValue) :
                                      ich->lowerValue();
                        uchar upper = source->feedbackValue(QLCInputFeedback::UpperValue) != UCHAR_MAX ?
                                          source->feedbackValue(QLCInputFeedback::UpperValue) :
                                          ich->upperValue();

                        source->setFeedbackValue(QLCInputFeedback::LowerValue, lower);
                        source->setFeedbackValue(QLCInputFeedback::UpperValue, upper);
                    }
                }
            }
        }
    }

    // Disconnect when there are no more input sources present
    if (m_inputs.isEmpty() == true)
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
        disconnect(m_doc->inputOutputMap(), SIGNAL(profileChanged(quint32,QString)),
                   this, SLOT(slotInputProfileChanged(quint32,QString)));
    }
}

QSharedPointer<QLCInputSource> VCWidget::inputSource(quint8 id) const
{
    return m_inputs.value(id);
}

void VCWidget::remapInputSources(int pgNum)
{
    foreach (quint8 s, m_inputs.keys())
    {
        QSharedPointer<QLCInputSource> src(m_inputs.value(s));
        src->setPage(pgNum);
        setInputSource(src, s);
    }
}

void VCWidget::sendFeedback(int value, quint8 id)
{
    /* Send input feedback */
    QSharedPointer<QLCInputSource> src = inputSource(id);
    sendFeedback(value, src);
}

void VCWidget::sendFeedback(int value, QSharedPointer<QLCInputSource> src, QVariant extraParams)
{
    if (src.isNull() || src->isValid() == false)
        return;

    // if in relative mode, send a "feedback" to this
    // input source so it can continue to emit values
    // from the right position
    if (src->needsUpdate())
        src->updateOuputValue(value);

    if (acceptsInput() == false)
        return;

    //qDebug() << "[VCWidget] Send feedback to uni" << src->universe() << "," << src->channel() << ", param" << extraParams;

    m_doc->inputOutputMap()->sendFeedBack(
        src->universe(), src->channel(), value,
        extraParams.isValid() ? extraParams : src->feedbackExtraParams(QLCInputFeedback::UpperValue));
}

void VCWidget::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    Q_UNUSED(universe);
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void VCWidget::slotInputProfileChanged(quint32 universe, const QString &profileName)
{
    qDebug() << "[VCWdget] input profile changed" << profileName;

    QLCInputProfile *profile = m_doc->inputOutputMap()->profile(profileName);

    foreach (QSharedPointer<QLCInputSource> const& source, m_inputs.values())
    {
        if (!source.isNull() && source->universe() == universe)
        {
            // if the profile has been unset, reset all the valid
            // input sources to work in absolute mode
            if (profile == NULL)
            {
                source->setWorkingMode(QLCInputSource::Absolute);
            }
            else
            {
                QLCInputChannel *ich = profile->channel(source->channel());
                if (ich != NULL)
                {
                    if (ich->movementType() == QLCInputChannel::Absolute)
                        source->setWorkingMode(QLCInputSource::Absolute);
                    else
                    {
                        source->setWorkingMode(QLCInputSource::Relative);
                        source->setSensitivity(ich->movementSensitivity());
                    }
                }
            }
        }
    }
}

/*****************************************************************************
 * Key sequence handler
 *****************************************************************************/

QKeySequence VCWidget::stripKeySequence(const QKeySequence& seq)
{
    /* In QLC 3.2.x it is possible to set shortcuts like CTRL+X, but since
       CTRL is now the tap modifier, it must be stripped away. */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int keys[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < (int)seq.count() && i < 4; i++)
    {
        if ((seq[i] & Qt::ControlModifier) != 0)
            keys[i] = seq[i] & (~Qt::ControlModifier);
        else
            keys[i] = seq[i];
    }
#else
    QKeyCombination keys[4] = { Qt::Key_unknown, QKeyCombination::fromCombined(0),
                               QKeyCombination::fromCombined(0), QKeyCombination::fromCombined(0)};
    for (int i = 0; i < (int)seq.count() && i < 4; i++)
    {
        if ((seq[i].toCombined() & Qt::ControlModifier) != 0)
            keys[i].fromCombined(seq[i].toCombined() & (~Qt::ControlModifier));
        else
            keys[i] = seq[i];
    }
#endif
    return QKeySequence(keys[0], keys[1], keys[2], keys[3]);
}

void VCWidget::slotKeyPressed(const QKeySequence& keySequence)
{
    emit keyPressed(keySequence);
}

void VCWidget::slotKeyReleased(const QKeySequence& keySequence)
{
    emit keyReleased(keySequence);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

void VCWidget::postLoad()
{
    /* NOP */
}

bool VCWidget::loadXMLCommon(QXmlStreamReader &root)
{
    if (root.device() == NULL || root.hasError())
        return false;

    QXmlStreamAttributes attrs = root.attributes();

    /* ID */
    if (attrs.hasAttribute(KXMLQLCVCWidgetID))
        setID(attrs.value(KXMLQLCVCWidgetID).toString().toUInt());

    /* Caption */
    if (attrs.hasAttribute(KXMLQLCVCCaption))
        setCaption(attrs.value(KXMLQLCVCCaption).toString());

    /* Page */
    if (attrs.hasAttribute(KXMLQLCVCWidgetPage))
        setPage(attrs.value(KXMLQLCVCWidgetPage).toString().toInt());

    return true;
}

bool VCWidget::loadXMLAppearance(QXmlStreamReader &root)
{
    if (root.device() == NULL || root.hasError())
        return false;

    if (root.name() != KXMLQLCVCWidgetAppearance)
    {
        qWarning() << Q_FUNC_INFO << "Appearance node not found!";
        return false;
    }

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCFrameStyle)
        {
            setFrameStyle(stringToFrameStyle(root.readElementText()));
        }
        else if (root.name() == KXMLQLCVCWidgetForegroundColor)
        {
            QString str = root.readElementText();
            if (str != KXMLQLCVCWidgetColorDefault)
                setForegroundColor(QColor(str.toUInt()));
            else if (hasCustomForegroundColor() == true)
                resetForegroundColor();
        }
        else if (root.name() == KXMLQLCVCWidgetBackgroundColor)
        {
            QString str = root.readElementText();
            if (str != KXMLQLCVCWidgetColorDefault)
                setBackgroundColor(QColor(str.toUInt()));
        }
        else if (root.name() == KXMLQLCVCWidgetBackgroundImage)
        {
            QString str = root.readElementText();
            if (str != KXMLQLCVCWidgetBackgroundImageNone)
                setBackgroundImage(m_doc->denormalizeComponentPath(str));
        }
        else if (root.name() == KXMLQLCVCWidgetFont)
        {
            QString str = root.readElementText();
            if (str != KXMLQLCVCWidgetFontDefault)
            {
                QFont font;
                font.fromString(str);
                setFont(font);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown appearance tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

QSharedPointer<QLCInputSource> VCWidget::getXMLInput(QXmlStreamReader &root)
{
    QXmlStreamAttributes attrs = root.attributes();

    quint32 uni = attrs.value(KXMLQLCVCWidgetInputUniverse).toString().toUInt();
    quint32 ch = attrs.value(KXMLQLCVCWidgetInputChannel).toString().toUInt();
    uchar min = 0, max = UCHAR_MAX, mon = UCHAR_MAX;

    QSharedPointer<QLCInputSource>newSrc = QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch));
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputLowerValue))
        min = uchar(attrs.value(KXMLQLCVCWidgetInputLowerValue).toString().toUInt());
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputUpperValue))
        max = uchar(attrs.value(KXMLQLCVCWidgetInputUpperValue).toString().toUInt());
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputMonitorValue))
        mon = uchar(attrs.value(KXMLQLCVCWidgetInputMonitorValue).toString().toUInt());

    newSrc->setFeedbackValue(QLCInputFeedback::LowerValue, min);
    newSrc->setFeedbackValue(QLCInputFeedback::UpperValue, max);
    newSrc->setFeedbackValue(QLCInputFeedback::MonitorValue, mon);

    // load feedback extra params
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputLowerParams))
        newSrc->setFeedbackExtraParams(QLCInputFeedback::LowerValue, attrs.value(KXMLQLCVCWidgetInputLowerParams).toInt());
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputUpperParams))
        newSrc->setFeedbackExtraParams(QLCInputFeedback::UpperValue, attrs.value(KXMLQLCVCWidgetInputUpperParams).toInt());
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputMonitorParams))
        newSrc->setFeedbackExtraParams(QLCInputFeedback::MonitorValue, attrs.value(KXMLQLCVCWidgetInputMonitorParams).toInt());

    return newSrc;
}

bool VCWidget::loadXMLInput(QXmlStreamReader &root, const quint8 &id)
{
    if (root.device() == NULL || root.hasError())
        return false;

    if (root.name() != KXMLQLCVCWidgetInput)
        return false;

    QSharedPointer<QLCInputSource>newSrc = getXMLInput(root);

    setInputSource(newSrc, id);

    root.skipCurrentElement();

    return true;
}

QString VCWidget::loadXMLSources(QXmlStreamReader &root, quint8 sourceID)
{
    QString keyText;
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInput(root, sourceID);
        }
        else if (root.name() == KXMLQLCVCWidgetKey)
        {
            keyText = root.readElementText();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown source tag" << root.name().toString();
            root.skipCurrentElement();
        }
    }
    return keyText;
}

bool VCWidget::loadXMLInput(QXmlStreamReader &root, quint32* uni, quint32* ch) const
{
    if (root.name() != KXMLQLCVCWidgetInput)
    {
        qWarning() << Q_FUNC_INFO << "Input node not found!";
        return false;
    }
    else
    {
        QXmlStreamAttributes attrs = root.attributes();
        *uni = attrs.value(KXMLQLCVCWidgetInputUniverse).toString().toUInt();
        *ch = attrs.value(KXMLQLCVCWidgetInputChannel).toString().toUInt();
        root.skipCurrentElement();
    }

    return true;
}

QString VCWidget::extraParamToString(QVariant param)
{
    if (param.isValid() == false)
        return QString();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (param.type() == QVariant::Int && param.toInt() != -1)
        return QString::number(param.toInt());
#else
    if (param.metaType().id() == QMetaType::Int && param.toInt() != -1)
        return QString::number(param.toInt());
#endif
    return QString();
}

bool VCWidget::saveXMLCommon(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Caption */
    doc->writeAttribute(KXMLQLCVCCaption, caption());

    /* ID */
    if (id() != VCWidget::invalidId())
        doc->writeAttribute(KXMLQLCVCWidgetID, QString::number(id()));

    /* Page */
    if (page() != 0)
        doc->writeAttribute(KXMLQLCVCWidgetPage, QString::number(page()));

    return true;
}

bool VCWidget::saveXMLAppearance(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    QString str;

    /* VC appearance entry */
    doc->writeStartElement(KXMLQLCVCWidgetAppearance);

    /* Frame style */
    doc->writeTextElement(KXMLQLCVCFrameStyle, frameStyleToString(frameStyle()));

    /* Foreground color */
    if (hasCustomForegroundColor() == true)
        str.setNum(foregroundColor().rgb());
    else
        str = KXMLQLCVCWidgetColorDefault;
    doc->writeTextElement(KXMLQLCVCWidgetForegroundColor, str);

    /* Background color */
    if (hasCustomBackgroundColor() == true)
        str.setNum(backgroundColor().rgb());
    else
        str = KXMLQLCVCWidgetColorDefault;
    doc->writeTextElement(KXMLQLCVCWidgetBackgroundColor, str);

    /* Background image */
    if (backgroundImage().isEmpty() == false)
        str = m_doc->normalizeComponentPath(m_backgroundImage);
    else
        str = KXMLQLCVCWidgetBackgroundImageNone;
    doc->writeTextElement(KXMLQLCVCWidgetBackgroundImage, str);

    /* Font */
    if (hasCustomFont() == true)
        str = font().toString();
    else
        str = KXMLQLCVCWidgetFontDefault;
    doc->writeTextElement(KXMLQLCVCWidgetFont, str);

    /* End the <Appearance> tag */
    doc->writeEndElement();

    return true;
}

bool VCWidget::saveXMLInput(QXmlStreamWriter *doc)
{
    return saveXMLInput(doc, inputSource());
}

bool VCWidget::saveXMLInput(QXmlStreamWriter *doc,
                            const QLCInputSource *src)
{
    Q_ASSERT(doc != NULL);

    if (src == NULL)
        return false;

    if (src->isValid() == true)
    {
        doc->writeStartElement(KXMLQLCVCWidgetInput);
        doc->writeAttribute(KXMLQLCVCWidgetInputUniverse, QString("%1").arg(src->universe()));
        doc->writeAttribute(KXMLQLCVCWidgetInputChannel, QString("%1").arg(src->channel()));
        if (src->feedbackValue(QLCInputFeedback::LowerValue) != 0)
            doc->writeAttribute(KXMLQLCVCWidgetInputLowerValue, QString::number(src->feedbackValue(QLCInputFeedback::LowerValue)));
        if (src->feedbackValue(QLCInputFeedback::UpperValue) != UCHAR_MAX)
            doc->writeAttribute(KXMLQLCVCWidgetInputUpperValue, QString::number(src->feedbackValue(QLCInputFeedback::UpperValue)));
        if (src->feedbackValue(QLCInputFeedback::MonitorValue) != UCHAR_MAX)
            doc->writeAttribute(KXMLQLCVCWidgetInputMonitorValue, QString::number(src->feedbackValue(QLCInputFeedback::MonitorValue)));

        // save feedback extra params
        QString extraParams = extraParamToString(src->feedbackExtraParams(QLCInputFeedback::LowerValue));

        if (!extraParams.isEmpty())
            doc->writeAttribute(KXMLQLCVCWidgetInputLowerParams, extraParams);

        extraParams = extraParamToString(src->feedbackExtraParams(QLCInputFeedback::UpperValue));

        if (!extraParams.isEmpty())
            doc->writeAttribute(KXMLQLCVCWidgetInputUpperParams, extraParams);

        extraParams = extraParamToString(src->feedbackExtraParams(QLCInputFeedback::MonitorValue));

        if (!extraParams.isEmpty())
            doc->writeAttribute(KXMLQLCVCWidgetInputMonitorParams, extraParams);

        doc->writeEndElement();
    }

    return true;
}

bool VCWidget::saveXMLInput(QXmlStreamWriter *doc,
                      QSharedPointer<QLCInputSource> const& src)
{
    return saveXMLInput(doc, src.data());
}

bool VCWidget::saveXMLWindowState(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Window state tag */
    doc->writeStartElement(KXMLQLCWindowState);

    /* Visible status */
    if (isVisible() == true)
        doc->writeAttribute(KXMLQLCWindowStateVisible, KXMLQLCTrue);
    else
        doc->writeAttribute(KXMLQLCWindowStateVisible, KXMLQLCFalse);

    doc->writeAttribute(KXMLQLCWindowStateX, QString::number(x()));
    doc->writeAttribute(KXMLQLCWindowStateY, QString::number(y()));
    doc->writeAttribute(KXMLQLCWindowStateWidth, QString::number(width()));
    doc->writeAttribute(KXMLQLCWindowStateHeight, QString::number(height()));

    doc->writeEndElement();

    return true;
}

bool VCWidget::loadXMLWindowState(QXmlStreamReader &tag, int* x, int* y,
                                  int* w, int* h, bool* visible)
{
    if (tag.device() == NULL || x == NULL || y == NULL || w == NULL || h == NULL ||
            visible == NULL)
        return false;

    if (tag.name() == KXMLQLCWindowState)
    {
        QXmlStreamAttributes attrs = tag.attributes();
        *x = attrs.value(KXMLQLCWindowStateX).toString().toInt();
        *y = attrs.value(KXMLQLCWindowStateY).toString().toInt();
        *w = attrs.value(KXMLQLCWindowStateWidth).toString().toInt();
        *h = attrs.value(KXMLQLCWindowStateHeight).toString().toInt();

        if (attrs.value(KXMLQLCWindowStateVisible).toString() == KXMLQLCTrue)
            *visible = true;
        else
            *visible = false;
        tag.skipCurrentElement();

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Window state not found";
        return false;
    }
}

/*****************************************************************************
 * QLC+ Mode change
 *****************************************************************************/

void VCWidget::setLiveEdit(bool liveEdit)
{
    if (m_doc->mode() == Doc::Design)
        return;

    m_liveEdit = liveEdit;

    if (m_disableState)
        setEnabled(m_liveEdit);
    else
        enableWidgetUI(!m_liveEdit);

    unsetCursor();
    update();
}

void VCWidget::cancelLiveEdit()
{
    m_liveEdit = false;
}

void VCWidget::slotModeChanged(Doc::Mode mode)
{
    // make sure to exit from a 'deep' disable state
    if (mode == Doc::Design)
        setEnabled(true);

    /* Reset mouse cursor */
    unsetCursor();

    /* Force an update to get rid of selection markers */
    update();
}

Doc::Mode VCWidget::mode() const
{
    Q_ASSERT(m_doc != NULL);
    if (m_liveEdit)
        return Doc::Design;
    return m_doc->mode();
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCWidget::invokeMenu(const QPoint& point)
{
    /* No point coming here if there is no VC instance */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    QMenu* menu = vc->editMenu();
    Q_ASSERT(menu != NULL);
    menu->exec(point);
}

/*****************************************************************************
 * Custom menu
 *****************************************************************************/

QMenu* VCWidget::customMenu(QMenu* parentMenu)
{
    Q_UNUSED(parentMenu);
    return NULL;
}

/*****************************************************************************
 * Widget move & resize
 *****************************************************************************/

void VCWidget::resize(const QSize& size)
{
    QSize sz(size);

    // Force grid settings
    sz.setWidth(size.width() - (size.width() % GRID_RESOLUTION));
    sz.setHeight(size.height() - (size.height() % GRID_RESOLUTION));

    // Resize
    QWidget::resize(sz);
}

void VCWidget::move(const QPoint& point)
{
    QPoint pt(point);

    // Force grid settings
    pt.setX(point.x() - (point.x() % GRID_RESOLUTION));
    pt.setY(point.y() - (point.y() % GRID_RESOLUTION));

    // Don't move beyond left or right
    if (pt.x() < 0)
        pt.setX(0);
    else if (pt.x() + rect().width() > parentWidget()->width())
        pt.setX(parentWidget()->width() - rect().width());

    // Don't move beyond top or bottom
    if (pt.y() < 0)
        pt.setY(0);
    else if (pt.y() + rect().height() > parentWidget()->height())
        pt.setY(parentWidget()->height() - rect().height());

    // Move
    QWidget::move(pt);

    m_doc->setModified();
}

QPoint VCWidget::lastClickPoint() const
{
    return m_mousePressPoint;
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCWidget::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);

    /* No point coming here if there is no VC instance */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    QPainter painter(this);

    /* Draw frame according to style */
    QStyleOptionFrame option;
    option.initFrom(this);

    if (frameStyle() == KVCFrameStyleSunken)
        option.state = QStyle::State_Sunken;
    else if (frameStyle() == KVCFrameStyleRaised)
        option.state = QStyle::State_Raised;
    else
        option.state = QStyle::State_None;

    if (mode() == Doc::Design)
        option.state |= QStyle::State_Enabled;

    /* Draw a frame border if such is specified for this widget */
    if (option.state != QStyle::State_None)
        style()->drawPrimitive(QStyle::PE_Frame, &option, &painter, this);

    QWidget::paintEvent(e);

    /* Draw selection frame */
    if (mode() == Doc::Design && vc->isWidgetSelected(this) == true)
    {
        /* Draw a dotted line around the widget */
        QPen pen(Qt::DashLine);
        pen.setColor(Qt::blue);
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidth(0);
        painter.setPen(pen);
        painter.drawRect(0, 0, rect().width() - 1, rect().height() - 1);

        /* Draw a resize handle */
        if (allowResize() == true)
        {
            QIcon icon(":/resize.png");
            painter.drawPixmap(rect().width() - 16, rect().height() - 16,
                               icon.pixmap(QSize(16, 16), QIcon::Normal, QIcon::On));
        }
    }
}

void VCWidget::mousePressEvent(QMouseEvent* e)
{
    Q_ASSERT(e != NULL);

    if (mode() == Doc::Operate)
    {
        QWidget::mousePressEvent(e);
        return;
    }

    /* Perform widget de/selection in virtualconsole's selection buffer */
    handleWidgetSelection(e);

    /* Resize mode */
    if (m_resizeMode == true)
    {
        setMouseTracking(false);
        m_resizeMode = false;
    }

    /* Move, resize or context menu invocation */
    if (e->button() & Qt::LeftButton || e->button() & Qt::MiddleButton)
    {
        /* Start moving or resizing based on where the click landed */
        if (e->pos().x() > rect().width() - 10 && e->pos().y() > rect().height() - 10 && allowResize())
        {
            m_resizeMode = true;
            setMouseTracking(true);
            setCursor(QCursor(Qt::SizeFDiagCursor));
        }
        else
        {
            m_mousePressPoint = QPoint(e->pos().x(), e->pos().y());
            setCursor(QCursor(Qt::SizeAllCursor));
        }
    }
    else if (e->button() & Qt::RightButton)
    {
        /* Menu invocation */
        m_mousePressPoint = QPoint(e->pos().x(), e->pos().y());
        invokeMenu(mapToGlobal(e->pos()));
    }
}

void VCWidget::handleWidgetSelection(QMouseEvent* e)
{
    /* No point coming here if there is no VC */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    /* Widget selection logic (like in Qt Designer) */
    if (e->button() == Qt::LeftButton)
    {
        if (e->modifiers() & Qt::ShiftModifier)
        {
            /* Toggle selection with LMB when shift is pressed */
            bool selected = vc->isWidgetSelected(this);
            vc->setWidgetSelected(this, !selected);
        }
        else
        {
            if (vc->isWidgetSelected(this) == false)
            {
                /* Select only this */
                vc->clearWidgetSelection();
                vc->setWidgetSelected(this, true);
            }
        }
    }
    else if (e->button() == Qt::RightButton)
    {
        if (vc->isWidgetSelected(this) == false)
        {
            /* Select only this */
            vc->clearWidgetSelection();
            vc->setWidgetSelected(this, true);
        }
    }
}

void VCWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (mode() == Doc::Design)
    {
        unsetCursor();
        m_resizeMode = false;
        setMouseTracking(false);
    }
    else
    {
        QWidget::mouseReleaseEvent(e);
    }
}

void VCWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (mode() == Doc::Design)
        editProperties();
    else
        QWidget::mouseDoubleClickEvent(e);
}

void VCWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (mode() == Doc::Design)
    {
        if (m_resizeMode == true)
        {
            QPoint p = mapToParent(e->pos());
            resize(QSize(p.x() - x(), p.y() - y()));
            m_doc->setModified();
        }
        else if (e->buttons() & Qt::LeftButton || e->buttons() & Qt::MiddleButton)
        {
            QPoint p = mapToParent(e->pos());
            p.setX(p.x() - m_mousePressPoint.x());
            p.setY(p.y() - m_mousePressPoint.y());

            move(p);
            m_doc->setModified();
        }
    }
    else
    {
        QWidget::mouseMoveEvent(e);
    }
}
