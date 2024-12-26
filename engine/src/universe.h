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

#include <QScopedPointer>
#include <QSemaphore>
#include <QByteArray>
#include <QThread>
#include <QSet>

#include "inputpatch.h"
#include "qlcchannel.h"

class QXmlStreamReader;
class QLCInputProfile;
class ChannelModifier;
class InputOutputMap;
class GenericFader;
class QLCIOPlugin;
class GrandMaster;
class OutputPatch;
class InputPatch;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define UNIVERSE_SIZE 512

#define KXMLQLCUniverse             QString("Universe")
#define KXMLQLCUniverseName         QString("Name")
#define KXMLQLCUniverseID           QString("ID")
#define KXMLQLCUniversePassthrough  QString("Passthrough")

#define KXMLQLCUniverseInputPatch    QString("Input")
#define KXMLQLCUniverseOutputPatch   QString("Output")
#define KXMLQLCUniverseFeedbackPatch QString("Feedback")

#define KXMLQLCUniversePlugin           QString("Plugin")
#define KXMLQLCUniverseLine             QString("Line")
#define KXMLQLCUniverseLineUID          QString("UID")
#define KXMLQLCUniverseProfileName      QString("Profile")
#define KXMLQLCUniversePluginParameters QString("PluginParameters")

/** Universe class contains input/output data for one DMX universe
 */
class Universe: public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(Universe)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(quint32 id READ id CONSTANT)
    Q_PROPERTY(bool passthrough READ passthrough WRITE setPassthrough NOTIFY passthroughChanged)
    Q_PROPERTY(InputPatch *inputPatch READ inputPatch NOTIFY inputPatchChanged)
    Q_PROPERTY(int outputPatchesCount READ outputPatchesCount NOTIFY outputPatchesCountChanged)
    Q_PROPERTY(bool hasFeedback READ hasFeedback NOTIFY hasFeedbackChanged)

public:
    /** Construct a new Universe */
    Universe(quint32 id = invalid(), GrandMaster *gm = NULL, QObject *parent = 0);

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

    uchar applyPassthrough(int channel, uchar value);

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

    uchar applyModifiers(int channel, uchar value);
    void updatePostGMValue(int channel);

signals:
    void nameChanged();
    void passthroughChanged();

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

    /** Sets an input patch for this Universe, and connect to it to receive signals */
    bool setInputPatch(QLCIOPlugin *plugin, quint32 input,
                       QLCInputProfile *profile = NULL);

    /** Add/Remove/Replace an output patch on this Universe */
    bool setOutputPatch(QLCIOPlugin *plugin, quint32 output, int index = 0);

    /** Sets a feedback patch for this Universe */
    bool setFeedbackPatch(QLCIOPlugin *plugin, quint32 output);

    /** Flag that indicates if this Universe has a patched feedback line */
    bool hasFeedback() const;

    /**
     * Get the reference to the input plugin associated to this universe.
     * If not present NULL is returned.
     */
    InputPatch *inputPatch() const;

    /**
     * Get the reference to the output plugin associated to this universe.
     * If not present NULL is returned.
     */
    Q_INVOKABLE OutputPatch *outputPatch(int index) const;

    /** Return the number of output patches associated to this Universe */
    int outputPatchesCount() const;

    /**
     * Get the reference to the feedback plugin associated to this universe.
     * If not present NULL is returned.
     */
    OutputPatch *feedbackPatch() const;

    /**
     * This is the actual function that writes data to an output patch
     * A flag indicates if data has changed since previous iteration
     */
    void dumpOutput(const QByteArray& data, bool dataChanged);

    void flushInput();

protected slots:
    /** Slot called every time an input patch sends data */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value, const QString& key = 0);

signals:
    /** Everyone interested in input data should connect to this signal */
    void inputValueChanged(quint32 universe, quint32 channel, uchar value, const QString& key = 0);

    /** Notify the listeners that the input patch has changed */
    void inputPatchChanged();

    /** Notify the listeners that one output patch has changed */
    void outputPatchChanged();

    /** Notify the listeners that the number of output patches has changed */
    void outputPatchesCountChanged();

    /** Notify the listeners that a feedback line has been patched/unpatched */
    void hasFeedbackChanged();

private:
    /** Reference to the input patch associated to this universe. */
    InputPatch *m_inputPatch;

    /** List of references to the output patches associated to this universe. */
    QList<OutputPatch*> m_outputPatchList;

    /** Reference to the feedback patch associated to this universe. */
    OutputPatch *m_fbPatch;

