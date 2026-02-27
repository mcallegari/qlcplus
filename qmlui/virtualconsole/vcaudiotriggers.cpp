/*
  Q Light Controller Plus
  vcaudiotriggers.cpp

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
#include <qmath.h>

#include "vcaudiotriggers.h"
#include "fixturemanager.h"
#include "virtualconsole.h"
#include "treemodelitem.h"
#include "fixtureutils.h"
#include "audiocapture.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "vcspeeddial.h"
#include "qlcmacros.h"
#include "vccuelist.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "app.h"
#include "doc.h"

#define INPUT_ENABLE_CAPTURE    0
#define INPUT_VOLUME_CONTROL    1

#define KXMLQLCAudioBarsNumber      QStringLiteral("BarsNumber")
#define KXMLQLCAudioTriggerBar      QStringLiteral("Bar")
#define KXMLQLCVolumeBar            QStringLiteral("VolumeBar")   // LEGACY
#define KXMLQLCSpectrumBar          QStringLiteral("SpectrumBar") // LEGACY

#define KXMLQLCAudioBarIndex        QStringLiteral("Index")
#define KXMLQLCAudioBarName         QStringLiteral("Name")
#define KXMLQLCAudioBarType         QStringLiteral("Type")
#define KXMLQLCAudioBarDMXChannels  QStringLiteral("DMXChannels")
#define KXMLQLCAudioBarFunction     QStringLiteral("FunctionID")
#define KXMLQLCAudioBarWidget       QStringLiteral("WidgetID")
#define KXMLQLCAudioBarMinThreshold QStringLiteral("MinThreshold")
#define KXMLQLCAudioBarMaxThreshold QStringLiteral("MaxThreshold")
#define KXMLQLCAudioBarDivisor      QStringLiteral("Divisor")

VCAudioTriggers::VCAudioTriggers(Doc *doc, VirtualConsole *vc, QObject *parent)
    : VCWidget(doc, parent)
    , m_vc(vc)
    , m_captureEnabled(false)
    , m_volumeLevel(100)
    , m_selectedBar(-1)
    , m_fixtureTree(nullptr)
    , m_searchFilter(QString())
    , m_applyToSameType(false)
    , m_isUpdating(false)
{
    setType(VCWidget::AudioTriggersWidget);

    registerExternalControl(INPUT_ENABLE_CAPTURE, tr("Enable Capture"), true);
    registerExternalControl(INPUT_VOLUME_CONTROL, tr("Volume Control"), false);

    QSharedPointer<AudioCapture> capture(m_doc->audioInputCapture());
    m_inputCapture = capture.data();

    // reserve for volume + spectrum bars
    m_spectrumBars.resize(m_inputCapture->defaultBarsNumber() + 1);
    m_audioLevels.resize(m_spectrumBars.count());
    setBarsNumber(m_spectrumBars.count());
}

VCAudioTriggers::~VCAudioTriggers()
{
    if (m_item)
        delete m_item;
}

QString VCAudioTriggers::defaultCaption() const
{
    return tr("Audio Trigger %1").arg(id() + 1);
}

void VCAudioTriggers::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    QFont wFont = font();
    wFont.setBold(true);
    wFont.setPointSize(pixelDensity * 5.0);
    setFont(wFont);
}

void VCAudioTriggers::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCAudioTriggersItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        delete component;
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("audioTriggerObj", QVariant::fromValue(this));
}

QString VCAudioTriggers::propertiesResource() const
{
    return QString("qrc:/VCAudioTriggersProperties.qml");
}

VCWidget *VCAudioTriggers::createCopy(VCWidget *parent) const
{
    Q_ASSERT(parent != nullptr);

    VCAudioTriggers *audioTrigger = new VCAudioTriggers(m_doc, m_vc, parent);
    if (audioTrigger->copyFrom(this) == false)
    {
        delete audioTrigger;
        audioTrigger = nullptr;
    }

    return audioTrigger;
}

bool VCAudioTriggers::captureEnabled() const
{
    return m_captureEnabled;
}

void VCAudioTriggers::setCaptureEnabled(bool enable)
{
    if (enable == m_captureEnabled)
        return;

    m_captureEnabled = enable;

    // in case the audio input device has been changed in the meantime...
    QSharedPointer<AudioCapture> capture(m_doc->audioInputCapture());
    bool captureIsNew = m_inputCapture != capture.data();
    m_inputCapture = capture.data();

    if (enable == true)
    {
        connect(m_inputCapture, SIGNAL(dataProcessed(double*,int,double,quint32)),
                this, SLOT(slotSpectrumDataChanged(double*,int,double,quint32)));
        connect(m_inputCapture, SIGNAL(volumeChanged(int)),
                this, SIGNAL(volumeLevelChanged()));
        m_inputCapture->registerBandsNumber(m_spectrumBars.count() - 1);

        // Invalid ID: Stop every other widget
        emit functionStarting(this, Function::invalidId());

        for (AudioBar &bar : m_spectrumBars)
        {
            if (bar.m_type == DMXBar)
            {
                m_doc->masterTimer()->registerDMXSource(this);
                break;
            }
        }
    }
    else
    {
        if (!captureIsNew)
        {
            m_inputCapture->unregisterBandsNumber(m_spectrumBars.count() - 1);
            disconnect(m_inputCapture, SIGNAL(dataProcessed(double*,int,double,quint32)),
                       this, SLOT(slotSpectrumDataChanged(double*,int,double,quint32)));
            disconnect(m_inputCapture, SIGNAL(volumeChanged(int)),
                       this, SIGNAL(volumeLevelChanged()));
        }

        m_doc->masterTimer()->unregisterDMXSource(this);

        // request to delete all the active faders
        foreach (QSharedPointer<GenericFader> fader, m_fadersMap)
        {
            if (!fader.isNull())
                fader->requestDelete();
        }
        m_fadersMap.clear();
    }
    emit captureEnabledChanged();
}

uchar VCAudioTriggers::volumeLevel() const
{
    return m_volumeLevel;
}

void VCAudioTriggers::setVolumeLevel(uchar level)
{
    if (level == m_volumeLevel)
        return;

    m_volumeLevel = level;

    m_doc->audioInputCapture()->setVolume(intensity() * qreal(level) / 100.0);

    emit volumeLevelChanged();
}

int VCAudioTriggers::barsNumber() const
{
    // number of bars includes volume bar
    return m_spectrumBars.count();
}

void VCAudioTriggers::setBarsNumber(int num)
{
    if (num == m_spectrumBars.count())
        return;

    if (num > m_spectrumBars.count())
    {
        int barsToAdd = num - m_spectrumBars.count();
        for (int i = 0 ; i < barsToAdd; i++)
        {
            AudioBar bar;
            m_spectrumBars.append(bar);
        }
    }
    else if (num < m_spectrumBars.count())
    {
        int barsToRemove = m_spectrumBars.count() - num;
        for (int i = 0 ; i < barsToRemove; i++)
            m_spectrumBars.takeLast();
    }

    m_audioLevels.clear();
    m_audioLevels.resize(m_spectrumBars.count());

    emit barsNumberChanged();
    emit barsInfoChanged();
}

QVariantList VCAudioTriggers::audioLevels() const
{
    return m_audioLevels;
}

bool VCAudioTriggers::copyFrom(const VCWidget *widget)
{
    const VCAudioTriggers *audioTrigger = qobject_cast<const VCAudioTriggers*> (widget);
    if (audioTrigger == nullptr)
        return false;

    /* Copy and set properties */

    /* Copy object lists */

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

