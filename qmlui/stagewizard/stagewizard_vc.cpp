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

#include <QtMath>

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

// External control IDs of the widgets the wizard creates. These are #defined
// privately inside each widget's .cpp, so they're mirrored here.
static const quint8 kBtnPressureID  = 0;   // VCButton  – Pressure
static const quint8 kSliderLevelID  = 0;   // VCSlider  – Slider Control
static const quint8 kXYPadPanID     = 0;   // VCXYPad   – Pan / horizontal
static const quint8 kXYPadTiltID    = 2;   // VCXYPad   – Tilt / vertical

// Logical slot bases for the rows replicated on every frame page, so each
// replica of a given tab / cue reuses the same controller button.
static const int kSlotTabBase = 0;
static const int kSlotCueBase = 1000;
// Blackout lives in the top-right corner rather than the cue row, but it is
// still one of the shared buttons, so it takes a slot of its own past the cues.
static const int kSlotBlackout = 100;

// Layout metrics — pixelDensity units, matching the VC default drop sizes.
static const int kBtnW  = 17;   // all buttons: VC default square (pd * 17)
static const int kBtnH  = 17;
static const int kSlW   = 15;   // slider width (VC default)
static const int kHeaderH = 8;  // frame title-bar height (~listItemHeight in pd)
static const int kFrmPad = 4;
// Width the VC's right-hand editing panel takes off the visible page area,
// mirroring UISettings.sidePanelWidth (screenPixelDensity * scalingFactor * 50)
// plus a little for the page's vertical scroll bar.
static const int kSidePanelW = 54;

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

    // ── External controller mapping (step 5) ────────────────────────────────
    // Widgets ask for a channel BY ROLE, not in creation order. On a controller
    // whose profile describes a pad matrix (APC mini, Launchpad, APC40 …) each
    // role owns a band of grid rows, so the VC's structure — tab row on top,
    // colours, effects, cue row at the bottom — is reproduced on the pads. On
    // anything else the roles collapse back to a plain in-order hand-out.
    CtrlSurface ctrlSurface;
    const bool ctrlMap = m_ctrlMap && m_ctrlUniverse >= 0;
    if (ctrlMap)
    {
        buildControllerSurface(ctrlSurface);
        ensureFeedbackPatch();

        // Size the colour band from the BUSIEST page, so the effect row below it
        // sits at the same height on every page. Sizing it per page would move
        // the effects up and down as the user switches groups, which defeats the
        // point of a fixed layout.
        if (ctrlSurface.kind == CtrlSurfaceGrid)
        {
            int maxColors = 0;
            auto countColors = [&](const FixtureGroupEntry &grp)
            {
                int n = 0;
                for (quint32 id : m_generatedFunctionIDs)
                {
                    Function *f = m_doc->function(id);
                    if (f && f->type() == Function::SceneType &&
                        (f->path().contains(grp.name + "/" + tr("Colors")) ||
                         f->path().contains(grp.name + "/" + tr("Color Wheel"))))
                        n++;
                }
                maxColors = qMax(maxColors, n);
            };
            if (m_hasAllGroups)
                countColors(m_allGroups);
            for (const FixtureGroupEntry *grp : pageGroups)
                countColors(*grp);

            const int cols = qMax(1, ctrlSurface.grid.cols);
            assignControllerBands(ctrlSurface, pageCount,
                                  (maxColors + cols - 1) / cols);
        }
    }

    // Hand a button/fader channel to $widget's control $controlID. No-ops when
    // mapping is off or the controller has run out of channels, so a small
    // controller simply maps as far as it reaches.
    auto mapButton = [&](VCWidget *w, CtrlRole role, quint8 controlID,
                         const QColor &color = QColor())
    {
        quint32 ch = 0;
        if (ctrlMap && w && nextRoleChannel(ctrlSurface, role, ch))
            mapWidgetControl(w, controlID, ch, color);
    };
    auto mapFader = [&](VCWidget *w, quint8 controlID)
    {
        quint32 ch = 0;
        if (ctrlMap && w && nextFaderChannel(ctrlSurface, ch))
            mapWidgetControl(w, controlID, ch);
    };

    // The tab row and the cue row are replicated on every page, and an input
    // source only fires while its own page is showing — so each replica needs
    // its own source, all bound to the SAME controller button. $sharedButtons
    // remembers the channel picked for logical slot $slot on the first page and
    // replays it for the rest, so N pages don't consume N times the buttons.
    QHash<int, quint32> sharedButtons;

    // How far the shared tab / cue rows have claimed into the overflow pools.
    // Filled in once those rows are reserved (just before the page loop); the
    // per-page reset in buildBody() rewinds to here, never past it.
    int auxFloor    = 0;
    int buttonFloor = 0;
    auto mapSharedButton = [&](VCWidget *w, CtrlRole role, int slot)
    {
        if (!ctrlMap || !w)
            return;

        auto it = sharedButtons.constFind(slot);
        if (it != sharedButtons.constEnd())
        {
            mapWidgetControl(w, kBtnPressureID, it.value());
            return;
        }

        quint32 ch = 0;
        if (!nextRoleChannel(ctrlSurface, role, ch))
            return;
        sharedButtons.insert(slot, ch);
        mapWidgetControl(w, kBtnPressureID, ch);
    };

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

    // ── Frame width ──────────────────────────────────────────────────────────
    // Start from what the widest fixed row needs, then CLAMP to the page canvas
    // so the frame is always fully on screen. A VC page is a fixed-size canvas
    // (VCPage::resetProperties sets 1920x1080) that the view scales to fit the
    // window, so "fits on screen" means "fits the page", not the pixel size of
    // whatever display this happens to be running on.
    int xyPadW = btnH * 3 + pad * 2;                            // square XY pad
    int tabsW = pad + pageCount * (txtW + pad);                 // row 1: page tabs
    // Row 2 wants the fader plus the three swatch strips SIDE BY SIDE. Measure
    // what they actually come to on the busiest page — asking for a fixed three
    // strips of 8 made the frame wider than any page needed, which is what left
    // an empty band down the right-hand side. Each strip is laid out as a
    // roughly 2:1 block (see buildSwatchSolo), so its width follows from its
    // scene count; the clamp below caps the total, and buildSwatchSolo() then
    // reflows to whatever width it really got.
    auto stripWidthFor = [&](int count) -> int
    {
        if (count <= 0)
            return 0;
        int cols = qMax(1, int(qCeil(qSqrt(qreal(count) * 2.0))));
        cols = qMin(cols, count);
        // Mirrors buildSwatchSolo(): soloW = cols*btnW + (cols-1)*pad + pad*2,
        // then the cursor advances by soloW + pad for the next strip.
        return cols * (kBtnW * pd) + (cols - 1) * pad + pad * 2 + pad;
    };

    int widestStrips = 0;
    auto measureStrips = [&](const FixtureGroupEntry &grp)
    {
        int colors = 0, wheel = 0, gobos = 0;
        for (quint32 id : m_generatedFunctionIDs)
        {
            Function *f = m_doc->function(id);
            if (!f || f->type() != Function::SceneType)
                continue;
            if (f->path().contains(grp.name + "/" + tr("Colors")))            colors++;
            else if (f->path().contains(grp.name + "/" + tr("Color Wheel")))  wheel++;
            else if (f->path().contains(grp.name + "/" + tr("Gobos")))        gobos++;
        }
        widestStrips = qMax(widestStrips, stripWidthFor(colors) +
                                          stripWidthFor(wheel) +
                                          stripWidthFor(gobos));
    };
    if (m_hasAllGroups)
        measureStrips(m_allGroups);
    for (const FixtureGroupEntry *grp : pageGroups)
        measureStrips(*grp);

    int row2W = pad + kSlW * pd + pad + widestStrips;
    int row3W = pad + xyPadW + pad + 4 * (txtW + pad);          // XY pad + 4 effects
    int cueW  = pad + 3 * (txtW + pad);                         // 3 show cues

    // The frame sits at (pad, pad), so it can be at most the page width less
    // one pad on each side — minus the VC's right-hand editing panel, which
    // overlays the page area (VirtualConsole.qml sizes centerView as
    // parent.width - rightSidePanel.width). Reserving its EXPANDED width
    // (UISettings.sidePanelWidth = pd * 50) means the frame stays fully visible
    // with the panel open, which is exactly when the user is looking at the
    // layout the wizard just built. A scroll bar takes another few units.
    const int sidePanelW = kSidePanelW * pd;
    const int pageW = int(page->geometry().width());
    const int maxFrameW = qMax(txtW * 4, pageW - sidePanelW - pad * 2);

    int frmW = qMin(qMax(qMax(tabsW, row2W), qMax(row3W, cueW)) + pad, maxFrameW);

    // The usable content width (inside the left/right frame padding).
    const int usableRight = frmW - pad;

    // Content must start below the frame's title bar, else the first row is
    // drawn underneath it (children fill the whole frame, header overlays them).
    const int contentTop = kHeaderH * pd + pad;

    quint32 blinderFlashID = findFunc("Blinder Flash");

    // A wrapping row cursor: places a widget of ($w x $h) at the current spot,
    // wrapping to the next row when it would cross $limit, so every widget
    // stays fully inside the frame. Returns the placement point.
    struct Cursor { int x; int y; int rowH; };

    auto place = [&](Cursor &c, int w, int h, int limit) -> QPoint
    {
        if (c.x > pad && c.x + w > limit)  // doesn't fit → next row
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

    // The tab row stops short of the Blackout button parked in the corner, so a
    // row that wraps never runs underneath it.
    const int tabRowRight = usableRight - txtW - pad;

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
            // $slot is the tab's position in the row — stable across pages, so
            // every replica of tab N binds to the same controller button.
            auto addPageButton = [&](const QString &caption, quint32 sceneID,
                                     int slot)
            {
                QPoint p = place(c, txtW, btnH, tabRowRight);
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
                mapSharedButton(btn, CtrlRolePageTab, kSlotTabBase + slot);
            };
            addPageButton(tr("All Groups"),
                          pageSceneIDs.value(0, Function::invalidId()), 0);
            for (int i = 0; i < pageGroups.count(); i++)
                addPageButton(pageGroups[i]->name,
                              pageSceneIDs.value(i + 1, Function::invalidId()),
                              i + 1);
        }
        topOut = c.y + c.rowH + pad;   // Y below the (possibly wrapped) tab row

        // ── Blackout, pinned to the top-right corner ─────────────────────────
        // A Blackout-action button drives the engine's blackout directly, so it
        // needs no scene behind it — and unlike a scene it kills EVERYTHING,
        // which is what you want from the panic button. Parked in the corner,
        // clear of the tab row, so it is always in the same place and never
        // gets hit by accident while switching pages.
        {
            const int bx = usableRight - txtW;
            VCButton *btn = qobject_cast<VCButton *>(
                frame->addWidget(nullptr, "Button", QPoint(bx, contentTop)));
            if (btn)
            {
                btn->setCaption(tr("Blackout"));
                btn->setGeometry(QRectF(bx, contentTop, txtW, btnH));
                btn->setActionType(VCButton::Blackout);
                mapSharedButton(btn, CtrlRoleShowCue, kSlotCueBase + kSlotBlackout);
            }
        }
    };

    auto buildSharedCueRow = [&](int pageIdx, int cueY)
    {
        frame->setCurrentPage(pageIdx);
        Cursor c { pad, cueY, 0 };

        // $slot is fixed per cue (not a running counter), so a cue that is
        // absent on this project doesn't shift the others onto other buttons.
        auto addCue = [&](const QString &caption, quint32 funcID, bool flash,
                          int slot)
        {
            if (funcID == Function::invalidId())
                return;
            QPoint p = place(c, txtW, btnH, usableRight);
            VCButton *btn = qobject_cast<VCButton *>(
                frame->addWidget(nullptr, "Button", p));
            if (!btn) return;
            btn->setCaption(caption);
            btn->setGeometry(QRectF(p.x(), p.y(), txtW, btnH));
            btn->setFunctionID(funcID);
            if (flash)
                btn->setActionType(VCButton::Flash);
            mapSharedButton(btn, CtrlRoleShowCue, kSlotCueBase + slot);
        };

        // Blackout is NOT here — it sits in the top-right corner, built by
        // buildSharedTopRow() as a Blackout-action button with no function.
        addCue(tr("Ambient"), findFunc("Ambient Loop"), false, 0);
        addCue(tr("Blinder"), blinderFlashID,           true,  1);
    };

    // ── Group page body ──────────────────────────────────────────────────────
    // The page layout is fixed, so the same control is always in the same place
    // whichever group is showing:
    //   row 2 — Intensity fader (spans the row), then the swatch strips flowing
    //           left to right — colours, colour wheel, gobos (the gobo strip
    //           exists only on pages whose fixtures declare gobo macros) — and
    //           below them the shutter buttons (Open / Strobe Slow / Strobe
    //           Fast, + Lamp On when the fixtures can strike a lamp)
    //   row 3 — XY pad (spans the row) + Fly Out / Fly In / Centre / Circle
    // $bodyTop is the Y just below the shared top row. Returns the bottom Y.
    auto buildBody = [&](int pageIdx, const FixtureGroupEntry &grp, int bodyTop) -> int
    {
        frame->setCurrentPage(pageIdx);

        // Restart the per-page bands. Only one frame page is ever visible, and
        // an input source only fires for the page it is tagged with, so every
        // page may reuse the SAME pads: page 2's first colour belongs on the
        // same pad as page 1's, directly under that page's tab. Letting the
        // cursors run on across pages instead pushed each page's content further
        // down the grid until it spilled into aux and then ran out — the VC
        // layout, which is identical on every page, stopped matching the pads.
        // The shared tab and cue rows are NOT reset: they are one physical row
        // of buttons replicated on each page, allocated once via $sharedButtons.
        ctrlSurface.bands[CtrlRoleColor].nextCell  = 0;
        ctrlSurface.bands[CtrlRoleEffect].nextCell = 0;

        // The overflow pools have to restart too, or a page whose colours spill
        // out of their band would take different spill buttons than the next
        // page — reintroducing the very drift the band reset removes. $auxFloor
        // keeps the buttons already claimed by the shared tab/cue rows (they are
        // allocated before the first page and must not be handed out again).
        ctrlSurface.nextAux    = auxFloor;
        ctrlSurface.nextButton = buttonFloor;

        // Faders are per-page for the same reason: page N's Intensity should be
        // fader 1, not fader N.
        ctrlSurface.nextFader = 0;

        int btnW = kBtnW * pd;                  // square swatches

        // ── Row 2: dimmer slider + colour swatches + shutter buttons ─────────
        // The slider spans the whole row; colours and shutter controls stack to
        // its right, each in its own strip.
        int row2Bottom = bodyTop;               // grows as content is placed
        int colStartX  = pad;                   // content left edge, past the fader
        int soloY      = bodyTop;               // top of the next strip in the row
        {
            // Provisional height; the fader is stretched to the row's real
            // height once the strips beside it have been placed, so it always
            // spans the row exactly however many lines they end up taking.
            int slH = btnH * 2 + pad;
            int slW = 0;
            VCSlider *rowSlider = nullptr;

            if (grp.hasDimmer || grp.hasRGB || grp.hasCMY)
            {
                int w = kSlW * pd;
                VCSlider *slider = qobject_cast<VCSlider *>(
                    frame->addWidget(nullptr, "Slider", QPoint(pad, bodyTop)));
                if (slider)
                {
                    rowSlider = slider;
                    slW = w;
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
                    mapFader(slider, kSliderLevelID);
                    row2Bottom = bodyTop + slH;
                }
            }

            // ── Swatch strips, in solo frames ────────────────────────────────
            // Each kind is mutually exclusive within itself, so it lives in a
            // VCSoloFrame: activating one scene releases the others. The header
            // is hidden — these are inline swatch strips, not user-managed
            // containers — so children start at y=0 inside the frame (the header
            // is visible: toggled in VCFrameItem.qml, it does not reserve space
            // when off).
            //
            // SEPARATE solo frames per kind, because the kinds are not mutually
            // exclusive with each other: the palette colours drive RGB/CMY
            // mixing, the wheel colours are raw values on a colour wheel, and
            // the gobos are a different wheel again. A spot can legitimately
            // hold a wheel position AND an RGB wash AND a gobo; one shared solo
            // frame would make each cancel the others.
            //
            // The strips flow LEFT TO RIGHT across the row and only drop to a
            // new line when the next one would not fit — gobos sit beside the
            // colours rather than under them, which is what was making the page
            // so tall. $soloX/$soloY is the cursor, $lineH the tallest strip on
            // the current line.
            //
            // $shutterY / $shutterRight track where the shutter buttons may go:
            // directly under the COLOUR strips, not under the tallest strip on
            // the line. A tall gobo block beside a short colour block would
            // otherwise push the shutter row all the way down past it, leaving
            // a large hole under the colours.
            int soloX = colStartX;
            int lineH = 0;
            int shutterY = soloY;
            int shutterRight = usableRight;

            // Build one headerless solo frame holding every scene under
            // $subPath. Does nothing when no scene matched, so no empty frame is
            // left behind. $gobo switches the buttons to gobo styling: white
            // background with the GoboMacro picture on it.
            auto buildSwatchSolo = [&](const QString &subPath, const QString &caption,
                                       CtrlRole role, bool gobo) -> void
            {
                // Collect first: an empty solo frame must not be created at all.
                QList<quint32> ids;
                for (quint32 id : m_generatedFunctionIDs)
                {
                    Function *f = m_doc->function(id);
                    if (!f || f->type() != Function::SceneType)
                        continue;
                    if (f->path().contains(grp.name + "/" + subPath))
                        ids.append(id);
                }
                if (ids.isEmpty())
                    return;

                // How many swatches fit on one line of THIS strip, given what is
                // left of the row.
                auto fitsFrom = [&](int fromX)
                {
                    int availW = usableRight - fromX - pad * 2;  // frame padding
                    return (availW + pad) / (btnW + pad);
                };

                // Prefer a roughly square block over one long line: 15 colours
                // become 8x2, not 15x1. A single stretched-out row is both
                // harder to scan and eats the width the NEXT strip needs, which
                // is what was leaving a gap at the right-hand end of the row.
                const int wanted = qMax(1, int(qCeil(qSqrt(qreal(ids.count()) * 2.0))));

                if (soloX > colStartX && fitsFrom(soloX) < qMin(4, ids.count()))
                {
                    soloX  = colStartX;               // next line
                    soloY += lineH + pad;
                    lineH  = 0;
                }

                const int cols = qBound(1, qMin(fitsFrom(soloX), wanted),
                                        ids.count());
                const int rows = (ids.count() + cols - 1) / cols;
                const int soloW = cols * btnW + (cols - 1) * pad + pad * 2;
                const int soloH = rows * btnH + (rows - 1) * pad + pad * 2;

                // "Solo frame" — lowercase 'f'. This is the key VCWidget::
                // stringToType() expects; VCWidget::typeToString() returns the
                // translated DISPLAY string ("Solo Frame"), which does NOT round
                // trip and silently yields UnknownWidget -> addWidget() == null.
                VCFrame *solo = qobject_cast<VCFrame *>(
                    frame->addWidget(nullptr, "Solo frame", QPoint(soloX, soloY)));
                if (!solo)
                    return;

                solo->setCaption(caption);
                solo->setShowHeader(false);
                solo->setGeometry(QRectF(soloX, soloY, soloW, soloH));

                // No setPage() needed: buildBody() already did
                // frame->setCurrentPage(pageIdx), and VCFrame::addWidget() stamps
                // a new child with the parent's currentPage() and registers it in
                // m_pagesMap under that same value — so the solo frame is already
                // on the right page. (setupWidget() is protected, so re-registering
                // from here isn't possible anyway.)

                for (int i = 0; i < ids.count(); i++)
                {
                    // Positions are relative to the solo frame, not the page.
                    int bx = pad + (i % cols) * (btnW + pad);
                    int by = pad + (i / cols) * (btnH + pad);

                    VCButton *btn = qobject_cast<VCButton *>(
                        solo->addWidget(nullptr, "Button", QPoint(bx, by)));
                    if (!btn) continue;

                    // NOTE: deliberately NOT setPage(pageIdx) here. The button's
                    // page is its page WITHIN the solo frame, which is single
                    // page, so it must stay 0 — setting it to pageIdx would make
                    // the solo frame's own setCurrentPage(0) hide it. Per-page
                    // visibility is already handled one level up, by the solo
                    // frame sitting on page pageIdx of the master frame.
                    // VCPage::inputValueChanged() matches source->page() against
                    // widget->page() and separately checks isEffectivelyVisible()
                    // (which walks the parent chain), so leaving both at 0 is
                    // both correct and what makes the mapping fire only while
                    // the owning page is shown.

                    Function *f = m_doc->function(ids.at(i));
                    QString cap = f ? f->name() : QString();
                    int dash = cap.indexOf(QStringLiteral(" – "));
                    if (dash >= 0) cap = cap.mid(dash + 3);
                    btn->setCaption(cap);
                    btn->setGeometry(QRectF(bx, by, btnW, btnH));
                    btn->setFunctionID(ids.at(i));
                    btn->setActionType(VCButton::Toggle);

                    QColor col;
                    if (gobo)
                    {
                        // Gobo pictures are dark line art on transparency, so
                        // they need a white ground to read at all.
                        btn->setBackgroundColor(Qt::white);
                        btn->setForegroundColor(Qt::black);

                        QString pic = goboPicture(ids.at(i));
                        if (!pic.isEmpty())
                        {
                            btn->setBackgroundImage(pic);
                            // The picture IS the label; the capability name on
                            // top of it would just obscure the pattern.
                            btn->setCaption(QString());
                        }
                    }
                    else
                    {
                        col = sceneColor(ids.at(i));
                        if (col.isValid())
                        {
                            btn->setBackgroundColor(col);
                            btn->setForegroundColor(contrastingTextColor(col));
                        }
                    }

                    // Pass the swatch colour so the controller's LED lights up in
                    // the same colour when the scene is active.
                    mapButton(btn, role, kBtnPressureID, col);
                }

                // The shutter row goes under the colour strips and to the LEFT
                // of anything taller sitting beside them (the gobo block).
                // Colours push the row down and widen how far it may run; a
                // gobo strip ON THE SAME LINE caps it at its own left edge so
                // the two never overlap. A gobo strip that wrapped to its own
                // line sits below everything and imposes no such limit.
                if (gobo && soloX > colStartX)
                    shutterRight = qMin(shutterRight, soloX - pad);  // beside
                else
                    shutterY = qMax(shutterY, soloY + soloH + pad);  // above

                soloX += soloW + pad;
                lineH  = qMax(lineH, soloH);
                row2Bottom = qMax(row2Bottom, soloY + soloH);
            };

            // Generic RGB/CMY palette colours, this group's colour wheel, then
            // its gobos — all on the same line while there is room for them.
            buildSwatchSolo(tr("Colors"), tr("Colors"), CtrlRoleColor, false);
            buildSwatchSolo(tr("Color Wheel"), tr("Color Wheel"), CtrlRoleColor, false);
            buildSwatchSolo(tr("Gobos"), tr("Gobos"), CtrlRoleEffect, true);

            // The shutter buttons tuck in directly under the colour swatches,
            // beside whatever taller strip shares the line with them.
            soloX = colStartX;
            soloY = shutterY;

            // ── Shutter strip: Open / Strobe Slow / Strobe Fast (+ Lamp On) ──
            // Matched on the FULL scene name, not a suffix: a suffix match would
            // also hit another group's scene whose name happens to contain this
            // group's name (e.g. "Spots" inside "Back Spots").
            auto addShutterBtn = [&](const QString &target, const QString &caption)
            {
                for (quint32 id : m_generatedFunctionIDs)
                {
                    Function *f = m_doc->function(id);
                    if (!f || f->type() != Function::SceneType || f->name() != target)
                        continue;

                    // Wrap rather than run under the strip on the right.
                    if (soloX > colStartX && soloX + btnW > shutterRight)
                    {
                        soloX  = colStartX;
                        soloY += btnH + pad;
                    }
                    VCButton *btn = qobject_cast<VCButton *>(
                        frame->addWidget(nullptr, "Button", QPoint(soloX, soloY)));
                    if (!btn)
                        return;
                    btn->setCaption(caption);
                    btn->setGeometry(QRectF(soloX, soloY, btnW, btnH));
                    btn->setFunctionID(id);
                    btn->setActionType(VCButton::Toggle);
                    mapButton(btn, CtrlRoleEffect, kBtnPressureID);
                    soloX += btnW + pad;
                    row2Bottom = qMax(row2Bottom, soloY + btnH);
                    return;
                }
            };

            if (grp.hasShutter)
            {
                // $target is built from the SAME translatable strings
                // generateGroupPalettes() used — a "%1 – %2" template plus a
                // separate label would diverge in a translated build and the
                // buttons would silently disappear.
                addShutterBtn(tr("%1 – Shutter Open").arg(grp.name), tr("Open"));
                addShutterBtn(tr("%1 – Strobe Slow").arg(grp.name), tr("Strobe Slow"));
                addShutterBtn(tr("%1 – Strobe Fast").arg(grp.name), tr("Strobe Fast"));
            }
            // Only groups whose fixtures declare a LampOn capability got a
            // "Lamp On" scene, so this button appears on those pages only.
            addShutterBtn(tr("%1 – Lamp On").arg(grp.name), tr("Lamp On"));

            // Stretch the fader to whatever height the row actually came to.
            if (rowSlider)
            {
                slH = qMax(slH, row2Bottom - bodyTop);
                rowSlider->setGeometry(QRectF(pad, bodyTop, slW, slH));
                row2Bottom = qMax(row2Bottom, bodyTop + slH);
            }
        }

        // ── Row 3: XY pad + effect buttons ───────────────────────────────────
        int row3Top = row2Bottom + pad;
        int row3Bottom = row3Top;

        // XY Pad (moving heads only) — leads the row, spanning three button rows
        // so it's large enough to be usable (square).
        int effStartX = pad;
        if (grp.hasMovement)
        {
            int h = btnH * 3 + pad * 2;   // three button rows tall
            int w = h;                    // square pad
            VCXYPad *xy = qobject_cast<VCXYPad *>(
                frame->addWidget(nullptr, "XYPad", QPoint(pad, row3Top)));
            if (xy)
            {
                xy->setCaption(tr("Position"));
                xy->setGeometry(QRectF(pad, row3Top, w, h));
                // addHead() is what actually puts a head under the pad's control
                // (appends to m_fixtures + computes its pan/tilt range).
                // addFixtureGroupHeadPreset() only creates a preset BUTTON for
                // heads the pad already drives — it runs the head list through
                // uniqueHeadsInPad() and bails out when the pad is still empty,
                // so calling it here left the pad with no heads and no presets.
                for (quint32 fxID : grp.fixtureIDs)
                {
                    Fixture *fx = m_doc->fixture(fxID);
                    if (!fx) continue;
                    // Only pan/tilt-capable fixtures belong on an XY pad; the
                    // group may also hold static units (grp.hasMovement is true
                    // as soon as ONE member moves).
                    if (fx->channelNumber(QLCChannel::Pan, QLCChannel::MSB) == QLCChannel::invalid() &&
                        fx->channelNumber(QLCChannel::Tilt, QLCChannel::MSB) == QLCChannel::invalid())
                        continue;
                    for (int h2 = 0; h2 < qMax(1, fx->heads()); ++h2)
                        xy->addHead(static_cast<int>(fxID), h2);
                }
                effStartX = pad + w + pad;   // effects start right of the XY pad
                row3Bottom = qMax(row3Bottom, row3Top + h);

                // Two faders/encoders drive the pad's two axes.
                mapFader(xy, kXYPadPanID);
                mapFader(xy, kXYPadTiltID);
            }
        }

        // Effect + blinder buttons: fixed size, max 4 per row, beside the XY pad.
        const int effPerRow = 4;
        int effCount = 0, effX = effStartX, effY = row3Top;

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
                // No colour: effect buttons keep the VC default background, so
                // there is nothing meaningful to translate to an LED colour —
                // the profile's own on/off feedback values are used instead.
                mapButton(btn, CtrlRoleEffect, kBtnPressureID);
                effX += txtW + pad;
                effCount++;
                row3Bottom = qMax(row3Bottom, effY + btnH);
            }
        };

        // Match on the FULL function name rather than a suffix: a suffix match
        // would also hit another group's function whose name happens to contain
        // this group's name (e.g. "Spots" inside "Back Spots").
        auto addEffect = [&](const QString &target, const QString &caption)
        {
            for (quint32 id : m_generatedFunctionIDs)
            {
                Function *f = m_doc->function(id);
                if (f && f->name() == target)
                {
                    addEffectBtn(caption, id, VCButton::Toggle);
                    return;
                }
            }
        };

        // The four movement controls that go beside the pad. "Circle" is the
        // Collection generateMovementEffect() built (base Centre position + the
        // 10 s relative circle EFX riding on it), not the bare EFX — running the
        // EFX alone would spin around wherever the heads happen to point.
        if (grp.hasMovement)
        {
            // Each target is rebuilt from the SAME translatable template the
            // generator used, so the two can't drift apart in a translated
            // build and leave the button silently missing.
            addEffect(tr("%1 – %2").arg(grp.name).arg(tr("Fly Out")),
                      tr("Fly Out"));
            addEffect(tr("%1 – %2").arg(grp.name).arg(tr("Fly In")),
                      tr("Fly In"));
            addEffect(tr("%1 – Position %2").arg(grp.name).arg(tr("Centre")),
                      tr("Centre"));
            addEffect(tr("%1 – %2").arg(grp.name).arg(tr("Circle Chase")),
                      tr("Circle"));
        }

        return row3Bottom;   // bottom Y reached by this page's content
    };

    // ── Reserve the shared rows before any page body ─────────────────────────
    // The tab and cue rows are one physical row of buttons each, replicated on
    // every page. Their channels must be claimed BEFORE the per-page content
    // starts resetting its cursors, otherwise (in linear mode, where there are
    // no bands to keep them apart) the cue row — which is built last, after
    // every page — would be handed buttons the page bodies already took.
    // Reserving here costs nothing in grid mode, where the bands already
    // separate them.
    if (ctrlMap)
    {
        for (int slot = 0; slot < pageCount; slot++)
        {
            quint32 ch = 0;
            if (nextRoleChannel(ctrlSurface, CtrlRolePageTab, ch))
                sharedButtons.insert(kSlotTabBase + slot, ch);
        }
        for (int slot = 0; slot < 2; slot++)   // Ambient, Blinder
        {
            quint32 ch = 0;
            if (nextRoleChannel(ctrlSurface, CtrlRoleShowCue, ch))
                sharedButtons.insert(kSlotCueBase + slot, ch);
        }
        // Blackout: same role, but it lives in the top-right corner instead of
        // the cue row, so it gets its own slot rather than a cue index.
        {
            quint32 ch = 0;
            if (nextRoleChannel(ctrlSurface, CtrlRoleShowCue, ch))
                sharedButtons.insert(kSlotCueBase + kSlotBlackout, ch);
        }
    }

    // Everything claimed above is off-limits to the per-page reset below.
    auxFloor    = ctrlSurface.nextAux;
    buttonFloor = ctrlSurface.nextButton;

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

    // Push the initial state out to the controller, so its LEDs show the (all
    // off) VC state right away instead of staying dark until the first press.
    if (ctrlMap && m_ctrlFeedback)
    {
        for (VCWidget *w : page->children(true))
            if (w)
                w->updateFeedback();
    }
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