private:
    // Connect to inputPatch's valueChanged signal
    void connectInputPatch();
    // Disconnect from inputPatch's valueChanged signal
    void disconnectInputPatch();

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

    /**
     * Set the default value of a DMX channel to be considered on reset
     *
     * @param channel The channel absolute index in the universe
     * @param value the default DMX value
     */
    void setChannelDefaultValue(ushort channel, uchar value);

    /** Assign a Channel Modifier to the given channel index
      * $modifier can be NULL if the channel has no modifier */
    void setChannelModifier(ushort channel, ChannelModifier *modifier);

    /** Return the Channel Modifier assigned to the given channel
      * or NULL if none or not valid */
    ChannelModifier *channelModifier(ushort channel);

protected:
    /** An array of each channel's capabilities. This helps to optimize HTP/LTP/Relative checks */
    QScopedPointer<QByteArray> m_channelsMask;

    /** Vector of pointer to ChannelModifier classes. If not NULL, they will modify
     *  a DMX value right before HTP/LTP check and before being assigned to preGM */
    QVector<ChannelModifier*> m_modifiers;

    /** Modified channels with the non-modified value at 0.
     *  This is used for ranged initialization operations. */
    QScopedPointer<QByteArray> m_modifiedZeroValues;

    /************************************************************************
     * Faders
     ************************************************************************/
public:
    enum FaderPriority
    {
        Auto = 0,
        Override,
        Flashing, /** Priority to override slider values and running chasers by flash scene */
        SimpleDesk
    };

    /** Request a new GenericFader used to compose the final values of
     *  this Universe. The caller is in charge of adding/removing
     *  FadeChannels and eventually dismiss a fader when no longer needed.
     *  If a fade out transition is needed, this Universe
     *  is in charge of completing it and dismissing the fader. */
    QSharedPointer<GenericFader> requestFader(FaderPriority priority = Auto);

    /** Dismiss a fader requested with requestFader, which is no longer needed */
    void dismissFader(QSharedPointer<GenericFader> fader);

    /** Request a new priority for a fader with the provided intance */
    void requestFaderPriority(QSharedPointer<GenericFader> fader, FaderPriority priority);

    /** Retrieve a modifiable list of the currently active faders */
    QList<QSharedPointer<GenericFader> > faders();

    /** Set every running fader with $functionID as parent,
     *  to the requested pause state */
    void setFaderPause(quint32 functionID, bool enable);

    /** Set a fade out time to every fader of this universe.
     *  This is used from the fadeAndStopAll functionality */
    void setFaderFadeOut(int fadeTime);

public slots:
    void tick();

protected:
    void processFaders();

    /** DMX writer thread worker method */
    void run();

signals:
    void universeWritten(quint32 universeID, const QByteArray& universeData);

protected:
    QSemaphore m_semaphore;

    /** Indicated if the DMX writer worker thread is running */
    bool m_running;

    /** IMPORTANT: this is the list of faders that will compose
     *  the Universe values. The order is very important ! */
    QList<QSharedPointer<GenericFader>> m_faders;

    /** Mutex used to protect the access to the m_faders array */
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex m_fadersMutex;
#else
    QRecursiveMutex m_fadersMutex;
#endif

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
     * Get the current post-Grand-Master value (used by functions and everyone
     * else INSIDE QLC+) at specified address.
     *
     * @return The current value at address
     */
    uchar postGMValue(int address) const;

    /**
     * Get the current post-Grand-Master values (to be written to output HW)
     * Don't write to the returned array to prevent copying. Not that it would
     * do anything to UniverseArray's internal values, but it would be just
     * pointless waste of CPU time.
     *
     * @return The current values
     */
    const QByteArray *postGMValues() const;

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
     * else INSIDE QLC+) at specified address.
     *
     * @return The current value at address
     */
    uchar preGMValue(int address) const;

    /** Set all intensity channel values to zero */
    void zeroIntensityChannels();

    /** Return a list with intensity channels and their values */
    QHash <int, uchar> intensityChannels();

