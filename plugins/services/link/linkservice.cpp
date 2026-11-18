/*
  Q Light Controller Plus
  linkservice.cpp

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

#include <ableton/Link.hpp>

#include "linkservice.h"

#define LINK_DEFAULT_TEMPO  120.0

LinkService::LinkService(QObject *parent)
    : QObject(parent)
    , m_link(new ableton::Link(LINK_DEFAULT_TEMPO))
{
    /* Link invokes these on its own thread. Emitting a signal from
     * there is fine: connections to objects in other threads are queued */
    m_link->setNumPeersCallback([this](std::size_t peers) {
        emit numPeersChanged(int(peers));
    });
    m_link->setTempoCallback([this](double bpm) {
        emit tempoChanged(bpm);
    });
    m_link->setStartStopCallback([this](bool playing) {
        emit playingChanged(playing);
    });

    /* the session play/stop is always shared */
    m_link->enableStartStopSync(true);
}

LinkService::~LinkService()
{
    m_link->setNumPeersCallback([](std::size_t) { });
    m_link->setTempoCallback([](double) { });
    m_link->setStartStopCallback([](bool) { });
    m_link->enable(false);
    delete m_link;
}

void LinkService::setEnabled(bool enable)
{
    m_link->enable(enable);
}

bool LinkService::isEnabled() const
{
    return m_link->isEnabled();
}

int LinkService::numPeers() const
{
    return int(m_link->numPeers());
}

bool LinkService::captureTimeline(double quantum, int offsetMs, QLCLinkTimeline &timeline)
{
    if (m_link->isEnabled() == false)
        return false;

    const auto state = m_link->captureAudioSessionState();
    const auto time = m_link->clock().micros() + std::chrono::milliseconds(offsetMs);

    timeline.beat = state.beatAtTime(time, quantum);
    timeline.phase = state.phaseAtTime(time, quantum);
    timeline.tempo = state.tempo();
    // a start can be scheduled in the future (e.g. on the next bar)
    timeline.playing = state.isPlaying() && time >= state.timeForIsPlaying();

    return true;
}

void LinkService::setTempo(double bpm)
{
    if (bpm <= 0)
        return;

    auto state = m_link->captureAppSessionState();
    state.setTempo(bpm, m_link->clock().micros());
    m_link->commitAppSessionState(state);
}

void LinkService::setPlaying(bool playing)
{
    auto state = m_link->captureAppSessionState();

    // committing the state the session is already in
    // would move the session start time
    if (state.isPlaying() == playing)
        return;

    state.setIsPlaying(playing, m_link->clock().micros());
    m_link->commitAppSessionState(state);
}
