/*
  Q Light Controller Plus
  audioplugincache.cpp

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

#include <QPluginLoader>
#include <QDebug>

#include "audioplugincache.h"
#include "audiodecoder.h"
#include "qlcfile.h"

AudioPluginCache::AudioPluginCache(QObject *parent)
    : QObject(parent)
{

}

AudioPluginCache::~AudioPluginCache()
{
    qDebug() << Q_FUNC_INFO;
    while (m_plugins.isEmpty() == false)
        delete m_plugins.takeFirst();
}

void AudioPluginCache::load(const QDir &dir)
{
    qDebug() << Q_FUNC_INFO << dir.path();

    /* Check that we can access the directory */
    if (dir.exists() == false || dir.isReadable() == false)
        return;

    /* Loop through all files in the directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        /* Attempt to load a plugin from the path */
        QString fileName(it.next());
        QString path = dir.absoluteFilePath(fileName);

        QPluginLoader loader(path, this);
        AudioDecoder* ptr = qobject_cast<AudioDecoder*> (loader.instance());
        if (ptr != NULL)
        {
            qDebug() << "Loaded audio decoder plugin from" << fileName;
            /* Valid plugin. Append to list */
            m_plugins << ptr;
            /* Append also the plugin path to be used at runtime
             * for dynamic creation of instances */
            m_pluginsPathList << path;
        }
    }
}

QStringList AudioPluginCache::getSupportedFormats()
{
    QStringList caps;
    foreach(AudioDecoder *dec, m_plugins)
        caps << dec->supportedFormats();

    return caps;
}

AudioDecoder *AudioPluginCache::getDecoderForFile(const QString &filename)
{
    QFile fn(filename);
    if (fn.exists() == false)
        return NULL;

    foreach(QString path, m_pluginsPathList)
    {
        QPluginLoader loader(path, this);
        AudioDecoder* ptr = qobject_cast<AudioDecoder*> (loader.instance());
        if (ptr != NULL)
        {
            if (ptr->initialize(filename) == false)
            {
                loader.unload();
                continue;
            }
            return ptr;
        }
    }

    return NULL;
}
