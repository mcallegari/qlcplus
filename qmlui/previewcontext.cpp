/*
  Q Light Controller Plus
  previewcontext.cpp

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

#include "previewcontext.h"
#include "doc.h"

PreviewContext::PreviewContext(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_enabled(false)
{

}

void PreviewContext::enableContext(bool enable)
{
    m_enabled = enable;
}

bool PreviewContext::isEnabled()
{
    return m_enabled;
}

void PreviewContext::setUniverseFilter(quint32 universeFilter)
{
    m_universeFilter = universeFilter;
}

QQuickView *PreviewContext::view()
{
    return m_view;
}

