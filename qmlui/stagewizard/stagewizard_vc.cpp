/*
  Q Light Controller Plus
  stagewizard_vc.cpp

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
#include "fixture.h"
#include "function.h"
#include "scene.h"
#include "qlcpalette.h"
#include "scenevalue.h"
#include "chaser.h"
#include "chaserstep.h"
#include "efx.h"
#include "collection.h"
#include "efxfixture.h"
#include "rgbmatrix.h"
#include "rgbalgorithm.h"
#include "fixturegroup.h"
#include "monitorproperties.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "qlcfixturehead.h"
#include "qlcfixturemode.h"

#include "fixturemanager.h"
#include "functionmanager.h"
#include "virtualconsole/virtualconsole.h"
#include "virtualconsole/vcpage.h"
#include "virtualconsole/vcframe.h"
#include "virtualconsole/vcsoloframe.h"
#include "virtualconsole/vcbutton.h"
#include "virtualconsole/vcslider.h"
#include "virtualconsole/vccuelist.h"
#include "virtualconsole/vcxypad.h"
#include "contextmanager.h"
#include "mainview3d.h"
#include "fixtureutils.h"
#include "inputoutputmap.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "qlcinputsource.h"
#include "qlcfixturedef.h"

#include <QtMath>
#include <QSet>
#include <QDebug>

// Layout metrics — pixelDensity units, matching the VC default drop sizes.
static const int kBtnW  = 17;   // all buttons: VC default square (pd * 17)
static const int kBtnH  = 17;
static const int kSlW   = 15;   // slider width (VC default)
static const int kHeaderH = 8;  // frame title-bar height (~listItemHeight in pd)
static const int kFrmPad = 4;

// ── Virtual Console layout ────────────────────────────────────────────────────

VCPage *StageWizard::pickTargetPage()
{
    int count = m_virtualConsole->pagesCount();

    // Reuse the first page that has no widgets yet
    for (int i = 0; i < count; i++)
    {
        VCPage *page = m_virtualConsole->page(i);
        if (page && page->children(false).isEmpty())
            return page;
    }

    // All existing pages are populated: append a fresh one
    m_virtualConsole->addPage(count);
    return m_virtualConsole->page(count);
}

void StageWizard::createVCLayout()
{
    VCPage *page = pickTargetPage();
    if (!page) return;

    int pd  = static_cast<int>(m_virtualConsole->pixelDensity());
    if (pd <= 0) pd = 1;

    int btnH = kBtnH * pd;
    int pad  = kFrmPad * pd;

    // Selected groups with fixtures, in page order (page 1..N).
    QList<const FixtureGroupEntry *> pageGroups;
    for (const FixtureGroupEntry &grp : m_groups)
        if (grp.selected && !grp.fixtureIDs.isEmpty())
            pageGroups.append(&grp);

    const int pageCount = pageGroups.count() + 1; // page 0 = All Groups

    // ── Loopback plumbing for page switching ────────────────────────────────
    quint32 loopInUni = InputOutputMap::invalidUniverse();
    QList<quint32> bridgeFx; // one generic dimmer per page
    bool loopback = ensureLoopbackPatch(pageCount, loopInUni, bridgeFx);

    QList<quint32> pageSceneIDs; // index 0 = All Groups, 1..N = groups
    if (loopback)
        generatePageSwitchScenes(bridgeFx, pageGroups, pageSceneIDs);

    // ── Find show cue functions by name suffix ──────────────────────────────
    auto findFunc = [&](const QString &suffix) -> quint32
    {
        for (quint32 id : m_generatedFunctionIDs)
        {
            Function *f = m_doc->function(id);
            if (f && f->name().endsWith(suffix))
                return id;
        }
        return Function::invalidId();
    };

    // ── Master multipage frame ──────────────────────────────────────────────
    VCFrame *frame = qobject_cast<VCFrame *>(
        page->addWidget(nullptr, "Frame", QPoint(pad, pad)));
    if (!frame) return;

    frame->setCaption(tr("Show Wizard"));
    frame->setMultiPageMode(true);
    frame->setTotalPagesNumber(pageCount);

    // Page labels + shortcut names
    frame->setShortcutName(0, tr("All Groups"));
    for (int i = 0; i < pageGroups.count(); i++)
        frame->setShortcutName(i + 1, pageGroups[i]->name);

    // Bind each page shortcut to the matching loopback INPUT channel, so the
    // top-row Flash buttons (which drive the loopback OUTPUT via a scene) flip
    // the frame's page. Control id = INPUT_SHORTCUT_BASE_ID + page.
    if (loopback && loopInUni != InputOutputMap::invalidUniverse())
    {
        for (int p = 0; p < pageCount; p++)
        {
            QSharedPointer<QLCInputSource> src(
                new QLCInputSource(loopInUni, static_cast<quint32>(p)));
            src->setID(INPUT_SHORTCUT_BASE_ID + p);
            frame->addInputSource(src);
        }
    }

    // Every button is the VC default square size (pixelDensity * 17).
    const int txtW = kBtnW * pd;

    // ── Frame width = exactly what the widest content row needs ───────────────
    // (No arbitrary minimum — that was making the frame too wide.) Content that
    // still doesn't fit wraps to a new row so nothing is ever cut.
    int xyPadW = btnH * 3 + pad * 2;                            // square XY pad
    int tabsW = pad + pageCount * (txtW + pad);                 // row 1: page tabs
    int row2W = pad + kSlW * pd + pad + 8 * (kBtnW * pd + pad); // fader + 8 swatches
    int row4W = pad + xyPadW + pad + 4 * (txtW + pad);          // XY pad + 4 effects
    int cueW  = pad + 6 * (txtW + pad);                         // 6 show cues

    int frmW = qMax(qMax(tabsW, row2W), qMax(row4W, cueW)) + pad;

    // The usable content width (inside the left/right frame padding).
    const int usableRight = frmW - pad;

    // Content must start below the frame's title bar, else the first row is
    // drawn underneath it (children fill the whole frame, header overlays them).
    const int contentTop = kHeaderH * pd + pad;

    quint32 blinderFlashID = findFunc("Blinder Flash");

    // A wrapping row cursor: places a widget of ($w x $h) at the current spot,
    // wrapping to the next row when it would cross the frame's right edge, so
    // every widget stays fully inside the frame. Returns the placement point.
    struct Cursor { int x; int y; int rowH; };

    auto place = [&](Cursor &c, int w, int h) -> QPoint
    {
        if (c.x > pad && c.x + w > usableRight)  // doesn't fit → next row
        {
            c.x = pad;
            c.y += c.rowH + pad;
            c.rowH = 0;
        }
        QPoint p(c.x, c.y);
        c.x += w + pad;
        c.rowH = qMax(c.rowH, h);
        return p;
    };

    // The qmlui multipage frame shows only the widgets whose page == current
    // page; there is no "all pages" flag. So the page-switch row and the
    // show-cue row are replicated on every page. Both wrap if too wide.
    // $topOut / $cueOut receive the Y just below each row so page bodies and the
    // frame height can be sized to fit.
    auto buildSharedTopRow = [&](int pageIdx, int &topOut)
    {
        frame->setCurrentPage(pageIdx);
        Cursor c { pad, contentTop, 0 };   // row 1, below the title bar

        if (loopback && !pageSceneIDs.isEmpty())
        {
            auto addPageButton = [&](const QString &caption, quint32 sceneID)
            {
                QPoint p = place(c, txtW, btnH);
                VCButton *btn = qobject_cast<VCButton *>(
                    frame->addWidget(nullptr, "Button", p));
                if (!btn) return;
                btn->setCaption(caption);
                btn->setGeometry(QRectF(p.x(), p.y(), txtW, btnH));
                if (sceneID != Function::invalidId())
                {
                    btn->setFunctionID(sceneID);
                    btn->setActionType(VCButton::Flash);
                }
            };
            addPageButton(tr("All Groups"),
                          pageSceneIDs.value(0, Function::invalidId()));
            for (int i = 0; i < pageGroups.count(); i++)
                addPageButton(pageGroups[i]->name,
                              pageSceneIDs.value(i + 1, Function::invalidId()));
        }
        topOut = c.y + c.rowH + pad;   // Y below the (possibly wrapped) tab row
    };

    auto buildSharedCueRow = [&](int pageIdx, int cueY)
    {
        frame->setCurrentPage(pageIdx);
        Cursor c { pad, cueY, 0 };

        auto addCue = [&](const QString &caption, quint32 funcID, bool flash)
        {
            if (funcID == Function::invalidId())
                return;
            QPoint p = place(c, txtW, btnH);
            VCButton *btn = qobject_cast<VCButton *>(
                frame->addWidget(nullptr, "Button", p));
            if (!btn) return;
            btn->setCaption(caption);
            btn->setGeometry(QRectF(p.x(), p.y(), txtW, btnH));
            btn->setFunctionID(funcID);
            if (flash)
                btn->setActionType(VCButton::Flash);
        };

        addCue(tr("Blackout"),   findFunc("Blackout"),    false);
        addCue(tr("Show Open"),  findFunc("Show Open"),    false);
        addCue(tr("Big Moment"), findFunc("Big Moment"),   false);
        addCue(tr("Show Close"), findFunc("Show Close"),   false);
        addCue(tr("Ambient"),    findFunc("Ambient Loop"), false);
        addCue(tr("Blinder"),    blinderFlashID,           true);
    };

    // ── Group page body ──────────────────────────────────────────────────────
    // Row 2 = dimmer fader (2 rows tall) + colour buttons (max 8/row); row 4 =
    // XY pad (2 rows tall) + effect/blinder buttons (max 4/row). $bodyTop is the
    // Y just below the shared top row. Returns the bottom Y reached.
    auto buildBody = [&](int pageIdx, const FixtureGroupEntry &grp, int bodyTop) -> int
    {
        frame->setCurrentPage(pageIdx);

        int btnW = kBtnW * pd;                  // square swatches

        // ── Row 2: dimmer slider (2 rows tall) + colour buttons (max 8/row) ──
        // The slider spans two button rows; colour buttons form a grid to its
        // right, wrapping at 8 per row.
        int row2Bottom = bodyTop;               // grows as content is placed
        {
            int slH = btnH * 2 + pad * 3;       // fader spans ~2 button rows (taller)

            int colStartX = pad;                // colour grid left edge
            if (grp.hasDimmer || grp.hasRGB || grp.hasCMY)
            {
                int w = kSlW * pd;
                VCSlider *slider = qobject_cast<VCSlider *>(
                    frame->addWidget(nullptr, "Slider", QPoint(pad, bodyTop)));
                if (slider)
                {
                    slider->setCaption(tr("Intensity"));
                    slider->setGeometry(QRectF(pad, bodyTop, w, slH));
                    slider->setSliderMode(VCSlider::Level);
                    for (quint32 fxID : grp.fixtureIDs)
                    {
                        Fixture *fx = m_doc->fixture(fxID);
                        if (!fx) continue;
                        for (quint32 ch = 0; ch < fx->channels(); ++ch)
                        {
                            const QLCChannel *cc = fx->channel(ch);
                            if (cc && cc->group() == QLCChannel::Intensity &&
                                cc->colour() == QLCChannel::NoColour)
                                slider->addLevelChannel(fxID, ch);
                        }
                    }
                    colStartX = pad + w + pad;  // colours start right of the fader
                }
            }
            row2Bottom = bodyTop + slH;

            // Colour buttons: grid of max 8 per row, beside the fader.
            const int perRow = 8;
            int colCount = 0, colX = colStartX, colY = bodyTop;
            for (quint32 id : m_generatedFunctionIDs)
            {
                Function *f = m_doc->function(id);
                if (!f || f->type() != Function::SceneType)
                    continue;
                if (!f->path().contains(grp.name + "/" + tr("Colors")))
                    continue;

                // Wrap after 8, or if the swatch would exceed the frame width.
                if (colCount > 0 &&
                    (colCount % perRow == 0 || colX + btnW > usableRight))
                {
                    colX = colStartX;
                    colY += btnH + pad;
                }

                VCButton *btn = qobject_cast<VCButton *>(
                    frame->addWidget(nullptr, "Button", QPoint(colX, colY)));
                if (!btn) continue;

                QString cap = f->name();
                int dash = cap.indexOf(QStringLiteral(" – "));
                if (dash >= 0) cap = cap.mid(dash + 3);
                btn->setCaption(cap);
                btn->setGeometry(QRectF(colX, colY, btnW, btnH));
                btn->setFunctionID(id);
                btn->setActionType(VCButton::Toggle);

                QColor col = sceneColor(id);
                if (col.isValid())
                    btn->setBackgroundColor(col);

                colX += btnW + pad;
                colCount++;
                row2Bottom = qMax(row2Bottom, colY + btnH);
            }
        }

        // ── Row 4: XY pad (3 rows tall) + effect/blinder buttons (max 4/row) ──
        int row4Top = row2Bottom + pad;
        int row4Bottom = row4Top;

        // XY Pad (moving heads only) — leads the row, spanning three button rows
        // so it's large enough to be usable (roughly square).
        int effStartX = pad;
        if (grp.hasMovement)
        {
            int h = btnH * 3 + pad * 2;   // three button rows tall
            int w = h;                    // square pad
            VCXYPad *xy = qobject_cast<VCXYPad *>(
                frame->addWidget(nullptr, "XYPad", QPoint(pad, row4Top)));
            if (xy)
            {
                xy->setCaption(tr("Position"));
                xy->setGeometry(QRectF(pad, row4Top, w, h));
                for (quint32 fxID : grp.fixtureIDs)
                {
                    Fixture *fx = m_doc->fixture(fxID);
                    if (!fx) continue;
                    for (int h2 = 0; h2 < qMax(1, fx->heads()); ++h2)
                        xy->addFixtureGroupHeadPreset(static_cast<int>(fxID), h2);
                }
                effStartX = pad + w + pad;   // effects start right of the XY pad
                row4Bottom = qMax(row4Bottom, row4Top + h);
            }
        }

        // Effect + blinder buttons: fixed size, max 4 per row, beside the XY pad.
        const int effPerRow = 4;
        int effCount = 0, effX = effStartX, effY = row4Top;

        auto addEffectBtn = [&](const QString &caption, quint32 id,
                                VCButton::ButtonAction action)
        {
            if (effCount > 0 &&
                (effCount % effPerRow == 0 || effX + txtW > usableRight))
            {
                effX = effStartX;
                effY += btnH + pad;
            }
            VCButton *btn = qobject_cast<VCButton *>(
                frame->addWidget(nullptr, "Button", QPoint(effX, effY)));
            if (btn)
            {
                btn->setCaption(caption);
                btn->setGeometry(QRectF(effX, effY, txtW, btnH));
                btn->setFunctionID(id);
                if (action != VCButton::Toggle)
                    btn->setActionType(action);
                effX += txtW + pad;
                effCount++;
                row4Bottom = qMax(row4Bottom, effY + btnH);
            }
        };

        auto addEffect = [&](const QString &suffix)
        {
            for (quint32 id : m_generatedFunctionIDs)
            {
                Function *f = m_doc->function(id);
                if (f && f->name().contains(grp.name) && f->name().endsWith(suffix))
                {
                    addEffectBtn(suffix, id, VCButton::Toggle);
                    return;
                }
            }
        };

        if (grp.hasMovement)
        {
            addEffect(tr("Fly Out"));
            addEffect(tr("Fly In"));
            addEffect(tr("Circle Chase"));
            addEffect(tr("Figure Eight"));
            addEffect(tr("Audience Sweep"));
        }
        addEffect(tr("Strobe Chase"));
        addEffect(tr("Heartbeat"));

        // Blinder Toggle + Flash pair
        for (quint32 id : m_generatedFunctionIDs)
        {
            Function *f = m_doc->function(id);
            if (!f || !f->name().contains(grp.name) ||
                !f->name().endsWith(tr("Blinder Hit")))
                continue;
            addEffectBtn(tr("Blinder"),       id, VCButton::Toggle);
            addEffectBtn(tr("Blinder Flash"), id, VCButton::Flash);
            break;
        }

        return row4Bottom;   // bottom Y reached by this page's content
    };

    // ── Build all pages, tracking the tallest so the frame fits everything ────
    int maxBodyBottom = 0;

    // Page 0 = "All Groups", then one page per group.
    for (int pageIdx = 0; pageIdx < pageCount; pageIdx++)
    {
        int topBottom = 0;
        buildSharedTopRow(pageIdx, topBottom);

        const FixtureGroupEntry *grp =
            (pageIdx == 0) ? (m_hasAllGroups ? &m_allGroups : nullptr)
                           : pageGroups[pageIdx - 1];
        if (grp)
        {
            int bottom = buildBody(pageIdx, *grp, topBottom);
            maxBodyBottom = qMax(maxBodyBottom, bottom);
        }
        else
        {
            maxBodyBottom = qMax(maxBodyBottom, topBottom);
        }
    }

    // Cue row sits below the tallest page body; frame height fits it + padding.
    int cueY = maxBodyBottom + pad;
    int frmH = cueY + btnH + pad;
    frame->setGeometry(QRectF(pad, pad, frmW, frmH));

    // Now add the shared cue row on every page (its Y is known).
    for (int pageIdx = 0; pageIdx < pageCount; pageIdx++)
        buildSharedCueRow(pageIdx, cueY);

    frame->setCurrentPage(0);

    // Register the frame's freshly-added page-shortcut input sources into the
    // page's input-source map. Without this the loopback page switching only
    // starts working after a project reload (the loader does this scan), which
    // is why it "didn't work the first time". Doing it here fixes it live.
    page->mapChildrenInputSources();
}

QColor StageWizard::sceneColor(quint32 sceneID) const
{
    Scene *s = qobject_cast<Scene *>(m_doc->function(sceneID));
    if (!s) return QColor();

    int r = 0, g = 0, b = 0;
    bool any = false;

    // A palette scene stores no direct channel values; resolve via its palette.
    for (quint32 pid : s->palettes())
    {
        QLCPalette *p = m_doc->palette(pid);
        if (p && p->type() == QLCPalette::Color)
        {
            QColor c = p->rgbValue();
            if (c.isValid())
                return c;
        }
    }

    // Colour-wheel scenes set a value on a Colour-group channel: resolve the
    // capability the value falls in and use its ColorMacro resource colour.
    for (const SceneValue &sv : s->values())
    {
        Fixture *fx = m_doc->fixture(sv.fxi);
        if (!fx) continue;
        const QLCChannel *c = fx->channel(sv.channel);
        if (!c || c->group() != QLCChannel::Colour) continue;

        const QLCCapability *cap = c->searchCapability(sv.value);
        if (cap && cap->preset() == QLCCapability::ColorMacro)
        {
            QColor wc = cap->resource(0).value<QColor>();
            if (wc.isValid())
                return wc;
        }
    }

    // Fallback: derive from raw RGB/CMY channel values set by the scene.
    for (const SceneValue &sv : s->values())
    {
        Fixture *fx = m_doc->fixture(sv.fxi);
        if (!fx) continue;
        const QLCChannel *c = fx->channel(sv.channel);
        if (!c || c->group() != QLCChannel::Intensity) continue;
        switch (c->colour())
        {
            case QLCChannel::Red:     r = qMax(r, int(sv.value)); any = true; break;
            case QLCChannel::Green:   g = qMax(g, int(sv.value)); any = true; break;
            case QLCChannel::Blue:    b = qMax(b, int(sv.value)); any = true; break;
            case QLCChannel::Cyan:    r = qMax(r, 255 - int(sv.value)); any = true; break;
            case QLCChannel::Magenta: g = qMax(g, 255 - int(sv.value)); any = true; break;
            case QLCChannel::Yellow:  b = qMax(b, 255 - int(sv.value)); any = true; break;
            case QLCChannel::White:
                r = qMax(r, int(sv.value)); g = qMax(g, int(sv.value));
                b = qMax(b, int(sv.value)); any = true; break;
            default: break;
        }
    }

    return any ? QColor(r, g, b) : QColor();
}

bool StageWizard::ensureLoopbackPatch(int pageCount, quint32 &loopInUniverse,
                                      QList<quint32> &bridgeFixtureIDs)
{
    InputOutputMap *iomap = m_doc->inputOutputMap();
    if (!iomap || pageCount <= 0) return false;

    const QString loopName = QStringLiteral("Loopback");

    // Which universes already hold at least one fixture?
    QSet<quint32> usedUniverses;
    for (Fixture *fx : m_doc->fixtures())
        if (fx) usedUniverses.insert(fx->universe());

    // First truly empty universe — no fixtures AND no in/out patch — hosts the
    // page-switch dimmers and the loopback output. (A universe with an existing
    // patch, e.g. an ArtNet output or the loopback input, must not be reused.)
    // Also reserve outUni+1 for the loopback input, so don't pick the last one.
    quint32 outUni = InputOutputMap::invalidUniverse();
    quint32 uniCount = iomap->universesCount();
    for (quint32 u = 0; u < uniCount; u++)
    {
        if (!usedUniverses.contains(u) &&
            iomap->outputPatch(u) == nullptr &&
            iomap->inputPatch(u) == nullptr)
        {
            outUni = u;
            break;
        }
    }
    if (outUni == InputOutputMap::invalidUniverse())
    {
        iomap->addUniverse();
        outUni = iomap->universesCount() - 1;
    }

    // Patch the output side to loopback (unless it already is).
    OutputPatch *op = iomap->outputPatch(outUni);
    if (op == nullptr || op->pluginName() != loopName)
    {
        if (!iomap->setOutputPatch(outUni, loopName, QString(), QString(), 0))
            return false;
        op = iomap->outputPatch(outUni);
    }
    if (op)
        op->setPluginParameter(QStringLiteral("UniverseChannels"), pageCount);

    // Loopback INPUT sits on the output universe + 1 (create it if needed).
    quint32 inUni = outUni + 1;
    while (iomap->universesCount() <= inUni)
        iomap->addUniverse();

    InputPatch *ip = iomap->inputPatch(inUni);
    if (ip == nullptr || ip->pluginName() != loopName)
    {
        if (!iomap->setInputPatch(inUni, loopName, QString(), QString(), 0))
            return false;
    }

    loopInUniverse = inUni;

    // Add one generic 1-channel dimmer per page (selected groups + 1), at
    // consecutive addresses on the empty output universe.
    for (int i = 0; i < pageCount; i++)
    {
        Fixture *dimmer = new Fixture(m_doc);
        dimmer->setName(tr("Page Switch %1").arg(i + 1));
        dimmer->setUniverse(outUni);
        dimmer->setAddress(static_cast<quint32>(i));
        QLCFixtureDef *def  = dimmer->genericDimmerDef(1);
        QLCFixtureMode *mode = dimmer->genericDimmerMode(def, 1);
        dimmer->setFixtureDefinition(def, mode);

        if (!m_doc->addFixture(dimmer))
        {
            delete dimmer;
            return false;
        }
        bridgeFixtureIDs.append(dimmer->id());
    }

    // A universe appended at runtime (addUniverse) is created but NOT started —
    // its DMX writer thread never ticks, so the loopback output→input relay
    // stays silent until the project is reloaded (which starts all universes).
    // Start them now so page switching works on the first run. start() on an
    // already-running Universe (a QThread) is a harmless no-op.
    iomap->startUniverses();

    return true;
}

void StageWizard::generatePageSwitchScenes(
    const QList<quint32> &bridgeFixtureIDs,
    const QList<const FixtureGroupEntry *> &pageGroups,
    QList<quint32> &outIDs)
{
    const QString path = wizardPath(tr("Page Switch"));

    // One dimmer per page; each scene turns its dimmer full. The dimmer's
    // universe channel index (== page index) is what the frame shortcut reads.
    auto makeScene = [&](const QString &name, int pageIdx) -> quint32
    {
        if (pageIdx < 0 || pageIdx >= bridgeFixtureIDs.count())
            return Function::invalidId();
        Scene *s = new Scene(m_doc);
        s->setName(name);
        s->setValue(SceneValue(bridgeFixtureIDs.at(pageIdx), 0, 255));
        s->setPath(path);
        m_doc->addFunction(s);
        m_generatedFunctionIDs.append(s->id());
        return s->id();
    };

    // Page 0 = All Groups, then one page per group.
    outIDs.append(makeScene(tr("All Groups On"), 0));
    for (int i = 0; i < pageGroups.count(); i++)
        outIDs.append(makeScene(tr("%1 On").arg(pageGroups[i]->name), i + 1));
}
