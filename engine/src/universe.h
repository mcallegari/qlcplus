/*
  Q Light Controller Plus
  universe.h

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


#ifndef UNIVERSE_H
#define UNIVERSE_H

#include <QByteArray>
#include <QSet>

#include "qlcchannel.h"

class QLCInputProfile;
class ChannelModifier;
class InputOutputMap;
class QLCIOPlugin;
class GrandMaster;
class OutputPatch;
class InputPatch;

/** @addtogroup engine Engine
 * @{
 */

#define UNIVERSE_SIZE 512

#define KXMLQLCUniverse "Universe"
#define KXMLQLCUniverseName "Name"
#define KXMLQLCUniverseID "ID"
#define KXMLQLCUniversePassthrough "Passthrough"

#define KXMLQLCUniverseInputPatch "Input"
#define KXMLQLCUniverseInputPlugin "Plugin"
#define KXMLQLCUniverseInputLine "Line"
#define KXMLQLCUniverseInputProfileName "Profile"

#define KXMLQLCUniverseOutputPatch "Output"
#define KXMLQLCUniverseOutputPlugin "Plugin"
#define KXMLQLCUniverseOutputLine "Line"

#define KXMLQLCUniverseFeedbackPatch "Feedback"
#define KXMLQLCUniverseFeedbackPlugin "Plugin"
#define KXMLQLCUniverseFeedbackLine "Line"

#define KXMLQLCUniversePluginParameters "PluginParameters"

/** Universe class contains input/output data for one DMX universe
 */
class Universe: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Universe)

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(quint32 id READ id CONSTANT)
    Q_PROPERTY(InputPatch* inputPatch READ inputPatch NOTIFY inputPatchChanged)
    Q_PROPERTY(OutputPatch* outputPatch READ outputPatch NOTIFY outputPatchChanged)

public:
    /** Construct a new Universe */
    Universe(quint32 id = invalid(), GrandMaster *gm = NULL, QObject* parent = 0);

    /** Destructor */
    virtual ~Universe();

    /**
     * Channel types.
     * This is a bit mask to facilitate easy AND-mode type filtering
     */
    enum ChannelType
    {
        Undefined  = 0,
        LTP        = 1 << 0,
        HTP        = 1 << 1,
        Intensity  = 1 << 2
    };

    static quint32 invalid() { return UINT_MAX; }

    /**
     * Set a friendly name for this universe
     */
    void setName(QString name);

    /**
     * Retrieve the universe friendly name
     */
    QString name() const;

    /**
     * Set the universe ID (or index)
     */
    void setID(quint32 id);

    /**
     * Retrieve the universe ID
     */
    quint32 id() const;

    /**
     * Returns the number of channels used in this universe
     */
    ushort usedChannels();

    /**
     * Returns the total number of channels in this universe
     */
    ushort totalChannels();

    /**
     * Returns if the universe has changed since the last MasterTimer tick
     */
    bool hasChanged();

    /**
     * Enable or disable the passthrough mode for this universe
     */
    void setPassthrough(bool enable);

    /**
     * Returns if the universe is in passthrough mode
     */
    bool passthrough() const;

    /**
     * Enable or disable the monitor mode for this universe
     */
    void setMonitor(bool enable);

    /**
     * Returns if the universe is in monitor mode
     */
    bool monitor() const;

protected slots:
    /**
     * Called every time the Grand Master changed value
     */
    void slotGMValueChanged();

protected:
    /**
     * Apply Grand Master to the value.
     *
     * @param channel The channel to apply Grand Master to
     * @param value The value to write
     *
     * @return Value filtered through grand master (if applicable)
     */
    uchar applyGM(int channel, uchar value);

signals:
    void nameChanged();

protected:
    /** The universe ID */
    quint32 m_id;
    /** The universe friendly name */
    QString m_name;
    /** Reference to the Grand Master to perform values scaling */
    GrandMaster *m_grandMaster;
    /** Variable that determine if a universe is in passthrough mode */
    bool m_passthrough;
    /** Flag to monitor the universe changes */
    bool m_monitor;

    /************************************************************************
     * Patches
     ************************************************************************/
public:
    /** Returns true if this universe is patched with an input, output OR feedback
     *  otherwise returns false */
    bool isPatched();

    bool setInputPatch(QLCIOPlugin *plugin, quint32 input,
                       QLCInputProfile *profile = NULL);

    bool setOutputPatch(QLCIOPlugin *plugin, quint32 output);

    bool setFeedbackPatch(QLCIOPlugin *plugin, quint32 output);

    /**
     * Get the reference to the input plugin associated to this universe.
     * If not present NULL is returned.
     */
    InputPatch* inputPatch() const;

    /**
     * Get the reference to the output plugin associated to this universe.
     * If not present NULL is returned.
     */
    OutputPatch* outputPatch() const;

    /**
     * Get the reference to the feedback plugin associated to this universe.
     * If not present NULL is returned.
     */
    OutputPatch* feedbackPatch() const;

    /**
     * This is the actual function that writes data to an output patch
     */
    void dumpOutput(const QByteArray& data);

    void flushInput();

protected slots:
    /** Slot called every time an input patch sends data */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value, const QString& key = 0);

signals:
    /** Everyone interested in input data should connect to this signal */
    void inputValueChanged(quint32 universe, quint32 channel, uchar value, const QString& key = 0);

    /** Notify the listeners that the input patch has changed */
    void inputPatchChanged();

    /** Notify the listeners that the output patch has changed */
    void outputPatchChanged();

