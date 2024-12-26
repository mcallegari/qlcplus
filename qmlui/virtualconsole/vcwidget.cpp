/*
  Q Light Controller Plus
  vcwidget.cpp

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

#include "qlcinputchannel.h"
#include "inputpatch.h"
#include "vcwidget.h"
#include "tardis.h"
#include "doc.h"

VCWidget::VCWidget(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_item(nullptr)
    , m_id(invalidId())
    , m_type(UnknownWidget)
    , m_geometry(QRect(0,0,0,0))
    , m_scaleFactor(1.0)
    , m_allowResize(true)
    , m_isDisabled(false)
    , m_isVisible(true)
    , m_caption(QString())
    , m_backgroundColor(QColor("#333"))
    , m_hasCustomBackgroundColor(false)
    , m_backgroundImage(QString())
    , m_foregroundColor(QColor(Qt::white))
    , m_hasCustomForegroundColor(false)
    , m_hasCustomFont(false)
    , m_page(0)
    , m_intensityOverrideId(Function::invalidAttributeId())
    , m_intensity(1.0)
    , m_isEditing(false)
{
    m_font = QFont("Roboto Condensed");
}

VCWidget::~VCWidget()
{
}

void VCWidget::setDocModified()
{
    if (m_doc != nullptr)
        m_doc->setModified();
}

void VCWidget::setupLookAndFeel(qreal pixelDensity, int page)
{
    setDefaultFontSize(pixelDensity * 2.7);
    setPage(page);
}

void VCWidget::render(QQuickView *, QQuickItem *)
{
}

QQuickItem *VCWidget::renderItem() const
{
    return m_item;
}

void VCWidget::enqueueTardisAction(int code, QVariant oldVal, QVariant newVal)
{
    if (Tardis::instance() == nullptr)
        return;

    Tardis *tardis = Tardis::instance();
    tardis->enqueueAction(code, id(), oldVal, newVal);
}

VCWidget *VCWidget::createCopy(VCWidget *parent)
{
    Q_UNUSED(parent)
    return nullptr;
}

bool VCWidget::copyFrom(const VCWidget* widget)
{
    if (widget == nullptr)
        return false;

    m_backgroundImage = widget->m_backgroundImage;

    m_hasCustomBackgroundColor = widget->m_hasCustomBackgroundColor;
    if (m_hasCustomBackgroundColor == true)
        setBackgroundColor(widget->backgroundColor());

    m_hasCustomForegroundColor = widget->m_hasCustomForegroundColor;
    if (m_hasCustomForegroundColor == true)
        setForegroundColor(widget->foregroundColor());

    m_hasCustomFont = widget->m_hasCustomFont;
    if (m_hasCustomFont == true)
        setFont(widget->font());

    setGeometry(widget->geometry());
    setCaption(widget->caption());

    m_allowResize = widget->m_allowResize;

    for (QSharedPointer<QLCInputSource> src : widget->m_inputSources)
    {
        QSharedPointer<QLCInputSource> dst(new QLCInputSource(src->universe(), src->channel()));
        dst->setID(src->id());
        dst->setFeedbackValue(QLCInputFeedback::LowerValue, src->feedbackValue(QLCInputFeedback::LowerValue));
        dst->setFeedbackValue(QLCInputFeedback::UpperValue, src->feedbackValue(QLCInputFeedback::UpperValue));
        dst->setFeedbackValue(QLCInputFeedback::MonitorValue, src->feedbackValue(QLCInputFeedback::MonitorValue));
        dst->setFeedbackExtraParams(QLCInputFeedback::LowerValue, src->feedbackExtraParams(QLCInputFeedback::LowerValue));
        dst->setFeedbackExtraParams(QLCInputFeedback::UpperValue, src->feedbackExtraParams(QLCInputFeedback::UpperValue));
        dst->setFeedbackExtraParams(QLCInputFeedback::MonitorValue, src->feedbackExtraParams(QLCInputFeedback::MonitorValue));
        addInputSource(dst);
    }

    QMapIterator<QKeySequence, quint32> it(m_keySequenceMap);
    while (it.hasNext())
    {
        it.next();

        QKeySequence seq = it.key();
        quint32 id = it.value();
        addKeySequence(seq, id);
    }

    m_page = widget->m_page;

    return true;
}

/*****************************************************************************
 * ID
 *****************************************************************************/

