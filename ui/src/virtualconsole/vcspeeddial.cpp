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
#include "speeddial.h"
#include "qlcmacros.h"
#include "qlcfile.h"
#include "function.h"

const quint8 VCSpeedDial::absoluteInputSourceId = 0;
const quint8 VCSpeedDial::tapInputSourceId = 1;
const QSize VCSpeedDial::defaultSize(QSize(200, 175));

/****************************************************************************
 * Initialization
 ****************************************************************************/

VCSpeedDial::VCSpeedDial(QWidget* parent, Doc* doc)
    : VCWidget(parent, doc)
    , m_dial(NULL)
    , m_absoluteValueMin(0)
    , m_absoluteValueMax(1000 * 10)
    , m_visibilityMask(SpeedDial::defaultVisibilityMask())
{
    new QVBoxLayout(this);
    layout()->setMargin(0);

    m_dial = new SpeedDial(this);
    layout()->addWidget(m_dial);
    connect(m_dial, SIGNAL(valueChanged(int)), this, SLOT(slotDialValueChanged(int)));
    connect(m_dial, SIGNAL(tapped()), this, SLOT(slotDialTapped()));

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

    slotModeChanged(m_doc->mode());
    setLiveEdit(m_liveEdit);
}

VCSpeedDial::~VCSpeedDial()
{
}

void VCSpeedDial::enableWidgetUI(bool enable)
{
    m_dial->setEnabled(enable);
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

    m_functions = dial->functions();

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

void VCSpeedDial::setFunctions(const QList <VCSpeedDialFunction> functions)
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

void VCSpeedDial::slotDialValueChanged(int ms)
{
    const QVector <quint32> multipliers = VCSpeedDialFunction::speedMultiplierValuesTimes1000();

    foreach (const VCSpeedDialFunction &speeddialfunction, m_functions)
    {
        Function* function = m_doc->function(speeddialfunction.functionId);
        if (function != NULL)
        {
            if (speeddialfunction.fadeInMultiplier != VCSpeedDialFunction::None)
                function->setFadeInSpeed(ms * multipliers[speeddialfunction.fadeInMultiplier] / 1000);
            if (speeddialfunction.fadeOutMultiplier != VCSpeedDialFunction::None)
                function->setFadeOutSpeed(ms * multipliers[speeddialfunction.fadeOutMultiplier] / 1000);
            if (speeddialfunction.durationMultiplier != VCSpeedDialFunction::None)
                function->setDuration(ms * multipliers[speeddialfunction.durationMultiplier] / 1000);
        }
    }
    updateFeedback();
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

/*****************************************************************************
+ * External input
+ *****************************************************************************/

void VCSpeedDial::updateFeedback()
{
    int fbv = (int)SCALE(float(m_dial->value()), float(m_absoluteValueMin),
                     float(m_absoluteValueMax), float(0), float(UCHAR_MAX));

    sendFeedback(fbv, absoluteInputSourceId);
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
    else if (checkInputSource(universe, pagedCh, value, sender(), absoluteInputSourceId))
    {
        int ms = static_cast<int> (SCALE(qreal(value), qreal(0), qreal(255),
                                         qreal(absoluteValueMin()),
                                         qreal(absoluteValueMax())));
        m_dial->setValue(ms, true);
    }
}

/*********************************************************************
 * Tap key sequence handler
 *********************************************************************/

void VCSpeedDial::setKeySequence(const QKeySequence& keySequence)
{
    m_tapKeySequence = QKeySequence(keySequence);
}

QKeySequence VCSpeedDial::keySequence() const
{
    return m_tapKeySequence;
}

void VCSpeedDial::slotKeyPressed(const QKeySequence& keySequence)
{
    if (isEnabled() == false)
        return;

    if (m_tapKeySequence == keySequence)
        m_dial->tap();
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

ushort VCSpeedDial::visibilityMask()
{
    return m_visibilityMask;
}

void VCSpeedDial::setVisibilityMask(ushort mask)
{
    if (m_dial != NULL)
        m_dial->setVisibilityMask(mask);
    m_visibilityMask = mask;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

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
                        setInputSource(new QLCInputSource(uni, ch), absoluteInputSourceId);
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
                        setInputSource(new QLCInputSource(uni, ch), tapInputSourceId);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown tap tag:" << tag.tagName();
                }

                sub = sub.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialTapKey)
        {
            setKeySequence(stripKeySequence(QKeySequence(tag.text())));
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
        else if (tag.tagName() == KXMLQLCVCSpeedDialVisibilityMask)
        {
            setVisibilityMask(tag.text().toUShort());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown speed dial tag:" << tag.tagName();
        }

        node = node.nextSibling();
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

    /* Key sequence */
    if (m_tapKeySequence.isEmpty() == false)
    {
        QDomElement tag = doc->createElement(KXMLQLCVCSpeedDialTapKey);
        root.appendChild(tag);
        QDomText text = doc->createTextNode(m_tapKeySequence.toString());
        tag.appendChild(text);
    }

    /* Functions */
    foreach (const VCSpeedDialFunction &speeddialfunction, m_functions)
    {
        speeddialfunction.saveXML(doc, &root);
    }

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
