/*
  Q Light Controller Plus
  showmanager.h

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

#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include <QObject>
#include <QPointer>
#include <QQuickItem>

#include "previewcontext.h"
#include "show.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class Doc;
class Track;
class Function;
class Chaser;
class ShowFunction;
class WaveformImageProvider;

typedef struct
{
    quint32 m_trackIndex;
    /* guarded pointers: Show items and ShowFunctions can be destroyed
     * while still referenced here (view rebuild, undo, show closing),
     * so use QPointer to have them automatically reset to nullptr */
    QPointer<ShowFunction> m_showFunc;
    QPointer<QQuickItem> m_item;
} SelectedShowItem;

#define KXMLQLCShowManager QStringLiteral("ShowManager")

class ShowManager final : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(int currentShowID READ currentShowID WRITE setCurrentShowID NOTIFY currentShowIDChanged)
    Q_PROPERTY(bool isEditing READ isEditing NOTIFY isEditingChanged)
    Q_PROPERTY(QString showName READ showName WRITE setShowName NOTIFY showNameChanged)
    Q_PROPERTY(QColor itemsColor READ itemsColor WRITE setItemsColor NOTIFY itemsColorChanged)

    Q_PROPERTY(bool stretchFunctions READ stretchFunctions WRITE setStretchFunctions NOTIFY stretchFunctionsChanged)
    Q_PROPERTY(bool gridEnabled READ gridEnabled WRITE setGridEnabled NOTIFY gridEnabledChanged)
    Q_PROPERTY(bool snapToItems READ snapToItems WRITE setSnapToItems NOTIFY snapToItemsChanged)
    Q_PROPERTY(double snapGuideX READ snapGuideX WRITE setSnapGuideX NOTIFY snapGuideXChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY isPausedChanged)
    Q_PROPERTY(int showDuration READ showDuration NOTIFY showDurationChanged)

    Q_PROPERTY(Show::TimeDivision timeDivision READ timeDivision WRITE setTimeDivision NOTIFY timeDivisionChanged)
    Q_PROPERTY(int beatsDivision READ beatsDivision NOTIFY beatsDivisionChanged)
    Q_PROPERTY(float timeScale READ timeScale WRITE setTimeScale NOTIFY timeScaleChanged)
    Q_PROPERTY(float tickSize READ tickSize NOTIFY tickSizeChanged)
    Q_PROPERTY(int currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)

    Q_PROPERTY(QVariant tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(int selectedTrackId READ selectedTrackId WRITE setSelectedTrackId NOTIFY selectedTrackIdChanged)
    Q_PROPERTY(int selectedItemsCount READ selectedItemsCount NOTIFY selectedItemsCountChanged)
    Q_PROPERTY(int clipboardItemsCount READ clipboardItemsCount NOTIFY clipboardItemsCountChanged)
    Q_PROPERTY(QVariantList cutItemIds READ cutItemIds NOTIFY cutItemIdsChanged)
    Q_PROPERTY(bool multipleSelection READ multipleSelection WRITE setMultipleSelection NOTIFY multipleSelectionChanged)
    Q_PROPERTY(bool groupDragActive READ groupDragActive WRITE setGroupDragActive NOTIFY groupDragActiveChanged)
    Q_PROPERTY(QPointF groupDragOffset READ groupDragOffset WRITE setGroupDragOffset NOTIFY groupDragOffsetChanged)
    Q_PROPERTY(bool boxSelectMode READ boxSelectMode WRITE setBoxSelectMode NOTIFY boxSelectModeChanged)

public:
    explicit ShowManager(QQuickView *view, Doc *doc, QObject *parent = 0);

    void initialize();

    /** Return the ID of the Show Function being edited */
    int currentShowID() const;

    /** Return a reference of the Show currently being edited */
    Show *currentShow() const;

    /** Flag to indicate if a Show is currently being edited */
    bool isEditing() const;

    /** Set the ID of the Show Function to edit */
    void setCurrentShowID(int currentShowID);

    /** Return the name of the Show Function being edited */
    QString showName() const;

    /** Set the name of the Show Function to edit */
    void setShowName(QString showName);

    /** Reset the Show Manager contents to an initial state */
    Q_INVOKABLE void resetContents();

    /** Clear all the current items in the ShowManager view */
    Q_INVOKABLE void resetView();

    /** Request to render the current Show items on screen */
    Q_INVOKABLE void renderView(QQuickItem *parent);

    Q_INVOKABLE void enableFlicking(bool enable);

    /** Return the current Show total duration in milliseconds */
    int showDuration() const;

    /** Get/Set the Function stretch flag */
    bool stretchFunctions() const;
    void setStretchFunctions(bool stretchFunctions);

    /** Get/Set the grid snapping functionality */
    bool gridEnabled() const;
    void setGridEnabled(bool gridEnabled);

    /** Get/Set the snapping of Show items to the nearby items' edges.
     *  Stored in the local computer settings, so it survives a restart */
    bool snapToItems() const;
    void setSnapToItems(bool snapToItems);

    /** Get/Set the X position of the snap guide line (-1 = hidden) */
    double snapGuideX() const;
    void setSnapGuideX(double snapGuideX);

    /** Play or resume the Show playback */
    Q_INVOKABLE void playShow();

    /** Stop or rewind the Show playback */
    Q_INVOKABLE void stopShow();

    /** Flag that indicates if the Show is currently being played */
    bool isPlaying() const;

    /** Flag that indicates if the Show playback is currently paused */
    bool isPaused() const;

signals:
    void currentShowIDChanged(int currentShowID);
    void isEditingChanged();
    void showNameChanged(QString showName);
    void stretchFunctionsChanged(bool stretchFunction);
    void gridEnabledChanged(bool gridEnabled);
    void snapToItemsChanged(bool snapToItems);
    void snapGuideXChanged();
    void isPlayingChanged(bool playing);
    void isPausedChanged(bool paused);
    void showDurationChanged(int showDuration);

private:
    void setPlaybackState(bool playing, bool paused);

    /** Track if cursor is interactively being moved during pause */
    bool m_cursorMovedDuringPause;

    /** Cached playback state for immediate UI updates */
    bool m_isPlaying;
    bool m_isPaused;

    /** A reference to the Show Function being edited */
    Show *m_currentShow;

    /** Flag that indicates if a Function should be stretched
     *  when the corresponding Show Item duration changes */
    bool m_stretchFunctions;

    /** Flag that indicates if the Show items should be
     *  snapped to the closest grid divisor */
    bool m_gridEnabled;

    /** Flag that indicates if the Show items should be
     *  snapped to the edges of the nearby items */
    bool m_snapToItems;

    /** X position of the snap guide line (-1 = hidden) */
    double m_snapGuideX;

    /*********************************************************************
      * Time
      ********************************************************************/
public:
    /** Get/Set the Show time division */
    Show::TimeDivision timeDivision() const;
    void setTimeDivision(Show::TimeDivision division);
    int beatsDivision() const;

    /** Return true if the current Show has any item on its tracks whose
     *  Function is beat (BPM) tempo based. Used to warn the user before
     *  switching the Show from a Time to a BPM based division, since doing
     *  so snaps those items' start/duration to the nearest whole beat
     *  (they may have been placed at an arbitrary fractional-beat pixel
     *  position while the Show was displaying a Time based ruler) */
    Q_INVOKABLE bool hasBeatBasedItems() const;

    /** Get/Set the current time scale of the Show Manager timeline */
    float timeScale() const;
    void setTimeScale(float timeScale);

    /** Get the size in pixels of the Show header time division */
    float tickSize() const;

    /** Get/Set the current time of the Show (aka cursor position) */
    int currentTime() const;
    void setCurrentTime(int currentTime);

signals:
    void timeDivisionChanged(Show::TimeDivision division);
    void beatsDivisionChanged(int beatsDivision);
    void timeScaleChanged(float timeScale);
    void tickSizeChanged(float tickSize);
    void currentTimeChanged(int currentTime);

private:
    /** The current time scale of the Show Manager timeline */
    float m_timeScale;

    /** Size in pixels of the Show Manager time division */
    float m_tickSize;

    /** The current time position of the Show in ms */
    int m_currentTime;

    /*********************************************************************
      * Tracks
      ********************************************************************/
public:
    /** Return a list of Track objects suitable for QML */
    QVariant tracks() const;

    /** Get/Set the selected track id */
    int selectedTrackId() const;
    void setSelectedTrackId(int id);

    Q_INVOKABLE void setTrackSolo(int index, bool solo);

    /** Move the track with the provided index in the provided direction */
    Q_INVOKABLE void moveTrack(int index, int direction);

    /** Delete the currently selected Show Track */
    Q_INVOKABLE void deleteSelectedTrack();

signals:
    void tracksChanged();
    void selectedTrackIdChanged(int id);

private:
    /** Select the topmost Track holding a selected Show item, without
     *  claiming the keyboard shortcuts for the Track like a click does */
    void selectTrackOfSelectedItems();

    /** The index of the currently selected track */
    int m_selectedTrackId;

    /*********************************************************************
      * Show Items
      ********************************************************************/
public:
    /**
     * This enumeration instructs the UI how to interpret the data
     * stored in what previewData returns. It is a numeric
     * prefix before the time value
     */
    enum PreviewDrawType
    {
        RepeatingDuration = 0,
        FadeIn,
        StepDivider,
        FadeOut,
        AudioData
    };
    Q_ENUM(PreviewDrawType)

    /** Return the currently selected color for Show Items */
    QColor itemsColor() const;

    /** Set the color of the currently selected Show Items */
    void setItemsColor(QColor itemsColor);

    /** Add a new Item to the timeline.
     *  This happens when dragging an existing Function from the Function Manager.
     *  If the current Show is NULL, a new Show is created.
     *  If the provided $trackIdx is not valid, a new Track is created.
     *  If $sourceFunc is not NULL (e.g. when pasting), the created ShowFunction
     *  inherits its duration, color and lock state from it.
     */
    Q_INVOKABLE void addItems(QQuickItem *parent, int trackIdx, int startTime, QVariantList idsList,
                              ShowFunction *sourceFunc = nullptr);

    /** Add a Show item from an existing ShowFunction reference and Track Id */
    void addShowItem(ShowFunction *sf, quint32 trackId);

    /** Delete the currently selected show items */
    Q_INVOKABLE void deleteShowItems(QVariantList data);

    /** Delete the item referencing the provided ShowFunction from the QML view */
    void deleteShowItem(ShowFunction *sf);

    /** Rebuild the whole timeline from the current Show contents.
      * Used when Tracks are added/removed outside of the normal UI flow,
      * for example by an undo/redo action */
    void refreshView();

    /** Method invoked when moving an existing Show Item on the timeline.
     *  The new position is checked for overlapping against existing items on the
     *  provided $newTrackIdx. On overlapping, false is returned and the UI
     *  will bring back the Item to its original position.
     *  If there is enough space, then the item is (in case) removed from the
     *  $originalTrackIdx and moved into $newTrackIdx and true is returned.
     */
    Q_INVOKABLE bool checkAndMoveItem(ShowFunction *sf,  int originalTrackIdx,
                                      int newTrackIdx, int newStartTime);

    /** Move several Show items at once, as when dragging a multiple selection.
     *  $items are the QML Show items, $trackIndexes and $startTimes their
     *  destination Track indices and start times, in the same order.
     *  All the destinations are checked for overlapping first, against the
     *  items that stay where they are and against each other, and nothing
     *  is moved unless every item fits. Unlike checkAndMoveItem, no Track
     *  is created: all the destination Tracks must already exist.
     *  Returns true if the items have been moved */
    Q_INVOKABLE bool moveShowItems(QVariantList items, QVariantList trackIndexes,
                                   QVariantList startTimes);

    /** Return the number of Tracks of the Show being edited */
    Q_INVOKABLE int tracksCount() const;

    /** Move a ShowFunction item to the Track at $trackIdx.
     *  This is used to apply a track change coming from an undo/redo or
     *  from a connected network peer, where the UI didn't move the item */
    bool moveShowItemToTrack(ShowFunction *sf, int trackIdx);

    /** Set the start time of a ShowFunction item (if not overlapping) */
    Q_INVOKABLE bool setShowItemStartTime(ShowFunction *sf, int startTime);

    /** Set the duration of a ShowFunction item (if not overlapping) */
    Q_INVOKABLE bool setShowItemDuration(ShowFunction *sf, int duration);

    /** Set both the start time and the duration of a ShowFunction item
     *  (if not overlapping), checking the resulting span as a whole */
    Q_INVOKABLE bool setShowItemStartTimeAndDuration(ShowFunction *sf, int startTime, int duration);

    /** Insert a time segment in a ShowFunction item, applying type-specific rules */
    Q_INVOKABLE bool insertShowItemTime(ShowFunction *sf, int length);

    /** Cut a time segment from a ShowFunction item, applying type-specific rules */
    Q_INVOKABLE bool cutShowItemTime(ShowFunction *sf, int length);

    /** Insert time at cursor position for all the items covering that position */
    Q_INVOKABLE bool insertTimeAtCursor(int length, int cursorTime);

    /** Cut time at cursor position for all the items covering that position */
    Q_INVOKABLE bool cutTimeAtCursor(int length, int cursorTime);

    /** Returns pixel X positions of all item edges (start + end) across all tracks,
     *  excluding the Show item with the given ID */
    Q_INVOKABLE QVariantList getSnapEdges(quint32 excludeItemId,
                                          double viewportLeft = -1, double viewportRight = -1) const;

    /** Returns the number of the currently selected Show items */
    int selectedItemsCount() const;

    /** Returns the number of Show items currently in the clipboard */
    int clipboardItemsCount() const;

    /** Get/Set multi selection mode for Show items */
    bool multipleSelection() const;
    void setMultipleSelection(bool multipleSelection);

    /** Get/Set the box selection mode for Show items. When enabled, dragging
     *  on the timeline draws a box selecting the items it wholly contains */
    bool boxSelectMode() const;
    void setBoxSelectMode(bool enable);

    /** Add an item to the selection tracking list */
    Q_INVOKABLE void setItemSelection(int trackIdx, ShowFunction *sf, QQuickItem *item, bool selected, int keyModifiers);

    /** Deselect all the selected items at once */
    Q_INVOKABLE void resetItemsSelection();

    /** Select all the items of the currently selected Track, or of the
     *  Track of the last selected item when no Track is selected.
     *  Returns false if there is no Track to select the items from */
    Q_INVOKABLE bool selectAllTrackItems();
    /** Select the Show items lying entirely within $rect, expressed in the
     *  coordinates of the items' parent. If $addToSelection is false,
     *  the current selection is replaced, otherwise it is extended */
    Q_INVOKABLE void selectItemsInRect(QRectF rect, bool addToSelection);

    Q_INVOKABLE QVariantList selectedItemRefs() const;

    /** Returns the QML items of the currently selected Show items */
    Q_INVOKABLE QVariantList selectedItemViews() const;

    /** Get/Set the flag telling the selected Show items to follow the
     *  one being dragged, by the offset in groupDragOffset */
    bool groupDragActive() const;
    void setGroupDragActive(bool active);

    /** Get/Set the offset, in pixels, of the Show item being dragged
     *  from its original position */
    QPointF groupDragOffset() const;
    void setGroupDragOffset(QPointF offset);
    Q_INVOKABLE QStringList selectedItemNames() const;

    /** Returns true if at least one of the selected items is locked */
    Q_INVOKABLE bool selectedItemsLocked() const;

    /** Lock/Unlock all the currently selected items */
    Q_INVOKABLE void setSelectedItemsLock(bool lock);

    /**
     * Returns an array of values coupled as: PreviewDrawType, time value
     * The UI will render the lines according to their time value and their type
     */
    Q_INVOKABLE QVariantList previewData(Function *f) const;

    /** Copy the selected items in the clipboard. Nothing happens
     *  when no item is selected */
    Q_INVOKABLE void copyToClipboard();

    /** Mark the selected items as cut: they stay where they are, dimmed,
     *  until they are moved by the next paste. Copying cancels the cut.
     *  Returns false, and emits clipboardActionFailed, if the selection
     *  contains locked items */
    Q_INVOKABLE bool cutToClipboard();

    /** Paste the clipboard items at the cursor position, the earliest one
     *  starting at the cursor. Items keep their relative start times and
     *  Track offsets, gap Tracks included. The topmost Track of the items
     *  lands on the selected Track, or the items land on their own Tracks
     *  when no Track is selected. Pasting cut items moves them.
     *  Nothing is pasted unless every item fits: on failure false is
     *  returned and clipboardActionFailed carries the reason */
    Q_INVOKABLE bool pasteFromClipboard();

    /** Returns the ShowFunction IDs of the items pending a cut */
    QVariantList cutItemIds() const;

protected slots:
    void slotFunctionRemoved(quint32 id);
    void slotTimeChanged(quint32 msec_time);
    void slotShowFinished();
    void slotShowStopped();

private:
    // Timeline mapping helpers
    int minimumTimelineDuration(Show::TimeDivision division) const;
    quint32 itemRelativeTimeFromCursor(const ShowFunction *sf, int cursorTime) const;
    quint32 mapCursorToChaserTime(const ShowFunction *sf, Chaser *chaser, int cursorTime) const;

    // Chaser-specific helpers
    quint32 chaserStepDuration(Chaser *chaser, int index) const;
    int chaserStepIndexFromTime(Chaser *chaser, quint32 timeValue) const;
    bool setChaserStepDurationWithUndo(Chaser *chaser, int stepIndex, quint32 newDuration);
    void convertChaserCommonToPerStep(Chaser *chaser);

    // Timeline mutation helpers
    void setShowItemDurationWithUndo(ShowFunction *sf, int newDuration);
    bool moveAllItemsAfterCursor(int cursorTime, int delta);

    bool insertShowItemTimeAt(ShowFunction *sf, int length, int cursorTime);
    bool cutShowItemTimeAt(ShowFunction *sf, int length, int cursorTime);

    /** Check items overlapping for the given track, ShowFunction,
     *  start time and duration. Returns true if overlapping is
     *  detected, otherwise false */
    bool checkOverlapping(Track *track, ShowFunction *sourceFunc,
                          quint32 startTime, quint32 duration) const;

    /** Same as above, but ignoring all the ShowFunctions in $exclude */
    bool checkOverlapping(Track *track, const QList<ShowFunction *> &exclude,
                          quint32 startTime, quint32 duration) const;

    /** Convert a time value between the unit of the Show timeline and
     *  the unit of $func, which differ when one of them is beat-based */
    quint32 showToFunctionTime(const Function *func, quint32 value) const;
    quint32 functionToShowTime(const Function *func, quint32 value) const;

    /** Drop the pending cut state, keeping the clipboard as a copy */
    void clearCutState();

signals:
    void itemsColorChanged(QColor itemsColor);
    void selectedItemsCountChanged(int count);
    void clipboardItemsCountChanged(int count);
    void cutItemIdsChanged();

    /** Notify the UI that a clipboard action could not be performed */
    void clipboardActionFailed(QString title, QString message);
    void multipleSelectionChanged();
    void groupDragActiveChanged();
    void groupDragOffsetChanged();
    void boxSelectModeChanged();

    /** Notify the UI that the Function with the given $fid has been modified,
     *  so Show Items referencing it can repaint their preview lines */
    void functionChanged(quint32 fid);

private:
    /** The background color for Show Items */
    QColor m_itemsColor;

    /** Pre-cached QML component for quick item creation */
    QQmlComponent *siComponent;

    /** Holds the currently selected Show items */
    QList<SelectedShowItem> m_selectedItems;

    /** Flag to enable multi selection in Show items */
    bool m_multipleSelection;

    /** State of a multiple selection being dragged */
    bool m_groupDragActive;
    QPointF m_groupDragOffset;

    /** Flag to enable box selection of Show items */
    bool m_boxSelectMode;

    /** Holds the item currently ready for pasting */
    QList<SelectedShowItem> m_clipboard;

    /** True when the clipboard items are cut, and so pending a move */
    bool m_clipboardIsCut;

    WaveformImageProvider *m_waveformProvider;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Save the Show Manager view state (the Show being edited and
     *  the timeline zoom level) to the workspace XML */
    bool saveXML(QXmlStreamWriter *doc) const;

    /** Restore the Show Manager view state from the workspace XML.
     *  The Functions must have been loaded already */
    bool loadXML(QXmlStreamReader &root);
};

#endif // SHOWMANAGER_H
