/*
  Q Light Controller Plus
  vcanimation.cpp

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
#include <QQmlEngine>

#include "doc.h"
#include "rgbmatrix.h"
#include "vcanimation.h"
#include "tardis.h"

#define INPUT_FADER_ID 0

VCAnimation::VCAnimation(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_functionID(Function::invalidId())
    , m_faderLevel(0)
    , m_instantChanges(true)
    , m_localAlgorithmIndex(0)
    , m_algorithmOverrideID(Function::invalidAttributeId())
{
    setType(VCWidget::AnimationWidget);

    registerExternalControl(INPUT_FADER_ID, tr("Intensity"), true);

    m_visibilityMask = defaultVisibilityMask();

    for (int i = 0; i < RGBAlgorithmColorDisplayCount; ++i)
    {
        m_localColors[i] = QColor();
        m_colorOverrideIDs[i] = Function::invalidAttributeId();
    }
}

VCAnimation::~VCAnimation()
{
    RGBMatrix *matrix = currentMatrix();
    if (matrix != nullptr)
    {
        releaseStyleOverrides(matrix);
        releaseIntensityOverride(matrix);
        matrix->stop(functionParent());
        matrix->applyStyleAttributes();
    }

    if (m_item)
        delete m_item;
}

QString VCAnimation::defaultCaption() const
{
    return tr("Animation %1").arg(id() + 1);
}

void VCAnimation::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    setDefaultFontSize(pixelDensity * 3.5);
}

void VCAnimation::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCAnimationItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        delete component;
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("animationObj", QVariant::fromValue(this));
}

QString VCAnimation::propertiesResource() const
{
    return QString("qrc:/VCAnimationProperties.qml");
}

VCWidget *VCAnimation::createCopy(VCWidget *parent) const
{
    Q_ASSERT(parent != nullptr);

    VCAnimation *animation = new VCAnimation(m_doc, parent);
    if (animation->copyFrom(this) == false)
    {
        delete animation;
        animation = nullptr;
    }

    return animation;
}

bool VCAnimation::copyFrom(const VCWidget *widget)
{
    const VCAnimation *animation = qobject_cast<const VCAnimation*> (widget);
    if (animation == nullptr)
        return false;

    /* Copy and set properties */
    setFunctionID(animation->functionID());
    setInstantChanges(animation->instantChanges());
    setVisibilityMask(animation->visibilityMask());

    /* Copy object lists */

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

FunctionParent VCAnimation::functionParent() const
{
    return FunctionParent(FunctionParent::AutoVCWidget, id());
}

/*********************************************************************
 * UI elements visibility
 *********************************************************************/

quint32 VCAnimation::defaultVisibilityMask() const
{
    return Fader | Label | Color1 | Color2 | PresetCombo;
}

quint32 VCAnimation::visibilityMask() const
{
    return m_visibilityMask;
}

void VCAnimation::setVisibilityMask(quint32 mask)
{
    if (mask == m_visibilityMask)
        return;

    m_visibilityMask = mask;
    emit visibilityMaskChanged();
}

/*********************************************************************
 * Function control
 *********************************************************************/

quint32 VCAnimation::functionID() const
{
    return m_functionID;
}

void VCAnimation::setFunctionID(quint32 newFunctionID)
{
    if (m_functionID == newFunctionID)
        return;

    RGBMatrix *oldMatrix = currentMatrix();
    if (oldMatrix != nullptr)
    {
        releaseStyleOverrides(oldMatrix);
        releaseIntensityOverride(oldMatrix);
        oldMatrix->stop(functionParent());
        oldMatrix->applyStyleAttributes();
    }

    RGBMatrix *matrix = qobject_cast<RGBMatrix*>(m_doc->function(newFunctionID));
    if (matrix == nullptr)
    {
        m_functionID = Function::invalidId();
        for (int i = 0; i < RGBAlgorithmColorDisplayCount; ++i)
            m_localColors[i] = QColor();
        m_localAlgorithmIndex = 0;
    }
    else
    {
        m_functionID = newFunctionID;
        for (int i = 0; i < RGBAlgorithmColorDisplayCount; ++i)
            m_localColors[i] = matrix->getColor(i);
        m_localAlgorithmIndex = matrix->algorithmIndex();
    }

    for (int i = 0; i < RGBAlgorithmColorDisplayCount; ++i)
        m_colorOverrideIDs[i] = Function::invalidAttributeId();
    m_algorithmOverrideID = Function::invalidAttributeId();
    if (m_faderLevel != 0)
    {
        m_faderLevel = 0;
        emit faderLevelChanged();
    }

    emit functionIDChanged();
    emit color1Changed();
    emit color2Changed();
    emit color3Changed();
    emit color4Changed();
    emit color5Changed();
    emit colorsChanged();
    emit algorithmIndexChanged();
}

int VCAnimation::faderLevel() const
{
    return m_faderLevel;
}

