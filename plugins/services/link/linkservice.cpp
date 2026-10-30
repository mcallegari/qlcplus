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

bool LinkService::captureTimeline(double quantum, QLCLinkTimeline &timeline)
{
    if (m_link->isEnabled() == false)
        return false;

    const auto state = m_link->captureAudioSessionState();
    const auto now = m_link->clock().micros();

    timeline.beat = state.beatAtTime(now, quantum);
    timeline.phase = state.phaseAtTime(now, quantum);
    timeline.tempo = state.tempo();
    timeline.playing = state.isPlaying();

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

void LinkService::setStartStopSyncEnabled(bool enable)
{
    m_link->enableStartStopSync(enable);
}

bool LinkService::isStartStopSyncEnabled() const
{
    return m_link->isStartStopSyncEnabled();
}

void LinkService::setPlaying(bool playing)
{
    auto state = m_link->captureAppSessionState();
    state.setIsPlaying(playing, m_link->clock().micros());
    m_link->commitAppSessionState(state);
}