void VCWidget::setID(quint32 id)
{
    /* Don't set doc modified status or emit changed signal, because this
       function is called only once during widget creation. */
    m_id = id;

    if (caption().isEmpty())
        m_caption = defaultCaption();
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
        case FrameWidget: return QString(tr("Frame"));
        case SoloFrameWidget: return QString(tr("Solo Frame"));
        case SpeedWidget: return QString(tr("Speed"));
        case XYPadWidget: return QString(tr("XY Pad"));
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

QString VCWidget::typeToIcon(int type)
{
    switch (type)
    {
        case ButtonWidget: return QString("qrc:/button.svg");
        case SliderWidget: return QString("qrc:/slider.svg");
        case FrameWidget: return QString("qrc:/frame.svg");
        case SoloFrameWidget: return QString("qrc:/soloframe.svg");
        case SpeedWidget: return QString("qrc:/speed.svg");
        case XYPadWidget: return QString("qrc:/xypad.svg");
        case CueListWidget: return QString("qrc:/cuelist.svg");
        case LabelWidget: return QString("qrc:/label.svg");
        case AudioTriggersWidget: return QString("qrc:/audioinput.svg");
        case AnimationWidget: return QString("qrc:/rgbmatrix.svg");
        case ClockWidget: return QString("qrc:/clock.svg");
        case UnknownWidget:
        default:
             return QString("qrc:/virtualconsole.svg");
    }
    return QString("qrc:/virtualconsole.svg");
}

VCWidget::WidgetType VCWidget::stringToType(QString str)
{
    if (str == "Button") return ButtonWidget;
    else if (str == "Slider") return SliderWidget;
    else if (str == "Knob") return SliderWidget;
    else if (str == "XYPad") return XYPadWidget;
    else if (str == "Frame") return FrameWidget;
    else if (str == "Solo frame") return SoloFrameWidget;
    else if (str == "Speed") return SpeedWidget;
    else if (str == "Cue list") return CueListWidget;
    else if (str == "Label") return LabelWidget;
    else if (str == "Audio Triggers") return AudioTriggersWidget;
    else if (str == "Animation") return AnimationWidget;
    else if (str == "Clock") return ClockWidget;

    return UnknownWidget;
}

/*********************************************************************
 * Geometry
 *********************************************************************/

QRectF VCWidget::geometry() const
{
    return QRectF(m_geometry.x() * m_scaleFactor, m_geometry.y() * m_scaleFactor,
                  m_geometry.width() * m_scaleFactor, m_geometry.height() * m_scaleFactor);
}

void VCWidget::setGeometry(QRectF rect)
{
    QRectF scaled = QRectF(rect.x() / m_scaleFactor, rect.y() / m_scaleFactor,
                           rect.width() / m_scaleFactor, rect.height() / m_scaleFactor);

    if (m_geometry == scaled)
        return;

    enqueueTardisAction(Tardis::VCWidgetGeometry, QVariant(m_geometry), QVariant(scaled));

    m_geometry = scaled;

    emit geometryChanged();
}

qreal VCWidget::scaleFactor() const
{
    return m_scaleFactor;
}

void VCWidget::setScaleFactor(qreal factor)
{
    if (m_scaleFactor == factor)
        return;

    m_scaleFactor = factor;

    emit geometryChanged();
}

/*********************************************************************
 * Allow resize
 *********************************************************************/

bool VCWidget::allowResize() const
{
    return m_allowResize;
}

void VCWidget::setAllowResize(bool allowResize)
{
    if (m_allowResize == allowResize)
        return;

    m_allowResize = allowResize;
    emit allowResizeChanged(allowResize);
}

/*********************************************************************
 * Disable state
 *********************************************************************/

bool VCWidget::isDisabled()
{
    return m_isDisabled;
}

void VCWidget::setDisabled(bool disable)
{
    if (m_isDisabled == disable)
        return;

    m_isDisabled = disable;
    setDocModified();
    emit disabledStateChanged(disable);
}

/*********************************************************************
 * Visibility state
 *********************************************************************/

void VCWidget::setVisible(bool isVisible)
{
    if (m_isVisible == isVisible)
        return;

    m_isVisible = isVisible;
    emit isVisibleChanged(isVisible);
}

bool VCWidget::isVisible() const
{
    return m_isVisible;
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

QString VCWidget::defaultCaption()
{
    return QString();
}

QString VCWidget::caption() const
{
    return m_caption;
}

void VCWidget::setCaption(QString caption)
{
    if (m_caption == caption)
        return;

    enqueueTardisAction(Tardis::VCWidgetCaption, m_caption, caption);
    m_caption = caption;

    emit captionChanged(caption);
}

/*****************************************************************************
 * Background color
 *****************************************************************************/

QColor VCWidget::backgroundColor() const
{
    return m_backgroundColor;
}

void VCWidget::setBackgroundColor(QColor backgroundColor)
{
    if (m_backgroundColor == backgroundColor)
        return;

    setBackgroundImage("");
    enqueueTardisAction(Tardis::VCWidgetBackgroundColor, m_backgroundColor, backgroundColor);

    m_backgroundColor = backgroundColor;
    m_hasCustomBackgroundColor = true;
    emit backgroundColorChanged(backgroundColor);
}

bool VCWidget::hasCustomBackgroundColor() const
{
    return m_hasCustomBackgroundColor;
}

void VCWidget::resetBackgroundColor()
{
    m_hasCustomBackgroundColor = false;
    m_backgroundColor = Qt::gray;
    setDocModified();
    emit backgroundColorChanged(m_backgroundColor);
}

/*********************************************************************
 * Background image
 *********************************************************************/
void VCWidget::setBackgroundImage(QString path)
{
    QString strippedPath = path.replace("file://", "");

    if (m_backgroundImage == strippedPath)
        return;

    enqueueTardisAction(Tardis::VCWidgetBackgroundImage, m_backgroundImage, strippedPath);

    m_hasCustomBackgroundColor = false;
    m_backgroundImage = strippedPath;

    emit backgroundImageChanged(strippedPath);
}

QString VCWidget::backgroundImage() const
{
    return m_backgroundImage;
}

/*****************************************************************************
 * Foreground color
 *****************************************************************************/

QColor VCWidget::foregroundColor() const
{
    return m_foregroundColor;
}

void VCWidget::setForegroundColor(QColor foregroundColor)
{
    if (m_foregroundColor == foregroundColor)
        return;

    enqueueTardisAction(Tardis::VCWidgetForegroundColor, m_foregroundColor, foregroundColor);

    m_foregroundColor = foregroundColor;
    m_hasCustomForegroundColor = true;

    emit foregroundColorChanged(foregroundColor);
}

bool VCWidget::hasCustomForegroundColor() const
{
    return m_hasCustomForegroundColor;
}

void VCWidget::resetForegroundColor()
{
    m_hasCustomForegroundColor = false;
    m_foregroundColor = Qt::white;
    setDocModified();
    emit foregroundColorChanged(m_foregroundColor);
}

void VCWidget::setDefaultFontSize(qreal size)
{
    m_font.setPixelSize(size);
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCWidget::setFont(const QFont& font)
{
    m_hasCustomFont = true;
    enqueueTardisAction(Tardis::VCWidgetFont, m_font, font);
    m_font = font;

    emit fontChanged();
}

QFont VCWidget::font() const
{
    return m_font;
}

bool VCWidget::hasCustomFont() const
{
    return m_hasCustomFont;
}

void VCWidget::resetFont()
{
    m_font = QFont("Roboto Condensed");
    m_font.setPixelSize(16);
    m_hasCustomFont = false;
    setDocModified();
    emit fontChanged();
}

/*********************************************************************
 * Page
 *********************************************************************/

void VCWidget::setPage(int pNum)
{
    if (pNum == m_page)
        return;

    m_page = pNum;
    emit pageChanged(pNum);
}

int VCWidget::page()
{
    return m_page;
}

/*********************************************************************
 * Widget Function
 *********************************************************************/

bool VCWidget::hasSoloParent()
{
    VCWidget *wParent = qobject_cast<VCWidget*>(parent());

    if (wParent == nullptr || wParent == this)
        return false;

    if (wParent->type() == VCWidget::SoloFrameWidget)
        return true;

    if (wParent->type() == VCWidget::FrameWidget)
        return wParent->hasSoloParent();

    return false;
}

void VCWidget::notifyFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity)
{
    Q_UNUSED(widget)
    Q_UNUSED(fid);
    Q_UNUSED(fIntensity);
}

/*********************************************************************
 * Intensity
 *********************************************************************/

void VCWidget::adjustFunctionIntensity(Function *f, qreal value)
{
    if (f == nullptr)
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

/*********************************************************************
 * QML Properties Component
 *********************************************************************/

bool VCWidget::isEditing() const
{
    return m_isEditing;
}

void VCWidget::setIsEditing(bool edit)
{
    if (edit == m_isEditing)
        return;

    m_isEditing = edit;
    emit isEditingChanged();
}

QString VCWidget::propertiesResource() const
{
    return QString();
}

/*********************************************************************
 * Controls
 *********************************************************************/

void VCWidget::registerExternalControl(quint8 id, QString name, bool allowKeyboard)
{
    ExternalControlInfo info;
    info.name = name;
    info.allowKeyboard = allowKeyboard;

    m_externalControlList.insert(id, info);
}

bool VCWidget::unregisterExternalControl(quint8 id)
{
    if (m_externalControlList.remove(id))
        return true;

    return false;
}

int VCWidget::externalControlsCount() const
{
    return m_externalControlList.count();
}

QVariant VCWidget::externalControlsList() const
{
    QVariantList controlsList;

    QMapIterator<quint8, ExternalControlInfo> it(m_externalControlList);
    while (it.hasNext())
    {
        it.next();
        ExternalControlInfo info = it.value();
        QVariantMap cMap;
        cMap.insert("mLabel", info.name);
        cMap.insert("mValue", it.key());
        controlsList.append(cMap);
    }

    return QVariant::fromValue(controlsList);
}

/*********************************************************************
 * Input sources
 *********************************************************************/

QSharedPointer<QLCInputSource> VCWidget::inputSource(quint32 universe, quint32 channel)
{
    for (QSharedPointer<QLCInputSource> &source : m_inputSources)
        if (source->universe() == universe && source->channel() == channel)
            return source;

    return QSharedPointer<QLCInputSource>();
}

QSharedPointer<QLCInputSource> VCWidget::inputSource(quint32 id, quint32 universe, quint32 channel) const
{
    for (QSharedPointer<QLCInputSource> source : m_inputSources)
    {
        if (source->id() == id && source->universe() == universe && source->channel() == channel)
            return source;
    }

    return QSharedPointer<QLCInputSource>();
}

QVariant VCWidget::inputSourceFullInfo(quint32 universe, quint32 channel)
{
    QSharedPointer<QLCInputSource> source = inputSource(universe, channel);
    if (source.isNull())
        return QVariant();

    uchar lowerValue = source->feedbackValue(QLCInputFeedback::LowerValue);
    uchar upperValue = source->feedbackValue(QLCInputFeedback::UpperValue);
    uchar monitorValue = source->feedbackValue(QLCInputFeedback::MonitorValue);

    QVariantMap infoMap;
    infoMap["lowerValue"] = lowerValue;
    infoMap["upperValue"] = upperValue;
    infoMap["monitorValue"] = monitorValue;

    InputPatch *ip = m_doc->inputOutputMap()->inputPatch(source->universe());
    if (ip != NULL && ip->profile() != NULL)
    {
        QLCInputProfile *profile = ip->profile();
        if (profile->hasColorTable())
        {
            infoMap["hasColorTable"] = true;
            QVariantList colorTable;
            QMapIterator <uchar, QPair<QString, QColor>> it(profile->colorTable());
            while (it.hasNext() == true)
            {
                it.next();
                QPair<QString, QColor> lc = it.value();
                QVariantMap colorInfoMap;
                colorInfoMap["index"] = it.key();
                colorInfoMap["name"] = lc.first;
                colorInfoMap["color"] = lc.second.name();

                if (it.key() == lowerValue)
                    infoMap["lowerColor"] = lc.second.name();
                if (it.key() == upperValue)
                    infoMap["upperColor"] = lc.second.name();
                if (it.key() == monitorValue)
                    infoMap["monitorColor"] = lc.second.name();

                colorTable.append(colorInfoMap);
            }
            infoMap["colorTable"] = QVariant::fromValue(colorTable);
        }
        else
        {
            infoMap["hasColorTable"] = false;
        }

        if (profile->type() == QLCInputProfile::MIDI && profile->hasMidiChannelTable())
        {
            infoMap["hasMIDIChannelTable"] = true;

            QVariantList midiChanelTable;

            midiChanelTable.append(tr("From plugin settings"));

            QMapIterator <uchar, QString> it(profile->midiChannelTable());
            while (it.hasNext() == true)
            {
                it.next();
                midiChanelTable.append(it.value());
            }

            QVariant extraParam = source->feedbackExtraParams(QLCInputFeedback::LowerValue);
            if (extraParam.isValid())
                infoMap["lowerChannel"] = extraParam.toInt() + 1;

            extraParam = source->feedbackExtraParams(QLCInputFeedback::UpperValue);
            if (extraParam.isValid())
                infoMap["upperChannel"] = extraParam.toInt() + 1;

            extraParam = source->feedbackExtraParams(QLCInputFeedback::MonitorValue);
            if (extraParam.isValid())
                infoMap["monitorChannel"] = extraParam.toInt() + 1;

            infoMap["midiChannelTable"] = QVariant::fromValue(midiChanelTable);
        }
        else
        {
            infoMap["hasMIDIChannelTable"] = false;
        }
    }

    return QVariant::fromValue(infoMap);
}

void VCWidget::addInputSource(QSharedPointer<QLCInputSource> const& source)
{
    if (source.isNull() || m_externalControlList.isEmpty())
        return;

    /** If the source ID is invalid, assign the first known ID to it.
     *  This is needed during the auto detection process, when the user
     *  haven't decided yet the source type */
    if (source->id() == QLCInputSource::invalidID)
        source->setID(m_externalControlList.firstKey());

    m_inputSources.append(source);

    // now check if the source is defined in the associated universe
    // profile and if it has specific settings
    InputPatch *ip = m_doc->inputOutputMap()->inputPatch(source->universe());
    if (ip != nullptr && ip->profile() != nullptr)
    {
        // Do not care about the page since input profiles don't do either
        QLCInputChannel *ich = ip->profile()->channel(source->channel() & 0x0000FFFF);
        if (ich != nullptr)
        {
            QLCInputProfile *profile = ip->profile();

            // retrieve plugin specific params for feedback
            if (source->feedbackExtraParams(QLCInputFeedback::LowerValue).toInt() == -1)
                source->setFeedbackExtraParams(QLCInputFeedback::LowerValue, profile->channelExtraParams(ich));
            if (source->feedbackExtraParams(QLCInputFeedback::UpperValue).toInt() == -1)
                source->setFeedbackExtraParams(QLCInputFeedback::UpperValue, profile->channelExtraParams(ich));
            if (source->feedbackExtraParams(QLCInputFeedback::MonitorValue).toInt() == -1)
                source->setFeedbackExtraParams(QLCInputFeedback::MonitorValue, profile->channelExtraParams(ich));

            if (ich->movementType() == QLCInputChannel::Relative)
            {
                source->setWorkingMode(QLCInputSource::Relative);
                source->setSensitivity(ich->movementSensitivity());
                connect(source.data(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                        this, SLOT(slotInputSourceValueChanged(quint32,quint32,uchar)));
            }
            else if (ich->type() == QLCInputChannel::Encoder)
            {
                source->setWorkingMode(QLCInputSource::Encoder);
                source->setSensitivity(ich->movementSensitivity());
                connect(source.data(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                        this, SLOT(slotInputSourceValueChanged(quint32,quint32,uchar)));
            }
            else if (ich->type() == QLCInputChannel::Button)
            {
                if (ich->sendExtraPress() == true)
                {
                    source->setSendExtraPressRelease(true);
                    connect(source.data(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                            this, SLOT(slotInputSourceValueChanged(quint32,quint32,uchar)));
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

    emit inputSourcesListChanged();
}

bool VCWidget::updateInputSource(QSharedPointer<QLCInputSource> const& source, quint32 universe, quint32 channel)
{
    if (source.isNull())
        return false;

    source->setUniverse(universe);
    source->setChannel(channel);
    source->setPage(page());

    emit inputSourcesListChanged();

    return true;
}

bool VCWidget::updateInputSourceControlID(quint32 universe, quint32 channel, quint32 id)
{
    QSharedPointer<QLCInputSource> source = inputSource(universe, channel);
    if (source.isNull())
        return false;

    source-> setID(id);

    return true;
}

bool VCWidget::updateInputSourceFeedbackValues(quint32 universe, quint32 channel,
                                               quint8 lower, quint8 upper, quint8 monitor)
{
    QSharedPointer<QLCInputSource> source = inputSource(universe, channel);
    if (source.isNull())
        return false;

    source->setFeedbackValue(QLCInputFeedback::LowerValue, lower);
    source->setFeedbackValue(QLCInputFeedback::UpperValue, upper);
    source->setFeedbackValue(QLCInputFeedback::MonitorValue, monitor);

    // TODO: tardis
    m_doc->setModified();

    updateFeedback();

    return true;
}

bool VCWidget::updateInputSourceExtraParams(quint32 universe, quint32 channel, int lower, int upper, int monitor)
{
    QSharedPointer<QLCInputSource> source = inputSource(universe, channel);
    if (source.isNull())
        return false;

    source->setFeedbackExtraParams(QLCInputFeedback::LowerValue, lower - 1);
    source->setFeedbackExtraParams(QLCInputFeedback::UpperValue, upper - 1);
    source->setFeedbackExtraParams(QLCInputFeedback::MonitorValue, monitor - 1);

    // TODO: tardis
    m_doc->setModified();

    updateFeedback();

    return true;
}

void VCWidget::deleteInputSurce(quint32 id, quint32 universe, quint32 channel)
{
    for (int i = 0; i < m_inputSources.count(); i++)
    {
        QSharedPointer<QLCInputSource> source = m_inputSources.at(i);

        if (source->id() == id && source->universe() == universe && source->channel() == channel)
        {
            m_inputSources.takeAt(i);
            source.clear();

            emit inputSourcesListChanged();
            break;
        }
    }
}

QList<QSharedPointer<QLCInputSource> > VCWidget::inputSources() const
{
    return m_inputSources;
}

QVariantList VCWidget::inputSourcesList()
{
    m_sourcesList.clear();

    for (QSharedPointer<QLCInputSource> &source : m_inputSources) // C++11
    {
        if (source.isNull())
            continue;

        QString uniName;
        QString chName;
        bool supportCustomFeedback = false;

        if (!source->isValid() || m_doc->inputOutputMap()->inputSourceNames(source, uniName, chName) == false)
        {
            uniName = tr("None");
            chName = tr("None");
        }

        InputPatch *ip = m_doc->inputOutputMap()->inputPatch(source->universe());
        if (ip != nullptr && ip->profile() != nullptr)
        {
            QLCInputChannel *ich = ip->profile()->channel(source->channel());
            if (ich != nullptr && ich->type() == QLCInputChannel::Button)
                supportCustomFeedback = true;
        }

        QVariantMap sourceMap;

        if (source->isValid() == false)
            sourceMap.insert("invalid", true);

        sourceMap.insert("type", Controller);
        sourceMap.insert("id", source->id());
        sourceMap.insert("uniString", uniName);
        sourceMap.insert("chString", chName);
        sourceMap.insert("universe", source->universe());
        sourceMap.insert("channel", source->channel());
        sourceMap.insert("customFeedback", supportCustomFeedback);
        m_sourcesList.append(sourceMap);
    }

    QMapIterator<QKeySequence, quint32> it(m_keySequenceMap);
    while (it.hasNext())
    {
        it.next();

        QKeySequence seq = it.key();
        quint32 id = it.value();

        QVariantMap keyMap;
        keyMap.insert("type", Keyboard);
        keyMap.insert("id", id);

        if (seq.isEmpty())
        {
            keyMap.insert("invalid", true);
            keyMap.insert("keySequence", "");
        }
        else
            keyMap.insert("keySequence", seq.toString());
        m_sourcesList.append(keyMap);
    }

    return m_sourcesList;
}

void VCWidget::slotInputValueChanged(quint8 id, uchar value)
{
    Q_UNUSED(id)
    Q_UNUSED(value)
}

void VCWidget::slotInputSourceValueChanged(quint32 universe, quint32 channel, uchar value)
{
    Q_UNUSED(universe)
    Q_UNUSED(channel)

    QLCInputSource *source = qobject_cast<QLCInputSource*>(sender());
    slotInputValueChanged(source->id(), value);
}

void VCWidget::sendFeedback(int value, quint8 id, SourceValueType type)
{
    if (isDisabled())
        return;

    for (QSharedPointer<QLCInputSource> &source : m_inputSources) // C++11
    {
        if (source->id() != id)
            continue;

        QVariant extraParam = source->feedbackExtraParams(QLCInputFeedback::UpperValue);

        if (type == LowerValue)
        {
            value = source->feedbackValue(QLCInputFeedback::LowerValue);
            extraParam = source->feedbackExtraParams(QLCInputFeedback::LowerValue);
        }
        else if (type == UpperValue)
        {
            value = source->feedbackValue(QLCInputFeedback::UpperValue);
        }
        else if (type == MonitorValue)
        {
            value = source->feedbackValue(QLCInputFeedback::MonitorValue);
            extraParam = source->feedbackExtraParams(QLCInputFeedback::MonitorValue);
        }

        // if in relative mode, send a "feedback" to this
        // input source so it can continue to emit values
        // from the right position
        if (source->needsUpdate())
            source->updateOuputValue(value);

        if (isDisabled()) // was acceptsInput()
            return;

        m_doc->inputOutputMap()->sendFeedBack(source->universe(), source->channel(), value, extraParam);
        return;
    }
}

void VCWidget::updateFeedback()
{
    /* NOP */
}

/*********************************************************************
 * Key sequences
 *********************************************************************/

void VCWidget::addKeySequence(const QKeySequence &keySequence, const quint32 &id)
{
    m_keySequenceMap[keySequence] = id;
    setDocModified();

    emit inputSourcesListChanged();
}

void VCWidget::deleteKeySequence(const QKeySequence &keySequence)
{
    m_keySequenceMap.remove(keySequence);
    setDocModified();

    emit inputSourcesListChanged();
}

void VCWidget::updateKeySequence(QKeySequence oldSequence, QKeySequence newSequence, const quint32 id)
{
    if (m_keySequenceMap.contains(oldSequence) == false)
        qDebug() << "Old key sequence not found!";

    m_keySequenceMap.remove(oldSequence);
    m_keySequenceMap[newSequence] = id;

    qDebug() << "Key sequence map items:" << m_keySequenceMap.count();

    setDocModified();

    emit inputSourcesListChanged();
}

void VCWidget::updateKeySequenceControlID(QKeySequence sequence, quint32 id)
{
    m_keySequenceMap[sequence] = id;
    //emit inputSourcesListChanged();
}

QMap<QKeySequence, quint32> VCWidget::keySequenceMap() const
{
    return m_keySequenceMap;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCWidget::loadXML(QXmlStreamReader &root)
{
    Q_UNUSED(root)
    return false;
}

bool VCWidget::saveXML(QXmlStreamWriter *doc)
{
    Q_UNUSED(doc)
    return false;
}

bool VCWidget::loadXMLCommon(QXmlStreamReader &root)
{
    if (root.device() == nullptr || root.hasError())
        return false;

    QXmlStreamAttributes attrs = root.attributes();

    /* ID */
    if (attrs.hasAttribute(KXMLQLCVCWidgetID))
        setID(attrs.value(KXMLQLCVCWidgetID).toUInt());

    /* Caption */
    if (attrs.hasAttribute(KXMLQLCVCCaption))
        setCaption(attrs.value(KXMLQLCVCCaption).toString());

    /* Page */
    if (attrs.hasAttribute(KXMLQLCVCWidgetPage))
        setPage(attrs.value(KXMLQLCVCWidgetPage).toInt());

    return true;
}

bool VCWidget::loadXMLAppearance(QXmlStreamReader &root)
{
    if (root.device() == nullptr || root.hasError())
        return false;

    if (root.name() != KXMLQLCVCWidgetAppearance)
    {
        qWarning() << Q_FUNC_INFO << "Appearance node not found!";
        return false;
    }

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCWidgetForegroundColor)
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
        else if (root.name() == KXMLQLCVCFrameStyle)
        {
            /** LEGACY: no more supported/needed */
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown appearance tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCWidget::loadXMLWindowState(QXmlStreamReader &root, int* x, int* y,
                                  int* w, int* h, bool* visible)
{
    if (root.device() == nullptr || x == nullptr || y == nullptr || w == nullptr || h == nullptr ||
            visible == nullptr)
        return false;

    if (root.name() == KXMLQLCWindowState)
    {
        QXmlStreamAttributes attrs = root.attributes();
        *x = attrs.value(KXMLQLCWindowStateX).toInt();
        *y = attrs.value(KXMLQLCWindowStateY).toInt();
        *w = attrs.value(KXMLQLCWindowStateWidth).toInt();
        *h = attrs.value(KXMLQLCWindowStateHeight).toInt();

        if (attrs.value(KXMLQLCWindowStateVisible).toString() == KXMLQLCTrue)
            *visible = true;
        else
            *visible = false;
        root.skipCurrentElement();

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Window state not found";
        return false;
    }
}

bool VCWidget::loadXMLInputSource(QXmlStreamReader &root, const quint8 &id)
{
    if (root.device() == nullptr || root.hasError())
        return false;

    if (root.name() != KXMLQLCVCWidgetInput)
        return false;

    QXmlStreamAttributes attrs = root.attributes();

    quint32 uni = UINT_MAX;
    quint32 ch = UINT_MAX;
    quint8 iId = id;
    uchar min = 0, max = UCHAR_MAX, mon = UCHAR_MAX;
    QString key;

    if (attrs.hasAttribute(KXMLQLCVCWidgetInputId))
        iId = attrs.value(KXMLQLCVCWidgetInputId).toString().toUInt();
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputUniverse))
        uni = attrs.value(KXMLQLCVCWidgetInputUniverse).toString().toUInt();
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputChannel))
        ch = attrs.value(KXMLQLCVCWidgetInputChannel).toString().toUInt();
    if (attrs.hasAttribute(KXMLQLCVCWidgetKey))
        key = attrs.value(KXMLQLCVCWidgetKey).toString();

    // check for valid input source
    if (uni != UINT_MAX && ch != UINT_MAX)
    {
        QSharedPointer<QLCInputSource>inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch));
        inputSource->setID(iId);

        if (attrs.hasAttribute(KXMLQLCVCWidgetInputLowerValue))
            min = uchar(attrs.value(KXMLQLCVCWidgetInputLowerValue).toString().toUInt());
        if (attrs.hasAttribute(KXMLQLCVCWidgetInputUpperValue))
            max = uchar(attrs.value(KXMLQLCVCWidgetInputUpperValue).toString().toUInt());
        if (attrs.hasAttribute(KXMLQLCVCWidgetInputMonitorValue))
            mon = uchar(attrs.value(KXMLQLCVCWidgetInputMonitorValue).toString().toUInt());

        inputSource->setFeedbackValue(QLCInputFeedback::LowerValue, min);
        inputSource->setFeedbackValue(QLCInputFeedback::UpperValue, max);
        inputSource->setFeedbackValue(QLCInputFeedback::MonitorValue, mon);

        // load feedback extra params
        if (attrs.hasAttribute(KXMLQLCVCWidgetInputLowerParams))
            inputSource->setFeedbackExtraParams(QLCInputFeedback::LowerValue, attrs.value(KXMLQLCVCWidgetInputLowerParams).toInt());
        if (attrs.hasAttribute(KXMLQLCVCWidgetInputUpperParams))
            inputSource->setFeedbackExtraParams(QLCInputFeedback::UpperValue, attrs.value(KXMLQLCVCWidgetInputUpperParams).toInt());
        if (attrs.hasAttribute(KXMLQLCVCWidgetInputMonitorParams))
            inputSource->setFeedbackExtraParams(QLCInputFeedback::MonitorValue, attrs.value(KXMLQLCVCWidgetInputMonitorParams).toInt());

        addInputSource(inputSource);
    }

    // check for valid key sequence
    if (!key.isEmpty())
    {
        QKeySequence seq(key);
        addKeySequence(seq, iId);
    }

    root.skipCurrentElement();

    return true;
}

bool VCWidget::loadXMLInputKey(QXmlStreamReader &root, const quint8 &id)
{
    if (root.device() == nullptr || root.hasError())
        return false;

    if (root.name() != KXMLQLCVCWidgetKey)
        return false;

    QKeySequence seq(root.readElementText());
    if (seq.isEmpty())
    {
        qDebug() << "Empty key sequence detected";
        return false;
    }

    addKeySequence(seq, id);

    return true;
}

bool VCWidget::loadXMLSources(QXmlStreamReader &root, const quint8 &id)
{
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInputSource(root, id);
        }
        else if (root.name() == KXMLQLCVCWidgetKey)
        {
            loadXMLInputKey(root, id);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown source tag" << root.name().toString();
            root.skipCurrentElement();
        }
    }
    return true;
}

bool VCWidget::saveXMLCommon(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

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
    Q_ASSERT(doc != nullptr);

    QString str;

    if (hasCustomForegroundColor() == false &&
        hasCustomBackgroundColor() == false &&
        //backgroundImage().isEmpty() &&
        hasCustomFont() == false)
            return true;

    /* VC widget appearance entry */
    doc->writeStartElement(KXMLQLCVCWidgetAppearance);

    /* Foreground color */
    if (hasCustomForegroundColor() == true)
    {
        str.setNum(foregroundColor().rgb());
    //else
    //    str = KXMLQLCVCWidgetColorDefault;
        doc->writeTextElement(KXMLQLCVCWidgetForegroundColor, str);
    }

    /* Background color */
    if (hasCustomBackgroundColor() == true)
    {
        str.setNum(backgroundColor().rgb());
    //else
    //    str = KXMLQLCVCWidgetColorDefault;
        doc->writeTextElement(KXMLQLCVCWidgetBackgroundColor, str);
    }

    /* Background image */
    if (backgroundImage().isEmpty() == false)
    {
        doc->writeTextElement(KXMLQLCVCWidgetBackgroundImage,
                              m_doc->normalizeComponentPath(m_backgroundImage));
    }

    /* Font */
    if (hasCustomFont() == true)
    {
        str = font().toString();
    //else
    //    str = KXMLQLCVCWidgetFontDefault;
        doc->writeTextElement(KXMLQLCVCWidgetFont, str);
    }

    /* End the <Appearance> tag */
    doc->writeEndElement();

    return true;
}

bool VCWidget::saveXMLWindowState(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

    QRectF r = geometry();

    /* Window state tag */
    doc->writeStartElement(KXMLQLCWindowState);

    /* Visible status */
    if (isVisible() == true)
        doc->writeAttribute(KXMLQLCWindowStateVisible, KXMLQLCTrue);
    else
        doc->writeAttribute(KXMLQLCWindowStateVisible, KXMLQLCFalse);

    doc->writeAttribute(KXMLQLCWindowStateX, QString::number((int)r.x()));
    doc->writeAttribute(KXMLQLCWindowStateY, QString::number((int)r.y()));
    doc->writeAttribute(KXMLQLCWindowStateWidth, QString::number((int)r.width()));
    doc->writeAttribute(KXMLQLCWindowStateHeight, QString::number((int)r.height()));

    doc->writeEndElement();

    return true;
}

bool VCWidget::saveXMLInputControl(QXmlStreamWriter *doc, quint8 controlId, bool unified, QString tagName)
{
    Q_ASSERT(doc != nullptr);

    bool controlTagWritten = false;
    bool inputTagWritten = false;

    for (QSharedPointer<QLCInputSource> &source : m_inputSources) // C++11
    {
        if (source->id() != controlId)
            continue;

        if (controlTagWritten == false && tagName.isEmpty() == false)
        {
            doc->writeStartElement(tagName);
            controlTagWritten = true;
        }

        if (unified == false || (unified == true && inputTagWritten == false))
        {
            doc->writeStartElement(KXMLQLCVCWidgetInput);
            doc->writeAttribute(KXMLQLCVCWidgetInputId, QString::number(controlId));
            inputTagWritten = true;
        }

        doc->writeAttribute(KXMLQLCVCWidgetInputUniverse, QString("%1").arg(source->universe()));
        doc->writeAttribute(KXMLQLCVCWidgetInputChannel, QString("%1").arg(source->channel()));

        if (source->feedbackValue(QLCInputFeedback::LowerValue) != 0)
            doc->writeAttribute(KXMLQLCVCWidgetInputLowerValue, QString::number(source->feedbackValue(QLCInputFeedback::LowerValue)));
        if (source->feedbackValue(QLCInputFeedback::UpperValue) != UCHAR_MAX)
            doc->writeAttribute(KXMLQLCVCWidgetInputUpperValue, QString::number(source->feedbackValue(QLCInputFeedback::UpperValue)));
        if (source->feedbackValue(QLCInputFeedback::MonitorValue) != UCHAR_MAX)
            doc->writeAttribute(KXMLQLCVCWidgetInputMonitorValue, QString::number(source->feedbackValue(QLCInputFeedback::MonitorValue)));

        // save feedback extra params
        QVariant extraParams = source->feedbackExtraParams(QLCInputFeedback::LowerValue);

        if (extraParams.isValid() && extraParams.type() == QVariant::Int && extraParams.toInt() != -1)
            doc->writeAttribute(KXMLQLCVCWidgetInputLowerParams, QString::number(extraParams.toInt()));

        extraParams = source->feedbackExtraParams(QLCInputFeedback::UpperValue);

        if (extraParams.isValid() && extraParams.type() == QVariant::Int && extraParams.toInt() != -1)
            doc->writeAttribute(KXMLQLCVCWidgetInputUpperParams, QString::number(extraParams.toInt()));

        extraParams = source->feedbackExtraParams(QLCInputFeedback::MonitorValue);

        if (extraParams.isValid() && extraParams.type() == QVariant::Int && extraParams.toInt() != -1)
            doc->writeAttribute(KXMLQLCVCWidgetInputMonitorParams, QString::number(extraParams.toInt()));
    }

    auto i = m_keySequenceMap.constBegin();
    while (i != m_keySequenceMap.constEnd())
    {
        if (i.value() != controlId)
        {
            ++i;
            continue;
        }

        if (controlTagWritten == false && tagName.isEmpty() == false)
        {
            doc->writeStartElement(tagName);
            controlTagWritten = true;
        }

        if (unified == true && inputTagWritten == false)
        {
            doc->writeStartElement(KXMLQLCVCWidgetInput);
            doc->writeAttribute(KXMLQLCVCWidgetInputId, QString::number(controlId));
            inputTagWritten = true;
        }

        if (unified == false)
            doc->writeTextElement(KXMLQLCVCWidgetKey, i.key().toString());
        else
            doc->writeAttribute(KXMLQLCVCWidgetKey, i.key().toString());

        ++i;
    }

    if (inputTagWritten)
        doc->writeEndElement();

    if (controlTagWritten)
        doc->writeEndElement();

    return true;
}

