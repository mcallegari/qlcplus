/*
  Q Light Controller Plus
  vcxypad.cpp

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
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QMessageBox>
#include <QGridLayout>
#include <QByteArray>
#include <QSettings>
#include <QPainter>
#include <QPixmap>
#include <QCursor>
#include <QSlider>
#include <qmath.h>
#include <QDebug>
#include <QPoint>
#include <QMenu>
#include <QList>

#include "qlcmacros.h"

#include "vcpropertieseditor.h"
#include "vcxypadproperties.h"
#include "ctkrangeslider.h"
#include "mastertimer.h"
#include "vcxypadarea.h"
#include "flowlayout.h"
#include "vcxypad.h"
#include "fixture.h"
#include "apputil.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

const quint8 VCXYPad::panInputSourceId = 0;
const quint8 VCXYPad::tiltInputSourceId = 1;
const quint8 VCXYPad::widthInputSourceId = 2;
const quint8 VCXYPad::heightInputSourceId = 3;
const quint8 VCXYPad::panFineInputSourceId = 4;
const quint8 VCXYPad::tiltFineInputSourceId = 5;

const qreal MAX_VALUE = 256.0;
const qreal MAX_DMX_VALUE = MAX_VALUE - 1.0/256;

static const QString presetBtnSS = "QPushButton { background-color: %1; height: 32px; border: 2px solid #6A6A6A; border-radius: 5px; }"
                                   "QPushButton:pressed { border: 2px solid #0000FF; }"
                                   "QPushButton:checked { border: 2px solid #0000FF; }"
                                   "QPushButton:unchecked { border: 2px solid #6A6A6A; }"
                                   "QPushButton:disabled { border: 2px solid #BBBBBB; color: #8f8f8f }";

/*****************************************************************************
 * VCXYPad Initialization
 *****************************************************************************/

VCXYPad::VCXYPad(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
{
    /* Set the class name "VCXYPad" as the object name as well */
    setObjectName(VCXYPad::staticMetaObject.className());

    m_mainVbox = new QVBoxLayout(this);

    m_padBox = new QHBoxLayout;
    m_mainVbox->addLayout(m_padBox);

    m_lvbox = new QVBoxLayout;
    m_lvbox->addSpacing(20);
    // left side vertical range slider
    m_vRangeSlider = new ctkRangeSlider(this);
    m_lvbox->addWidget(m_vRangeSlider);
    m_lvbox->addSpacing(25);

    m_padBox->addLayout(m_lvbox);

    m_cvbox = new QVBoxLayout;
    m_padBox->addLayout(m_cvbox);

    // top horizontal range slider
    m_hRangeSlider = new ctkRangeSlider(Qt::Horizontal, this);
    m_cvbox->addWidget(m_hRangeSlider);

    // central XYPad
    m_area = new VCXYPadArea(this);
    m_cvbox->addWidget(m_area);

    // bottom horizontal slider
    m_hSlider = new QSlider(Qt::Horizontal, this);
    m_cvbox->addWidget(m_hSlider);

    m_rvbox = new QVBoxLayout;
    m_padBox->addLayout(m_rvbox);
    m_rvbox->addSpacing(20);

    // left side vertical slider
    m_vSlider = new QSlider(this);
    m_rvbox->addWidget(m_vSlider);
    m_rvbox->addSpacing(25);

    // bottom preset space
    m_presetsLayout = new FlowLayout();
    m_mainVbox->addLayout(m_presetsLayout);
    m_efx = NULL;
    m_efxStartXOverrideId = Function::invalidAttributeId();
    m_efxStartYOverrideId = Function::invalidAttributeId();
    m_efxWidthOverrideId = Function::invalidAttributeId();
    m_efxHeightOverrideId = Function::invalidAttributeId();

    m_scene = NULL;

    m_vSlider->setRange(0, 256);
    m_hSlider->setRange(0, 256);
    m_vSlider->setInvertedAppearance(true);
    m_vSlider->setTickPosition(QSlider::TicksLeft);
    m_vSlider->setTickInterval(16);
    m_hSlider->setTickPosition(QSlider::TicksAbove);
    m_hSlider->setTickInterval(16);
    m_vSlider->setStyle(AppUtil::saneStyle());
    m_hSlider->setStyle(AppUtil::saneStyle());

    m_hRangeSlider->setRange(0, 256);
    m_vRangeSlider->setInvertedAppearance(true);
    m_vRangeSlider->setRange(0, 256);
    m_hRangeSlider->setMaximumPosition(256);
    m_vRangeSlider->setMaximumPosition(256);

    connect(m_area, SIGNAL(positionChanged(const QPointF&)),
            this, SLOT(slotPositionChanged(const QPointF&)));
    connect(this, SIGNAL(fixturePositions(const QVariantList)),
            m_area, SLOT(slotFixturePositions(const QVariantList)));
    connect(m_vSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderValueChanged()));
    connect(m_hSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderValueChanged()));
    connect(m_hRangeSlider, SIGNAL(positionsChanged(int,int)),
            this, SLOT(slotRangeValueChanged()));
    connect(m_vRangeSlider, SIGNAL(positionsChanged(int,int)),
            this, SLOT(slotRangeValueChanged()));

    setFrameStyle(KVCFrameStyleSunken);
    setType(VCWidget::XYPadWidget);
    setCaption("XY Pad");
    setMinimumSize(20, 20);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_XYPAD_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(QSize(230, 230));
    m_padInteraction = false;
    m_sliderInteraction = false;
    m_inputValueChanged = false;

    slotModeChanged(m_doc->mode());
    setLiveEdit(m_liveEdit);

    m_doc->masterTimer()->registerDMXSource(this);
    connect(m_doc->inputOutputMap(), SIGNAL(universeWritten(quint32,QByteArray)),
            this, SLOT(slotUniverseWritten(quint32,QByteArray)));
}