private:
    /** Reference to the input patch associated to this universe. */
    InputPatch* m_inputPatch;

    /** Reference to the output patch associated to this universe. */
    OutputPatch* m_outputPatch;

    /** Reference to the feedback patch associated to this universe. */
    OutputPatch* m_fbPatch;

    /************************************************************************
     * Channels capabilities and modifiers
     ************************************************************************/
public:
    /**
     * Define the capabilities of a channel in this universe
     *
     * @param channel The channel absolute index in the universe
     * @param group The group this channel belongs to
     * @param isHTP Flag to force HTP/LTP behaviour
     */
    void setChannelCapability(ushort channel, QLCChannel::Group group, ChannelType forcedType = Undefined);

    /** Retrieve the capability mask of the given channel index
     *
     * @param channel The channel absolute index in the universe
     */
    uchar channelCapabilities(ushort channel);

    /** Assign a Channel Modifier to the given channel index
      * $modifier can be NULL if the channel has no modifier */
    void setChannelModifier(ushort channel, ChannelModifier *modifier);

    /** Return the Channel Modifier assigned to the given channel
      * or NULL if none or not valid */
    ChannelModifier *channelModifier(ushort channel);

protected:
    /** An array of each channel's capabilities. This helps to optimize HTP/LTP/Relative checks */
    QByteArray* m_channelsMask;

    /** Vector of pointer to ChannelModifier classes. If not NULL, they will modify
     *  a DMX value right before HTP/LTP check and before being assigned to preGM */
    QVector<ChannelModifier*> m_modifiers;

    /************************************************************************
     * Values
     ************************************************************************/
public:
    /**
     * Reset all values to 0.
     */
    void reset();

    /**
     * Unapplies Grand Master to the given address range and resets values to 0.
     *
     * @param address Starting address
     * @param range Number of channels, starting from address, to reset
     */
    void reset(int address, int range);

    /**
     * Get the current post-Grand-Master values (to be written to output HW)
     * Don't write to the returned array to prevent copying. Not that it would
     * do anything to UniverseArray's internal values, but it would be just
     * pointless waste of CPU time.
     *
     * @return The current values
     */
    const QByteArray* postGMValues() const;

    /**
     * Get the current pre-Grand-Master values (used by functions and everyone
     * else INSIDE QLC). Don't write to the returned array to prevent copying.
     * Not that it would do anything to UniverseArray's internal values, but it
     * would be just pointless waste of CPU time.
     *
     * @return The current values
     */
    const QByteArray preGMValues() const;

    /**
     * Get the current pre-Grand-Master value (used by functions and everyone
     * else INSIDE QLC) at specified address.
     *
     * @return The current value at address
     */
    uchar preGMValue(int address) const;

    /** Set all intensity channel values to zero */
    void zeroIntensityChannels();

    /** Return a list with intesity channels and their values */
    QHash <int, uchar> intensityChannels();

    /** Set all channel relative values to zero */
    void zeroRelativeValues();

protected:
    /**
     * Number of channels used in this universe to optimize the dump to plugins.
     * This is a dynamic counter that can only increase depending on the
     * channels used in this universe starting from when a workspace
     * is loaded
     */
    ushort m_usedChannels;
    /**
     * Total number of channels used in this Universe.
     * This is set only when a Universe is instructed about Fixture
     * channel capabilities. Basically just set once if loading an
     * existing workspace, or several times when adding/removing
     * Fixtures
     */
    ushort m_totalChannels;
    /**
     *  Flag that holds if the total number of channels have changed.
     *  This is used to inform the output patch (if present) how many
     *  channels to expect
     */
    bool m_totalChannelsChanged;
    /** A list of intensity channels to optimize operations on HTP/LTP channels */
    QSet <int> m_intensityChannels;
    /** A list of non-intensity channels to optimize operations on HTP/LTP channels */
    QSet <int> m_nonIntensityChannels;
    /** Array of values BEFORE the Grand Master changes */
    QByteArray* m_preGMValues;
    /** Array of values AFTER the Grand Master changes (applyGM) */
    QByteArray* m_postGMValues;
    /** Array of the last preGM values written before the zeroIntensityChannels call  */
    QByteArray* m_lastPostGMValues;

    QVector<short> m_relativeValues;

    /************************************************************************
     * Writing
     ************************************************************************/
public:
    /**
     * Write a value to a DMX channel, taking Grand Master and HTP into
     * account, if applicable.
     *
     * @param channel The channel number to write to
     * @param value The value to write
     *
     * @return true if successful, otherwise false
     */
    bool write(int channel, uchar value, bool forceLTP = false);

    /**
     * Write a relative value to a DMX channel, taking Grand Master and HTP into
     * account, if applicable.
     *
     * @param channel The channel number to write to
     * @param value The value to write
     *
     * @return true if successful, otherwise false
     */
    bool writeRelative(int channel, uchar value);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:

    /**
     * Load a universe contents from the given XML node.
     *
     * @param root An XML subtree containing the universe contents
     * @return true if the map was loaded successfully, otherwise false
     */
    bool loadXML(const QDomElement& root, int index, InputOutputMap* ioMap);

    /**
     * Save the universe instance into an XML document, under the given
     * XML element (tag).
     *
     * @param doc The master XML document to save to.
     * @param wksp_root The workspace root element
     */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root) const;

    /**
     * Save a plugin custom parameters (if available) into a tag nested
     * to the related Input/Output patch
     *
     * @param doc The master XML document to save to.
     * @param wksp_root The workspace root element
     * @param parameters The map of custom parameters to save
     */
    bool savePluginParametersXML(QDomDocument* doc, QDomElement* wksp_root,
                                 QMap<QString, QVariant>parameters) const;
};

/** @} */

#endif
