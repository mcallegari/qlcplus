/*
  Q Light Controller Plus
  webaccessupload.cpp

  Copyright (c) Q Light Controller Plus

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

#include <QFileInfo>

#include "webaccessupload.h"

bool isPlainUploadedFileName(const QString &rawName)
{
    if (rawName.isEmpty() || rawName.length() > 255 ||
        rawName == "." || rawName == "..")
        return false;

    if (rawName.contains('/') || rawName.contains('\\'))
        return false;

    if (!rawName.endsWith(".qxf", Qt::CaseInsensitive) &&
        !rawName.endsWith(".d4", Qt::CaseInsensitive))
        return false;

    // Backstop for any path component Qt normalizes internally.
    return QFileInfo(rawName).fileName() == rawName;
}