VCXYPad::~VCXYPad()
{
    m_doc->masterTimer()->unregisterDMXSource(this);
    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->requestDelete();
    }
    m_fadersMap.clear();
}

void VCXYPad::enableWidgetUI(bool enable)
{
    m_vSlider->setEnabled(enable);
    m_hSlider->setEnabled(enable);
    m_area->setMode(enable ? Doc::Operate : Doc::Design);

    QMutableListIterator <VCXYPadFixture> it(m_fixtures);
    while (it.hasNext() == true)
    {
        VCXYPadFixture fxi = it.next();
        if (enable)
            fxi.arm();
        else
            fxi.disarm();
        it.setValue(fxi);
    }

    foreach (QWidget *presetBtn, m_presets.keys())
        presetBtn->setEnabled(enable);

    /* Reset the changed flag in m_area so that the pad won't immediately set a value
       when mode is changed */
    m_area->position();
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCXYPad::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCXYPad* xypad = new VCXYPad(parent, m_doc);
    if (xypad->copyFrom(this) == false)
    {
        delete xypad;
        xypad = NULL;
    }

    for (QHash<QWidget*, VCXYPadPreset*>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        VCXYPadPreset *preset = it.value();
        xypad->addPreset(*preset);
    }

    return xypad;
}

bool VCXYPad::copyFrom(const VCWidget* widget)
{
    const VCXYPad* xypad = qobject_cast <const VCXYPad*> (widget);
    if (xypad == NULL)
        return false;
    resize(xypad->size());

    /* Get rid of existing channels */
    m_fixtures.clear();

    /* Copy the other widget's fixtures */
    m_fixtures = xypad->fixtures();

    /* Copy the current position */
    m_area->setPosition(xypad->m_area->position());
    m_vSlider->setValue(xypad->m_vSlider->value());
    m_hSlider->setValue(xypad->m_hSlider->value());

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCXYPad::setCaption(const QString& text)
{
    m_area->setWindowTitle(text);
    VCWidget::setCaption(text);
}

bool VCXYPad::invertedAppearance() const
{
    return !(m_vSlider->invertedAppearance());
}

void VCXYPad::setInvertedAppearance(bool invert)
{
    if (invert == true)
        m_vSlider->setInvertedAppearance(false);
    else
        m_vSlider->setInvertedAppearance(true);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCXYPad::editProperties()
{
    VCXYPadProperties prop(this, m_doc);
    if (prop.exec() == QDialog::Accepted)
        m_doc->setModified();
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void VCXYPad::appendFixture(const VCXYPadFixture& fxi)
{
    if (fxi.head().isValid() && m_fixtures.indexOf(fxi) == -1)
        m_fixtures.append(fxi);

    updateDegreesRange();
}

void VCXYPad::removeFixture(GroupHead const & head)
{
    VCXYPadFixture fixture(m_doc);
    fixture.setHead(head);

    m_fixtures.removeAll(fixture);

    updateDegreesRange();
}

void VCXYPad::clearFixtures()
{
    m_fixtures.clear();

    updateDegreesRange();
}

QList <VCXYPadFixture> VCXYPad::fixtures() const
{
    return m_fixtures;
}

QRectF VCXYPad::computeCommonDegreesRange() const
{
    QRectF commonRange;

    foreach (VCXYPadFixture fixture, m_fixtures)
    {
        QRectF range = fixture.degreesRange();
        if (!range.isValid())
            return QRectF();

        if (commonRange.isValid())
        {
            if (range != commonRange)
                return QRectF();
        }
        else
        {
            commonRange = range;
        }
    }

    return commonRange;
}

void VCXYPad::updateDegreesRange()
{
    QRectF range = computeCommonDegreesRange();

    m_area->setDegreesRange(range);
}

/*****************************************************************************
 * Current XY position
 *****************************************************************************/

void VCXYPad::writeDMX(MasterTimer* timer, QList<Universe *> universes)
{
    if (m_scene != NULL)
        writeScenePositions(timer, universes);
    else
        writeXYFixtures(timer, universes);
}

void VCXYPad::writeXYFixtures(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);

    if (m_area->hasPositionChanged() == false)
        return;

    // This call also resets the m_changed flag in m_area
    QPointF pt = m_area->position();

    /* Scale XY coordinate values to 0.0 - 1.0 */
    qreal x = SCALE(pt.x(), qreal(0), qreal(256), qreal(0), qreal(1));
    qreal y = SCALE(pt.y(), qreal(0), qreal(256), qreal(0), qreal(1));

    if (invertedAppearance())
        y = qreal(1) - y;

    /* Write values outside of mutex lock to keep UI snappy */
    foreach (VCXYPadFixture fixture, m_fixtures)
    {
        if (fixture.isEnabled())
        {
            quint32 universe = fixture.universe();
            if (universe == Universe::invalid())
                continue;

            QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
            if (fader.isNull())
            {
                fader = universes[universe]->requestFader();
                fader->adjustIntensity(intensity());
                m_fadersMap[universe] = fader;
            }
            fixture.writeDMX(x, y, fader, universes[universe]);
        }
    }
}

void VCXYPad::updateSceneChannel(FadeChannel *fc, uchar value)
{
    fc->addFlag(FadeChannel::Relative);
    fc->setStart(value);
    fc->setCurrent(value);
    fc->setTarget(value);
    fc->setElapsed(0);
    fc->setReady(false);
}

void VCXYPad::writeScenePositions(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);

    if (m_scene == NULL || m_scene->isRunning() == false)
        return;

    QPointF pt = m_area->position();
    uchar panCoarse = uchar(qFloor(pt.x()));
    uchar panFine = uchar((pt.x() - qFloor(pt.x())) * 256);
    uchar tiltCoarse = uchar(qFloor(pt.y()));
    uchar tiltFine = uchar((pt.y() - qFloor(pt.y())) * 256);

    foreach (SceneChannel sc, m_sceneChannels)
    {
        if (sc.m_universe >= (quint32)universes.count())
            continue;

        QSharedPointer<GenericFader> fader = m_fadersMap.value(sc.m_universe, QSharedPointer<GenericFader>());
        if (fader.isNull())
        {
            fader = universes[sc.m_universe]->requestFader();
            fader->adjustIntensity(intensity());
            m_fadersMap[sc.m_universe] = fader;
        }

        if (sc.m_group == QLCChannel::Pan)
        {
            if (sc.m_subType == QLCChannel::MSB)
            {
                FadeChannel *fc = fader->getChannelFader(m_doc, universes[sc.m_universe], sc.m_fixture, sc.m_channel);
                updateSceneChannel(fc, panCoarse);
            }
            else
            {
                FadeChannel *fc = fader->getChannelFader(m_doc, universes[sc.m_universe], sc.m_fixture, sc.m_channel);
                updateSceneChannel(fc, panFine);
            }
        }
        else
        {
            if (sc.m_subType == QLCChannel::MSB)
            {
                FadeChannel *fc = fader->getChannelFader(m_doc, universes[sc.m_universe], sc.m_fixture, sc.m_channel);
                updateSceneChannel(fc, tiltCoarse);
            }
            else
            {
                FadeChannel *fc = fader->getChannelFader(m_doc, universes[sc.m_universe], sc.m_fixture, sc.m_channel);
                updateSceneChannel(fc, tiltFine);
            }
        }
    }
}

void VCXYPad::slotPositionChanged(const QPointF& pt)
{
    if (m_sliderInteraction == true)
        return;

    m_padInteraction = true;
    m_hSlider->setValue(pt.x());
    if (invertedAppearance() == false)
    {
        m_vSlider->setValue(pt.y());
    }
    else
    {
        m_vSlider->setValue(MAX_DMX_VALUE - pt.y());
    }

    if (m_inputValueChanged == false)
        updateFeedback();
    m_padInteraction = false;
    m_inputValueChanged = false;
}

void VCXYPad::slotSliderValueChanged()
{
    if (m_padInteraction == true)
        return;

    QPointF pt = m_area->position(false);

    m_sliderInteraction = true;
    if (QObject::sender() == m_hSlider)
    {
        pt.setX(m_hSlider->value());

        int Xfb = (int)SCALE(float(m_hSlider->value()), float(m_hSlider->minimum()),
                             float(m_hSlider->maximum()), float(0), float(UCHAR_MAX));
        sendFeedback(Xfb, panInputSourceId);
    }
    else
    {
        if (invertedAppearance() == false)
            pt.setY(m_vSlider->value());
        else
            pt.setY(MAX_DMX_VALUE - m_vSlider->value());

        int Yfb = (int)SCALE(float(m_vSlider->value()), float(m_vSlider->minimum()),
                             float(m_vSlider->maximum()), float(0), float(UCHAR_MAX));
        sendFeedback(Yfb, tiltInputSourceId);
    }

    m_area->setPosition(pt);
    m_area->update();
    m_sliderInteraction = false;
}

void VCXYPad::slotRangeValueChanged()
{
    QRectF rect(QPointF(m_hRangeSlider->minimumPosition(), m_vRangeSlider->minimumPosition()),
               QPointF(m_hRangeSlider->maximumPosition(), m_vRangeSlider->maximumPosition()));
    m_area->setRangeWindow(rect);
    if (m_efx != NULL && m_efx->isRunning())
    {
        m_efx->adjustAttribute(rect.x() + rect.width() / 2, m_efxStartXOverrideId);
        m_efx->adjustAttribute(rect.y() + rect.height() / 2, m_efxStartYOverrideId);
        m_efx->adjustAttribute(rect.width() / 2, m_efxWidthOverrideId);
        m_efx->adjustAttribute(rect.height() / 2, m_efxHeightOverrideId);

        // recalculate preview polygons
        QPolygonF polygon;
        m_efx->preview(polygon);

        QVector <QPolygonF> fixturePoints;
        m_efx->previewFixtures(fixturePoints);

        m_area->setEFXPolygons(polygon, fixturePoints);
        m_area->setEFXInterval(m_efx->duration());
    }
    m_area->update();
    if (QObject::sender() == m_hRangeSlider)
        sendFeedback(m_hRangeSlider->maximumValue(), heightInputSourceId);
    else if (QObject::sender() == m_vRangeSlider)
        sendFeedback(m_vRangeSlider->maximumValue(), widthInputSourceId);
}

void VCXYPad::slotUniverseWritten(quint32 idx, const QByteArray &universeData)
{
    QVariantList positions;

    if (m_scene)
    {
        QMap <quint32, QPointF> fxMap;

        foreach (SceneChannel sc, m_sceneChannels)
        {
            if (sc.m_universe != idx)
                continue;

            qreal x = fxMap[sc.m_fixture].x();
            qreal y = fxMap[sc.m_fixture].y();

            if (sc.m_group == QLCChannel::Pan)
            {
                if (sc.m_subType == QLCChannel::MSB)
                    x += (uchar)universeData.at(sc.m_channel);
                else
                    x += ((uchar)universeData.at(sc.m_channel) / 255);
            }
            else
            {
                if (sc.m_subType == QLCChannel::MSB)
                    y += (uchar)universeData.at(sc.m_channel);
                else
                    y += ((uchar)universeData.at(sc.m_channel) / 255);
            }
            fxMap[sc.m_fixture] = QPointF(x, y);
        }

        foreach (QPointF pt, fxMap.values())
        {
            if (invertedAppearance())
                pt.setY(256 - pt.y());
            positions.append(pt);
        }
    }
    else
    {
        foreach (VCXYPadFixture fixture, m_fixtures)
        {
            if (fixture.isEnabled() == false)
                continue;

            if (fixture.universe() != idx)
                continue;

            qreal x(-1), y(-1);
            fixture.readDMX(universeData, x, y);
            if (x != -1.0 && y != -1.0)
            {
                if (invertedAppearance())
                    y = qreal(1) - y;

               x *= 256;
               y *= 256;
               positions.append(QPointF(x, y));
            }
        }
    }

    emit fixturePositions(positions);
}

/*********************************************************************
 * Presets
 *********************************************************************/

void VCXYPad::addPreset(const VCXYPadPreset &preset)
{
    QString label = preset.m_name;

    if (label.isEmpty())
    {
        qDebug() << "VCXYPad Preset label empty. Not adding it";
        return;
    }

    QPushButton *presetButton = new QPushButton(this);
    QWidget *presetWidget = presetButton;
    presetButton->setStyleSheet(presetBtnSS.arg(preset.getColor()));
    presetButton->setMinimumWidth(36);
    presetButton->setMaximumWidth(80);
    presetButton->setFocusPolicy(Qt::TabFocus);
    presetButton->setText(fontMetrics().elidedText(label, Qt::ElideRight, 72));
    if (preset.m_type == VCXYPadPreset::EFX ||
        preset.m_type == VCXYPadPreset::Scene ||
        preset.m_type == VCXYPadPreset::FixtureGroup)
            presetButton->setCheckable(true);

    connect(presetButton, SIGNAL(clicked(bool)),
            this, SLOT(slotPresetClicked(bool)));

    if (mode() == Doc::Design)
        presetWidget->setEnabled(false);

    m_presets[presetWidget] = new VCXYPadPreset(preset);
    m_presetsLayout->addWidget(presetWidget);

    if (m_presets[presetWidget]->m_inputSource != NULL)
    {
        setInputSource(m_presets[presetWidget]->m_inputSource, m_presets[presetWidget]->m_id);
    }
}

void VCXYPad::resetPresets()
{
    for (QHash<QWidget *, VCXYPadPreset *>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        QWidget* widget = it.key();
        m_presetsLayout->removeWidget(widget);
        delete widget;

        VCXYPadPreset* preset = it.value();
        if (!preset->m_inputSource.isNull())
            setInputSource(QSharedPointer<QLCInputSource>(), preset->m_id);
        delete preset;
    }
    m_presets.clear();
}

QList<VCXYPadPreset *> VCXYPad::presets() const
{
    QList<VCXYPadPreset*> presets = m_presets.values();
    std::sort(presets.begin(), presets.end(), VCXYPadPreset::compare);
    return presets;
}

QMap<quint32,QString> VCXYPad::presetsMap() const
{
    QMap<quint32,QString> map;

    foreach (VCXYPadPreset *control, m_presets.values())
        map.insert(control->m_id, VCXYPadPreset::typeToString(control->m_type));

    return map;
}

void VCXYPad::slotPresetClicked(bool checked)
{
    if (mode() == Doc::Design)
        return;

    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    VCXYPadPreset *preset = m_presets[btn];

    Q_ASSERT(preset != NULL);

    // stop any previously started EFX
    if (m_efx != NULL && m_efx->isRunning())
    {
        disconnect(m_efx, SIGNAL(durationChanged(uint)), this, SLOT(slotEFXDurationChanged(uint)));

        m_efx->stopAndWait();
        m_efx = NULL;
        m_efxStartXOverrideId = Function::invalidAttributeId();
        m_efxStartYOverrideId = Function::invalidAttributeId();
        m_efxWidthOverrideId = Function::invalidAttributeId();
        m_efxHeightOverrideId = Function::invalidAttributeId();
    }

    // stop any previously started Scene
    if (m_scene != NULL)
    {
        m_scene->stop(functionParent());
        m_scene = NULL;
        foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
        {
            if (!fader.isNull())
                fader->requestDelete();
        }
        m_fadersMap.clear();
    }

    // deactivate all previously activated buttons first
    for (QHash<QWidget *, VCXYPadPreset *>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        QPushButton* cBtn = reinterpret_cast<QPushButton*>(it.key());
        VCXYPadPreset *cPr = it.value();
        if (preset->m_id == cPr->m_id)
            continue;

        cBtn->blockSignals(true);
        if (preset->m_type == VCXYPadPreset::FixtureGroup)
        {
            if (cPr->m_type == VCXYPadPreset::FixtureGroup &&
                cBtn->isChecked() == true)
            {
                cBtn->setChecked(false);
                if (cPr->m_inputSource.isNull() == false)
                    sendFeedback(cPr->m_inputSource->feedbackValue(QLCInputFeedback::LowerValue), cPr->m_inputSource);
            }
        }
        else if (cPr->m_type == VCXYPadPreset::EFX ||
            cPr->m_type == VCXYPadPreset::Scene)
        {
            if (cBtn->isChecked() == true)
            {
                cBtn->setChecked(false);
                if (cPr->m_inputSource.isNull() == false)
                    sendFeedback(cPr->m_inputSource->feedbackValue(QLCInputFeedback::LowerValue), cPr->m_inputSource);
            }
        }
        else
        {
            if (cBtn->isDown() == true)
            {
                cBtn->setDown(false);
                if (cPr->m_inputSource.isNull() == false)
                    sendFeedback(cPr->m_inputSource->feedbackValue(QLCInputFeedback::LowerValue), cPr->m_inputSource);
            }
        }
        cBtn->blockSignals(false);
        if (cPr->m_inputSource.isNull() == false)
            sendFeedback(cPr->m_inputSource->feedbackValue(QLCInputFeedback::LowerValue), cPr->m_inputSource);
    }

    if (preset->m_type == VCXYPadPreset::EFX)
    {
        if (checked == false)
        {
            m_area->enableEFXPreview(false);
            return;
        }

        Function *f = m_doc->function(preset->m_funcID);
        if (f == NULL || f->type() != Function::EFXType)
            return;
        m_efx = qobject_cast<EFX*>(f);

        QRectF rect(QPointF(m_hRangeSlider->minimumPosition(), m_vRangeSlider->minimumPosition()),
                   QPointF(m_hRangeSlider->maximumPosition(), m_vRangeSlider->maximumPosition()));
        m_area->setRangeWindow(rect);
        if (rect.isValid())
        {
            m_efxStartXOverrideId = m_efx->requestAttributeOverride(EFX::XOffset, rect.x() + rect.width() / 2);
            m_efxStartYOverrideId = m_efx->requestAttributeOverride(EFX::YOffset, rect.y() + rect.height() / 2);
            m_efxWidthOverrideId = m_efx->requestAttributeOverride(EFX::Width, rect.width() / 2);
            m_efxHeightOverrideId = m_efx->requestAttributeOverride(EFX::Height, rect.height() / 2);
        }

        QPolygonF polygon;
        m_efx->preview(polygon);

        QVector <QPolygonF> fixturePoints;
        m_efx->previewFixtures(fixturePoints);

        m_area->enableEFXPreview(true);
        m_area->setEFXPolygons(polygon, fixturePoints);
        m_area->setEFXInterval(m_efx->duration());
        m_efx->start(m_doc->masterTimer(), functionParent());

        connect(m_efx, SIGNAL(durationChanged(uint)), this, SLOT(slotEFXDurationChanged(uint)));

        if (preset->m_inputSource.isNull() == false)
            sendFeedback(preset->m_inputSource->feedbackValue(QLCInputFeedback::UpperValue), preset->m_inputSource);
    }
    else if (preset->m_type == VCXYPadPreset::Scene)
    {
        if (checked == false)
            return;

        Function *f = m_doc->function(preset->m_funcID);
        if (f == NULL || f->type() != Function::SceneType)
            return;

        m_scene = qobject_cast<Scene*>(f);
        m_sceneChannels.clear();

        foreach (SceneValue scv, m_scene->values())
        {
            Fixture *fixture = m_doc->fixture(scv.fxi);
            if (fixture == NULL)
                continue;
            const QLCChannel *ch = fixture->channel(scv.channel);
            if (ch == NULL)
                continue;
            if (ch->group() != QLCChannel::Pan && ch->group() != QLCChannel::Tilt)
                continue;

            SceneChannel sChan;
            sChan.m_universe = fixture->universe();
            sChan.m_fixture = fixture->id();
            sChan.m_channel = scv.channel;
            sChan.m_group = ch->group();
            sChan.m_subType = ch->controlByte();
            m_sceneChannels.append(sChan);
        }

        m_area->enableEFXPreview(false);
        // reset the area window as we're switching to relative
        m_area->setRangeWindow(QRectF());
        m_area->setPosition(QPointF(128, 128));
        m_area->repaint();
        m_scene->start(m_doc->masterTimer(), functionParent());

        if (preset->m_inputSource.isNull() == false)
            sendFeedback(preset->m_inputSource->feedbackValue(QLCInputFeedback::UpperValue), preset->m_inputSource);
    }
    else if (preset->m_type == VCXYPadPreset::Position)
    {
        m_area->enableEFXPreview(false);
        QRectF rect(QPointF(m_hRangeSlider->minimumPosition(), m_vRangeSlider->minimumPosition()),
                   QPointF(m_hRangeSlider->maximumPosition(), m_vRangeSlider->maximumPosition()));
        m_area->setRangeWindow(rect);
        m_area->setPosition(preset->m_dmxPos);
        m_area->repaint();
        if (preset->m_inputSource.isNull() == false)
            sendFeedback(preset->m_inputSource->feedbackValue(QLCInputFeedback::UpperValue), preset->m_inputSource);
        btn->blockSignals(true);
        btn->setDown(true);
        btn->blockSignals(false);
    }
    else if (preset->m_type == VCXYPadPreset::FixtureGroup)
    {
        QList<GroupHead> heads = preset->fixtureGroup();

        for (int i = 0; i < m_fixtures.count(); i++)
        {
            if (checked == false)
            {
                m_fixtures[i].setEnabled(true);
            }
            else
            {
                if (heads.contains(m_fixtures[i].head()))
                {
                    qDebug() << "Enabling head" << m_fixtures[i].head().fxi << m_fixtures[i].head().head;
                    m_fixtures[i].setEnabled(true);
                }
                else
                {
                    qDebug() << "Disabling head" << m_fixtures[i].head().fxi << m_fixtures[i].head().head;
                    m_fixtures[i].setEnabled(false);
                }
            }
        }
    }
}

void VCXYPad::slotEFXDurationChanged(uint duration)
{
    if (m_efx == NULL)
        return;

    m_area->setEFXInterval(duration);
}

FunctionParent VCXYPad::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCXYPad::updateFeedback()
{
    int Xfb = (int)SCALE(float(m_hSlider->value()), float(m_hSlider->minimum()),
                         float(m_hSlider->maximum()), float(0), float(UCHAR_MAX));
    sendFeedback(Xfb, panInputSourceId);

    int Yfb = (int)SCALE(float(m_vSlider->value()), float(m_vSlider->minimum()),
                         float(m_vSlider->maximum()), float(0), float(UCHAR_MAX));
    sendFeedback(Yfb, tiltInputSourceId);

/*
    for (QHash<QWidget*, VCXYPadPreset*>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        VCXYPadPreset* preset = it.value();
        if (preset->m_inputSource != NULL)
        {
            {
                QPushButton* button = reinterpret_cast<QPushButton*>(it.key());
                if (preset->m_inputSource.isNull() == false)
                    sendFeedback(button->isDown() ?
                                 preset->m_inputSource->upperValue() :
                                 preset->m_inputSource->lowerValue(),
                                 preset->m_inputSource);
            }
        }
    }
*/
}

void VCXYPad::updatePosition()
{
    QPointF pt = m_area->position(false);
    qreal xOffset = 0;
    qreal yOffset = 0;
    qreal areaWidth = MAX_VALUE;
    qreal areaHeight = MAX_VALUE;

    QRectF rangeWindow = m_area->rangeWindow();
    if (rangeWindow.isValid())
    {
        xOffset = rangeWindow.x();
        yOffset = rangeWindow.y();
        areaWidth = rangeWindow.width();
        areaHeight = rangeWindow.height();
    }

    pt.setX(xOffset + SCALE((qreal(m_lastPos.x()) * 256.0) + qreal(m_lastPos.width()), qreal(0), qreal(65535),
                            qreal(0), areaWidth));

    if (invertedAppearance() == false)
        pt.setY(yOffset + SCALE((qreal(m_lastPos.y()) * 256.0) + qreal(m_lastPos.height()), qreal(0), qreal(65535),
                                qreal(0), areaHeight));
    else
        pt.setY(yOffset + SCALE((qreal(m_lastPos.y()) * 256.0) + qreal(m_lastPos.height()), qreal(65535), qreal(0),
                                qreal(0), areaHeight));

    m_inputValueChanged = true;

    m_area->setPosition(pt);
    m_area->update();
}

void VCXYPad::slotInputValueChanged(quint32 universe, quint32 channel,
                                     uchar value)
{
    /* Don't let input data through in design mode or if disabled */
    if (acceptsInput() == false)
        return;

    quint32 pagedCh = (page() << 16) | channel;

    if (checkInputSource(universe, pagedCh, value, sender(), panInputSourceId))
    {
        if (m_efx == NULL)
        {
            m_lastPos.setX(value);
            updatePosition();
        }
        else
        {
            if (m_efx->isRunning() == false)
                return;

            m_hRangeSlider->setMinimumValue(value);
            slotRangeValueChanged();
            return;
        }
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), panFineInputSourceId))
    {
        if (m_efx == NULL)
        {
            m_lastPos.setWidth(value);
            updatePosition();
        }
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), tiltInputSourceId))
    {
        if (m_efx == NULL)
        {
            m_lastPos.setY(value);
            updatePosition();
        }
        else
        {
            if (m_efx->isRunning() == false)
                return;

            m_vRangeSlider->setMinimumValue(value);
            slotRangeValueChanged();
        }
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), tiltFineInputSourceId))
    {
        if (m_efx == NULL)
        {
            m_lastPos.setHeight(value);
            updatePosition();
        }
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), widthInputSourceId))
    {
        if (m_efx != NULL && m_efx->isRunning())
        {
            m_hRangeSlider->setMaximumValue(value);
            slotRangeValueChanged();
        }
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), heightInputSourceId))
    {
        if (m_efx != NULL && m_efx->isRunning())
        {
            m_vRangeSlider->setMaximumValue(value);
            slotRangeValueChanged();
        }
    }
    else
    {
        for (QHash<QWidget*, VCXYPadPreset*>::iterator it = m_presets.begin();
                it != m_presets.end(); ++it)
        {
            VCXYPadPreset *preset = it.value();
            if (preset->m_inputSource != NULL &&
                    preset->m_inputSource->universe() == universe &&
                    preset->m_inputSource->channel() == pagedCh)
            {
                {
                    QPushButton *button = reinterpret_cast<QPushButton*>(it.key());
                    button->click();
                    return;
                }
            }
        }
    }
}