FunctionParent VCAudioTriggers::functionParent() const
{
    return FunctionParent(FunctionParent::AutoVCWidget, id());
}

void VCAudioTriggers::selectBarForEditing(int index)
{
    m_selectedBar = index;
    updateFixtureTree();
}

QVariantList VCAudioTriggers::barsInfo() const
{
    QVariantList bList;
    const int spectrumBars = barsNumber() - 1; // exclude volume bar
    const double minFreq = AudioCapture::minFrequency();
    const double maxFreq = m_inputCapture ? m_inputCapture->maxFrequency() : AudioCapture::maxFrequency();
    const double logRange = (spectrumBars > 0 && maxFreq > minFreq) ? qLn(maxFreq / minFreq) : 0.0;

    int index = 0;
    for (const AudioBar &bar : m_spectrumBars)
    {
        QVariantMap barMap;

        if (index == 0)
        {
            barMap.insert("bLabel", "Volume Bar");
        }
        else
        {
            const int bandIndex = index - 1;
            double bandStartFreq = minFreq;
            double bandEndFreq = maxFreq;
            if (logRange > 0.0)
            {
                bandStartFreq = minFreq * qExp(logRange * (double(bandIndex) / double(spectrumBars)));
                bandEndFreq = minFreq * qExp(logRange * (double(bandIndex + 1) / double(spectrumBars)));
            }

            int bandStartHz = qCeil(bandStartFreq);
            int bandEndHz = (bandIndex == spectrumBars - 1) ? int(maxFreq) : (qCeil(bandEndFreq) - 1);
            if (bandEndHz <= bandStartHz)
                bandEndHz = bandStartHz;

            barMap.insert("bLabel", QString("#%1 (%2Hz - %3Hz)").arg(index)
                                       .arg(bandStartHz).arg(bandEndHz));
        }

        barMap.insert("index", index);
        barMap.insert("type", bar.m_type);

        if (bar.m_type == VCAudioTriggers::DMXBar)
        {
            barMap.insert("intVal", bar.m_dmxChannels.count());
        }
        else if (bar.m_type == VCAudioTriggers::FunctionBar)
        {
            barMap.insert("intVal", bar.m_functionId == Function::invalidId() ? -1 : int(bar.m_functionId));
        }
        else if (bar.m_type == VCAudioTriggers::VCWidgetBar)
        {
            barMap.insert("intVal", bar.m_widgetId == VCWidget::invalidId() ? -1 : int(bar.m_widgetId));
            VCWidget *widget = m_vc ? m_vc->widget(bar.m_widgetId) : nullptr;
            barMap.insert("strVal", widget ? widget->caption() : tr("No widget assigned"));
            barMap.insert("iconVal", widget ? VCWidget::typeToIcon(widget->type()) : QString());
        }
        else
        {
            barMap.insert("intVal", 0);
        }

        barMap.insert("minThreshold", qRound(SCALE(float(bar.m_minThreshold), 0.0, 255.0, 0.0, 100.0)));
        barMap.insert("maxThreshold", qRound(SCALE(float(bar.m_maxThreshold), 0.0, 255.0, 0.0, 100.0)));

        bList.append(barMap);
        index++;
    }

    return bList;
}

