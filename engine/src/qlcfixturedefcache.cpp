/*
  Q Light Controller
  qlcfixturedefcache.cpp

  Copyright (c) Heikki Junnila

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

#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QDebug>
#include <QList>
#include <QSet>

#if defined(WIN32) || defined(Q_OS_WIN)
#   include <windows.h>
#else
#   include <unistd.h>
#endif

#include "qlcfixturedefcache.h"
#include "avolitesd4parser.h"
#include "qlcfixturedef.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#define FIXTURES_MAP_NAME "FixturesMap.xml"
#define KXMLQLCFixtureMap "FixturesMap"

QLCFixtureDefCache::QLCFixtureDefCache()
{
}

QLCFixtureDefCache::~QLCFixtureDefCache()
{
    clear();
}

QLCFixtureDef* QLCFixtureDefCache::fixtureDef(
    const QString& manufacturer, const QString& model) const
{
    QListIterator <QLCFixtureDef*> it(m_defs);
    while (it.hasNext() == true)
    {
        QLCFixtureDef* def = it.next();
        if (def->manufacturer() == manufacturer && def->model() == model)
        {
            def->checkLoaded();
            return def;
        }
    }

    return NULL;
}

QStringList QLCFixtureDefCache::manufacturers() const
{
    QSet <QString> makers;

    // Gather a list of manufacturers
    QListIterator <QLCFixtureDef*> it(m_defs);
    while (it.hasNext() == true)
        makers << it.next()->manufacturer();

    // Bounce the QSet into a QStringList
    QStringList list;
    foreach (QString manuf, makers)
        list << manuf;

    return list;
}

QStringList QLCFixtureDefCache::models(const QString& manufacturer) const
{
    QSet <QString> models;
    QListIterator <QLCFixtureDef*> it(m_defs);
    while (it.hasNext() == true)
    {
        QLCFixtureDef* def = it.next();
        if (def->manufacturer() == manufacturer)
            models << def->model();
    }

    // Bounce the QSet into a QStringList
    QStringList list;
    foreach (QString model, models)
        list << model;

    return list;
}

bool QLCFixtureDefCache::addFixtureDef(QLCFixtureDef* fixtureDef)
{
    if (fixtureDef == NULL)
        return false;

    if (models(fixtureDef->manufacturer()).contains(fixtureDef->model()) == false)
    {
        m_defs << fixtureDef;
        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Cache already contains"
                   << fixtureDef->name();
        return false;
    }
}

bool QLCFixtureDefCache::storeFixtureDef(QString filename, QString data)
{
    QDir userFolder = userDefinitionDirectory();

    QFile file(userFolder.absoluteFilePath(filename));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;

    file.write(data.toUtf8());
    file.close();

    // reload user definitions
    load(userDefinitionDirectory());

    return true;
}

bool QLCFixtureDefCache::load(const QDir& dir)
{
    qDebug() << Q_FUNC_INFO << dir.path();

    if (dir.exists() == false || dir.isReadable() == false)
        return false;

    /* Attempt to read all specified files from the given directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        QString path(dir.absoluteFilePath(it.next()));

        if (path.toLower().endsWith(KExtFixture) == true)
            loadQXF(path);
        else if (path.toLower().endsWith(KExtAvolitesFixture) == true)
            loadD4(path);
        else
            qWarning() << Q_FUNC_INFO << "Unrecognized fixture extension:" << path;
    }

    return true;
}

bool QLCFixtureDefCache::loadMap(const QDir &dir)
{
    qDebug() << Q_FUNC_INFO << dir.path();

    if (dir.exists() == false || dir.isReadable() == false)
        return false;

    QString mapPath(dir.absoluteFilePath(FIXTURES_MAP_NAME));

    if (mapPath.isEmpty() == true)
        return false;

    QXmlStreamReader *doc = QLCFile::getXMLReader(mapPath);
    if (doc == NULL || doc->device() == NULL || doc->hasError())
    {
        qWarning() << Q_FUNC_INFO << "Unable to read from" << mapPath;
        return false;
    }

    while (!doc->atEnd())
    {
        if (doc->readNext() == QXmlStreamReader::DTD)
            break;
    }
    if (doc->hasError())
    {
        QLCFile::releaseXMLReader(doc);
        return false;
    }

    if (doc->dtdName() == KXMLQLCFixtureMap)
    {
        if (doc->readNextStartElement() == false)
        {
            QLCFile::releaseXMLReader(doc);
            return false;
        }

        if (doc->name() == KXMLQLCFixtureMap)
        {
            int fxCount = 0;

            while (doc->readNextStartElement())
            {
                if (doc->name() == "F")
                {
                    QString defFile = "";
                    QString manufacturer = "";
                    QString model = "";

                    if (doc->attributes().hasAttribute("n"))
                    {
                        QString filename = QString("%1%2").arg(doc->attributes().value("n").toString()).arg(KExtFixture);
                        defFile = QString(dir.absoluteFilePath(filename));
                    }

                    if(doc->attributes().hasAttribute("m"))
                        manufacturer = doc->attributes().value("m").toString();

                    if(doc->attributes().hasAttribute("d"))
                        model = doc->attributes().value("d").toString();

                    if (defFile.isEmpty() == false &&
                        manufacturer.isEmpty() == false &&
                        model.isEmpty() == false)
                    {
                        QLCFixtureDef* fxi = new QLCFixtureDef();
                        Q_ASSERT(fxi != NULL);

                        fxi->setDefinitionSourceFile(defFile);
                        fxi->setManufacturer(manufacturer);
                        fxi->setModel(model);

                        /* Delete the def if it's a duplicate. */
                        if (addFixtureDef(fxi) == false)
                            delete fxi;
                        fxi = NULL;
                        fxCount++;
                    }
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown Fixture Map tag: " << doc->name();
                }
                doc->skipCurrentElement();
            }
            qDebug() << fxCount << "fixtures found in map";
        }
        else
        {
            qWarning() << Q_FUNC_INFO << mapPath
                       << "is not a fixture map file";
            QLCFile::releaseXMLReader(doc);
            return false;
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << mapPath
                   << "is not a fixture map file";
        QLCFile::releaseXMLReader(doc);
        return false;
    }

    /* Attempt to read all files not in FixtureMap */
    QStringList definitionPaths;

    // Gather a list of manufacturers
    QListIterator <QLCFixtureDef*> mfit(m_defs);
    while (mfit.hasNext() == true)
        definitionPaths << mfit.next()->definitionSourceFile();

    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        QString path(dir.absoluteFilePath(it.next()));
        if (definitionPaths.contains(path))
            continue;

        qWarning() << path << "not in" << FIXTURES_MAP_NAME;

        if (path.toLower().endsWith(KExtFixture) == true)
            loadQXF(path);
        else if (path.toLower().endsWith(KExtAvolitesFixture) == true)
            loadD4(path);
        else
            qWarning() << Q_FUNC_INFO << "Unrecognized fixture extension:" << path;
    }

    return true;
}

