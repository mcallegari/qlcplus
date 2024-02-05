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
#include <QQuickItem>

#include "previewcontext.h"
#include "show.h"

class Doc;
class Track;
class Function;
class ShowFunction;

typedef struct
{
    quint32 m_trackIndex;
    ShowFunction *m_showFunc;
    QQuickItem *m_item;
} SelectedShowItem;

class ShowManager : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(int currentShowID READ currentShowID WRITE setCurrentShowID NOTIFY currentShowIDChanged)
    Q_PROPERTY(bool isEditing READ isEditing NOTIFY isEditingChanged)
    Q_PROPERTY(QString showName READ showName WRITE setShowName NOTIFY showNameChanged)
    Q_PROPERTY(QColor itemsColor READ itemsColor WRITE setItemsColor NOTIFY itemsColorChanged)

    Q_PROPERTY(bool stretchFunctions READ stretchFunctions WRITE setStretchFunctions NOTIFY stretchFunctionsChanged)
    Q_PROPERTY(bool gridEnabled READ gridEnabled WRITE setGridEnabled NOTIFY gridEnabledChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(int showDuration READ showDuration NOTIFY showDurationChanged)

    Q_PROPERTY(Show::TimeDivision timeDivision READ timeDivision WRITE setTimeDivision NOTIFY timeDivisionChanged)
    Q_PROPERTY(int beatsDivision READ beatsDivision NOTIFY beatsDivisionChanged)
    Q_PROPERTY(float timeScale READ timeScale WRITE setTimeScale NOTIFY timeScaleChanged)
    Q_PROPERTY(float tickSize READ tickSize NOTIFY tickSizeChanged)
    Q_PROPERTY(int currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)

    Q_PROPERTY(QVariant tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(int selectedTrackIndex READ selectedTrackIndex WRITE setSelectedTrackIndex NOTIFY selectedTrackIndexChanged)
    Q_PROPERTY(int selectedItemsCount READ selectedItemsCount NOTIFY selectedItemsCountChanged)

public:
    explicit ShowManager(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Return the ID of the Show Function being edited */
    int currentShowID() const;

    /** Return a reference of the Show currently being edited */
    Show *currentShow() const;

    /** Flag to indicate if a Show is currently being edited */
    bool isEditing();

    /** Set the ID of the Show Function to edit */
    void setCurrentShowID(int currentShowID);

    /** Return the name of the Show Function being edited */
    QString showName() const;

    /** Set the name of the Show Function to edit */
    void setShowName(QString showName);

    /** Reset the Show Manager contents to an initial state */
    void resetContents();

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

    /** Play or resume the Show playback */
    Q_INVOKABLE void playShow();

    /** Pause or rewind the Show playback */
    Q_INVOKABLE void stopShow();

    /** Flag that indicates if the Show is currently being played */
    bool isPlaying() const;

signals:
    void currentShowIDChanged(int currentShowID);
    void isEditingChanged();
    void showNameChanged(QString showName);
    void stretchFunctionsChanged(bool stretchFunction);
    void gridEnabledChanged(bool gridEnabled);
    void isPlayingChanged(bool playing);
    void showDurationChanged(int showDuration);

private:
    /** A reference to the Show Function being edited */
    Show *m_currentShow;

    /** Flag that indicates if a Function should be stretched
     *  when the corresponding Show Item duration changes */
    bool m_stretchFunctions;

    /** Flag that indicates if the Show items should be
     *  snapped to the closest grid divisor */
    bool m_gridEnabled;

    /*********************************************************************
      * Time
      ********************************************************************/
public:
    /** Get/Set the Show time division */
    Show::TimeDivision timeDivision();
    void setTimeDivision(Show::TimeDivision division);
    int beatsDivision();

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
    QVariant tracks();

    /** Get/Set the selected track index */
    int selectedTrackIndex() const;
    void setSelectedTrackIndex(int index);

    Q_INVOKABLE void setTrackSolo(int index, bool solo);

    /** Move the track with the provided index in the provided direction */
    Q_INVOKABLE void moveTrack(int index, int direction);

signals:
    void tracksChanged();
    void selectedTrackIndexChanged(int index);

private:
    /** A list of references to the selected Show Tracks */
    QList <Track*> m_tracksList;

    /** The index of the currently selected track */
    int m_selectedTrackIndex;

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
     *  If the provided $trackIdx is not valid, a new Track is created
     */
    Q_INVOKABLE void addItems(QQuickItem *parent, int trackIdx, int startTime, QVariantList idsList);

    /** Add a Show item from an existing ShowFunction reference and Track Id */
    void addShowItem(ShowFunction *sf, quint32 trackId);

    /** Delete the currently selected show items */
    Q_INVOKABLE void deleteShowItems(QVariantList data);

    /** Delete the item referencing the provided ShowFunction from the QML view */
    void deleteShowItem(ShowFunction *sf);

    /** Method invoked when moving an existing Show Item on the timeline.
     *  The new position is checked for overlapping against existing items on the
     *  provided $newTrackIdx. On overlapping, false is returned and the UI
     *  will bring back the Item to its original position.
     *  If there is enough space, then the item is (in case) removed from the
     *  $originalTrackIdx and moved into $newTrackIdx and true is returned.
     */
    Q_INVOKABLE bool checkAndMoveItem(ShowFunction *sf,  int originalTrackIdx,
                                      int newTrackIdx, int newStartTime);

    /** Set the start time of a ShowFunction item (if not overlapping) */
    Q_INVOKABLE bool setShowItemStartTime(ShowFunction *sf, int startTime);

    /** Set the duration of a ShowFunction item (if not overlapping) */
    Q_INVOKABLE bool setShowItemDuration(ShowFunction *sf, int duration);

    /** Returns the number of the currently selected Show items */
    int selectedItemsCount() const;

    /** Add an item to the selection tracking list */
    Q_INVOKABLE void setItemSelection(int trackIdx, ShowFunction *sf, QQuickItem *item, bool selected);

    /** Deselect all the selected items at once */
    Q_INVOKABLE void resetItemsSelection();

    Q_INVOKABLE QVariantList selectedItemRefs();
    Q_INVOKABLE QStringList selectedItemNames();

    /** Returns true if at least one of the selected items is locked */
    Q_INVOKABLE bool selectedItemsLocked();

    /** Lock/Unlock all the currently selected items */
    Q_INVOKABLE void setSelectedItemsLock(bool lock);

    /**
     * Returns an array of values coupled as: PreviewDrawType, time value
     * The UI will render the lines according to their time value and their type
     */
    Q_INVOKABLE QVariantList previewData(Function *f) const;

    Q_INVOKABLE void copyToClipboard();
    Q_INVOKABLE void pasteFromClipboard();

protected slots:
    void slotTimeChanged(quint32 msec_time);

private:
    /** Check items overlapping for the given track, ShowFunction,
     *  start time and duration. Returns true if overlapping is
     *  detected, otherwise false */
    bool checkOverlapping(Track *track, ShowFunction *sourceFunc,
                          quint32 startTime, quint32 duration);

signals:
    void itemsColorChanged(QColor itemsColor);
    void selectedItemsCountChanged(int count);

private:
    /** The background color for Show Items */
    QColor m_itemsColor;

    /** Pre-cached QML component for quick item creation */
    QQmlComponent *siComponent;

    /** Holds the currently selected Show items */
    QList<SelectedShowItem> m_selectedItems;

    /** Holds the item currenly ready for pasting */
    QList<SelectedShowItem> m_clipboard;
};

#endif // SHOWMANAGER_H
