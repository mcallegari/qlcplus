/*
  Q Light Controller
  addfixture.h

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

#ifndef ADDFIXTURE_H
#define ADDFIXTURE_H

#include <QWidget>
#include <QList>

#include "ui_addfixture.h"
#include "fixture.h"

class QLCFixtureDefCache;
class QTreeWidgetItem;
class QLCFixtureMode;
class QLCFixtureDef;
class OutputMap;
class QString;
class Doc;

/** @addtogroup ui_fixtures
 * @{
 */

#define SETTINGS_EXPANDED "addfixture/expanded"

class AddFixture : public QDialog, public Ui_AddFixture
{
    Q_OBJECT
    Q_DISABLE_COPY(AddFixture)

public:
    /**
     * Create a new dialog for fixture definition selection.
     * If selectManufacturer and selectModel parameters are omitted, the
     * dialog selects "Generic Dimmer".
     *
     * @param parent The parent object that owns the dialog
     * @param doc QLC engine instance
     * @param fxi Fixture to edit (optional)
     */
    AddFixture(QWidget* parent, const Doc* doc, const Fixture* fxi = NULL);

    /** Destructor */
    ~AddFixture();

    /*********************************************************************
     * Value getters
     *********************************************************************/
public:
    /** Get the selected QLCFixtureDef */
    QLCFixtureDef *fixtureDef() const;

    /** Get the selected QLCFixtureMode */
    QLCFixtureMode* mode() const;

    /** Get the assigned friendly name */
    QString name() const;

    /** Get the assigned DMX address */
    quint32 address() const;

    /** Get the assigned DMX universe */
    quint32 universe() const;

    /** Get the number of similar fixtures to add */
    int amount() const;

    /** Get the number of channels to leave between two fixtures */
    quint32 gap() const;

    /** Get the number of channels to use (ONLY for generic dimmers) */
    quint32 channels() const;

    /** Get if the entered address is valid or not */
    bool invalidAddress();

protected:
    const Doc* m_doc;
    QLCFixtureDef* m_fixtureDef;
    QLCFixtureMode* m_mode;
    quint32 m_fixtureID;

    QString m_nameValue;

    quint32 m_addressValue;
    quint32 m_universeValue;
    int m_amountValue;
    quint32 m_gapValue;
    quint32 m_channelsValue;
    bool m_invalidAddressFlag;

    /*********************************************************************
     * Fillers
     *********************************************************************/
protected:
    /** Fill all known fixture definitions to the tree view. Select the
        given model from the given manufacturer. */
    void fillTree(const QString& selectManufacturer,
                  const QString& selectModel);

    /** Fill all modes of the current fixture to the mode combo */
    void fillModeCombo(const QString& text = QString());

    /** Find the next free address space for current fixture selection,
        amount and address gap. Sets the address to address spin. */
    void findAddress();

    /**
     * Attempt to find the next contiguous free address space for the given
     * number of channels. The address will not span multiple universes.
     * If a suitable address space cannot be found, QLCChannel::invalid() is
     * returned
     *
     * @param numChannels Number of channels in the address space
     * @return The address or QLCChannel::invalid() if not found
     */
    static quint32 findAddress(quint32 numChannels, QList<Fixture*> const& fixtures,
                               quint32 maxUniverses);

    /**
     * Try to find the next free address from the given universe for
     * the given number of channels. QLCChannel::invalid() is returned if
     * an adequate address range cannot be found.
     *
     * @param universe The universe to search from
     * @param numChannels Number of free channels required
     * @param fixtures Current fixtures
     * @param currentFixture ID of current fixture, that will be ignored
     * @return An address or QLCChannel::invalid() if address space not available
     */
    static quint32 findAddress(quint32 universe, quint32 numChannels,
                               QList<Fixture*> const& fixtures, quint32 currentFixture = Fixture::invalidId());

    /** Update the maximum amount of fixtures for the universe */
    void updateMaximumAmount();

    /** Check if an address is available for contiguous channels.
     *  Note that value is an absolute address.
     */
    bool checkAddressAvailability(int value, int channels);

protected:
    int m_fxiCount;

    /*********************************************************************
     * Slots
     *********************************************************************/
protected slots:
    /** Callback for mode selection changes */
    void slotModeActivated(int modeIndex);

    /** Callback for universe combo activations */
    void slotUniverseActivated(int universe);

    /** Callback for address spin changes */
    void slotAddressChanged(int value);

    /** Callback for channels spin value changes */
    void slotChannelsChanged(int value);

    /** Callback for tree view selection changes */
    void slotSelectionChanged();

    /** Callback for tree double clicks (same as select + OK) */
    void slotTreeDoubleClicked(QTreeWidgetItem* item);

    /** Callback for friendly name editing */
    void slotNameEdited(const QString &text);

    /** Callback for fixture amount value changes */
    void slotAmountSpinChanged(int value);

    /** Callback for address gap value changes */
    void slotGapSpinChanged(int value);

    /** Callback for fixture search filter changes */
    void slotSearchFilterChanged(QString filter);

    /** Callback for button to open the Address Tool */
    void slotDiptoolButtonClicked();

protected:
    void checkOverlapping();
};

/** @} */

#endif