void VCAudioTriggers::setBarType(BarType type)
{
    if (m_selectedBar < 0 || m_selectedBar >= m_spectrumBars.count())
        return;

    // reset everything in any case
    AudioBar &bar = m_spectrumBars[m_selectedBar];
    bar.m_absDmxChannels.clear();
    bar.m_dmxChannels.clear();
    bar.m_minThreshold = 51;
    bar.m_maxThreshold = 204;
    bar.m_functionId = Function::invalidId();
    bar.m_function = nullptr;
    bar.m_widgetId = VCWidget::invalidId();
    bar.m_widget = nullptr;
    bar.m_tapped = false;
    bar.m_skippedBeats = 0;
    
    // set the type
    bar.m_type = type;

    emit barsInfoChanged();
}

void VCAudioTriggers::setBarThresholds(uchar minThr, uchar maxThr)
{
    if (m_selectedBar < 0 || m_selectedBar >= m_spectrumBars.count())
        return;

    AudioBar &bar = m_spectrumBars[m_selectedBar];
    bar.m_minThreshold = SCALE(float(minThr), 0.0, 100.0, 0.0, 255.0);
    bar.m_maxThreshold = SCALE(float(maxThr), 0.0, 100.0, 0.0, 255.0);
    emit barsInfoChanged();
}

