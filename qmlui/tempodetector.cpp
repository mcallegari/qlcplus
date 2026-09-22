/*
  Q Light Controller Plus
  tempodetector.cpp

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

#include <QThread>
#include <cstring>
#include <vector>

#include "audioplugincache.h"
#include "tempoanalyzer.h"
#include "audiodecoder.h"
#include "tempodetector.h"
#include "doc.h"

#define READ_FRAMES 4096

static TempoAnalyzer::Result analyzeFile(Doc *doc, const QString &fileName, quint32 duration,
                                         const std::atomic<bool> &stop)
{
    TempoAnalyzer::Result none = { 0.0, 0.0 };

    AudioDecoder *decoder = doc->audioPluginCache()->getDecoderForFile(fileName);
    if (decoder == nullptr)
        return none;

    AudioParameters ap = decoder->audioParameters();
    const int channels = ap.channels();
    if (channels <= 0 || ap.sampleRate() == 0)
    {
        delete decoder;
        return none;
    }

    // the decoders deliver 16 bit samples whatever the file format, as
    // the audio renderers expect (see AudioDecoderSndFile::read)
    const qint64 frameBytes = qint64(channels) * qint64(sizeof(qint16));
    qint64 framesLeft = qint64(ap.sampleRate()) * duration / 1000;

    TempoAnalyzer analyzer(int(ap.sampleRate()));
    std::vector<char> buffer(size_t(frameBytes * READ_FRAMES));
    std::vector<float> mono;
    qint64 pending = 0;

    while (framesLeft > 0 && stop.load() == false)
    {
        qint64 read = decoder->read(buffer.data() + pending, qint64(buffer.size()) - pending);
        if (read <= 0)
            break;

        qint64 bytes = pending + read;
        qint64 frames = qMin(bytes / frameBytes, framesLeft);
        const qint16 *samples = reinterpret_cast<const qint16 *>(buffer.data());

        mono.resize(size_t(frames));
        for (qint64 f = 0; f < frames; f++)
        {
            int sum = 0;
            for (int c = 0; c < channels; c++)
                sum += samples[f * channels + c];
            mono[size_t(f)] = float(sum) / (32768.0f * channels);
        }
        analyzer.push(mono.data(), int(frames));
        framesLeft -= frames;

        // keep a partial frame for the next read
        pending = bytes - frames * frameBytes;
        if (pending > 0)
            memmove(buffer.data(), buffer.data() + frames * frameBytes, size_t(pending));
    }

    delete decoder;

    // when stopped, the audio analysed so far can still give a tempo
    return analyzer.result();
}

TempoDetector::TempoDetector(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_stop(std::make_shared<std::atomic<bool>>(false))
    , m_done(0)
{
    m_pool.setMaxThreadCount(qMax(1, QThread::idealThreadCount() - 1));
}

TempoDetector::~TempoDetector()
{
    m_stop->store(true);
    m_pool.waitForDone();
}

bool TempoDetector::isRunning() const
{
    return m_done < m_jobs.count();
}

void TempoDetector::start(const QList<Job> &jobs)
{
    if (isRunning())
        return;

    // a fresh flag, so that a job of a stopped run can't clear it
    m_stop = std::make_shared<std::atomic<bool>>(false);
    m_jobs = jobs;
    m_done = 0;
    m_results.clear();

    for (const Job &job : jobs)
    {
        QVariantMap result;
        result.insert("itemId", job.itemId);
        result.insert("name", job.name);
        result.insert("bpm", 0.0);
        result.insert("agreement", 0.0);
        result.insert("stopped", false);
        m_results.append(result);
    }

    emit progress(0, m_jobs.count());

    for (int i = 0; i < jobs.count(); i++)
    {
        Doc *doc = m_doc;
        Job job = jobs.at(i);
        std::shared_ptr<std::atomic<bool>> stop = m_stop;

        // the destructor waits for the jobs, so this outlives them
        m_pool.start([this, doc, job, stop, i]()
        {
            QThread::currentThread()->setPriority(QThread::LowPriority);

            TempoAnalyzer::Result res = { 0.0, 0.0 };
            if (stop->load() == false)
                res = analyzeFile(doc, job.fileName, job.duration, *stop);
            bool stopped = stop->load();

            QMetaObject::invokeMethod(this, [this, i, res, stopped]()
            {
                jobDone(i, res.bpm, res.agreement, stopped);
            }, Qt::QueuedConnection);
        });
    }

    if (jobs.isEmpty())
        emit finished(m_results);
}

void TempoDetector::stop()
{
    m_stop->store(true);
}

void TempoDetector::jobDone(int index, double bpm, double agreement, bool stopped)
{
    if (index < 0 || index >= m_results.count())
        return;

    QVariantMap result = m_results.at(index).toMap();
    result.insert("bpm", bpm);
    result.insert("agreement", agreement);
    result.insert("stopped", stopped);
    m_results[index] = result;

    m_done++;
    emit progress(m_done, m_jobs.count());

    if (m_done == m_jobs.count())
        emit finished(m_results);
}
