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

class Doc;
class Show;
class Track;
class Function;
class ShowFunction;

typedef struct
{
    quint32 m_trackIndex;
    ShowFunction *m_showFunc;
    QQuickItem *m_item;
} selectedShowItem;

class ShowManager : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(int currentShowID READ currentShowID WRITE setCurrentShowID NOTIFY currentShowIDChanged)
    Q_PROPERTY(QString showName READ showName WRITE setShowName NOTIFY showNameChanged)
    Q_PROPERTY(QColor itemsColor READ itemsColor WRITE setItemsColor NOTIFY itemsColorChanged)
    Q_PROPERTY(float timeScale READ timeScale WRITE setTimeScale NOTIFY timeScaleChanged)
    Q_PROPERTY(bool stretchFunctions READ stretchFunctions WRITE setStretchFunctions NOTIFY stretchFunctionsChanged)
    Q_PROPERTY(int currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(int showDuration READ showDuration NOTIFY showDurationChanged)
    Q_PROPERTY(QQmlListProperty<Track> tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(int selectedItemsCount READ selectedItemsCount NOTIFY selectedItemsCountChanged)

public:
    explicit ShowManager(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Return the ID of the Show Function being edited */
    int currentShowID() const;

    /** Set the ID of the Show Function to edit */
    void setCurrentShowID(int currentShowID);

    /** Return the name of the Show Function being edited */
    QString showName() const;

    /** Set the name of the Show Function to edit */
    void setShowName(QString showName);

    /** Return the currently selected color for Show Items */
    QColor itemsColor() const;

    /** Set the color of the currently selected Show Items */
    void setItemsColor(QColor itemsColor);

    /** Return the current time scale of the Show Manager timeline */
    float timeScale() const;

    /** Set the time scale of the Show Manager timeline */
    void setTimeScale(float timeScale);

    /** Return the stretch flag */
    bool stretchFunctions() const;

    /** Set the stretch flag */
    void setStretchFunctions(bool stretchFunctions);

    /** Add a new Item to the timeline.
     *  This happens when dragging an existing Function from the Function Manager.
     *  If the current Show is NULL, a new Show is created.
     *  If the provided $trackIdx is no valid, a new Track is created
     */
    Q_INVOKABLE void addItem(QQuickItem *parent, int trackIdx, int startTime, quint32 functionID);

    void deleteShowItems(QVariantList data);

    /** Method invoked when moving an existing Show Item on the timeline.
     *  The new position is checked for overlapping against existing items on the
     *  provided $newTrackIdx. On overlapping, false is returned and the UI
     *  will bring back the Item to its original position.
     *  If there is enough space, then the item is (in case) removed from the
     *  $originalTrackIdx and moved into $newTrackIdx and true is returned.
     */
    Q_INVOKABLE bool checkAndMoveItem(ShowFunction *sf,  int originalTrackIdx,
                                      int newTrackIdx, int newStartTime);

    QQmlListProperty<Track> tracks();

    /** Reset the Show Manager contents to an initial state */
    void resetContents();

    Q_INVOKABLE void resetView();

    /** Request to render the current Show items on screen */
    Q_INVOKABLE void renderView(QQuickItem *parent);

    Q_INVOKABLE void enableFlicking(bool enable);

    /** Return the current Show total duration in milliseconds */
    int showDuration() const;

    /** Return the current time of the Show */
    int currentTime() const;

    /** Set the Show time position */
    void setCurrentTime(int currentTime);

    Q_INVOKABLE void playShow();
    Q_INVOKABLE void stopShow();

    bool isPlaying() const;

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

protected slots:
    void slotTimeChanged(quint32 msec_time);

private:
    bool checkOverlapping(Track *track, ShowFunction *sourceFunc,
                          quint32 startTime, quint32 duration);

signals:
    void currentShowIDChanged(int currentShowID);
    void showNameChanged(QString showName);
    void itemsColorChanged(QColor itemsColor);
    void timeScaleChanged(float timeScale);
    void stretchFunctionsChanged(bool stretchFunction);
    void currentTimeChanged(int currentTime);
    void isPlayingChanged(bool playing);
    void showDurationChanged(int showDuration);
    void tracksChanged();
    void selectedItemsCountChanged(int count);

private:
    /** A reference to the Show Function being edited */
    Show *m_currentShow;

    /** The background color for Show Items */
    QColor m_itemsColor;

    /** The current time scale of the Show Manager timeline */
    float m_timeScale;

    /** Flag that indicates if a Function should be stretched
     *  when the corresponding Show Item duration changes */
    bool m_stretchFunctions;

    /** The current time position of the Show */
    int m_currentTime;

    /** A list of references to the selected Show Tracks */
    QList <Track*> m_tracksList;

    /** Pre-cached QML component for quick item creation */
    QQmlComponent *siComponent;

    QList<selectedShowItem> m_selectedItems;
};

#endif // SHOWMANAGER_H
