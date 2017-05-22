/*
  Q Light Controller Plus
  videoprovider.cpp

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

#include "videoprovider.h"
#include "video.h"
#include "doc.h"

VideoProvider::VideoProvider(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    for (Function *f : m_doc->functionsByType(Function::VideoType))
        m_videoMap[f->id()] = NULL;

    connect(m_doc, &Doc::functionAdded, this, &VideoProvider::slotFunctionAdded);
    connect(m_doc, &Doc::functionRemoved, this, &VideoProvider::slotFunctionRemoved);
}

VideoProvider::~VideoProvider()
{
    m_videoMap.clear();
}

void VideoProvider::slotFunctionAdded(quint32 id)
{
    Function *func = m_doc->function(id);
    if (func == NULL)
        return;

    if(func->type() == Function::VideoType)
        m_videoMap[id] = NULL;
}

void VideoProvider::slotFunctionRemoved(quint32 id)
{
    if (m_videoMap.contains(id))
    {
        QQuickItem *vi = m_videoMap.take(id);
        delete vi;
    }
}
