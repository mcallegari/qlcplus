/*
  Q Light Controller Plus
  rgbscriptproperty.h

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

#ifndef RGBSCRIPTPROPERTY_H
#define RGBSCRIPTPROPERTY_H

#include <QStringList>

class RGBScriptProperty
{
public:
    RGBScriptProperty()
    {
        m_name = QString();
        m_displayName = QString();
        m_type = None;
        m_listValues = QStringList();
        m_rangeMinValue = 0;
        m_rangeMaxValue = 0;
        m_readMethod = QString();
        m_writeMethod = QString();
    }

    ~RGBScriptProperty() { /* NOP */ }

    enum ValueType
    {
        None,
        List,
        Range,
        Float,
        String
    };

public:
    QString m_name;
    QString m_displayName;
    ValueType m_type;
    QStringList m_listValues;
    int m_rangeMinValue;
    int m_rangeMaxValue;
    QString m_readMethod;
    QString m_writeMethod;
};

#endif // RGBSCRIPTPROPERTY_H
