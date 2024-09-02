/*
  Q Light Controller
  vcsliderproperties.h

  Copyright (c) Heikki Junnila

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

#ifndef VCSLIDERPROPERTIES_H
#define VCSLIDERPROPERTIES_H

#include <QDialog>

#include "ui_vcsliderproperties.h"

class InputSelectionWidget;
class QTreeWidgetItem;
class QLCCapability;
class QLCChannel;
class VCSlider;
class Fixture;
class Doc;

/** @addtogroup ui_vc_props
 * @{
 */

class VCSliderProperties : public QDialog, public Ui_VCSliderProperties
{
    Q_OBJECT
    Q_DISABLE_COPY(VCSliderProperties)

public:
    VCSliderProperties(VCSlider* slider, Doc* doc);
    ~VCSliderProperties();

private:
    Doc* m_doc;

    /*********************************************************************
     * General page
     *********************************************************************/
protected slots:
    void slotModeLevelClicked();
    void slotModePlaybackClicked();
    void slotModeSubmasterClicked();

protected:
    void setLevelPageVisibility(bool visible);
    void setPlaybackPageVisibility(bool visible);
    void setSubmasterPageVisibility(bool visible);

protected:
    int m_sliderMode;
    InputSelectionWidget *m_inputSelWidget;

    /*********************************************************************
     * Level page
     *********************************************************************/
protected:
    /** Update fixtures to the listview on the level page */
    void levelUpdateFixtures();

    /** Update a fixture node in the listview on the level page */
    void levelUpdateFixtureNode(quint32 id);

    /** Get a fixture node from the listview on the level page */
    QTreeWidgetItem* levelFixtureNode(quint32 id);

    /** Update fixture channels to the listview on the level page */
    void levelUpdateChannels(QTreeWidgetItem* parent, Fixture* fxi);

    /** Update a fixture channel node to the listview on the level page */
    void levelUpdateChannelNode(QTreeWidgetItem* parent, Fixture* fxi, quint32 ch);

    /** Update a channel's capabilities */
    void levelUpdateCapabilities(QTreeWidgetItem* parent, const QLCChannel* channel);

    /** Update a channel's capability node */
    void levelUpdateCapabilityNode(QTreeWidgetItem* parent, QLCCapability* cap);

    /** Get a fixture channel node from the listview on the level page */
    QTreeWidgetItem* levelChannelNode(QTreeWidgetItem* parent, quint32 ch);

    /** Update channel selections from the slider's level channel list */
    void levelUpdateChannelSelections();

    /**
     * Select all channels matching the given group name from the listview
     * on the level page
     */
    void levelSelectChannelsByGroup(QString group);

protected slots:
    /** Callback for low level limit spin value changes */
    void slotLevelLowSpinChanged(int value);

    /** Callback for high level limit spin value changes */
    void slotLevelHighSpinChanged(int value);

    /** Callback for "set limits by capability" button clicks */
    void slotLevelCapabilityClicked();

    /** Callback for level list item clicks */
    void slotLevelListClicked(QTreeWidgetItem* item);

    /** Callback for All button clicks */
    void slotLevelAllClicked();

    /** Callback for None button clicks */
    void slotLevelNoneClicked();

    /** Callback for Invert button clicks */
    void slotLevelInvertClicked();

    /** Callback for "channel selection by group" button clicks */
    void slotLevelByGroupClicked();

    /** Callback for tree item expanded/collapsed */
    void slotItemExpanded();

    /** Callback for monitoring enable */
    void slotMonitorCheckClicked(bool checked);

protected:
    InputSelectionWidget *m_ovrResetSelWidget;

    /*************************************************************************
     * Playback page
     *************************************************************************/
public slots:
    /** Callback for playback function attach clicks */
    void slotAttachPlaybackFunctionClicked();

    /** Callback for playback function detach clicks */
    void slotDetachPlaybackFunctionClicked();

    /** Callback for flah button enable */
    void slotFlashCheckClicked(bool checked);

protected:
    /** Update the name of the playback function, based on m_playbackFunctionId */
    void updatePlaybackFunctionName();

protected:
    /** The currently selected playback function */
    quint32 m_playbackFunctionId;

    InputSelectionWidget *m_flashInputWidget;

    /*************************************************************************
     * Submaster page
     *************************************************************************/

    /*********************************************************************
     * OK & Cancel
     *********************************************************************/
protected:
    /** Check if the given comp color is the predominant and set the
     *  proper value for ClickAndGo color functionality */
    void checkMajorColor(int *comp, int *max, int type);

    /** Store selected channels to the slider */
    void storeLevelChannels();

protected slots:
    /** Callback for OK button clicks */
    void accept();

protected:
    /** The slider, whose properties are being edited */
    VCSlider* m_slider;
};

/** @} */

#endif
