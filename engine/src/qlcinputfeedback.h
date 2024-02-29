/*
  Q Light Controller Plus
  qlcinputfeedback.h

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

#ifndef QLCINPUTFEEDBACK_H
#define QLCINPUTFEEDBACK_H

#include <QObject>
#include <QVariant>

class QLCInputFeedback : public QObject
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Standard constructor */
    QLCInputFeedback();

    /** Copy constructor */
    QLCInputFeedback *createCopy();

    /** Destructor */
    virtual ~QLCInputFeedback();

    /** Feedback type */
    enum FeedbackType
    {
        Undefinded   = -1,
        LowerValue   =  0,
        UpperValue   =  1,
        MonitorValue =  2
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(FeedbackType)
#endif

    FeedbackType type() const;
    void setType(FeedbackType type);

    uchar value() const;
    void setValue(uchar value);

    QVariant extraParams() const;
    void setExtraParams(QVariant params);

protected:
    FeedbackType m_type;
    uchar m_value;
    QVariant m_extraParams;
};

#endif /* QLCINPUTFEEDBACK_H */
