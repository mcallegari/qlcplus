/*
  Q Light Controller Plus
  stagewizard.h

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

#ifndef STAGEWIZARD_H
#define STAGEWIZARD_H

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QVector3D>
#include <QColor>

class Doc;
class Fixture;
class Chaser;
class Scene;
class EFX;
class RGBMatrix;
class FixtureGroup;
class VirtualConsole;
class VCPage;
class ContextManager;
class FixtureManager;
class FunctionManager;
class MainView3D;

class StageWizard : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(StageWizard)

    // ── Navigation ──────────────────────────────────────────────────────────
    Q_PROPERTY(int currentStep READ currentStep WRITE setCurrentStep NOTIFY currentStepChanged)
    Q_PROPERTY(int totalSteps  READ totalSteps  CONSTANT)

    // ── Step 1 – Show type ───────────────────────────────────────────────────
    Q_PROPERTY(int showType READ showType WRITE setShowType NOTIFY showTypeChanged)

    // ── Step 2 – Fixture groups / roles ──────────────────────────────────────
    Q_PROPERTY(QVariant fixtureRoleModel   READ fixtureRoleModel   NOTIFY fixtureRoleModelChanged)
    Q_PROPERTY(QVariant groupsModel        READ groupsModel        NOTIFY groupsModelChanged)

    // ── Step 3 – Venue / Stage ────────────────────────────────────────────────
    Q_PROPERTY(int stageType READ stageType WRITE setStageType NOTIFY stageTypeChanged)
    // Environment size in metres (width, height, depth)
    Q_PROPERTY(qreal envWidth  READ envWidth  WRITE setEnvWidth  NOTIFY envSizeChanged)
    Q_PROPERTY(qreal envHeight READ envHeight WRITE setEnvHeight NOTIFY envSizeChanged)
    Q_PROPERTY(qreal envDepth  READ envDepth  WRITE setEnvDepth  NOTIFY envSizeChanged)

    // ── Step 4 – Effects ─────────────────────────────────────────────────────
    Q_PROPERTY(QVariant effectsModel READ effectsModel NOTIFY effectsModelChanged)

    // ── State ────────────────────────────────────────────────────────────────
    Q_PROPERTY(bool canGoNext  READ canGoNext  NOTIFY canGoNextChanged)
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(QVariant summaryModel READ summaryModel NOTIFY summaryModelChanged)

public:
    // Show type – drives stage selection and effect defaults
    enum ShowType
    {
        ClubNight     = 0,
        Concert       = 1,
        Theatrical    = 2,
        Architectural = 3,
        Custom        = 4
    };
    Q_ENUM(ShowType)

    // Fixture role – determines 3D placement and which VC widgets are created
    enum FixtureRole
    {
        RoleKey      = 0,  ///< Front/top wash, key illumination
        RoleFill     = 1,  ///< Fill/side supplemental wash
        RoleBack     = 2,  ///< Rear backlight / uplighter
        RoleSide     = 3,  ///< Boom / side light (theatre wings)
        RoleEffect   = 4,  ///< Aerial effect / mid-air beam
        RoleStrip    = 5,  ///< LED strip / batten across the rig
        RoleBlinder  = 6,  ///< Audience blinder / strobe
        RoleHazer    = 7,  ///< Hazer / fogger
        RoleFloor    = 8   ///< Floor up-lighter
    };
    Q_ENUM(FixtureRole)

    // Effect family flags – shown as checkboxes in Step 4
    enum EffectFlag
    {
        EffectColorPalette  = 1 << 0,
        EffectGoboPalette   = 1 << 1,
        EffectShutter       = 1 << 2,
        EffectPositionPreset= 1 << 3,
        EffectFlyOut        = 1 << 4,
        EffectFlyIn         = 1 << 5,
        EffectCircleChase   = 1 << 6,
        EffectFigureEight   = 1 << 7,
        EffectAudienceSweep = 1 << 8,
        EffectColorRainbow  = 1 << 9,
        EffectSplitColor    = 1 << 10,
        EffectBlinderHit    = 1 << 11,
        EffectStrobeChase   = 1 << 12,
        EffectHeartbeat     = 1 << 13,
        EffectPixelChase    = 1 << 14,
        EffectWave          = 1 << 15,
        EffectFireworks     = 1 << 16,
        EffectPlasma        = 1 << 17,
        EffectMarquee       = 1 << 18,
        EffectShowOpen      = 1 << 19,
        EffectShowClose     = 1 << 20,
        EffectBigMoment     = 1 << 21,
        EffectAmbientLoop   = 1 << 22
    };
    Q_ENUM(EffectFlag)
    Q_DECLARE_FLAGS(EffectFlags, EffectFlag)
    Q_FLAG(EffectFlags)

public:
    explicit StageWizard(Doc *doc,
                         FixtureManager *fixtureManager,
                         FunctionManager *functionManager,
                         VirtualConsole *virtualConsole,
                         ContextManager  *contextManager,
                         QObject *parent = nullptr);
    ~StageWizard() override;

    // ── Navigation ──────────────────────────────────────────────────────────
    int currentStep() const;
    void setCurrentStep(int step);
    int totalSteps() const { return 6; }

    // ── Step 1 ──────────────────────────────────────────────────────────────
    int showType() const;
    void setShowType(int type);

    // ── Step 2 ──────────────────────────────────────────────────────────────
    QVariant fixtureRoleModel() const;

    /** List of user-defined group boxes: { index, name, fixtureCount, selected }. */
    QVariant groupsModel() const;

    /** Create a new empty group box (named "Group N"). Returns its index. */
    Q_INVOKABLE int  addGroup();

    /** Remove the group box at $groupIndex. */
    Q_INVOKABLE void removeGroup(int groupIndex);

    /** Rename the group box at $groupIndex. */
    Q_INVOKABLE void renameGroup(int groupIndex, const QString &name);

    /** Toggle the checkbox of a group box. Selected boxes drive column 3. */
    Q_INVOKABLE void setGroupSelected(int groupIndex, bool selected);

    /** Selection state of a group box. Bound live in column 2 so toggling the
     *  checkbox doesn't rebuild the groups model (which would reset scroll). */
    Q_INVOKABLE bool isGroupSelected(int groupIndex) const;

    /** Assign an already-patched fixture to the group box at $groupIndex. */
    Q_INVOKABLE void assignFixtureToGroup(int groupIndex, quint32 fixtureID);

    /** Remove a fixture from the group box at $groupIndex. */
    Q_INVOKABLE void removeFixtureFromGroup(int groupIndex, quint32 fixtureID);

    /** Fixtures inside the group box at $groupIndex, as a list of
     *  { id, name, address, universe } maps (address/universe 1-based). */
    Q_INVOKABLE QVariant groupFixtures(int groupIndex) const;

    /** Arm a drop target: the next fixture patched via the browser drag is
     *  auto-assigned to this group box. Pass -1 to disarm. */
    Q_INVOKABLE void setPendingDropGroup(int groupIndex);

    /** Change the role of a fixture group identified by its list index. */
    Q_INVOKABLE void setGroupRole(int groupIndex, int role);

    /** Suggest roles based on detected capabilities (called automatically on step entry). */
    Q_INVOKABLE void autoDetectRoles();

    /** True if any selected box is a NEW (wizard-created) group, i.e. the wizard
     *  will place fixtures in 3D. When false, the Venue & Stage step is skipped
     *  and generation leaves stage type, environment size and fixture positions
     *  untouched (functions/VC only). */
    Q_INVOKABLE bool hasNewGroups() const;

    // ── Step 3 ──────────────────────────────────────────────────────────────
    int stageType() const;
    void setStageType(int type);

    qreal envWidth() const  { return m_envSize.x(); }
    qreal envHeight() const { return m_envSize.y(); }
    qreal envDepth() const  { return m_envSize.z(); }
    void setEnvWidth(qreal w);
    void setEnvHeight(qreal h);
    void setEnvDepth(qreal d);

    /** Suggest an environment size (metres) from the current fixture count. */
    void suggestEnvSize();

    // ── Step 4 ──────────────────────────────────────────────────────────────
    QVariant effectsModel() const;

    /** Toggle an individual effect on/off. */
    Q_INVOKABLE void setEffectEnabled(int effectFlag, bool enabled);

    /** Enable/disable every available effect of a family in one shot (emits the
     *  model-changed signal once). If $enabled is not given, toggles: turns all
     *  on when any is currently off, otherwise turns all off. */
    Q_INVOKABLE void setFamilyEffectsEnabled(const QString &family);

    /** Returns a human-readable preview string, e.g. "3 scenes + 1 chaser". */
    Q_INVOKABLE QString effectPreview(int effectFlag) const;

    // ── State ────────────────────────────────────────────────────────────────
    bool canGoNext() const;
    bool isGenerating() const;
    QVariant summaryModel() const;

    /** Reset wizard to initial state (called on cancel or re-open). */
    Q_INVOKABLE void reset();

    /** Run the full generation pipeline. Emits generationComplete() when done. */
    Q_INVOKABLE void generate();