void VCAnimation::setFaderLevel(int level)
{
    if (m_faderLevel == level)
        return;

    RGBMatrix *matrix = currentMatrix();
    if (matrix == nullptr)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCAnimationSetFaderLevel, id(), m_faderLevel, level);

    if (level == 0)
    {
        // Make sure we ignore the fade out time
        adjustFunctionIntensity(matrix, 0);
        releaseIntensityOverride(matrix);
        releaseStyleOverrides(matrix);
        matrix->stop(functionParent());
        matrix->applyStyleAttributes();
    }
    else
    {
        qreal pIntensity = qreal(level) / qreal(UCHAR_MAX);
        emit functionStarting(this, m_functionID, pIntensity);
        applyStyleOverrides(matrix);
        adjustFunctionIntensity(matrix, pIntensity * intensity());
        if (matrix->stopped() == true)
            matrix->start(m_doc->masterTimer(), functionParent());
    }

    m_faderLevel = level;
    emit faderLevelChanged();
}


bool VCAnimation::instantChanges() const
{
    return m_instantChanges;
}

void VCAnimation::setInstantChanges(bool newInstantChanges)
{
    if (m_instantChanges == newInstantChanges)
        return;

    m_instantChanges = newInstantChanges;
    emit instantChangesChanged();
}

/*********************************************************************
 * Colors and presets
 *********************************************************************/

QColor VCAnimation::getColor1() const
{
    return m_localColors[0];
}

void VCAnimation::setColor1(QColor color)
{
    if (m_localColors[0] == color)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCAnimationSetColor1, id(), m_localColors[0], color);
    m_localColors[0] = color;

    if (m_faderLevel > 0)
    {
        RGBMatrix *matrix = currentMatrix();
        if (matrix != nullptr)
            applyStyleOverrides(matrix);
    }

    emit color1Changed();
    emit colorsChanged();
}

QColor VCAnimation::getColor2() const
{
    return m_localColors[1];
}

void VCAnimation::setColor2(QColor color)
{
    if (m_localColors[1] == color)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCAnimationSetColor2, id(), m_localColors[1], color);
    m_localColors[1] = color;

    if (m_faderLevel > 0)
    {
        RGBMatrix *matrix = currentMatrix();
        if (matrix != nullptr)
            applyStyleOverrides(matrix);
    }

    emit color2Changed();
    emit colorsChanged();
}

QColor VCAnimation::getColor3() const
{
    return m_localColors[2];
}

void VCAnimation::setColor3(QColor color)
{
    if (m_localColors[2] == color)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCAnimationSetColor3, id(), m_localColors[2], color);
    m_localColors[2] = color;

    if (m_faderLevel > 0)
    {
        RGBMatrix *matrix = currentMatrix();
        if (matrix != nullptr)
            applyStyleOverrides(matrix);
    }

    emit color3Changed();
    emit colorsChanged();
}

QColor VCAnimation::getColor4() const
{
    return m_localColors[3];
}

void VCAnimation::setColor4(QColor color)
{
    if (m_localColors[3] == color)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCAnimationSetColor4, id(), m_localColors[3], color);
    m_localColors[3] = color;

    if (m_faderLevel > 0)
    {
        RGBMatrix *matrix = currentMatrix();
        if (matrix != nullptr)
            applyStyleOverrides(matrix);
    }

    emit color4Changed();
    emit colorsChanged();
}

QColor VCAnimation::getColor5() const
{
    return m_localColors[4];
}

void VCAnimation::setColor5(QColor color)
{
    if (m_localColors[4] == color)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCAnimationSetColor5, id(), m_localColors[4], color);
    m_localColors[4] = color;

    if (m_faderLevel > 0)
    {
        RGBMatrix *matrix = currentMatrix();
        if (matrix != nullptr)
            applyStyleOverrides(matrix);
    }

    emit color5Changed();
    emit colorsChanged();
}

int VCAnimation::colorCount() const
{
    if (m_doc == nullptr || m_localAlgorithmIndex < 0)
        return 0;

    QStringList algoList = algorithms();
    if (m_localAlgorithmIndex >= algoList.count())
        return 0;

    RGBAlgorithm *algorithm = RGBAlgorithm::algorithm(m_doc, algoList.at(m_localAlgorithmIndex));
    if (algorithm == nullptr)
        return 0;

    int acceptedColors = qBound(0, algorithm->acceptColors(), RGBAlgorithmColorDisplayCount);
    delete algorithm;
    return acceptedColors;
}

QVariantList VCAnimation::colors() const
{
    QVariantList ret;
    ret.reserve(RGBAlgorithmColorDisplayCount);

    for (int i = 0; i < RGBAlgorithmColorDisplayCount; ++i)
        ret << m_localColors[i];

    return ret;
}

QColor VCAnimation::colorAt(int index) const
{
    if (index < 0 || index >= RGBAlgorithmColorDisplayCount)
        return QColor();

    return m_localColors[index];
}

void VCAnimation::setColorAt(int index, QColor color)
{
    switch (index)
    {
    case 0: setColor1(color); break;
    case 1: setColor2(color); break;
    case 2: setColor3(color); break;
    case 3: setColor4(color); break;
    case 4: setColor5(color); break;
    default:
        if (index < 0 || index >= RGBAlgorithmColorDisplayCount || m_localColors[index] == color)
            return;

        m_localColors[index] = color;
        if (m_faderLevel > 0)
        {
            RGBMatrix *matrix = currentMatrix();
            if (matrix != nullptr)
                applyStyleOverrides(matrix);
        }

        emit colorsChanged();
        break;
    }
}

