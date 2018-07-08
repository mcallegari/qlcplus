/*
  Q Light Controller Plus
  functionparent.h

  Copyright (C) 2016 Massimo Callegari
                     David Garyga

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

#ifndef FUNCTIONPARENT_H
#define FUNCTIONPARENT_H

/** @addtogroup engine_functions Functions
 * @{
 */

/**
 * Start/Stop source
 */
class FunctionParent
{
public:
    // The type of the FunctionParent has 2 purposes:
    //
    // 1. It allows to differentiate the ID of a VCWidget
    // and the ID of a Function, because they could overlap
    // otherwise
    //
    // 2. It allows to define a special behavior for some
    // types. Example: a Master FunctionParent can stop any
    // function, regardless of what started it.
    //
    // AutoVCWidget and ManualVCWidget are separated.
    // In order to keep some parts of the current behavior,
    // ManualVCWidget acts like the "Master" type and can stop a
    // running function when the user uses a manual VCWidget.
    enum Type
    {
        // Another function (Chaser, Collection...)
        Function = 0,
        // An automatic VC widget (VCAudioTriggers)
        AutoVCWidget,
        // A manual VC widget (Button, Slider...)
        ManualVCWidget,
        // Override anything (MasterTimer, test facilities...)
        Master = 0xffffffff,
    };

private:
    quint64 m_id;

public:
    explicit FunctionParent(Type type, quint32 id)
    {
        m_id = quint64((quint64(type) & 0xffffffff) << 32)
            | quint64(id & 0xffffffff);
    }

    bool operator ==(FunctionParent const& right) const
    {
        return m_id == right.m_id;
    }

    quint32 type() const
    {
        return (m_id >> 32) & 0xffffffff;
    }

    quint32 id() const
    {
        return m_id & 0xffffffff;
    }

    static FunctionParent master()
    {
        return FunctionParent(Master, 0);
    }
};

/** @} */

#endif
