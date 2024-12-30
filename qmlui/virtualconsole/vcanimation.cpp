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

#define INPUT_FADER_ID 0

VCAnimation::VCAnimation(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_matrix(nullptr)
    , m_faderLevel(0)
    , m_instantChanges(true)
{
    setType(VCWidget::AnimationWidget);

    registerExternalControl(INPUT_FADER_ID, tr("Intensity"), true);

    m_visibilityMask = defaultVisibilityMask();
}

VCAnimation::~VCAnimation()
{
    if (m_matrix)
        delete m_matrix;

    if (m_item)
        delete m_item;
}

QString VCAnimation::defaultCaption()
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

VCWidget *VCAnimation::createCopy(VCWidget *parent)
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

quint32 VCAnimation::defaultVisibilityMask()
{
    return Fader | Label | StartColor | EndColor | PresetCombo;
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

    RGBMatrix *matrix = qobject_cast<RGBMatrix*>(m_doc->function(newFunctionID));
    if (matrix == NULL)
        return;

    if (m_matrix != nullptr)
        delete m_matrix;

    Function *func = matrix->createCopy(m_doc, false);
    m_matrix = qobject_cast<RGBMatrix*>(func);

    m_functionID = newFunctionID;

    emit functionIDChanged();
    emit color1Changed();
    emit color2Changed();
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

    if (m_matrix == NULL)
        return;

    if (level == 0)
    {
        // Make sure we ignore the fade out time
        adjustFunctionIntensity(m_matrix, 0);
        if (m_matrix->stopped() == false)
        {
            m_matrix->stop(functionParent());
            resetIntensityOverrideAttribute();
        }
    }
    else
    {
        qreal pIntensity = qreal(level) / qreal(UCHAR_MAX);
        emit functionStarting(this, m_functionID, pIntensity);
        adjustFunctionIntensity(m_matrix, pIntensity * intensity());
        if (m_matrix->stopped() == true)
            m_matrix->start(m_doc->masterTimer(), functionParent());
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
    if (m_matrix == NULL)
        return QColor();

    return m_matrix->getColor(0);
}

void VCAnimation::setColor1(QColor color)
{
    if (m_matrix == NULL)
        return;

    if (m_matrix->getColor(0) != color)
    {
        m_matrix->setColor(0, color);
        if (instantChanges())
            m_matrix->updateColorDelta();
        emit color1Changed();
    }
}

QColor VCAnimation::getColor2() const
{
    if (m_matrix == NULL)
        return QColor();

    return m_matrix->getColor(1);
}

void VCAnimation::setColor2(QColor color)
{
    if (m_matrix == NULL)
        return;

    if (m_matrix->getColor(1) != color)
    {
        m_matrix->setColor(1, color);
        if (instantChanges())
            m_matrix->updateColorDelta();
        emit color2Changed();
    }
}

QColor VCAnimation::getColor3() const
{
    if (m_matrix == NULL)
        return QColor();

    return m_matrix->getColor(2);
}

void VCAnimation::setColor3(QColor color)
{
    if (m_matrix == NULL)
        return;

    if (m_matrix->getColor(2) != color)
    {
        m_matrix->setColor(2, color);
        emit color3Changed();
    }
}

QColor VCAnimation::getColor4() const
{
    if (m_matrix == NULL)
        return QColor();

    return m_matrix->getColor(3);
}

void VCAnimation::setColor4(QColor color)
{
    if (m_matrix == NULL)
        return;

    if (m_matrix->getColor(3) != color)
    {
        m_matrix->setColor(3, color);
        emit color4Changed();
    }
}

QColor VCAnimation::getColor5() const
{
    if (m_matrix == NULL)
        return QColor();

    return m_matrix->getColor(4);
}

void VCAnimation::setColor5(QColor color)
{
    if (m_matrix == NULL)
        return;

    if (m_matrix->getColor(4) != color)
    {
        m_matrix->setColor(4, color);
        emit color5Changed();
    }
}

QStringList VCAnimation::algorithms() const
{
    return RGBAlgorithm::algorithms(m_doc);
}

int VCAnimation::algorithmIndex() const
{
    if (m_matrix == NULL)
        return 0;

    QStringList algoList = algorithms();
    return algoList.indexOf(m_matrix->algorithm()->name());
}

void VCAnimation::setAlgorithmIndex(int index)
{
    if (m_matrix == NULL)
        return;

    QStringList algoList = algorithms();
    if (index < 0 || index >= algorithms().count())
        return;

    RGBAlgorithm *algo = RGBAlgorithm::algorithm(m_doc, algoList.at(index));
    if (algo != nullptr)
    {
        /** if we're setting the same algorithm, then there's nothing to do */
        if (m_matrix->algorithm() != nullptr && m_matrix->algorithm()->name() == algo->name())
            return;
        algo->setColors(m_matrix->getColors());
    }

    m_matrix->setAlgorithm(algo);
    if (instantChanges())
        m_matrix->updateColorDelta();

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
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown animation tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCAnimation::saveXML(QXmlStreamWriter *doc)
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

    /* Write the <end> tag */
    doc->writeEndElement();

    return true;
}