signals:
    void currentStepChanged(int step);
    void showTypeChanged(int type);
    void fixtureRoleModelChanged();
    void groupsModelChanged();
    void groupSelectionChanged();
    void stageTypeChanged(int type);
    void envSizeChanged();
    void effectsModelChanged();
    void canGoNextChanged();
    void isGeneratingChanged();
    void summaryModelChanged();
    void generationComplete();

private:
    // ── Internal helpers ─────────────────────────────────────────────────────

    // Anchor points a relative movement EFX is centred on.
    enum MovementAnchor { AnchorCentre, AnchorAerial, AnchorAudience };

    struct FixtureGroupEntry
    {
        quint32     groupId;        ///< Existing Doc FixtureGroup id, or invalidId() for new boxes
        QString     name;           ///< Display name, e.g. "Group 1"
        QList<quint32> fixtureIDs;  ///< IDs of fixtures in this box
        bool        selected;       ///< Checkbox state (selected boxes drive column 3)
        FixtureRole role;           ///< Assigned role
        bool        roleUserSet;    ///< True once the user picked the role manually
        bool        hasMovement;
        bool        hasRGB;
        bool        hasCMY;
        bool        hasColorWheel;  ///< Has a colour-wheel channel (QLCChannel::Colour)
        bool        hasGobo;
        bool        hasShutter;
        bool        hasDimmer;
    };

    struct EffectEntry
    {
        int         flag;           ///< EffectFlag value
        QString     name;           ///< Display name
        QString     family;         ///< "Movement", "Color", "Intensity", "Matrix", "Show Cues"
        bool        enabled;
        bool        available;      ///< False when no compatible fixtures found
    };

    // Step 2 helpers
    void loadExistingGroups();              ///< Populate boxes from existing Doc FixtureGroups
    void detectGroupCapabilities(FixtureGroupEntry &e) const;
    void rebuildRoleModel();                ///< Recompute roles/caps for selected boxes
    QString capabilityGroupName(const Fixture *fx) const;

    int  m_pendingDropGroup = -1;           ///< Box index armed for the next browser drop
    int  m_groupCounter = 0;                ///< For default "Group N" naming
    QList<quint32> m_droppedFixtureIDs;     ///< IDs patched via drag, pending box assignment

