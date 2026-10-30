/*
  Q Light Controller Plus
  linkservice.h

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

#ifndef LINKSERVICE_H
#define LINKSERVICE_H

#include <QObject>

#include "qlclinkservice.h"

namespace ableton { class Link; }

/**
 * The Ableton Link service. The Link SDK headers are included only by
 * linkservice.cpp, so nothing else depends on them.
 */
class LinkService final : public QObject, public QLCLinkService
{
    Q_OBJECT
    Q_INTERFACES(QLCLinkService)

public:
    explicit LinkService(QObject *parent = nullptr);
    ~LinkService();

    /** @reimp */
    void setEnabled(bool enable) override;

    /** @reimp */
    bool isEnabled() const override;

    /** @reimp */
    int numPeers() const override;

    /** @reimp */
    bool captureTimeline(double quantum, QLCLinkTimeline &timeline) override;

    /** @reimp */
    void setTempo(double bpm) override;

    /** @reimp */
    void setStartStopSyncEnabled(bool enable) override;

    /** @reimp */
    bool isStartStopSyncEnabled() const override;

    /** @reimp */
    void setPlaying(bool playing) override;

signals:
    void numPeersChanged(int peers);
    void tempoChanged(double bpm);
    void playingChanged(bool playing);

private:
    ableton::Link *m_link;
};

#endif
