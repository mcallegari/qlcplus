/*
  Q Light Controller Plus
  stagewizard_controller.cpp

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

#include "stagewizard.h"

#include "doc.h"
#include "function.h"
#include "inputoutputmap.h"
#include "inputpatch.h"
#include "outputpatch.h"
#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "qlcinputsource.h"
#include "qlcinputfeedback.h"

#include "virtualconsole/vcwidget.h"

#include <QDebug>
#include <climits>
#include <limits>

// Without an input profile we can still map, but we have to guess the layout.
// These match the most common MIDI controller convention: the first eight
// channels are continuous controls, everything above is a note/button.
static const int kNoProfileFaders  = 8;
static const int kNoProfileButtons = 56;

// ── Controller enumeration ───────────────────────────────────────────────────

QLCInputProfile *StageWizard::controllerProfile() const
{
    if (m_ctrlUniverse < 0)
        return nullptr;

    InputOutputMap *iomap = m_doc->inputOutputMap();
    if (iomap == nullptr)
        return nullptr;

    InputPatch *ip = iomap->inputPatch(static_cast<quint32>(m_ctrlUniverse));
    return ip ? ip->profile() : nullptr;
}

QVariant StageWizard::controllersModel() const
{
    QVariantList list;

    InputOutputMap *iomap = m_doc->inputOutputMap();
    if (iomap == nullptr)
        return QVariant::fromValue(list);

    // Walk every universe and report the ones that carry an input patch. This
    // is what the user actually connected in the I/O panel — the previous
    // implementation listed ioManager->universeInputSources(), which returns
    // the plugin lines still AVAILABLE for patching, so a controller that was
    // already patched was excluded and the list came up empty.
    for (quint32 u = 0; u < iomap->universesCount(); u++)
    {
        InputPatch *ip = iomap->inputPatch(u);
        if (ip == nullptr || !ip->isPatched())
            continue;

        // The wizard patches Loopback itself for VC page switching; it is not a
        // controller and must never be offered as one.
        if (ip->pluginName() == QStringLiteral("Loopback"))
            continue;

        QLCInputProfile *profile = ip->profile();

        int buttons = 0, faders = 0;
        if (profile != nullptr)
        {
            QMapIterator<quint32, QLCInputChannel *> it(profile->channels());
            while (it.hasNext())
            {
                it.next();
                switch (it.value()->type())
                {
                    case QLCInputChannel::Button:
                        buttons++;
                        break;
                    case QLCInputChannel::Slider:
                    case QLCInputChannel::Knob:
                    case QLCInputChannel::Encoder:
                        faders++;
                        break;
                    default:
                        break;
                }
            }
        }
        else
        {
            buttons = kNoProfileButtons;
            faders  = kNoProfileFaders;
        }

        QVariantMap m;
        m["universe"]      = int(u);
        m["universeName"]  = iomap->getUniverseNameByIndex(u);
        m["plugin"]        = ip->pluginName();
        m["line"]          = int(ip->input());
        m["lineName"]      = ip->inputName();
        m["profile"]       = ip->profileName();
        m["hasProfile"]    = profile != nullptr;
        m["buttons"]       = buttons;
        m["faders"]        = faders;
        m["hasColorTable"] = profile != nullptr && profile->hasColorTable();
        // A feedback patch is an output patch on the same universe flagged as
        // feedback; when present the controller can light its LEDs.
        m["feedback"]      = iomap->feedbackPatch(u) != nullptr;

        list.append(m);
    }

    return QVariant::fromValue(list);
}

void StageWizard::refreshControllers()
{
    InputOutputMap *iomap = m_doc->inputOutputMap();

    // Drop a stale selection (e.g. the user unpatched it in the I/O panel) and
    // auto-select when there is exactly one candidate, so the common
    // "one controller connected" case needs no click at all.
    bool selectionValid = false;
    int  firstUniverse  = -1;
    int  candidates     = 0;

    if (iomap != nullptr)
    {
        for (quint32 u = 0; u < iomap->universesCount(); u++)
        {
            InputPatch *ip = iomap->inputPatch(u);
            if (ip == nullptr || !ip->isPatched() ||
                ip->pluginName() == QStringLiteral("Loopback"))
                continue;

            candidates++;
            if (firstUniverse < 0)
                firstUniverse = int(u);
            if (int(u) == m_ctrlUniverse)
                selectionValid = true;
        }
    }

    if (!selectionValid)
        m_ctrlUniverse = (candidates == 1) ? firstUniverse : -1;

    emit controllersModelChanged();
    emit controllerChanged();
}

void StageWizard::setControllerUniverse(int universe)
{
    if (universe == m_ctrlUniverse)
        return;

    m_ctrlUniverse = universe;
    emit controllerChanged();
}

void StageWizard::setMapController(bool map)
{
    if (map == m_ctrlMap)
        return;

    m_ctrlMap = map;
    emit controllerChanged();
}

void StageWizard::setSendFeedback(bool enable)
{
    if (enable == m_ctrlFeedback)
        return;

    m_ctrlFeedback = enable;
    emit controllerChanged();
}

void StageWizard::setMapColors(bool enable)
{
    if (enable == m_ctrlColors)
        return;

    m_ctrlColors = enable;
    emit controllerChanged();
}

QString StageWizard::controllerMappingPreview() const
{
    if (m_ctrlUniverse < 0 || !m_ctrlMap)
        return QString();

    CtrlPools pools;
    buildControllerPools(pools);

    // Estimate demand from the layout createVCLayout() will build. Kept in step
    // with the VC generator: page tabs + show cues are shared across pages, and
    // each group page adds its colours, effects, a fader and (movers only) a pad.
    QList<const FixtureGroupEntry *> pageGroups;
    for (const FixtureGroupEntry &grp : m_groups)
        if (grp.selected && !grp.fixtureIDs.isEmpty())
            pageGroups.append(&grp);

    const int pageCount = pageGroups.count() + 1;   // + "All Groups"

    int buttons = pageCount   // one page tab per page
                + 6;          // Blackout / Open / Big Moment / Close / Ambient / Blinder
    int faders  = 0;

    auto countPage = [&](const FixtureGroupEntry &grp)
    {
        if (grp.hasDimmer || grp.hasRGB || grp.hasCMY)
            faders++;                       // Intensity slider
        if (grp.hasMovement)
        {
            faders  += 2;                   // XY pad pan + tilt
            buttons += 5;                   // Fly Out/In, Circle, Eight, Sweep
        }
        buttons += 2;                       // Strobe Chase, Heartbeat
        // Colour buttons: one per generated colour scene for this group.
        for (quint32 id : m_generatedFunctionIDs)
        {
            Function *f = m_doc->function(id);
            if (f && f->type() == Function::SceneType &&
                f->path().contains(grp.name + "/" + tr("Colors")))
                buttons++;
        }
    };

    if (m_hasAllGroups)
        countPage(m_allGroups);
    for (const FixtureGroupEntry *grp : pageGroups)
        countPage(*grp);

    QStringList parts;
    parts << tr("%1 of %2 buttons").arg(qMin(buttons, pools.buttonTotal))
                                   .arg(pools.buttonTotal);
    if (pools.faderTotal > 0 || faders > 0)
        parts << tr("%1 of %2 faders").arg(qMin(faders, pools.faderTotal))
                                      .arg(pools.faderTotal);

    QString text = parts.join(tr(", "));

    if (buttons > pools.buttonTotal || faders > pools.faderTotal)
        text += tr(" — not everything fits; the rest stays unmapped");

    return text;
}

// ── Channel pools ────────────────────────────────────────────────────────────

void StageWizard::buildControllerPools(CtrlPools &pools) const
{
    pools = CtrlPools();

    if (m_ctrlUniverse < 0)
        return;

    QLCInputProfile *profile = controllerProfile();

    if (profile != nullptr)
    {
        // Profile channels are a QMap keyed by channel number, so iteration is
        // already in controller order: faders and buttons come out in the same
        // left-to-right / top-to-bottom order the user sees on the device.
        QMapIterator<quint32, QLCInputChannel *> it(profile->channels());
        while (it.hasNext())
        {
            it.next();
            const QLCInputChannel *ich = it.value();
            CtrlChannel cc { it.key(), ich->name() };

            switch (ich->type())
            {
                case QLCInputChannel::Button:
                    pools.buttons.append(cc);
                    break;
                case QLCInputChannel::Slider:
                case QLCInputChannel::Knob:
                case QLCInputChannel::Encoder:
                    pools.faders.append(cc);
                    break;
                default:
                    // NextPage / PrevPage / PageSet / NoType are left alone:
                    // they have a dedicated meaning the user may already rely on.
                    break;
            }
        }
    }
    else
    {
        // No profile: synthesise a plausible layout so a bare patch still maps.
        for (int i = 0; i < kNoProfileFaders; i++)
            pools.faders.append({ quint32(i), QString() });
        for (int i = 0; i < kNoProfileButtons; i++)
            pools.buttons.append({ quint32(kNoProfileFaders + i), QString() });
    }

    pools.buttonTotal = pools.buttons.count();
    pools.faderTotal  = pools.faders.count();
}

bool StageWizard::nextButtonChannel(CtrlPools &pools, quint32 &channel) const
{
    if (pools.nextButton >= pools.buttons.count())
        return false;

    channel = pools.buttons.at(pools.nextButton++).channel;
    return true;
}

bool StageWizard::nextFaderChannel(CtrlPools &pools, quint32 &channel) const
{
    if (pools.nextFader >= pools.faders.count())
        return false;

    channel = pools.faders.at(pools.nextFader++).channel;
    return true;
}

// ── Colour feedback ──────────────────────────────────────────────────────────

int StageWizard::closestProfileColor(const QColor &color) const
{
    QLCInputProfile *profile = controllerProfile();
    if (profile == nullptr || !profile->hasColorTable() || !color.isValid())
        return -1;

    // Nearest entry by squared RGB distance. Good enough for the coarse LED
    // palettes controllers expose (typically 8-128 entries), and it degrades
    // gracefully: an off-palette colour lands on the closest one available.
    int    bestValue = -1;
    qint64 bestDist  = std::numeric_limits<qint64>::max();

    QMapIterator<uchar, QPair<QString, QColor>> it(profile->colorTable());
    while (it.hasNext())
    {
        it.next();
        const QColor &c = it.value().second;
        if (!c.isValid())
            continue;

        qint64 dr = c.red()   - color.red();
        qint64 dg = c.green() - color.green();
        qint64 db = c.blue()  - color.blue();
        qint64 dist = dr * dr + dg * dg + db * db;

        if (dist < bestDist)
        {
            bestDist  = dist;
            bestValue = int(it.key());
        }
    }

    return bestValue;
}

void StageWizard::ensureFeedbackPatch()
{
    if (!m_ctrlFeedback || m_ctrlUniverse < 0)
        return;

    InputOutputMap *iomap = m_doc->inputOutputMap();
    if (iomap == nullptr)
        return;

    quint32 uni = static_cast<quint32>(m_ctrlUniverse);

    // Already set up (e.g. the user patched it themselves in the I/O panel).
    if (iomap->feedbackPatch(uni) != nullptr)
        return;

    InputPatch *ip = iomap->inputPatch(uni);
    if (ip == nullptr)
        return;

    // Feedback goes back out over the plugin's output line that carries the
    // same name as the input line — that is the same physical device.
    const QString inputName = ip->inputName();
    int line = 0;
    for (const QString &pLine : iomap->pluginOutputs(ip->pluginName()))
    {
        if (pLine == inputName)
        {
            iomap->setOutputPatch(uni, ip->pluginName(), QString(), QString(),
                                  quint32(line), true);
            m_doc->setModified();
            return;
        }
        line++;
    }

    qDebug() << "[StageWizard] no matching feedback output line for" << inputName;
}

// ── Widget mapping ───────────────────────────────────────────────────────────

void StageWizard::mapWidgetControl(VCWidget *widget, quint8 controlID,
                                   quint32 channel, const QColor &color)
{
    if (widget == nullptr || m_ctrlUniverse < 0)
        return;

    QSharedPointer<QLCInputSource> src(
        new QLCInputSource(static_cast<quint32>(m_ctrlUniverse), channel));
    src->setID(controlID);

    // VCPage::inputValueChanged() only dispatches when the source's page equals
    // the widget's page, so a widget sitting on frame page 2 needs a source
    // tagged page 2. The widget already knows its page (setupLookAndFeel() set
    // it when the multipage frame took ownership), so mirror it here — leaving
    // the default 0 would silently break every page but the first.
    src->setPage(static_cast<ushort>(qMax(0, widget->page())));

    // Colour feedback: drive the LED with the colour-table value nearest to the
    // button's background. The "on" (upper) state gets the colour, the "off"
    // (lower) state stays dark. Must be set BEFORE addInputSource(), because
    // applyInputProfileSettings() only fills in values the source hasn't
    // customised (it treats lower != 0 / upper != UCHAR_MAX as user intent).
    if (m_ctrlColors && color.isValid())
    {
        int fbValue = closestProfileColor(color);

        // A value of 255 is indistinguishable from "untouched" to
        // applyInputProfileSettings() (it tests upper != UCHAR_MAX), so it would
        // be replaced by the profile default. Colour tables index LED palettes
        // and effectively never use the last slot, so skipping it costs nothing.
        if (fbValue >= 0 && fbValue != UCHAR_MAX)
        {
            src->setFeedbackValue(QLCInputFeedback::LowerValue, 0);
            src->setFeedbackValue(QLCInputFeedback::UpperValue, uchar(fbValue));
        }
    }

    // addInputSource() runs applyInputProfileSettings(), which pulls the
    // profile's per-channel feedback extra params (MIDI channel, OSC path, …),
    // relative/encoder working mode and the button lower/upper values.
    widget->addInputSource(src);
}
