/*
  Q Light Controller Plus
  webaccessconfiguration.h

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

#ifndef WEBACCESSCONFIGURATION_H
#define WEBACCESSCONFIGURATION_H

#include <QObject>
#include <QString>

class Doc;
class WebAccessAuth;

class WebAccessConfiguration : public QObject
{
public:
    WebAccessConfiguration();

    static QString getIOConfigHTML(Doc *doc);
    static QString getAudioConfigHTML(Doc *doc);
    static QString getUserFixturesConfigHTML();
    static QString getPasswordsConfigHTML(WebAccessAuth *auth);
    static QString getHTML(Doc *doc, WebAccessAuth *auth);
};

#endif // WEBACCESSCONFIGURATION_H