QString StageWizard::goboPicture(quint32 sceneID) const
{
    Scene *s = qobject_cast<Scene *>(m_doc->function(sceneID));
    if (!s) return QString();

    // A gobo scene sets one value on a Gobo-group channel; resolve the
    // capability that value falls in and take its picture. The resource is
    // already an absolute path — QLCCapability::loadXML() expands the relative
    // form against the system gobo directory when the definition is read.
    for (const SceneValue &sv : s->values())
    {
        Fixture *fx = m_doc->fixture(sv.fxi);
        if (!fx) continue;
        const QLCChannel *c = fx->channel(sv.channel);
        if (!c || c->group() != QLCChannel::Gobo) continue;

        const QLCCapability *cap = c->searchCapability(sv.value);
        if (cap && cap->preset() == QLCCapability::GoboMacro)
        {
            QString path = cap->resource(0).toString();
            if (!path.isEmpty())
                return path;
        }
    }

    return QString();
}

QColor StageWizard::contrastingTextColor(const QColor &background)
{
    if (!background.isValid())
        return Qt::white;

    // Relative luminance (WCAG): linearise each sRGB component, then weight.
    auto linear = [](int c)
    {
        qreal v = c / 255.0;
        return v <= 0.03928 ? v / 12.92 : qPow((v + 0.055) / 1.055, 2.4);
    };

    qreal lum = 0.2126 * linear(background.red()) +
                0.7152 * linear(background.green()) +
                0.0722 * linear(background.blue());

    // Contrast ratio against white is (1.05 / (lum + 0.05)), against black
    // ((lum + 0.05) / 0.05). They cross at lum = sqrt(1.05 * 0.05) - 0.05.
    return lum > 0.1791 ? QColor(Qt::black) : QColor(Qt::white);
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

        // These dimmers are pure plumbing for the VC page switching — they have
        // no physical counterpart on the rig. Hide them from the 2D/3D preview,
        // else they show up as unexplained extra fixtures parked at the stage
        // origin (they were being mistaken for real PARs).
        m_doc->monitorProperties()->setFixtureFlags(
            dimmer->id(), 0, 0, MonitorProperties::HiddenFlag);

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