void VCAudioTriggers::setBarFunction(quint32 functionId)
{
    if (m_selectedBar < 0 || m_selectedBar >= m_spectrumBars.count())
        return;

    AudioBar &bar = m_spectrumBars[m_selectedBar];
    bar.m_functionId = functionId;
    bar.m_function = (functionId != Function::invalidId() && m_doc)
                         ? m_doc->function(functionId)
                         : nullptr;
    emit barsInfoChanged();
}

void VCAudioTriggers::setBarWidget(quint32 widgetId)
{
    if (m_selectedBar < 0 || m_selectedBar >= m_spectrumBars.count())
        return;

    AudioBar &bar = m_spectrumBars[m_selectedBar];
    bar.m_widgetId = widgetId;
    bar.m_tapped = false;
    bar.m_skippedBeats = 0;
    updateBarWidgetReference(bar);
    emit barsInfoChanged();
}

void VCAudioTriggers::setBarDmxChannels(QList<SceneValue> list)
{
    if (m_selectedBar < 0 || m_selectedBar >= m_spectrumBars.count())
        return;

    AudioBar &bar = m_spectrumBars[m_selectedBar];
    bar.m_dmxChannels.clear();
    bar.m_absDmxChannels.clear();

    for (SceneValue &scv : list)
    {
        bar.m_dmxChannels.append(scv);

        if (Fixture *fx = m_doc->fixture(scv.fxi))
        {
            const quint32 absAddr = fx->universeAddress() + scv.channel;
            bar.m_absDmxChannels.append(int(absAddr));
        }
    }
}

void VCAudioTriggers::updateBarWidgetReference(AudioBar &bar)
{
    if (bar.m_widgetId == VCWidget::invalidId())
    {
        bar.m_widget = nullptr;
        return;
    }

    bar.m_widget = m_vc ? m_vc->widget(bar.m_widgetId) : nullptr;
}

void VCAudioTriggers::checkWidgetFunctionality(AudioBar &bar)
{
    if (bar.m_widgetId == VCWidget::invalidId())
        return;

    updateBarWidgetReference(bar);
    VCWidget *widget = bar.m_widget;
    if (widget == nullptr)
        return;

    switch (widget->type())
    {
        case VCWidget::ButtonWidget:
        {
            VCButton *button = qobject_cast<VCButton *>(widget);
            if (button == nullptr)
                return;

            if (bar.m_value >= bar.m_maxThreshold && button->state() == VCButton::Inactive)
                button->requestStateChange(true);
            else if (bar.m_value < bar.m_minThreshold && button->state() != VCButton::Inactive)
                button->requestStateChange(false);
        }
        break;
        case VCWidget::SliderWidget:
        {
            VCSlider *slider = qobject_cast<VCSlider *>(widget);
            if (slider != nullptr)
                slider->setValue(bar.m_value, true, true);
        }
        break;
        case VCWidget::SpeedWidget:
        {
            VCSpeedDial *speedDial = qobject_cast<VCSpeedDial *>(widget);
            if (speedDial == nullptr)
                return;

            int divisor = qMax(1, bar.m_divisor);
            if (bar.m_value >= bar.m_maxThreshold && !bar.m_tapped)
            {
                if (bar.m_skippedBeats == 0)
                    speedDial->tap();

                bar.m_tapped = true;
                bar.m_skippedBeats = (bar.m_skippedBeats + 1) % divisor;
            }
            else if (bar.m_value < bar.m_minThreshold)
            {
                bar.m_tapped = false;
            }
        }
        break;
        case VCWidget::CueListWidget:
        {
            VCCueList *cueList = qobject_cast<VCCueList *>(widget);
            if (cueList == nullptr)
                return;

            int divisor = qMax(1, bar.m_divisor);
            if (bar.m_value >= bar.m_maxThreshold && !bar.m_tapped)
            {
                if (bar.m_skippedBeats == 0)
                    cueList->nextClicked();

                bar.m_tapped = true;
                bar.m_skippedBeats = (bar.m_skippedBeats + 1) % divisor;
            }
            else if (bar.m_value < bar.m_minThreshold)
            {
                bar.m_tapped = false;
            }
        }
        break;
        default:
        break;
    }
}