private slots:
    void slotFixtureAdded(quint32 fixtureID);

    // Step 3 – placement
    void applyStageLayout();
    QVector3D computePosition(int index, int total, FixtureRole role,
                              const QVector3D &envSize,
                              const QVector3D &fxSizeMM) const;
    QVector3D computeRotation(FixtureRole role, int index, int total) const;

    // Step 4 defaults
    void applyShowTypeDefaults();
    void buildEffectsModel();

    // Generation sub-steps
    void createFixtureGroups();         ///< Persist selected boxes as Doc FixtureGroups
    void generateColorPalette(const FixtureGroupEntry &grp, const QString &prefix);
    void generateGoboPalette(const FixtureGroupEntry &grp, const QString &prefix);
    void generateShutterEffects(const FixtureGroupEntry &grp, const QString &prefix);
    void generatePositionPresets(const FixtureGroupEntry &grp, const QString &prefix);

    /** Position scene aiming a group's heads at a stage anchor (created once per
     *  group+anchor). Returns its function id. */
    quint32 baseMovementPosition(const FixtureGroupEntry &grp, MovementAnchor anchor);

    /** Relative movement EFX (Circle/Eight/…): builds the EFX and wraps it with a
     *  base Position scene in a Collection so it starts from a defined aim. */
    void generateMovementEffect(const FixtureGroupEntry &grp, const QString &prefix,
                                int algorithm, const QString &label, MovementAnchor anchor);

    void generateEFX(const FixtureGroupEntry &grp, const QString &prefix,
                     int algorithm, const QString &label);
    void generateRGBMatrix(const FixtureGroupEntry &grp, const QString &scriptName,
                           const QString &label);
    Chaser *generateStrobeChase(const FixtureGroupEntry &grp, const QString &prefix);
    Chaser *generateHeartbeat(const FixtureGroupEntry &grp, const QString &prefix);
    Chaser *generateColorRainbow(const FixtureGroupEntry &grp, const QString &prefix);
    Chaser *generateSplitColor(const FixtureGroupEntry &grp, const QString &prefix);
    Scene  *generateBlinderHit(const FixtureGroupEntry &grp, const QString &prefix);
    /** Show-level scene: all blinder-role fixtures at full, 300 ms fade-out, for
     *  a momentary Flash button. Returns nullptr when there are no blinders. */
    Scene  *generateBlinderFlash();
    Chaser *generateShowOpen();
    Chaser *generateShowClose();
    Chaser *generateBigMoment();
    Chaser *generateAmbientLoop();

    // ── Palette-based generation ────────────────────────────────────────────
    /** Find an existing Doc palette of $type whose values match, or invalidId(). */
    quint32 findPalette(int type, const QVariantList &values) const;

    /** Create a Color palette (rgb) owned by the Doc. Returns its id. */
    quint32 makeColorPalette(const QString &name, const QColor &color);
    /** Create a Dimmer palette (0-255). */
    quint32 makeDimmerPalette(const QString &name, int value);
    /** Create a Shutter palette from a capability preset (+ optional percent). */
    quint32 makeShutterPalette(const QString &name, int preset, int percent = 100);
    /** Create a Position3D palette aiming at a target point (mm, monitor frame). */
    quint32 makePosition3DPalette(const QString &name, const QVector3D &targetMM);

    /** Build a Scene that references a group + one palette. Adds it to the Doc,
     *  optionally under a function-tree $path. */
    Scene *makePaletteScene(const QString &name, quint32 groupID, quint32 paletteID,
                            const QString &path = QString());

    /** Root of the wizard's function-tree folder. */
    QString wizardPath(const QString &sub = QString()) const;

    /** Create the standard palette set (colour/dimmer/shutter) + palette scenes
     *  for a group, using its persisted FixtureGroup id. */
    void generateGroupPalettes(const FixtureGroupEntry &grp);

    /** Full function set for one group: palettes, musician key scenes and all
     *  enabled per-group effects (pathed under the group's Effects folder).
     *  Runs for each selected group and for the "All Groups" aggregate. */
    void generateGroupFunctions(const FixtureGroupEntry &grp);

    /** Count of moving heads (Pan/Tilt fixtures, per head) in a group. */
    int movingHeadCount(const FixtureGroupEntry &grp) const;

    /** Moving-head fixture IDs of a group paired with their placed X (mm). */
    QList<QPair<quint32, float>> movingHeadsByX(const FixtureGroupEntry &grp) const;

    /** Representative pan/tilt max travel (degrees) of a group's first moving
     *  head. Falls back to 540/270 when the physical data is missing. */
    void groupPanTiltMax(const FixtureGroupEntry &grp, float &panDeg, float &tiltDeg) const;

    /** EFX Width/Height (0-127) that yields $degrees of total pan/tilt swing on a
     *  head whose full range is $maxDeg. */
    int efxSizeForDegrees(float degrees, float maxDeg) const;

    /** For a front Key group: musician key-light scenes with Position3D palettes.
     *  Spot count = frontMovingHeads / 2 (key+fill); a few pushed upstage when
     *  Back-role moving heads are present for depth. */
    void generateMusicianKeyScenes(const FixtureGroupEntry &grp);

    // VC layout
    void createVCLayout();
    /** Return the first empty VC page; if all pages have widgets, add and
     *  return a new one. */
    VCPage *pickTargetPage();

    // ── Multipage master frame: loopback page switching ─────────────────────
    /** Set up page switching: find the first universe with no fixtures, patch it
     *  to loopback OUTPUT (input on output+1), and add $pageCount generic
     *  1-channel dimmers there — one per page. Fills $loopInUniverse and appends
     *  the dimmer fixture ids to $bridgeFixtureIDs (page order). Returns false if
     *  the loopback plugin is unavailable or patching fails. */
    bool ensureLoopbackPatch(int pageCount, quint32 &loopInUniverse,
                             QList<quint32> &bridgeFixtureIDs);

    /** Generate the page-switch scenes, one per dimmer in $bridgeFixtureIDs.
     *  Index 0 = "All Groups", 1..N = each selected group, in the same order
     *  createVCLayout() assigns pages. Appends their ids to $outIDs. */
    void generatePageSwitchScenes(const QList<quint32> &bridgeFixtureIDs,
                                  const QList<const FixtureGroupEntry *> &pageGroups,
                                  QList<quint32> &outIDs);

    /** Representative RGB of a wizard color scene, for a color button's
     *  background. Reads the scene's actual channel values. */
    QColor sceneColor(quint32 sceneID) const;

    // Helpers
    Scene *makeBlackoutScene();
    Scene *makeFullScene(const FixtureGroupEntry &grp, const QString &name);
    Chaser *makeChaserFromScenes(const QList<Scene *> &scenes,
                                 const QString &name,
                                 uint fadeMs, uint holdMs);

private:
    Doc             *m_doc;
    FixtureManager  *m_fixtureManager;
    FunctionManager *m_functionManager;
    VirtualConsole  *m_virtualConsole;
    ContextManager  *m_contextManager;

    int              m_currentStep;
    int              m_showType;
    int              m_stageType;
    QVector3D        m_envSize;      ///< Environment size in metres (W, H, D)
    bool             m_isGenerating;

    QList<FixtureGroupEntry> m_groups;
    QList<EffectEntry>       m_effects;

    // Synthetic "All Groups" aggregate (union of all selected groups' fixtures
    // and capabilities), backed by a real FixtureGroup created at generation.
    // Drives the master frame's page 0. Valid only while generating.
    FixtureGroupEntry        m_allGroups;
    bool                     m_hasAllGroups = false;

    // IDs collected during generation (for undo and VC wiring)
    QList<quint32>   m_generatedFunctionIDs;
    QList<quint32>   m_generatedGroupIDs;

    // Cache of base movement-position scene IDs, keyed by (groupId, anchor),
    // so effects sharing an anchor reuse the same Position scene.
    QHash<quint64, quint32> m_basePositionScenes;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StageWizard::EffectFlags)

#endif // STAGEWIZARD_H