protected:
    void applyPassthroughValues(int address, int range);

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
    QVector<int> m_intensityChannels;
    /** A flag set to know when m_intensityChannelsRanges must be updated */
    bool m_intensityChannelsChanged;
    /**
     * Intensity channels sorted as ranges, to further optimize ranged operations
     * (ie set all to zero)
     */
    QVector<int> m_intensityChannelsRanges;
    /** A list of non-intensity channels to optimize operations on HTP/LTP channels */
    QVector<int> m_nonIntensityChannels;
    /** Array of values BEFORE the Grand Master changes */
    QScopedPointer<QByteArray> m_preGMValues;
    /** Array of values AFTER the Grand Master changes (applyGM) */
    QScopedPointer<QByteArray> m_postGMValues;
    /** Array of the last preGM values written before the zeroIntensityChannels call  */
    QScopedPointer<QByteArray> m_lastPostGMValues;
    /** Array of non-intensity only values */
    QScopedPointer<QByteArray> m_blackoutValues;

    /** Array of values from input line, when passtrhough is enabled */
    QScopedPointer<QByteArray> m_passthroughValues;

    /* impl speedup */
    void updateIntensityChannelsRanges();

    /************************************************************************
     * Blend mode
     ************************************************************************/
public:
    enum BlendMode {
        NormalBlend = 0,
        MaskBlend,
        AdditiveBlend,
        SubtractiveBlend
    };

    /** Return a blend mode from a string */
    static BlendMode stringToBlendMode(QString mode);

    /** Return a string from a blend mode, to be saved into a XML */
    static QString blendModeToString(BlendMode mode);

    /************************************************************************
     * Writing
     ************************************************************************/
public:
    /**
     * Write a value to a DMX channel, taking Grand Master and HTP into
     * account, if applicable.
     *
     * @param address The DMX start address to write to
     * @param value The value to write
     *
     * @return true if successful, otherwise false
     */
    bool write(int address, uchar value, bool forceLTP = false);

    /**
     * Write a value representing one or multiple channels
     *
     * @param address The DMX start address to write to
     * @param value the DMX value(s) to set
     * @param channelCount number of channels that value represents
     * @return always true
     */
    bool writeMultiple(int address, quint32 value, int channelCount);

    /**
     * Write a relative value to a DMX channel, taking Grand Master and HTP into
     * account, if applicable.
     *
     * @param address The DMX start address to write to
     * @param value The value to write
     * @param channelCount number of channels that value represents
     *
     * @return true if successful, otherwise false
     */
    bool writeRelative(int address, quint32 value, int channelCount);

    /**
     * Write DMX values with the given blend mode.
     * If blend == NormalBlend the generic write method is called
     * and all the HTP/LTP checks are performed
     *
     * @param address The DMX start address to write to
     * @param value The value to write
     * @param channelCount The number of channels that value represents
     * @param blend The blend mode to be used on $value
     *
     * @return true if successful, otherwise false
     */
    bool writeBlended(int address, quint32 value, int channelCount, BlendMode blend);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:

    enum PatchTagType { InputPatchTag, OutputPatchTag, FeedbackPatchTag };

    /**
     * Load a universe contents from the given XML node.
     *
     * @param root An XML subtree containing the universe contents
     * @param index The QLC+ Universe index
     * @param ioMap Reference to the QLC+ Input/Output map class
     * @return true if the Universe was loaded successfully, otherwise false
     */
    bool loadXML(QXmlStreamReader &root, int index, InputOutputMap *ioMap);

    /**
     * Load an optional tag defining the plugin specific parameters
     * @param root An XML subtree containing the plugin parameters contents
     * @param currentTag The type of Patch where the parameters should be set
     * @param patchIndex Index of the patch to configure (ATM used only for output)
     * @return true if the parameters were loaded successfully, otherwise false
     */
    bool loadXMLPluginParameters(QXmlStreamReader &root, PatchTagType currentTag, int patchIndex);

    /**
     * Save the universe instance into an XML document, under the given
     * XML element (tag).
     *
     * @param doc The master XML document to save to.
     * @param wksp_root The workspace root element
     */
    bool saveXML(QXmlStreamWriter *doc) const;

    /**
     * Save one patch (input/output/feedback)
     *
     * @param doc
     * @param tag
     * @param pluginName
     * @param line
     * @param profileName
     * @param parameters
     */
    void savePatchXML(QXmlStreamWriter *doc,
        QString const & tag,
        QString const & pluginName, const QString &lineName,
        quint32 line,
        QString profileName,
        QMap<QString, QVariant>parameters) const;

    /**
     * Save a plugin custom parameters (if available) into a tag nested
     * to the related Input/Output patch
     *
     * @param doc The master XML document to save to.
     * @param wksp_root The workspace root element
     * @param parameters The map of custom parameters to save
     */
    bool savePluginParametersXML(QXmlStreamWriter *doc,
                                 QMap<QString, QVariant>parameters) const;
};

/** @} */

#endif
