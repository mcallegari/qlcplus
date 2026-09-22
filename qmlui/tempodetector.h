/*
  Q Light Controller Plus
  tempodetector.h

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

#ifndef TEMPODETECTOR_H
#define TEMPODETECTOR_H

#include <QThreadPool>
#include <QVariantList>
#include <QObject>
#include <atomic>
#include <memory>

class Doc;

/** Detects the tempo of audio files in the background, several files at a
 *  time on low priority threads, leaving a core free for the rest of QLC+ */
class TempoDetector final : public QObject
{
    Q_OBJECT

public:
    struct Job
    {
        quint32 itemId;
        QString name;
        QString fileName;
        /** The length of audio to analyze, in ms */
        quint32 duration;
    };

    explicit TempoDetector(Doc *doc, QObject *parent = nullptr);
    ~TempoDetector();

    bool isRunning() const;

    /** Analyze the audio of $jobs, reporting progress() and then finished() */
    void start(const QList<Job> &jobs);

    /** Stop analyzing. The jobs being analyzed report the tempo of the audio
     *  analyzed so far, the others are reported as not detected */
    void stop();

signals:
    void progress(int done, int total);

    /** $results is a list of maps, in the order of the jobs, with the keys
     *  itemId, name, bpm (0 when not detected), agreement (0 to 1) and
     *  stopped (true when stop() cut the analysis short or skipped it) */
    void finished(const QVariantList &results);

private:
    void jobDone(int index, double bpm, double agreement, bool stopped);

private:
    Doc *m_doc;
    QThreadPool m_pool;
    std::shared_ptr<std::atomic<bool>> m_stop;
    QList<Job> m_jobs;
    QVariantList m_results;
    int m_done;
};

#endif // TEMPODETECTOR_H
