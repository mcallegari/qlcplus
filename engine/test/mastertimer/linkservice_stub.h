/*
  Q Light Controller Plus
  linkservice_stub.h

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

#ifndef LINKSERVICE_STUB_H
#define LINKSERVICE_STUB_H

#include <QObject>

#include "qlclinkservice.h"

/** A Link service returning a timeline set by the test */
class LinkService_Stub final : public QObject, public QLCLinkService
{
    Q_OBJECT
    Q_INTERFACES(QLCLinkService)

public:
    LinkService_Stub()
        : m_enabled(false)
        , m_proposedTempo(0)
        , m_playingCalls(0)
        , m_lastPlaying(false)
        , m_lastOffset(0)
    {
        m_timeline.beat = 0;
        m_timeline.phase = 0;
        m_timeline.tempo = 120;
        m_timeline.playing = false;
    }

    void setEnabled(bool enable) override { m_enabled = enable; }
    bool isEnabled() const override { return m_enabled; }
    int numPeers() const override { return 0; }

    bool captureTimeline(double quantum, int offsetMs, QLCLinkTimeline &timeline) override
    {
        Q_UNUSED(quantum)
        if (m_enabled == false)
            return false;
        m_lastOffset = offsetMs;
        timeline = m_timeline;
        return true;
    }

    void setTempo(double bpm) override { m_proposedTempo = bpm; }

    /** Like the real service, a state the session is already in is not committed */
    void setPlaying(bool playing) override
    {
        if (playing == m_timeline.playing)
            return;
        m_playingCalls++;
        m_lastPlaying = playing;
    }

signals:
    void numPeersChanged(int peers);

public:
    QLCLinkTimeline m_timeline;
    bool m_enabled;
    double m_proposedTempo;
    int m_playingCalls;
    bool m_lastPlaying;
    int m_lastOffset;
};

#endif
