/*
  Q Light Controller Plus
  qlcinputfeedback.cpp

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

#include "qlcinputfeedback.h"


QLCInputFeedback::QLCInputFeedback()
    : m_type(Undefinded)
    , m_value(0)
{
}

QLCInputFeedback *QLCInputFeedback::createCopy()
{
    QLCInputFeedback *copy = new QLCInputFeedback();
    copy->setType(this->type());
    copy->setValue(this->value());
    copy->setExtraParams(this->extraParams());

    return copy;
}

QLCInputFeedback::~QLCInputFeedback()
{
}

QLCInputFeedback::FeedbackType QLCInputFeedback::type() const
{
    return m_type;
}

void QLCInputFeedback::setType(FeedbackType type)
{
    m_type = type;
}

uchar QLCInputFeedback::value() const
{
    return m_value;
}

void QLCInputFeedback::setValue(uchar value)
{
    m_value = value;
}

QVariant QLCInputFeedback::extraParams() const
{
    return m_extraParams;
}

void QLCInputFeedback::setExtraParams(QVariant params)
{
    m_extraParams = params;
}
