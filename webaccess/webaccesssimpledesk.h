/*
  Q Light Controller Plus
  webaccesssimpledesk.h

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

#ifndef WEBACCESSSIMPLEDESK_H
#define WEBACCESSSIMPLEDESK_H

#include <QObject>

class SimpleDesk;
class Doc;

class WebAccessSimpleDesk : public QObject
{
    Q_OBJECT
public:
    explicit WebAccessSimpleDesk(QObject *parent = 0);

    static QString getHTML(Doc *doc, SimpleDesk *sd);
    static QString getChannelsMessage(Doc *doc, SimpleDesk *sd,
                                      quint32 universe, int startAddr, int chNumber);

signals:

public slots:

};

#endif // WEBACCESSSIMPLEDESK_H