void QLCFixtureDefCache::clear()
{
    while (m_defs.isEmpty() == false)
        delete m_defs.takeFirst();
}

QDir QLCFixtureDefCache::systemDefinitionDirectory()
{   
    return QLCFile::systemDirectory(QString(FIXTUREDIR), QString(KExtFixture));
}

QDir QLCFixtureDefCache::userDefinitionDirectory()
{
    QStringList filters;
    filters << QString("*%1").arg(KExtFixture);
    filters << QString("*%1").arg(KExtAvolitesFixture);

    return QLCFile::userDirectory(QString(USERFIXTUREDIR), QString(FIXTUREDIR), filters);
}

void QLCFixtureDefCache::loadQXF(const QString& path)
{
    QLCFixtureDef* fxi = new QLCFixtureDef();
    Q_ASSERT(fxi != NULL);

    QFile::FileError error = fxi->loadXML(path);
    if (error == QFile::NoError)
    {
        /* Delete the def if it's a duplicate. */
        if (addFixtureDef(fxi) == false)
            delete fxi;
        fxi = NULL;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Fixture definition loading from"
                   << path << "failed:" << QLCFile::errorString(error);
        delete fxi;
        fxi = NULL;
    }
}

void QLCFixtureDefCache::loadD4(const QString& path)
{
    QLCFixtureDef* fxi = new QLCFixtureDef();
    AvolitesD4Parser parser;
    if (parser.loadXML(path, fxi) == false)
    {
        qWarning() << Q_FUNC_INFO << "Unable to load D4 fixture from" << path
                   << ":" << parser.lastError();
        delete fxi;
        return;
    }

    /* Delete the def if it's a duplicate. */
    if (addFixtureDef(fxi) == false)
    {
        qDebug() << Q_FUNC_INFO << "Deleting duplicate" << path;
        delete fxi;
    }
    fxi = NULL;
}
