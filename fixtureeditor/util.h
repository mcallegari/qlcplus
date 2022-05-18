/*
  Q Light Controller - Fixture Definition Editor
  util.h

  Copyright (C) Heikki Junnila

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

#ifndef UTIL_H
#define UTIL_H

#include <QRegularExpression>
#include <QRegularExpressionValidator>

/** @addtogroup fixtureeditor Fixture Editor
 * @{
 */

//! Prevent people from using ALL CAPS. It's fucking annoying. Allow max. 5 consecutive CAPS in a row
#define REGEXP_CAPS "[^A-Z]*[A-Z]{0,5}([^A-Z]+[A-Z]{0,5})*"
#define CAPS_VALIDATOR(parent) new QRegularExpressionValidator(QRegularExpression(REGEXP_CAPS), parent)

/** @} */

#endif