void VCAudioTriggers::slotSpectrumDataChanged(double *spectrumBands,
                                             int size,
                                             double maxMagnitude,
                                             quint32 power)
{
    // First element of m_spectrumBars is the volume bar.
    // We registered bandsNumber = m_spectrumBars.count() - 1 with AudioCapture,
    // so 'size' must match that.
    if (size != m_spectrumBars.count() - 1)
        return;

    m_audioLevels.clear();
    m_audioLevels.reserve(size + 1);

    // --- 1) Volume (index 0) normalized to 0..255
    //      'power' comes from AudioCapture::processData() as an aggregated value;
    //      map it to [0,1] by clamping against 0x7FFF (15-bit) for a stable UI scale.
    //      If you want it "hotter", tweak kPowerMax.
    static constexpr double kPowerMax = 32767.0; // 0x7FFF
    const double vol01 = qBound(0.0, double(power) / kPowerMax, 1.0);
    const int    vol255 = int(vol01 * 255.0 + 0.5);

    m_spectrumBars[0].m_value = uchar(vol255);
    m_audioLevels.append(vol255);

    // --- 2) Spectrum bands normalized to 0..255 by current-frame maxMagnitude
    //      Optional gamma for nicer perception; 1.0 = linear, <1 brightens, >1 darkens.
    static constexpr double kGamma = 1.0;

    if (maxMagnitude <= 0.0)
    {
        // No usable energy: zero all bands
        for (int i = 0; i < size; ++i)
        {
            m_spectrumBars[i + 1].m_value = 0;
            m_audioLevels.append(0);
        }
        emit audioLevelsChanged();
        return;
    }

    for (int i = 0; i < size; ++i)
    {
        // Normalize this band to [0..1]
        double v = spectrumBands[i] / maxMagnitude;
        v = qBound(0.0, v, 1.0);

        // Perceptual shaping
        if (kGamma != 1.0)
            v = qPow(v, kGamma);

        static constexpr double kAlpha = 0.25; // 0..1, higher = snappier
        double old01 = m_spectrumBars[i + 1].m_value / 255.0;
        v = kAlpha * v + (1.0 - kAlpha) * old01;
        const int v255 = int(v * 255.0 + 0.5);

        // Store in bars (for DMX) and in UI list (index aligned: +1 for volume)
        m_spectrumBars[i + 1].m_value = uchar(v255);        
        m_audioLevels.append(v255);
    }

    for (int i = 0; i < m_spectrumBars.count(); i++)
    {
        AudioBar &bar = m_spectrumBars[i];
        if (bar.m_type == FunctionBar)
        {
            if (bar.m_function == nullptr && bar.m_functionId != Function::invalidId())
                bar.m_function = m_doc->function(bar.m_functionId);

            if (bar.m_function != nullptr)
            {
                if (bar.m_value >= bar.m_maxThreshold)
                    bar.m_function->start(m_doc->masterTimer(), functionParent());
                else if (bar.m_value < bar.m_minThreshold)
                    bar.m_function->stop(functionParent());
            }
        }
        else if (bar.m_type == VCWidgetBar)
        {
            checkWidgetFunctionality(bar);
        }
    }

    emit audioLevelsChanged();
}

/*********************************************************************
 * Fixture tree methods
 *********************************************************************/

void VCAudioTriggers::updateFixtureTree()
{
    if (m_fixtureTree == nullptr)
        return;

    m_fixtureTree->clear();
    FixtureManager::updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter,
                                     FixtureManager::ShowCheckBoxes | FixtureManager::ShowGroups | FixtureManager::ShowChannels,
                                     m_spectrumBars[m_selectedBar].m_dmxChannels);
}