QStringList VCAnimation::algorithms() const
{
    return RGBAlgorithm::algorithms(m_doc);
}

int VCAnimation::algorithmIndex() const
{
    return m_localAlgorithmIndex;
}

void VCAnimation::setAlgorithmIndex(int index)
{
    QStringList algoList = algorithms();
    if (index < 0 || index >= algoList.count() || m_localAlgorithmIndex == index)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCAnimationSetAlgorithmIndex, id(), m_localAlgorithmIndex, index);
    m_localAlgorithmIndex = index;

    if (m_faderLevel > 0)
    {
        RGBMatrix *matrix = currentMatrix();
        if (matrix != nullptr)
            applyStyleOverrides(matrix);
    }

    emit algorithmIndexChanged();
}

void VCAnimation::updateFeedback()
{

}

void VCAnimation::slotInputValueChanged(quint8 id, uchar value)
{
    if (id == INPUT_FADER_ID)
        setFaderLevel(value);
}

RGBMatrix *VCAnimation::currentMatrix() const
{
    if (m_functionID == Function::invalidId())
        return nullptr;

    return qobject_cast<RGBMatrix *>(m_doc->function(m_functionID));
}

void VCAnimation::applyStyleOverrides(RGBMatrix *matrix)
{
    if (matrix == nullptr)
        return;

    for (int i = 0; i < RGBAlgorithmColorDisplayCount; ++i)
    {
        int attrIndex = RGBMatrix::Color1Attr + i;
        int colorValue = packColorForOverride(m_localColors[i]);
        if (m_colorOverrideIDs[i] == Function::invalidAttributeId())
            m_colorOverrideIDs[i] = matrix->requestAttributeOverride(attrIndex, colorValue);
        else
            matrix->adjustAttribute(colorValue, m_colorOverrideIDs[i]);
    }

    if (m_algorithmOverrideID == Function::invalidAttributeId())
        m_algorithmOverrideID = matrix->requestAttributeOverride(RGBMatrix::PatternAttr, m_localAlgorithmIndex);
    else
        matrix->adjustAttribute(m_localAlgorithmIndex, m_algorithmOverrideID);
}

void VCAnimation::releaseStyleOverrides(RGBMatrix *matrix)
{
    if (matrix == nullptr)
        return;

    bool released = false;
    for (int i = 0; i < RGBAlgorithmColorDisplayCount; ++i)
    {
        if (m_colorOverrideIDs[i] == Function::invalidAttributeId())
            continue;

        matrix->releaseAttributeOverride(m_colorOverrideIDs[i]);
        m_colorOverrideIDs[i] = Function::invalidAttributeId();
        released = true;
    }

    if (m_algorithmOverrideID != Function::invalidAttributeId())
    {
        matrix->releaseAttributeOverride(m_algorithmOverrideID);
        m_algorithmOverrideID = Function::invalidAttributeId();
        released = true;
    }

    if (released)
        matrix->applyStyleAttributes();
}

void VCAnimation::releaseIntensityOverride(RGBMatrix *matrix)
{
    if (matrix == nullptr || m_intensityOverrideId == Function::invalidAttributeId())
        return;

    matrix->releaseAttributeOverride(m_intensityOverrideId);
    resetIntensityOverrideAttribute();
}

int VCAnimation::packColorForOverride(const QColor &color)
{
    return color.isValid() ? int(color.rgb() & 0x00FFFFFF) : -1;
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCAnimation::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCAnimation)
    {
        qWarning() << Q_FUNC_INFO << "Animation node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();

    /* Widget commons */
    loadXMLCommon(root);

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCAnimationFunction)
        {
            QXmlStreamAttributes attrs = root.attributes();
            setFunctionID(attrs.value(KXMLQLCVCAnimationFunctionID).toUInt());
            if (attrs.hasAttribute(KXMLQLCVCAnimationInstantApply))
                setInstantChanges(true);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCAnimationVisibilityMask)
        {
            setVisibilityMask(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInputSource(root, INPUT_FADER_ID);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown animation tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCAnimation::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != nullptr);

    /* VC object entry */
    doc->writeStartElement(KXMLQLCVCAnimation);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Function */
    doc->writeStartElement(KXMLQLCVCAnimationFunction);
    doc->writeAttribute(KXMLQLCVCAnimationFunctionID, QString::number(functionID()));

    if (instantChanges() == true)
        doc->writeAttribute(KXMLQLCVCAnimationInstantApply, "true");
    doc->writeEndElement();

    /* Controls visibility mask */
    if (m_visibilityMask != defaultVisibilityMask())
        doc->writeTextElement(KXMLQLCVCAnimationVisibilityMask, QString::number(m_visibilityMask));

    /* External control */
    saveXMLInputControl(doc, INPUT_FADER_ID, false);

    /* Write the <end> tag */
    doc->writeEndElement();

    return true;
}
