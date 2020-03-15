/*
  Q Light Controller Plus
  script.h

  Copyright (C) Jan Koester

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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QStringList>
#include <QObject>
#include <QMap>
#include <QList>

#include "function.h"

class ScriptRunner;
class MasterTimer;
class Universe;
class Doc;

const QStringList knownKeywords(QStringList() << "ch" << "val" << "arg");

class ScriptApi : public Function
{
    Q_OBJECT
public:
    ScriptApi(Doc* doc);
    virtual ~ScriptApi();
    virtual QIcon getIcon() const=0;
    virtual quint32 totalDuration()=0;
public:
    virtual Function* createCopy(Doc* doc, bool addToDoc = true)=0;
    virtual bool copyFrom(const Function* function)=0;
public:
    virtual bool setData(const QString& str)=0;
    virtual bool appendData(const QString& str)=0;
    virtual QString data() const =0;
    virtual QStringList dataLines() const =0;
    virtual QList<quint32> functionList() const=0;
    virtual QList<quint32> fixtureList() const=0;
    virtual bool loadXML(QXmlStreamReader &root)=0;
    virtual bool saveXML(QXmlStreamWriter *doc)=0;
    virtual QList<int> syntaxErrorsLines()=0;
    virtual QStringList syntaxErrorsLinesString()=0;
public:
    virtual void preRun(MasterTimer *timer)=0;
    virtual void write(MasterTimer *timer, QList<Universe*> universes)=0;
    virtual void postRun(MasterTimer *timer, QList<Universe*> universes)=0;
};

class Script : public Function 
{
    Q_OBJECT

public:
     /************************************************************************
     * Script keywords
     ************************************************************************/
    static const QString startFunctionCmd;
    static const QString stopFunctionCmd;
    static const QString blackoutCmd;

    static const QString waitCmd;
    static const QString waitKeyCmd;

    static const QString setFixtureCmd;
    static const QString systemCmd;

    static const QString labelCmd;
    static const QString jumpCmd;

    static const QString blackoutOn;
    static const QString blackoutOff;

public:
    static short ScriptVersion;
    Script(Doc* doc);
    virtual ~Script();
    QIcon getIcon() const;
    quint32 totalDuration();
    Function* createCopy(Doc* doc, bool addToDoc = true);
    bool copyFrom(const Function* function);
    bool setData(const QString& str);
    bool appendData(const QString& str);
    QString data() const;
    QStringList dataLines() const;
    QList<quint32> functionList() const;
    QList<quint32> fixtureList() const;
    QList<int> syntaxErrorsLines();
    QStringList syntaxErrorsLinesString();
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);
private:
    ScriptApi *CallApi;
};

#endif
 
