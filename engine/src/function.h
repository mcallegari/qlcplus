/*
  Q Light Controller
  function.h

  Copyright (C) 2004 Heikki Junnila

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

class QDomDocument;
class QDomElement;

class GenericFader;
class MasterTimer;
class Function;
class Universe;
class Doc;

class FunctionUiState;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCFunction "Function"
#define KXMLQLCFunctionName "Name"
#define KXMLQLCFunctionID "ID"
#define KXMLQLCFunctionType "Type"
#define KXMLQLCFunctionData "Data"
#define KXMLQLCFunctionPath "Path"

#define KXMLQLCFunctionValue "Value"
#define KXMLQLCFunctionValueType "Type"
#define KXMLQLCFunctionChannel "Channel"

#define KXMLQLCFunctionStep "Step"
#define KXMLQLCFunctionNumber "Number"

#define KXMLQLCFunctionDirection "Direction"
#define KXMLQLCFunctionRunOrder "RunOrder"

#define KXMLQLCFunctionEnabled "Enabled"

#define KXMLQLCFunctionSpeed         "Speed"
#define KXMLQLCFunctionSpeedFadeIn   "FadeIn"
#define KXMLQLCFunctionSpeedHold     "Hold"
#define KXMLQLCFunctionSpeedFadeOut  "FadeOut"
#define KXMLQLCFunctionSpeedDuration "Duration"

typedef struct
{
    QString name;
    qreal value;
} Attribute;

class Function : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Function)

public:
    /**
     * All known function types.
     * This is a bit mask to facilitate easy AND-mode type filtering
     */
    enum Type
    {
        Undefined  = 0,
        Scene      = 1 << 0,
        Chaser     = 1 << 1,
        EFX        = 1 << 2,
        Collection = 1 << 3,
        Script     = 1 << 4,
        RGBMatrix  = 1 << 5,
        Show       = 1 << 6,
        Audio      = 1 << 7
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        , Video    = 1 << 8
#endif
    };

    /**
     * Common attributes
     */
    enum Attr
    {
        Intensity = 0,
    };

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
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

    /** Signal telling that the name of this function have changed */
    void nameChanged(quint32 fid);

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
    virtual Function* createCopy(Doc* doc, bool addToDoc = true) = 0;

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

    /**
     * Convert a type to an icon
     *
     * @param type The type to convert
     */
    static QIcon typeToIcon(Function::Type type);

private:
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
     * Common XML
     *********************************************************************/
protected:
    /** Save function's common attributes in $doc, under $root */
    bool saveXMLCommon(QDomElement* root) const;

    /*********************************************************************
     * Running order
     *********************************************************************/
public:
    enum RunOrder { Loop, SingleShot, PingPong, Random };

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
    /** Save function's running order in $doc, under $root */
    bool saveXMLRunOrder(QDomDocument* doc, QDomElement* root) const;

    /** Load function's direction from $root */
    bool loadXMLRunOrder(const QDomElement& root);

private:
    RunOrder m_runOrder;

    /*********************************************************************
     * Direction
     *********************************************************************/
public:
    enum Direction { Forward, Backward };

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
    /** Save function's direction in $doc, under $root */
    bool saveXMLDirection(QDomDocument* doc, QDomElement* root) const;

    /** Load function's direction from $root */
    bool loadXMLDirection(const QDomElement& root);

private:
    Direction m_direction;

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

protected:
    /** Load the contents of a speed node */
    bool loadXMLSpeed(const QDomElement& speedRoot);

    /** Save function's speed values under the given $root element in $doc */
    bool saveXMLSpeed(QDomDocument* doc, QDomElement* root) const;

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
    FunctionUiState * uiState();
    const FunctionUiState * uiState() const;

private:
    virtual FunctionUiState * createUiState();

private:
    FunctionUiState * m_uiState;

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
     * @param wksp_root A QLC workspace XML root node to save under
     */
    virtual bool saveXML(QDomDocument* doc, QDomElement* wksp_root) = 0;

    /**
     * Read this function's contents from an XML document
     *
     * @param doc An XML document to load from
     * @param root An XML root element of a function
     */
    virtual bool loadXML(const QDomElement& root) = 0;

    /**
     * Load a new function from an XML tag and add it to the given doc
     * object, if loading was successful.
     *
     * @param root An XML root element of a function
     * @param doc The QLC document object, that owns all functions
     * @return true if successful, otherwise false
     */
    static bool loader(const QDomElement& root, Doc* doc);

    /**
     * Called for each Function-based object after everything has been loaded.
     * Do any post-load cleanup, function mappings etc. if needed. Default
     * implementation does nothing.
     */
    virtual void postLoad();

    /*********************************************************************
     * Flash
     *********************************************************************/
public:
    /** Flash the function */
    virtual void flash(MasterTimer* timer);

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
    virtual void write(MasterTimer* timer, QList<Universe*> universes) = 0;

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

protected:
    /** Reset elapsed timer ticks to zero */
    void resetElapsed();

    /** Increment the elapsed timer ticks by one */
    void incrementElapsed();

private:
    quint32 m_elapsed;

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
     */
    void start(MasterTimer* timer, bool child = false, quint32 startTime = 0,
               uint overrideFadeIn = defaultSpeed(),
               uint overrideFadeOut = defaultSpeed(),
               uint overrideDuration = defaultSpeed());

	/**
     * Check, whether the function was started by another function i.e.
     * as the other function's child.
     *
	 * @return true If the function was started by another function.
     *              Otherwise false.
	 */
    bool startedAsChild() const;

    /**
     * Mark the function to be stopped ASAP. MasterTimer will stop running
     * the function on the next pass after this method has been called.
     * There is no way to cancel it, but the function can be started again
     * normally.
     */
    void stop();

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

private:
    /** Stop flag, private to keep functions from modifying it. */
    bool m_stop;
    bool m_running;

    QMutex m_stopMutex;
    QWaitCondition m_functionStopped;

    /*************************************************************************
     * Intensity
     *************************************************************************/
public:
    /**
     * Register a new attribute for this function.
     * If the attribute already exists, it will be overwritten.
     *
     * @param name The attribute name
     * @param value The attribute initial value
     */
    int registerAttribute(QString name, qreal value = 1.0);

    /**
     * Unregister a previously created attribute for this function.
     * If the attribute doesn't exist, false will be returned.
     *
     * @param name The attribute name
     */
    bool unregisterAttribute(QString name);

    /**
     * Rename an existing atribute
     *
     * @param idx the attribute index
     * @param newName the new name for the attribute
     * @return true on success, false on error
     */
    bool renameAttribute(int idx, QString newName);

    /**
     * Adjust the intensity of the function by a fraction.
     *
     * @param fraction Intensity as a fraction (0.0 - 1.0)
     */
    virtual void adjustAttribute(qreal fraction, int attributeIndex);

    /**
     * Reset intensity to the default value (1.0).
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
     * Get the function's attributes
     *
     * @return a list of Attributes
     */
    QList <Attribute> attributes();

signals:
    /** Informs that an attribute of the function has changed */
    void attributeChanged(int index, qreal fraction);

private:
    bool m_startedAsChild;
    //qreal m_intensity;
    QList <Attribute> m_attributes;
};

/** @} */

#endif
