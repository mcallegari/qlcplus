/*
  Q Light Controller Plus
  webaccessupload.h

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

#ifndef WEBACCESSUPLOAD_H
#define WEBACCESSUPLOAD_H

#include <QString>

bool isPlainUploadedFileName(const QString &rawName);

#endif // WEBACCESSUPLOAD_H