void VCXYPad::slotKeyPressed(const QKeySequence &keySequence)
{
    if (acceptsInput() == false)
        return;

    for (QHash<QWidget*, VCXYPadPreset*>::iterator it = m_presets.begin();
            it != m_presets.end(); ++it)
    {
        VCXYPadPreset *preset = it.value();
        if (preset->m_keySequence == keySequence)
        {
            QPushButton *button = reinterpret_cast<QPushButton*>(it.key());
            button->click();
        }
    }
}

/*****************************************************************************
 * QLC mode
 *****************************************************************************/

void VCXYPad::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate && isDisabled() == false)
    {
        enableWidgetUI(true);
    }
    else
    {
        enableWidgetUI(false);
    }

    VCWidget::slotModeChanged(mode);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCXYPad::loadXML(QXmlStreamReader &root)
{
    bool visible = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    int xpos = 0;
    int ypos = 0;

    if (root.name() != KXMLQLCVCXYPad)
    {
        qWarning() << Q_FUNC_INFO << "XY Pad node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    QXmlStreamAttributes attrs = root.attributes();

    if (attrs.hasAttribute(KXMLQLCVCXYPadInvertedAppearance))
    {
        if (attrs.value(KXMLQLCVCXYPadInvertedAppearance).toString() == "0")
            setInvertedAppearance(false);
        else
            setInvertedAppearance(true);
    }

    // Sorted list for new presets
    QList<VCXYPadPreset> newPresets;

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCXYPadPan)
        {
            xpos = root.attributes().value(KXMLQLCVCXYPadPosition).toString().toInt();
            loadXMLSources(root, panInputSourceId);
        }
        else if (root.name() == KXMLQLCVCXYPadTilt)
        {
            ypos = root.attributes().value(KXMLQLCVCXYPadPosition).toString().toInt();
            loadXMLSources(root, tiltInputSourceId);
        }
        else if (root.name() == KXMLQLCVCXYPadWidth)
        {
            loadXMLSources(root, widthInputSourceId);
        }
        else if (root.name() == KXMLQLCVCXYPadHeight)
        {
            loadXMLSources(root, heightInputSourceId);
        }
        else if (root.name() == KXMLQLCVCXYPadRangeWindow)
        {
            QXmlStreamAttributes wAttrs = root.attributes();
            if (wAttrs.hasAttribute(KXMLQLCVCXYPadRangeHorizMin))
                m_hRangeSlider->setMinimumPosition(wAttrs.value(KXMLQLCVCXYPadRangeHorizMin).toString().toInt());
            if (wAttrs.hasAttribute(KXMLQLCVCXYPadRangeHorizMax))
                m_hRangeSlider->setMaximumPosition(wAttrs.value(KXMLQLCVCXYPadRangeHorizMax).toString().toInt());
            if (wAttrs.hasAttribute(KXMLQLCVCXYPadRangeVertMin))
                m_vRangeSlider->setMinimumPosition(wAttrs.value(KXMLQLCVCXYPadRangeVertMin).toString().toInt());
            if (wAttrs.hasAttribute(KXMLQLCVCXYPadRangeVertMax))
                m_vRangeSlider->setMaximumPosition(wAttrs.value(KXMLQLCVCXYPadRangeVertMax).toString().toInt());
            slotRangeValueChanged();
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCXYPadPosition) // Legacy
        {
            QXmlStreamAttributes pAttrs = root.attributes();
            xpos = pAttrs.value(KXMLQLCVCXYPadPositionX).toString().toInt();
            ypos = pAttrs.value(KXMLQLCVCXYPadPositionY).toString().toInt();
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCXYPadFixture)
        {
            VCXYPadFixture fxi(m_doc);
            if (fxi.loadXML(root) == true)
                appendFixture(fxi);
        }
        else if (root.name() == KXMLQLCVCXYPadPreset)
        {
            VCXYPadPreset preset(0xff);
            if (preset.loadXML(root))
                newPresets.insert(std::lower_bound(newPresets.begin(), newPresets.end(), preset), preset);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown XY Pad tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    foreach (VCXYPadPreset const& preset, newPresets)
        addPreset(preset);

    setGeometry(x, y, w, h);
    show(); // Qt doesn't update the widget's geometry without this.
    m_area->setPosition(QPointF(xpos, ypos));

    return true;
}

bool VCXYPad::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* VC XY Pad entry */
    doc->writeStartElement(KXMLQLCVCXYPad);

    saveXMLCommon(doc);

    doc->writeAttribute(KXMLQLCVCXYPadInvertedAppearance, QString::number(invertedAppearance()));

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Fixtures */
    foreach (VCXYPadFixture fixture, m_fixtures)
        fixture.saveXML(doc);

    /* Current XY position */
    QPointF pt(m_area->position(false));

    /* Custom range window */
    if (m_hRangeSlider->minimumPosition() != 0 ||
        m_hRangeSlider->maximumPosition() != 256 ||
        m_vRangeSlider->minimumPosition() != 0 ||
        m_vRangeSlider->maximumPosition() != 256)
    {
        doc->writeStartElement(KXMLQLCVCXYPadRangeWindow);
        doc->writeAttribute(KXMLQLCVCXYPadRangeHorizMin, QString::number(m_hRangeSlider->minimumPosition()));
        doc->writeAttribute(KXMLQLCVCXYPadRangeHorizMax, QString::number(m_hRangeSlider->maximumPosition()));
        doc->writeAttribute(KXMLQLCVCXYPadRangeVertMin, QString::number(m_vRangeSlider->minimumPosition()));
        doc->writeAttribute(KXMLQLCVCXYPadRangeVertMax, QString::number(m_vRangeSlider->maximumPosition()));
        doc->writeEndElement();
    }

    /* Pan */
    doc->writeStartElement(KXMLQLCVCXYPadPan);
    doc->writeAttribute(KXMLQLCVCXYPadPosition, QString::number(int(pt.x())));
    saveXMLInput(doc, inputSource(panInputSourceId));
    doc->writeEndElement();

    /* Tilt */
    doc->writeStartElement(KXMLQLCVCXYPadTilt);
    doc->writeAttribute(KXMLQLCVCXYPadPosition, QString::number(int(pt.y())));
    saveXMLInput(doc, inputSource(tiltInputSourceId));
    doc->writeEndElement();

    /* Width */
    QSharedPointer<QLCInputSource> wSrc = inputSource(widthInputSourceId);
    if (!wSrc.isNull() && wSrc->isValid())
    {
        doc->writeStartElement(KXMLQLCVCXYPadWidth);
        saveXMLInput(doc, wSrc);
        doc->writeEndElement();
    }

    /* Height */
    QSharedPointer<QLCInputSource> hSrc = inputSource(heightInputSourceId);
    if (!hSrc.isNull() && hSrc->isValid())
    {
        doc->writeStartElement(KXMLQLCVCXYPadHeight);
        saveXMLInput(doc, hSrc);
        doc->writeEndElement();
    }

    // Presets
    foreach (VCXYPadPreset *preset, presets())
        preset->saveXML(doc);

    /* End the >XYPad> tag */
    doc->writeEndElement();

    return true;
}
