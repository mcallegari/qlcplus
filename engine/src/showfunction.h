/*
  Q Light Controller Plus
  showfunction.h

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

#ifndef SHOWFUNCTION_H
#define SHOWFUNCTION_H

#include <QColor>

#include "function.h"

class QXmlStreamReader;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLShowFunction QString("ShowFunction")
#define KXMLShowFunctionUid QString("UID")
#define KXMLShowFunctionTrackId QString("TrackID")

class ShowFunction: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ShowFunction)

    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(int functionID READ functionID WRITE setFunctionID NOTIFY functionIDChanged)
    Q_PROPERTY(int startTime READ startTime WRITE setStartTime NOTIFY startTimeChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool locked READ isLocked WRITE setLocked NOTIFY lockedChanged)

public:
    ShowFunction(quint32 id, QObject *parent = 0);
    virtual ~ShowFunction() {}

    /** Get the ShowFunction unique identifier in a Show */
    quint32 id() const;

    /** Get/Set the Function ID this class represents */
    void setFunctionID(quint32 id);
    quint32 functionID() const;

    /** Get/Set the Function start time over a Show timeline */
    void setStartTime(quint32 time);
    quint32 startTime() const;

    /** Get/Set this item duration, not necessarily corresponding
     *  to the original Function duration */
    void setDuration(quint32 duration);
    quint32 duration() const;
    quint32 duration(const Doc *doc) const;

    /** Get/Set the color of the item when rendered in the Show Manager */
    void setColor(QColor color);
    QColor color() const;

    static QColor defaultColor(Function::Type type);

    /** Get/Set the lock state of this ShowFunction */
    void setLocked(bool locked);
    bool isLocked() const;

    /** Get/Set the intensity attribute override ID to
     *  control a Function intensity */
    int intensityOverrideId() const;
    void setIntensityOverrideId(int id);

signals:
    void functionIDChanged();
    void startTimeChanged();
    void durationChanged();
    void colorChanged();
    void lockedChanged();

private:
    /** ID of this class to uniquely identify it within a Show */
    quint32 m_id;

    /** ID of the QLC+ Function this class represents */
    quint32 m_functionId;

    /** Start time of the Function in milliseconds */
    quint32 m_startTime;

    /** Duration of the Function in milliseconds */
    quint32 m_duration;

    /** Background color to be used when displaying the Function in
     *  the Show Manager */
    QColor m_color;

    /** Flag to indicate if this function is locked in the Show Manager timeline */
    bool m_locked;

    /** Intensity attribute override ID */
    int m_intensityOverrideId;

    /************************************************************************
     * Load & Save
     ***********************************************************************/
public:
    /** Load ShowFunction contents from $root */
    bool loadXML(QXmlStreamReader &root);

    /** Save ShowFunction contents to $doc */
    bool saveXML(QXmlStreamWriter *doc, quint32 trackId = UINT_MAX) const;
};

/** @} */

#endif // SHOWFUNCTION_H