QVariant VCAudioTriggers::groupsTreeModel()
{
    if (m_selectedBar < 0 || m_selectedBar >= m_spectrumBars.count())
        return QVariant();

    if (m_fixtureTree == nullptr)
    {
        m_fixtureTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";
        m_fixtureTree->setColumnNames(treeColumns);
        m_fixtureTree->enableSorting(false);
        updateFixtureTree();

        connect(m_fixtureTree, SIGNAL(roleChanged(TreeModelItem*,int,const QVariant&)),
                this, SLOT(slotTreeDataChanged(TreeModelItem*,int,const QVariant&)));
    }

    return QVariant::fromValue(m_fixtureTree);
}

QString VCAudioTriggers::searchFilter() const
{
    return m_searchFilter;
}

void VCAudioTriggers::setSearchFilter(QString searchFilter)
{
    if (m_searchFilter == searchFilter)
        return;

    int currLen = m_searchFilter.length();

    m_searchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS ||
        (currLen >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS))
    {
        FixtureManager::updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter,
                                         FixtureManager::ShowCheckBoxes | FixtureManager::ShowGroups | FixtureManager::ShowChannels,
                                         m_spectrumBars[m_selectedBar].m_dmxChannels);
        emit groupsTreeModelChanged();
    }

    emit searchFilterChanged();
}

void VCAudioTriggers::applyToSameType(bool enable)
{
    m_applyToSameType = enable;
}

void VCAudioTriggers::checkFixtureTree(TreeModel *tree, Fixture *sourceFixture,
                                      quint32 channelIndex, bool checked)
{
    if (tree == nullptr)
        return;

    for (TreeModelItem *item : tree->items())
    {
        QVariantList itemData = item->data();

        // itemData must be "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";
        if (itemData.count() == 6 && itemData.at(1).toInt() == App::ChannelDragItem)
        {
            quint32 itemID = itemData.at(2).toUInt();
            quint32 chIndex = itemData.at(4).toUInt();
            quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
            quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
            Fixture *destFixture = m_doc->fixture(fixtureID);

            if (destFixture == nullptr)
                continue;

            if (sourceFixture->fixtureDef() == destFixture->fixtureDef() &&
                sourceFixture->fixtureMode() == destFixture->fixtureMode() &&
                chIndex == channelIndex && linkedIndex == 0)
            {
                tree->setItemRoleData(item, checked, TreeModel::IsCheckedRole);

                SceneValue scv(fixtureID, chIndex);

                if (checked)
                {
                    if (m_spectrumBars[m_selectedBar].m_dmxChannels.contains(scv) == false)
                        m_spectrumBars[m_selectedBar].m_dmxChannels.append(scv);
                }
                else
                {
                    m_spectrumBars[m_selectedBar].m_dmxChannels.removeAll(scv);
                }
            }
        }

        if (item->hasChildren())
            checkFixtureTree(item->children(), sourceFixture, channelIndex, checked);
    }
}

