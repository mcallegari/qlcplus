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

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLShowFunction "ShowFunction"

class ShowFunction: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ShowFunction)

    Q_PROPERTY(int functionID READ functionID WRITE setFunctionID NOTIFY functionIDChanged)
    Q_PROPERTY(int startTime READ startTime WRITE setStartTime NOTIFY startTimeChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool locked READ isLocked WRITE setLocked NOTIFY lockedChanged)

public:
    ShowFunction(QObject *parent = 0);
    virtual ~ShowFunction() {}

    void setFunctionID(quint32 id);
    quint32 functionID() const;

    void setStartTime(quint32 time);
    quint32 startTime() const;

    void setDuration(quint32 duration);
    quint32 duration() const;

    void setColor(QColor color);
    QColor color() const;

    static QColor defaultColor(Function::Type type);

    /** Set the lock state of this ShowFunction */
    void setLocked(bool locked);

    /** Get the lock state of this ShowFunction */
    bool isLocked() const;

signals:
    void functionIDChanged();
    void startTimeChanged();
    void durationChanged();
    void colorChanged();
    void lockedChanged();

private:
    /** ID of the QLC+ Function this class represents */
    quint32 m_id;

    /** Start time of the Function in milliseconds */
    quint32 m_startTime;

    /** Duration of the Function in milliseconds */
    quint32 m_duration;

    /** Background color to be used when displaying the Function in
     *  the Show Manager */
    QColor m_color;

    /** Flag to indicate if this function is locked in the Show Manager timeline */
    bool m_locked;

    /************************************************************************
     * Load & Save
     ***********************************************************************/
public:
    /** Load ShowFunction contents from $root */
    bool loadXML(QXmlStreamReader &root);

    /** Save ShowFunction contents to $doc */
    bool saveXML(QXmlStreamWriter *doc) const;
};

/** @} */

#endif // SHOWFUNCTION_H
