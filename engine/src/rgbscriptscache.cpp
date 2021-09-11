/*
  Q Light Controller Plus
  rgbscriptscache.cpp

  Copyright (c) David Garyga

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
#include <QDir>

#include "rgbscriptscache.h"
#ifdef QT_QML_LIB
  #include "rgbscriptv4.h"
#else
  #include "rgbscript.h"
#endif
#include "qlcconfig.h"
#include "qlcfile.h"

RGBScriptsCache::RGBScriptsCache(Doc* doc)
    : m_doc(doc)
{
    m_dummyScript = new RGBScript(doc);
}

QStringList RGBScriptsCache::names() const
{
    QStringList names;

    QListIterator <RGBScript*> it(m_scriptsMap.values());
    while (it.hasNext() == true)
        names << it.next()->name();

    return names;
}

RGBScript const& RGBScriptsCache::script(QString name) const
{
    QListIterator <RGBScript*> it(m_scriptsMap.values());
    while (it.hasNext() == true)
    {
        RGBScript* script(it.next());
        if (script->name() == name)
            return *script;
    }

    Q_ASSERT(m_dummyScript != NULL);
    return *m_dummyScript;
}

bool RGBScriptsCache::load(const QDir& dir)
{
    qDebug() << "Loading RGB scripts in " << dir.path() << "...";

    if (dir.exists() == false || dir.isReadable() == false)
        return false;

    foreach (QString file, dir.entryList())
    {
        if (!file.toLower().endsWith(".js"))
        {
            qDebug() << "    " << file << " skipped (special file or does not end on *.js)";
            continue;
        }
        if (!m_scriptsMap.contains(file))
        {
            RGBScript* script = new RGBScript(m_doc);
            if (script->load(dir, file))
            {
                qDebug() << "    " << file << " loaded";
                m_scriptsMap.insert(file, script);
            }
            else
            {
                qDebug() << "    " << file << " loading failed";
                delete script;
            }
        }
        else
        {
            qDebug() << "    " << file << " already known";
        }
    }
    return true;
}

QDir RGBScriptsCache::systemScriptsDirectory()
{
    return QLCFile::systemDirectory(QString(RGBSCRIPTDIR), QString(".js"));
}

QDir RGBScriptsCache::userScriptsDirectory()
{
    return QLCFile::userDirectory(QString(USERRGBSCRIPTDIR), QString(RGBSCRIPTDIR),
            QStringList() << QString("*%1").arg(".js"));
}