void VCAudioTriggers::slotTreeDataChanged(TreeModelItem *item, int role, const QVariant &value)
{
    if (m_isUpdating)
        return;

    qDebug() << "VCAudioTriggers tree data changed" << value.toInt();
    qDebug() << "Item data:" << item->data();

    if (role != TreeModel::IsCheckedRole)
        return;

    QVariantList itemData = item->data();
    // itemData must be "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";
    if (itemData.count() != 6)
        return;

    //QString type = itemData.at(1).toString();
    quint32 itemID = itemData.at(2).toUInt();
    quint32 chIndex = itemData.at(4).toUInt();
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return;

    bool checked = value.toInt() == 0 ? false : true;

    if (m_applyToSameType)
    {
        m_isUpdating = true;
        checkFixtureTree(m_fixtureTree, fixture, chIndex, checked);
        m_isUpdating = false;
    }
    else
    {
        SceneValue scv(fixtureID, chIndex);

        if (checked)
        {
            if (m_spectrumBars[m_selectedBar].m_dmxChannels.contains(scv) == false)
                m_spectrumBars[m_selectedBar].m_dmxChannels.append(scv);
        }
        else
        {
            m_spectrumBars[m_selectedBar].m_dmxChannels.removeAll(scv);
        }
    }
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCAudioTriggers::updateFeedback()
{
    sendFeedback(m_captureEnabled ? UCHAR_MAX : 0, INPUT_ENABLE_CAPTURE,
                 m_captureEnabled ? VCWidget::UpperValue : VCWidget::LowerValue);
}

void VCAudioTriggers::slotInputValueChanged(quint8 id, uchar value)
{
    switch (id)
    {
        case INPUT_ENABLE_CAPTURE:
            setCaptureEnabled(value ? true : false);
        break;
        case INPUT_VOLUME_CONTROL:
            setVolumeLevel(SCALE(value, 0, 255, 0, 100));
        break;
    }
}

/*********************************************************************
 * DMXSource
 *********************************************************************/

void VCAudioTriggers::writeDMX(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);

    quint32 lastUniverse = Universe::invalid();
    QSharedPointer<GenericFader> fader;

    for (AudioBar &bar : m_spectrumBars)
    {
        if (bar.m_type == DMXBar)
        {
            for (int i = 0; i < bar.m_absDmxChannels.count(); i++)
            {
                int absAddress = bar.m_absDmxChannels.at(i);
                //quint32 address = absAddress & 0x01FF;
                quint32 universe = absAddress >> 9;
                if (universe != lastUniverse)
                {
                    fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
                    if (fader == NULL)
                    {
                        fader = universes[universe]->requestFader();
                        fader->adjustIntensity(intensity());
                        m_fadersMap[universe] = fader;
                    }
                    fader->setEnabled(m_captureEnabled);
                    lastUniverse = universe;
                }

                FadeChannel *fc = fader->getChannelFader(m_doc, universes[universe], Fixture::invalidId(), absAddress);
                fc->setStart(fc->current());
                fc->setTarget(bar.m_value);
                fc->setReady(false);
                fc->setElapsed(0);
            }
        }
    }
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCAudioTriggers::loadBarXML(QXmlStreamReader &root)
{
    QXmlStreamAttributes attrs = root.attributes();

    if (attrs.hasAttribute(KXMLQLCAudioBarType) == false)
        return false;

    int barIndex = attrs.value(KXMLQLCAudioBarIndex).toString().toInt();
    // Transpose legacy volume bar index
    if (barIndex == 1000)
        barIndex = 0;

    if (barIndex < 0 || barIndex >= m_spectrumBars.count())
    {
        qDebug() << "Audio Triggers bar index out of bounds!" << barIndex;
        return false;
    }

    AudioBar &bar = m_spectrumBars[barIndex];

    bar.m_type = BarType(attrs.value(KXMLQLCAudioBarType).toString().toInt());
    bar.m_minThreshold = attrs.value(KXMLQLCAudioBarMinThreshold).toString().toInt();
    bar.m_maxThreshold = attrs.value(KXMLQLCAudioBarMaxThreshold).toString().toInt();
    bar.m_divisor = qMax(1, attrs.value(KXMLQLCAudioBarDivisor).toString().toInt());

    switch (bar.m_type)
    {
        case FunctionBar:
        {
            if (attrs.hasAttribute(KXMLQLCAudioBarFunction))
            {
                bar.m_functionId = attrs.value(KXMLQLCAudioBarFunction).toUInt();
                Function *func = m_doc->function(bar.m_functionId);
                if (func != NULL)
                    bar.m_function = func;
            }
        }
        break;
        case VCWidgetBar:
        {
            if (attrs.hasAttribute(KXMLQLCAudioBarWidget))
            {
                quint32 wid = attrs.value(KXMLQLCAudioBarWidget).toString().toUInt();
                bar.m_widgetId = wid;
                bar.m_widget = nullptr;
                bar.m_tapped = false;
                bar.m_skippedBeats = 0;
            }
        }
        break;
        case DMXBar:
        {
            QXmlStreamReader::TokenType tType = root.readNext();

            if (tType == QXmlStreamReader::EndElement)
            {
                root.readNext();
                return true;
            }

            if (tType == QXmlStreamReader::Characters)
                root.readNext();

            if (root.name() == KXMLQLCAudioBarDMXChannels)
            {
                QString dmxValues = root.readElementText();
                if (dmxValues.isEmpty() == false)
                {
                    QList<SceneValue> channels;
                    QStringList varray = dmxValues.split(",");
                    for (int i = 0; i < varray.count(); i+=2)
                    {
                        channels.append(SceneValue(QString(varray.at(i)).toUInt(),
                                                   QString(varray.at(i + 1)).toUInt(), 0));
                    }
                    selectBarForEditing(barIndex);
                    setBarDmxChannels(channels);
                    selectBarForEditing(-1);
                }
            }
        }
        break;
        default:
        break;
    }

    return true;
}

bool VCAudioTriggers::saveBarXML(QXmlStreamWriter *doc, int index) const
{
    Q_ASSERT(doc != NULL);

    if (index < 0 || index >= m_spectrumBars.count())
    {
        qDebug() << "Audio Triggers bar index out of bounds!" << index;
        return false;
    }

    AudioBar bar = m_spectrumBars[index];

    doc->writeStartElement(KXMLQLCAudioTriggerBar);
    doc->writeAttribute(KXMLQLCAudioBarType, QString::number(bar.m_type));
    doc->writeAttribute(KXMLQLCAudioBarMinThreshold, QString::number(bar.m_minThreshold));
    doc->writeAttribute(KXMLQLCAudioBarMaxThreshold, QString::number(bar.m_maxThreshold));
    doc->writeAttribute(KXMLQLCAudioBarDivisor, QString::number(bar.m_divisor));
    doc->writeAttribute(KXMLQLCAudioBarIndex, QString::number(index));

    if (bar.m_type == DMXBar && bar.m_dmxChannels.count() > 0)
    {
        QString chans;
        foreach (SceneValue scv, bar.m_dmxChannels)
        {
            if (chans.isEmpty() == false)
                chans.append(",");
            chans.append(QString("%1,%2").arg(scv.fxi).arg(scv.channel));
        }
        if (chans.isEmpty() == false)
        {
            doc->writeTextElement(KXMLQLCAudioBarDMXChannels, chans);
        }
    }
    else if (bar.m_type == FunctionBar && bar.m_functionId != Function::invalidId())
    {
        doc->writeAttribute(KXMLQLCAudioBarFunction, QString::number(bar.m_functionId));
    }
    else if (bar.m_type == VCWidgetBar && bar.m_widgetId != VCWidget::invalidId())
    {
        doc->writeAttribute(KXMLQLCAudioBarWidget, QString::number(bar.m_widgetId));
    }

    /* End <Bar> tag */
    doc->writeEndElement();

    return true;
}

bool VCAudioTriggers::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCAudioTriggers)
    {
        qWarning() << Q_FUNC_INFO << "Audio trigger node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();
    if (root.attributes().hasAttribute(KXMLQLCAudioBarsNumber))
    {
        int barsNum = root.attributes().value(KXMLQLCAudioBarsNumber).toInt();
        setBarsNumber(barsNum + 1);
    }

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
        else if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInputSource(root);
        }
        else if (root.name() == KXMLQLCVCWidgetKey)
        {
            loadXMLInputKey(root);
        }
        else if (root.name() == KXMLQLCAudioTriggerBar ||
                 root.name() == KXMLQLCVolumeBar ||
                 root.name() == KXMLQLCSpectrumBar)
        {
            loadBarXML(root);
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown audio trigger tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCAudioTriggers::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != nullptr);

    /* VC object entry */
    doc->writeStartElement(KXMLQLCVCAudioTriggers);
    doc->writeAttribute(KXMLQLCAudioBarsNumber, QString::number(barsNumber() - 1));

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* External control */
    saveXMLInputControl(doc, INPUT_ENABLE_CAPTURE);
    saveXMLInputControl(doc, INPUT_VOLUME_CONTROL);

    /* Save only configured triggers */
    int barIndex = 0;
    for (const AudioBar &bar : m_spectrumBars)
    {
        if (bar.m_type != None)
            saveBarXML(doc, barIndex);
        barIndex++;
    }

    /* Write the <end> tag */
    doc->writeEndElement();

    return true;
}
