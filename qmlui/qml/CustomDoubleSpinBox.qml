/*
  Q Light Controller Plus
  CustomDoubleSpinBox.qml

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

import QtQuick 2.3
import QtQuick.Controls 2.14
import "."

CustomSpinBox
{
    id: controlRoot
    from: realFrom * Math.pow(10, decimals)
    to: realTo * Math.pow(10, decimals)
    value: realValue * Math.pow(10, decimals)
    stepSize: realStep * Math.pow(10, decimals)
    suffix: "°"

    property real realFrom: 0
    property real realTo: 100
    property real realValue: 0
    property real realStep: 0.5
    property int decimals: 2

    validator: DoubleValidator {
        bottom: Math.min(controlRoot.from, controlRoot.to)
        top:  Math.max(controlRoot.from, controlRoot.to)
    }

    textFromValue: function(value, locale) {
        return Number(value / Math.pow(10, decimals)).toLocaleString(locale, 'f', decimals) + suffix
    }

    valueFromText: function(text, locale) {
        return Number.fromLocaleString(locale, text.replace(suffix, "")) * Math.pow(10, decimals)
    }

    onValueModified: realValue = value / Math.pow(10, decimals)

    function setValue(newValue)
    {
        value = newValue
        realValue = newValue / Math.pow(10, decimals)
    }
}
