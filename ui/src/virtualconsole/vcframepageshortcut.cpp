/*
  Q Light Controller Plus
  vcframepageshortcut.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#include "vcframepageshortcut.h"

VCFramePageShortcut::VCFramePageShortcut(int page)
    : m_id(page + 3)
    , m_page(page)
{
}

VCFramePageShortcut::VCFramePageShortcut(VCFramePageShortcut const& shortcut)
    : m_id(shortcut.m_id)
    , m_page(shortcut.m_page)
    , m_keySequence(shortcut.m_keySequence)
{
    if (shortcut.m_inputSource != NULL)
    {
        m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(shortcut.m_inputSource->universe(),
                                                       shortcut.m_inputSource->channel()));
        m_inputSource->setRange(shortcut.m_inputSource->lowerValue(), shortcut.m_inputSource->upperValue());
    }
}

VCFramePageShortcut::~VCFramePageShortcut()
{
}

bool VCFramePageShortcut::operator<(VCFramePageShortcut const& right) const
{
    return m_id < right.m_id;
}

bool VCFramePageShortcut::compare(VCFramePageShortcut const* left, VCFramePageShortcut const* right)
{
    return *left < *right;
}
