/*
  Q Light Controller Plus
  scriptv4.cpp

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

#include <QDebug>

#include "function.h"
#include "scriptv3.h"
#include "scriptv4.h"
#include "script.h"

short Script::ScriptVersion=0;

const QString Script::startFunctionCmd = QString("startfunction");
const QString Script::stopFunctionCmd = QString("stopfunction");
const QString Script::blackoutCmd = QString("blackout");

const QString Script::waitCmd = QString("wait");
const QString Script::waitKeyCmd = QString("waitkey");

const QString Script::setFixtureCmd = QString("setfixture");
const QString Script::systemCmd = QString("systemcommand");

const QString Script::labelCmd = QString("label");
const QString Script::jumpCmd = QString("jump");

const QString Script::blackoutOn = QString("on");
const QString Script::blackoutOff = QString("off");

Script::Script(Doc* doc) : Function (doc, Function::ScriptType){
    switch(ScriptVersion){
        case 4:
            CallApi=new ScriptV4(doc);
            break;
        case 3:
            CallApi=new ScriptV3(doc);
            break;
        default:
            exit(1);
    }
}

Script::~Script(){
    delete CallApi;
}

QIcon Script::getIcon() const{
    return CallApi->getIcon();
}

quint32 Script::totalDuration(){
    return CallApi->totalDuration();
}


Function* Script::createCopy(Doc* doc, bool addToDoc){
    return CallApi->createCopy(doc,addToDoc);
}


bool Script::copyFrom(const Function* function){
    return CallApi->copyFrom(function);
}


bool Script::setData(const QString& str){
    return CallApi->setData(str);
}


bool Script::appendData(const QString& str){
    return CallApi->appendData(str);
}


QString Script::data() const{
    return CallApi->data();
}


QStringList Script::dataLines() const{
    return CallApi->dataLines();
}


QList<quint32> Script::functionList() const{
    return CallApi->functionList();
}


QList<quint32> Script::fixtureList() const{
    return CallApi->fixtureList();
}

QList<int>  Script::syntaxErrorsLines(){
    return CallApi->syntaxErrorsLines();
}

QStringList Script::syntaxErrorsLinesString(){
    return CallApi->syntaxErrorsLinesString();
}

void Script::slotRunnerFinished(){
  if(ScriptVersion==4)
      CallApi->slotRunnerFinished();
}

bool Script::loadXML(QXmlStreamReader &root){
    return CallApi->loadXML(root);
}


bool Script::saveXML(QXmlStreamWriter *doc){
    return CallApi->saveXML(doc);
}

ScriptApi::ScriptApi(Doc* doc) : Function(doc,Function::ScriptType){
    
}

ScriptApi::~ScriptApi(){
    
}
