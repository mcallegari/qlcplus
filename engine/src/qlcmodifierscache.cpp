/*
  Q Light Controller Plus
  qlcmodifierscache.cpp

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

#include <QDebug>

#include "qlcmodifierscache.h"
#include "channelmodifier.h"
#include "qlcconfig.h"
#include "qlcfile.h"

QLCModifiersCache::QLCModifiersCache()
{
}

bool QLCModifiersCache::addModifier(ChannelModifier *modifier)
{
    if (m_modifiers.contains(modifier->name()))
        return false;

    //qDebug() << "[QLCModifiersCache] added modifier" << modifier->name();
    m_modifiers[modifier->name()] = modifier;
    return true;
}

QList<QString> QLCModifiersCache::templateNames()
{
    return m_modifiers.keys();
}

ChannelModifier *QLCModifiersCache::modifier(QString name)
{
    if (m_modifiers.contains(name))
        return m_modifiers[name];

    return NULL;
}

QDir QLCModifiersCache::systemTemplateDirectory()
{
    return QLCFile::systemDirectory(QString(MODIFIERSTEMPLATEDIR), QString(KExtModifierTemplate));
}

QDir QLCModifiersCache::userTemplateDirectory()
{
    return QLCFile::userDirectory(QString(USERMODIFIERSTEMPLATEDIR), QString(MODIFIERSTEMPLATEDIR),
                                  QStringList() << QString("*%1").arg(KExtModifierTemplate));
}

bool QLCModifiersCache::load(const QDir& dir, bool systemTemplates)
{
    qDebug() << Q_FUNC_INFO << dir.path();

    if (dir.exists() == false || dir.isReadable() == false)
        return false;

    ChannelModifier::Type type;
    if (systemTemplates ==true)
        type = ChannelModifier::SystemTemplate;
    else
        type = ChannelModifier::UserTemplate;

    /* Attempt to read all specified files from the given directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        QString path(dir.absoluteFilePath(it.next()));

        if (path.toLower().endsWith(KExtModifierTemplate) == true)
        {
            ChannelModifier* chMod = new ChannelModifier();
            Q_ASSERT(chMod != NULL);

            QFile::FileError error = chMod->loadXML(path, type);
            if (error == QFile::NoError)
            {
                /* Delete the modifier if it's a duplicate. */
                if (addModifier(chMod) == false)
                {
                    delete chMod;
                    chMod = NULL;
                }
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Channel modifier template loading from"
                           << path << "failed:" << QLCFile::errorString(error);
                delete chMod;
                chMod = NULL;
            }
        }
        else
            qWarning() << Q_FUNC_INFO << "Unrecognized template extension:" << path;
    }

    return true;
}
