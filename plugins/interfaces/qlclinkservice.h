/*
  Q Light Controller Plus
  qlclinkservice.h

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

#ifndef QLCLINKSERVICE_H
#define QLCLINKSERVICE_H

#include <QtPlugin>

/** @addtogroup engine Engine
 * @{
 */

/** The id under which the Ableton Link service plugin registers itself */
#define QLCLINK_SERVICE_ID "link"

/** A snapshot of the shared Link timeline, taken at the time of capture */
struct QLCLinkTimeline
{
    double beat;    //! absolute beat position (fractional)
    double phase;   //! position within the quantum, in [0, quantum)
    double tempo;   //! session tempo in BPM (fractional)
    bool playing;   //! session transport state (start/stop sync)
};

/**
 * The Ableton Link feature interface, implemented by the Link service plugin
 * and consumed by the engine (MasterTimer) as a beat source.
 *
 * Like QLCRDMService, this is a plain interface (no QObject, no Q_OBJECT) so
 * it can be compiled both into QLC+ and into the plugin. The IID carries the
 * interface version, so qobject_cast<QLCLinkService*>(obj) fails safely for a
 * plugin built against a different version.
 *
 * The object implementing the interface must also be a QObject and must
 * emit the following signals (connect with the string based SIGNAL() syntax).
 * They are emitted from a Link thread, so connections are queued:
 *
 *   void numPeersChanged(int peers);   // peers in the session changed
 *   void tempoChanged(double bpm);     // session tempo changed
 *   void playingChanged(bool playing); // session transport changed
 *
 * Threading: captureTimeline() is realtime safe and must only be called by
 * one thread, which is the MasterTimer thread. Every other method is thread
 * safe but not realtime safe, so it must NOT be called by MasterTimer.
 */
class QLCLinkService
{
public:
    virtual ~QLCLinkService() { }

    /** Join (true) or leave (false) the Link session on the network */
    virtual void setEnabled(bool enable) = 0;
    virtual bool isEnabled() const = 0;

    /** Number of other peers in the current session */
    virtual int numPeers() const = 0;

    /** Capture the timeline at the current time. Realtime safe.
     *  @param quantum beats per bar used to align the phase (e.g. 4.0)
     *  @param timeline [out] the captured values
     *  @return false if Link is not enabled */
    virtual bool captureTimeline(double quantum, QLCLinkTimeline &timeline) = 0;

    /** Propose a new tempo to the session */
    virtual void setTempo(double bpm) = 0;

    /** Enable/disable the start/stop transport sync */
    virtual void setStartStopSyncEnabled(bool enable) = 0;
    virtual bool isStartStopSyncEnabled() const = 0;

    /** Start/stop the session transport (only with start/stop sync enabled) */
    virtual void setPlaying(bool playing) = 0;
};

#define QLCLinkService_iid "org.qlcplus.QLCLinkService/1"

Q_DECLARE_INTERFACE(QLCLinkService, QLCLinkService_iid)

/** @} */

#endif
