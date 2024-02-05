/*
  Q Light Controller Plus
  function.h

  Copyright (C) 2004 Heikki Junnila
                     Massimo Callegari

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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <QWaitCondition>
#include <QObject>
#include <QString>
#include <QMutex>
#include <QList>
#include <QIcon>
#include <QMap>

#include "universe.h"
#include "functionparent.h"

class QXmlStreamReader;

class GenericFader;
class MasterTimer;
class Function;
class Doc;

class FunctionUiState;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCFunction QString("Function")
#define KXMLQLCFunctionName QString("Name")
#define KXMLQLCFunctionID QString("ID")
#define KXMLQLCFunctionType QString("Type")
#define KXMLQLCFunctionData QString("Data")
#define KXMLQLCFunctionPath QString("Path")
#define KXMLQLCFunctionHidden QString("Hidden")
#define KXMLQLCFunctionBlendMode QString("BlendMode")

#define KXMLQLCFunctionValue QString("Value")
#define KXMLQLCFunctionValueType QString("Type")
#define KXMLQLCFunctionChannel QString("Channel")

#define KXMLQLCFunctionStep QString("Step")
#define KXMLQLCFunctionNumber QString("Number")

#define KXMLQLCFunctionDirection QString("Direction")
#define KXMLQLCFunctionRunOrder QString("RunOrder")

#define KXMLQLCFunctionEnabled QString("Enabled")

#define KXMLQLCFunctionSpeed         QString("Speed")
#define KXMLQLCFunctionSpeedFadeIn   QString("FadeIn")
#define KXMLQLCFunctionSpeedHold     QString("Hold")
#define KXMLQLCFunctionSpeedFadeOut  QString("FadeOut")
#define KXMLQLCFunctionSpeedDuration QString("Duration")

typedef struct
{
    QString m_name;
    qreal m_value;
    qreal m_min;
    qreal m_max;
    int m_flags;
    bool m_isOverridden;
    qreal m_overrideValue;
} Attribute;

typedef struct
{
    int m_attrIndex;
    qreal m_value;
} AttributeOverride;

class Function : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Function)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(quint32 id READ id CONSTANT)
    Q_PROPERTY(Type type READ type CONSTANT)
    Q_PROPERTY(quint32 totalDuration READ totalDuration WRITE setTotalDuration NOTIFY totalDurationChanged)
    Q_PROPERTY(RunOrder runOrder READ runOrder WRITE setRunOrder NOTIFY runOrderChanged)
    Q_PROPERTY(TempoType tempoType READ tempoType WRITE setTempoType NOTIFY tempoTypeChanged FINAL)

public:
    /**
     * All known function types.
     * This is a bit mask to facilitate easy AND-mode type filtering
     */
    enum Type
    {
        Undefined      = 0,
        SceneType      = 1 << 0,
        ChaserType     = 1 << 1,
        EFXType        = 1 << 2,
        CollectionType = 1 << 3,
        ScriptType     = 1 << 4,
        RGBMatrixType  = 1 << 5,
        ShowType       = 1 << 6,
        SequenceType   = 1 << 7,
        AudioType      = 1 << 8
#if QT_VERSION >= 0x050000
        , VideoType    = 1 << 9
#endif
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(Type)
#endif

    /**
     * Common attributes
     */
    enum Attr
    {
        Intensity = 0
    };

public:
    enum PropType { Name = 0, FadeIn, Hold, FadeOut, Duration, Notes };
#if QT_VERSION >= 0x050500
    Q_ENUM(PropType)
#endif

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Create a new function instance with the given QObject parent. */
    Function(QObject* parent = 0);

    /**
     * Create a new function
     *
     * @param doc The parent object that owns this function (Doc)
     * @param t The function type (see enum Type)
     */
    Function(Doc* doc, Type t);

    /**
     * Destroy this function
     */
    virtual ~Function();

    /** Get the parent Doc object */
    Doc* doc() const;

signals:
    /** Signal telling that the contents of this function have changed */
    void changed(quint32 fid);

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /**
     * Create a copy of a function to the given doc object and return the
     * newly-created function, unless an error has occurred.
     *
     * @param doc The QLC Doc object that owns all functions
     * @param addToDoc enable/disable addition of the function copy to Doc
     * @return The newly-created function or NULL in case of an error
     */
    virtual Function* createCopy(Doc* doc, bool addToDoc = true);

    /**
     * Copy this function's contents from the given function. Finally emits
     * the changed() signal so don't emit it from subclasses' copyFrom()
     * implementations, but call this one just before return.
     *
     * @param function The function to copy from
     * @return true if successful, otherwise false
     */
    virtual bool copyFrom(const Function* function);

    /*********************************************************************
     * ID
     *********************************************************************/
public:
    /**
     * Set this function's unique ID
     *
     * @param id This function's unique ID
     */
    void setID(quint32 id);

    /**
     * Get this function's unique ID
     */
    quint32 id() const;

    /**
     * Get the value for an invalid function ID (for comparison etc.)
     */
    static quint32 invalidId();

private:
    quint32 m_id;

    /*********************************************************************
     * Name
     *********************************************************************/
public:
    /**
     * Set a name for this function
     *
     * @param name The function's new name
     */
    void setName(const QString& name);

    /**
     * Return the name of this function
     */
    QString name() const;

signals:
    /** Signal telling that the name of this function have changed */
    void nameChanged(quint32 fid);

private:
    QString m_name;

    /*********************************************************************
     * Type
     *********************************************************************/
public:
    /**
     * Return the type of this function (see the enum above)
     */
    Type type() const;

    /**
     * Return the type of this function as a string
     */
    QString typeString() const;

    /**
     * Convert a type to string
     *
     * @param type The type to convert
     */
    static QString typeToString(Function::Type type);

    /**
     * Convert a string to Type
     *
     * @param str The string to convert
     */
    static Type stringToType(const QString& str);

    /** Virtual method to retrieve a QIcon based on a Function type.
      * Subclasses should reimplement this */
    virtual QIcon getIcon() const;

protected:
    Type m_type;

    /*********************************************************************
     * Path
     *********************************************************************/
public:
    /** Set the function path for folder grouping */
    void setPath(QString path);

    /** Retrieve the currently set path */
    QString path(bool simplified = false) const;

private:
    QString m_path;

    /*********************************************************************
     * Visibility
     *********************************************************************/
public:
    /** Set the function visibility status. Hidden Functions will not be displayed in the UI */
    void setVisible(bool visible);

    /** Retrieve the current visibility status */
    bool isVisible() const;

private:
    bool m_visible;

    /*********************************************************************
     * Common XML
     *********************************************************************/
protected:
    /** Save function's common attributes in $doc, under $root */
    bool saveXMLCommon(QXmlStreamWriter *doc) const;

    /*********************************************************************
     * Running order
     *********************************************************************/
public:
    enum RunOrder { Loop = 0, SingleShot, PingPong, Random };
#if QT_VERSION >= 0x050500
    Q_ENUM(RunOrder)
#endif

public:
    /**
     * Set this function's running order
     *
     * @param dir This function's running order
     */
    void setRunOrder(const Function::RunOrder& order);

    /**
     * Get this function's running order
     */
    Function::RunOrder runOrder() const;

    /**
     * Convert a RunOrder to string
     *
     * @param order RunOrder to convert
     */
    static QString runOrderToString(const Function::RunOrder& order);

    /**
     * Convert a string to RunOrder
     *
     * @param str The string to convert
     */
    static Function::RunOrder stringToRunOrder(const QString& str);

protected:
    /** Save function's running order in $doc */
    bool saveXMLRunOrder(QXmlStreamWriter *doc) const;

    /** Load function's direction from $root */
    bool loadXMLRunOrder(QXmlStreamReader &root);

signals:
    void runOrderChanged();

private:
    RunOrder m_runOrder;

    /*********************************************************************
     * Direction
     *********************************************************************/
public:
    enum Direction { Forward = 0, Backward };
#if QT_VERSION >= 0x050500
    Q_ENUM(Direction)
#endif

public:
    /**
     * Set this function's direction
     *
     * @param dir This function's direction
     */
    void setDirection(const Function::Direction& dir);

    /**
     * Get this function's direction
     */
    Function::Direction direction() const;

    /**
     * Convert a Direction to a string
     *
     * @param dir Direction to convert
     */
    static QString directionToString(const Function::Direction& dir);

    /**
     * Convert a string to Direction
     *
     * @param str The string to convert
     */
    static Function::Direction stringToDirection(const QString& str);

protected:
    /** Save function's direction in $doc */
    bool saveXMLDirection(QXmlStreamWriter *doc) const;

    /** Load function's direction from $root */
    bool loadXMLDirection(QXmlStreamReader &root);

private:
    Direction m_direction;

    /*********************************************************************
     * Tempo type
     *********************************************************************/
public:
    enum TempoType
    {
        Original = -1,
        Time = 0,
        Beats = 1
    };
    enum FractionsType
    {
        NoFractions = 0,
        ByTwoFractions,
        AllFractions
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(TempoType)
    Q_ENUM(FractionsType)
#endif

public:
    /**
     * Set the speed type of this function.
     * When switching from a type to another, the current fade in, hold, fade out
     * and duration times will be converted to the new type.
     *
     * @param type the speed type
     */
    void setTempoType(const Function::TempoType& type);

    /**
     * Get the Function current speed type
     */
    Function::TempoType tempoType() const;

    /**
     * Convert a tempo type to a string
     *
     * @param type Tempo type to convert
     */
    static QString tempoTypeToString(const Function::TempoType& type);

    /**
     * Convert a string to a tempo type
     *
     * @param str The string to convert
     */
    static Function::TempoType stringToTempoType(const QString& str);

    /** Convert a time value in milliseconds to a beat value */
    static uint timeToBeats(uint time, int beatDuration);

    /** Convert a beat value to a time value in milliseconds */
    static uint beatsToTime(uint beats, int beatDuration);

    /** Get the override speed type (done by a Chaser) */
    TempoType overrideTempoType() const;

    /** Set the override speed type (done by a Chaser) */
    void setOverrideTempoType(TempoType type);

signals:
    void tempoTypeChanged();

protected slots:
    /**
     * This slot is connected to the Master Timer and it is invoked
     * when this Function is in 'Beats' tempo type and the BPM
     * number changed. Subclasses should reimplement this.
     */
    virtual void slotBPMChanged(int bpmNumber);

private:
    TempoType m_tempoType;
    TempoType m_overrideTempoType;
    bool m_beatResyncNeeded;

    /*********************************************************************
     * Speed
     *********************************************************************/
public:
    /** Set the fade in time in milliseconds */
    void setFadeInSpeed(uint ms);

    /** Get the fade in time in milliseconds */
    uint fadeInSpeed() const;

    /** Set the fade out time in milliseconds */
    void setFadeOutSpeed(uint ms);

    /** Get the fade out time in milliseconds */
    uint fadeOutSpeed() const;

    /** Set the duration in milliseconds */
    virtual void setDuration(uint ms);

    /** Get the duration in milliseconds */
    uint duration() const;

    /** Get the total duration in milliseconds.
     *  This differs from duration as it considers
     *  the steps or the specific Function parameters */
    virtual quint32 totalDuration();

    /** Set the total duration in milliseconds.
     *  This method should be reimplemented only
     *  by Functions supporting the stretch functionality */
    virtual void setTotalDuration(quint32 msec);

    /** Set the override fade in speed (done by chaser in Common speed mode) */
    void setOverrideFadeInSpeed(uint ms);

    /** Get the override fade in speed */
    uint overrideFadeInSpeed() const;

    /** Set the override fade out speed (done by chaser in Common speed mode) */
    void setOverrideFadeOutSpeed(uint ms);

    /** Get the override fade out speed */
    uint overrideFadeOutSpeed() const;

    /** Set the override duration */
    void setOverrideDuration(uint ms);

    /** Get the override duration */
    uint overrideDuration() const;

    /** Tell the function that it has been "tapped". Default implementation does nothing. */
    virtual void tap();

    static uint defaultSpeed();
    static uint infiniteSpeed();

    /** Pretty-print the given speed into a QString */
    static QString speedToString(uint ms);

    /** returns value in msec of a string created by speedToString */
    static uint stringToSpeed(QString speed);

    /** Safe speed operations */
    static uint speedNormalize(uint speed);
    static uint speedAdd(uint left, uint right);
    static uint speedSubtract(uint left, uint right);

signals:
    void totalDurationChanged();

protected:
    /** Load the contents of a speed node */
    bool loadXMLSpeed(QXmlStreamReader &speedRoot);

    /** Save function's speed values in $doc */
    bool saveXMLSpeed(QXmlStreamWriter *doc) const;

private:
    uint m_fadeInSpeed;
    uint m_fadeOutSpeed;
    uint m_duration;

    uint m_overrideFadeInSpeed;
    uint m_overrideFadeOutSpeed;
    uint m_overrideDuration;

    /*********************************************************************
     * UI State
     *********************************************************************/
public:
    /** Get/Set a generic UI property specific to this Function */
    QVariant uiStateValue(QString property);
    void setUiStateValue(QString property, QVariant value);

    /** Get the whole UI state map */
    QMap<QString, QVariant> uiStateMap() const;

private:
    /** A generic map to temporary store key/value
     *  pairs that the UI can use to remember how an editor
     *  was configured. Note: this is not saved into XML */
    QMap<QString, QVariant> m_uiState;

    /*********************************************************************
     * Fixtures
     *********************************************************************/
public slots:
    /** Slot that captures Doc::fixtureRemoved signals */
    virtual void slotFixtureRemoved(quint32 fxi_id);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Save this function to an XML document
     *
     * @param doc The XML document to save to
     */
    virtual bool saveXML(QXmlStreamWriter *doc);

    /**
     * Read this function's contents from an XML document
     *
     * @param doc An XML document to load from
     * @param root An XML root element of a function
     */
    virtual bool loadXML(QXmlStreamReader &root);

    /**
     * Load a new function from an XML tag and add it to the given doc
     * object, if loading was successful.
     *
     * @param root An XML root element of a function
     * @param doc The QLC document object, that owns all functions
     * @return true if successful, otherwise false
     */
    static bool loader(QXmlStreamReader &root, Doc* doc);

    /**
     * Called for each Function-based object after everything has been loaded.
     * Do any post-load cleanup, function mappings etc. if needed. Default
     * implementation does nothing.
     */
    virtual void postLoad();

    /**
     * Check if a Function ID is included/controlled by this Function.
     * Subclasses should reimplement this.
     */
    virtual bool contains(quint32 functionId);

    /**
     * Return a list of components such as Functions/Fixtures with unique IDs.
     * Subclasses should reimplement this.
     */
    virtual QList<quint32> components();

    /*********************************************************************
     * Flash
     *********************************************************************/
public:
    /** Flash the function */
    virtual void flash(MasterTimer* timer, bool shouldOverride, bool forceLTP);

    /** UnFlash the function */
    virtual void unFlash(MasterTimer* timer);

    /** Check, whether the function is flashing */
    virtual bool flashing() const;

signals:
    /**
     * Tells listeners that this function is flashing or that is just
     * stopped flashing.
     *
     * @param fid The flashing function's ID
     * @param state true if the function flashing, false if the function
     *              just stopped flashing
     */
    void flashing(quint32 fid, bool state);

private:
    bool m_flashing;

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /**
     * Called by MasterTimer when the function is started. MasterTimer's
     * function list mutex is locked during this call, so functions must
     * not attempt to start/stop additional functions from their preRun()
     * methods because it would result in a deadlock.
     *
     * @param timer The MasterTimer instance that takes care of running
     *              the function in correct intervals.
     */
    virtual void preRun(MasterTimer* timer);

    /**
     * Write next values to universes. This method is called periodically
     * by the MasterTimer instance that the
     * function has been set to run in, to write the next values to the
     * given DMX universe buffer. This method is called for each function
     * in the order that they were set to run. When this method has been
     * called for each running function, the buffer is written to OutputMap.
     *
     * MasterTimer calls this method for each function to get their DMX
     * data for the given array of universes. This method will be called
     * for each running function until Function::stopped() returns true.
     *
     * @param timer The MasterTimer that is running the function
     * @param universes The DMX universe buffer to write values into
     */
    virtual void write(MasterTimer* timer, QList<Universe*> universes);

    /**
     * Called by MasterTimer when the function is stopped. No more write()
     * calls will arrive to the function after this call. The function may
     * still write its last data packet to $universes during this call or
     * hand over channel zero-fading to $timer's generic fader.
     *
     * MasterTimer's function list mutex is locked during this call, so
     * functions must not attempt to start/stop additional functions from
     * their postRun() method as it would result in a deadlock situation.
     *
     * @param timer The MasterTimer that has stopped running the function
     * @param universes Universe buffer to write the function's exit data
     */
    virtual void postRun(MasterTimer* timer, QList<Universe*> universes);

protected:
    /** Helper method to dismiss all the faders previously added to
     *  m_fadersMap. This is usually called on Function postRun when
     *  no fade out is requested */
    virtual void dismissAllFaders();

signals:
    /**
     * Emitted when a function is started (i.e. added to MasterTimer's
     * list of running functions).
     *
     * @param id The ID of the started function
     */
    void running(quint32 id);

    /**
     * Emitted when a function is really finished (i.e. removed from
     * MasterTimer's list of running functions).
     *
     * @param id The ID of the stopped function
     */
    void stopped(quint32 id);

protected:
    /** Map used to lookup a GenericFader instance for a Universe ID */
    QMap<quint32, QSharedPointer <GenericFader> > m_fadersMap;

    /*********************************************************************
     * Elapsed
     *********************************************************************/
public:
    /**
     * Get number of elapsed ticks for this function (0 unless the function
     * is running).
     *
     * @return Number of elapsed timer ticks since the function was started
     */
    quint32 elapsed() const;

    quint32 elapsedBeats() const;

protected:
    /** Reset elapsed timer ticks to zero */
    void resetElapsed();

    /** Increment the elapsed timer ticks by one */
    void incrementElapsed();

    /** Increment the elapsed beats by one */
    void incrementElapsedBeats();

    void roundElapsed(quint32 roundTime);

private:
    /* The elapsed time in ms when tempoType is Time */
    quint32 m_elapsed;
    /* The elapsed beats when tempoType is Beats */
    quint32 m_elapsedBeats;

    /*********************************************************************
     * Start & Stop
     *********************************************************************/
public:
    /**
     * Start running the function in the given MasterTimer instance.
     *
     * @param timer The MasterTimer that should run the function
     * @param child Use true if called from another function
     * @param overrideFadeIn Override the function's default fade in speed
     * @param overrideFadeOut Override the function's default fade out speed
     * @param overrideDuration Override the function's default duration
     * @param overrideTempoType Override the tempo type of the function
     */
    void start(MasterTimer* timer, FunctionParent parent, quint32 startTime = 0,
               uint overrideFadeIn = defaultSpeed(),
               uint overrideFadeOut = defaultSpeed(),
               uint overrideDuration = defaultSpeed(),
               TempoType overrideTempoType = Original);

    /**
     * Pause a running Function. Subclasses should check the paused state
     * immediately in the write call and, in case, return, to avoid performing
     * any action and moreover to increment the elapsed time.
     * This is declared as virtual for those subclasses where the actual
     * Function progress is handled in a separate thread, like multimedia
     * functions, or where the Function handles a few children Functions.
     */
    virtual void setPause(bool enable);

    /**
     * Mark the function to be stopped ASAP. MasterTimer will stop running
     * the function on the next pass after this method has been called.
     * There is no way to cancel it, but the function can be started again
     * normally.
     */
    void stop(FunctionParent parent, bool preserveAttributes = false);

    /**
     * Check, whether the function should be stopped ASAP. Functions can use this
     * to check, whether they should continue running or bail out.
     *
     * @return true if the function should be stopped, otherwise false.
     */
    bool stopped() const;

    /**
     * Mark the function to be stopped and block the calling thread until it is
     * actually stopped. To prevent deadlocks the function only waits for 2s.
     *
     * @return true if the function was stopped. false if the function did not
     *              stop withing two seconds
     */
    bool stopAndWait();

    /**
     * Check, whether the function is currently running (preRun() has been run)
     * or not (postRun() has been completed). This should be used only from MasterTimer.
     *
     * @return true if function is running, false if not
     */
    bool isRunning() const;

    /**
     * Check if the function is currently in a paused state. This is invoked
     * by subclasses to understand if they have to do something during the
     * write call
     */
    bool isPaused() const;

    bool startedAsChild() const;

private:
    /** Running state flags. The rules are:
     *  - m_stop resets also m_paused
     *  - m_paused and m_running can be both true
     *  - if m_paused is true, a start(...) call will just reset it to false
     *  These are private to prevent Functions from modifying them. */
    bool m_stop;
    bool m_running;
    bool m_paused;

    QList<FunctionParent> m_sources;
    QMutex m_sourcesMutex;

    QMutex m_stopMutex;
    QWaitCondition m_functionStopped;

    /*************************************************************************
     * Attributes
     *************************************************************************/
public:
    enum OverrideFlags
    {
        Multiply = (1 << 0), /** The original attribute value should be multiplied by the overridden values */
        LastWins = (1 << 1), /** The original attribute value is overridden by the last requested override value */
        Single   = (1 << 2)  /** Only one attribute override ID will be allowed */
    };

    static int invalidAttributeId();

    /**
     * Register a new attribute for this function.
     * If the attribute already exists, it will be overwritten.
     *
     * @param name The attribute name
     * @param min The attribute minimum value
     * @param max The attribute maximum value
     * @param value The attribute initial value
     */
    int registerAttribute(QString name, int flags = Multiply, qreal min = 0.0, qreal max = 1.0, qreal value = 1.0);

    /**
     * Request a new attribute override ID. A Function will always return a new ID,
     * that the caller can use in the adjustAttribute method.
     *
     * @param attributeIndex the attribute index that will be overridden by the caller
     * @param value an initial override value
     *
     * @return an override ID to be used to adjust the overridden value,
     *         or -1 if the specified $attributeIndex is not valid
     */
    int requestAttributeOverride(int attributeIndex, qreal value = 1.0);

    /**
     * Release an attribute override no longer needed
     *
     * @param attributeId and ID previously acquired with requestAttributeOverride
     */
    void releaseAttributeOverride(int attributeId);

    /**
     * Unregister a previously created attribute for this function.
     * If the attribute doesn't exist, false will be returned.
     *
     * @param name The attribute name
     */
    bool unregisterAttribute(QString name);

    /**
     * Rename an existing attribute
     *
     * @param idx the attribute index
     * @param newName the new name for the attribute
     * @return true on success, false on error
     */
    bool renameAttribute(int idx, QString newName);

    /**
     * Adjust an attribute value with the given new $value.
     * If $attributeId is within the registered attributes range,
     * the oiginal attribute value will be changed.
     * Warning: only Function editors or the Function itself should do this !
     * Otherwise, if $attributeId is >= OVERRIDE_ATTRIBUTE_START_ID
     * it means the caller wants to control a value override.
     * This operation will then recalculate the final override value
     * according to the original attribute flags
     *
     * @param value the new attribute value
     * @param attributeId the ID of the attribute to control
     *
     * @return the original attribute index or -1 on error
     */
    virtual int adjustAttribute(qreal value, int attributeId);

    /**
     * Reset the overridden attributes, while keeping
     * the original attribute values unchanged
     */
    void resetAttributes();

    /**
     * Get a specific function attribute by index
     *
     * @param attributeIndex the attribute index
     * @return the requested attribute value (on error return 0.0)
     */
    qreal getAttributeValue(int attributeIndex) const;

    /**
     * Get a specific function attribute by name
     *
     * @param name the attribute name
     * @return the requested attribute index
     */
    int getAttributeIndex(QString name) const;

    /**
     * Get a list of the registered function's attributes
     *
     * @return a list of Attributes
     */
    QList <Attribute> attributes() const;

protected:
    /**
     * Mark an attribute with the given $attributeIndex as overridden, and
     * calculates the final override value according to the registered attribute flags
     *
     * @param attributeIndex the attribute index
     */
    void calculateOverrideValue(int attributeIndex);

signals:
    /** Notify the listeners that an attribute has changed */
    void attributeChanged(int index, qreal fraction);

private:
    /** A list of the registered attributes */
    QList <Attribute> m_attributes;

    /** A map of the overridden attributes */
    QMap <int, AttributeOverride> m_overrideMap;

    /** Last assigned override ID */
    int m_lastOverrideAttributeId;

    /** Flag to preserve or discard attributes on stop calls */
    bool m_preserveAttributes;

    /*************************************************************************
     * Blend mode
     *************************************************************************/
public:
    /**
     * Set a specific blend mode to be used by this Function
     * For now this is used only by RGBMatrix but it's been
     * placed here for generic usage
     */
    virtual void setBlendMode(Universe::BlendMode mode);

    /**
     * Return the blend mode set on this Function
     */
    virtual Universe::BlendMode blendMode() const;

private:
    Universe::BlendMode m_blendMode;
};

/** @} */

#endif
