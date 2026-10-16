/*
  Q Light Controller Plus
  vcaudiotimer.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QInputDialog>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPolygonF>
#include <QLineEdit>
#include <QPainter>
#include <QString>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QSet>
#include <QDebug>
#include <cmath>
#include <QSize>

#include "audioplugincache.h"
#include "audiodecoder.h"
#include "vcaudiotimer.h"
#include "function.h"
#include "audio.h"
#include "doc.h"

/* Repaint interval while a track is playing (about 60 frames per second) */
#define FRAME_INTERVAL 16
/* Polling interval when nothing is playing */
#define IDLE_INTERVAL 100

/* Number of peaks stored for the waveform of a track */
#define WAVEFORM_POINTS 1500


/*****************************************************************************
 * Track analysis (waveform and BPM), shared by all the widgets
 *****************************************************************************/

struct AudioAnalysis
{
    bool ready = false;
    QVector<float> peaks;   // 0.0 - 1.0, WAVEFORM_POINTS values over the whole track
    double bpm = 0;         // 0 if not detected
};

static QMutex analysisMutex;
static QHash<QString, AudioAnalysis> analysisCache;

/* Estimate the tempo from an onset strength envelope sampled at envRate Hz */
static double estimateBPM(QVector<float> onset, double envRate)
{
    if (onset.size() < envRate * 8)
        return 0;

    double mean = 0;
    for (float v : onset)
        mean += v;
    mean /= onset.size();
    for (float &v : onset)
        v -= mean;

    /* autocorrelation over the 60 - 200 BPM range */
    int minLag = qMax(1, (int)(envRate * 60.0 / 200.0));
    int maxLag = (int)(envRate * 60.0 / 60.0);
    int acMax = maxLag * 2 + 2;
    QVector<double> ac(acMax + 1, 0);
    for (int lag = 1; lag <= acMax; lag++)
    {
        double sum = 0;
        for (int i = lag; i < onset.size(); i++)
            sum += onset[i] * onset[i - lag];
        ac[lag] = sum;
    }

    int best = 0;
    double bestScore = -1e30;
    for (int lag = minLag; lag <= maxLag; lag++)
    {
        double bpm = 60.0 * envRate / lag;
        /* the beat period and its double both reinforce the pulse */
        double score = ac[lag] + 0.5 * ac[2 * lag];
        /* log-gaussian preference around 120 BPM, to avoid octave errors */
        score *= exp(-0.5 * pow(log2(bpm / 120.0), 2));
        if (score > bestScore)
        {
            bestScore = score;
            best = lag;
        }
    }
    if (best == 0)
        return 0;

    /* parabolic interpolation around the peak */
    double y0 = ac[best - 1], y1 = ac[best], y2 = ac[best + 1];
    double denom = y0 - 2 * y1 + y2;
    double shift = denom != 0 ? 0.5 * (y0 - y2) / denom : 0;
    double bpm = 60.0 * envRate / (best + qBound(-0.5, shift, 0.5));

    return qRound(bpm * 10) / 10.0;
}

/* Decode a whole file and compute its waveform peaks and BPM. Runs in a worker thread */
static AudioAnalysis analyzeTrack(AudioDecoder *decoder)
{
    AudioAnalysis result;

    AudioParameters ap = decoder->audioParameters();
    int channels = qMax(1, ap.channels());
    int sampleSize = ap.sampleSize();
    quint32 rate = ap.sampleRate();
    qint64 totalMs = decoder->totalTime();
    if (sampleSize <= 0 || rate == 0 || totalMs <= 0)
        return result;

    qint64 totalFrames = totalMs * rate / 1000;
    qint64 framesPerPeak = qMax<qint64>(1, totalFrames / WAVEFORM_POINTS);
    const int hop = rate / 100;      // 10ms energy frames for the BPM envelope

    /* one-pole filters to split bass (< 200Hz) and highs (> 2kHz) for the onsets */
    const float aLow = 1 - exp(-2 * M_PI * 200.0 / rate);
    const float aHigh = 1 - exp(-2 * M_PI * 2000.0 / rate);
    float lowPass = 0, highCut = 0;

    QVector<float> peaks;
    QVector<float> envAll, envLow, envHigh;
    peaks.reserve(WAVEFORM_POINTS + 16);
    float peak = 0;
    qint64 peakCount = 0;
    double eAll = 0, eLow = 0, eHigh = 0;
    int energyCount = 0;

    QByteArray buffer(channels * sampleSize * 4096, 0);
    qint64 read;
    while ((read = decoder->read(buffer.data(), buffer.size())) > 0)
    {
        int frames = read / (channels * sampleSize);
        const char *p = buffer.constData();

        for (int f = 0; f < frames; f++)
        {
            /* mix down to mono, normalized to -1.0 - 1.0 */
            float sum = 0;
            for (int c = 0; c < channels; c++)
            {
                const char *s = p + (f * channels + c) * sampleSize;
                switch (ap.format())
                {
                    case PCM_S8: sum += *(const qint8 *)s / 128.0f; break;
                    case PCM_S16LE: sum += *(const qint16 *)s / 32768.0f; break;
                    case PCM_S24LE: sum += (*(const qint32 *)s << 8 >> 8) / 8388608.0f; break;
                    case PCM_S32LE: sum += *(const qint32 *)s / 2147483648.0f; break;
                    default: break;
                }
            }
            float v = sum / channels;

            peak = qMax(peak, qAbs(v));
            if (++peakCount >= framesPerPeak)
            {
                peaks.append(peak);
                peak = 0;
                peakCount = 0;
            }

            lowPass += aLow * (v - lowPass);
            highCut += aHigh * (v - highCut);
            float high = v - highCut;
            eAll += v * v;
            eLow += lowPass * lowPass;
            eHigh += high * high;
            if (++energyCount >= hop)
            {
                envAll.append(log(1e-7 + eAll / energyCount));
                envLow.append(log(1e-7 + eLow / energyCount));
                envHigh.append(log(1e-7 + eHigh / energyCount));
                eAll = eLow = eHigh = 0;
                energyCount = 0;
            }
        }
    }

    if (peakCount)
        peaks.append(peak);
    if (peaks.isEmpty())
        return result;

    float maxPeak = 0;
    for (float v : peaks)
        maxPeak = qMax(maxPeak, v);
    if (maxPeak > 0)
        for (float &v : peaks)
            v /= maxPeak;

    result.peaks = peaks;
    /* onset strength: positive difference of the log energy, summed over the bands */
    QVector<float> onset(envAll.size(), 0);
    for (int i = 1; i < envAll.size(); i++)
        onset[i] = qMax(0.0f, envAll[i] - envAll[i - 1]) +
                   qMax(0.0f, envLow[i] - envLow[i - 1]) +
                   qMax(0.0f, envHigh[i] - envHigh[i - 1]);

    result.bpm = estimateBPM(onset, 100.0);
    result.ready = true;

    return result;
}

/* Return the analysis of a file, starting it in the background if needed */
static AudioAnalysis trackAnalysis(Doc *doc, const QString &file)
{
    if (file.isEmpty())
        return AudioAnalysis();

    {
        QMutexLocker locker(&analysisMutex);
        if (analysisCache.contains(file))
            return analysisCache.value(file);
        /* mark as in progress */
        analysisCache.insert(file, AudioAnalysis());
    }

    AudioDecoder *decoder = doc->audioPluginCache()->getDecoderForFile(file);
    if (decoder == NULL)
        return AudioAnalysis();

    QThread *thread = QThread::create([decoder, file]()
    {
        AudioAnalysis result = analyzeTrack(decoder);
        QMutexLocker locker(&analysisMutex);
        analysisCache.insert(file, result);
    });
    QObject::connect(thread, &QThread::finished, decoder, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start(QThread::LowPriority);

    return AudioAnalysis();
}

static QString msToString(quint32 ms)
{
    quint32 secs = ms / 1000;
    return QString("%1:%2.%3").arg(secs / 60, 2, 10, QChar('0'))
                              .arg(secs % 60, 2, 10, QChar('0'))
                              .arg(ms % 1000, 3, 10, QChar('0'));
}

/* Render a waveform as mirrored vertical lines, one per pixel column */
static QPixmap renderWaveform(const QVector<float> &peaks, QSize size, qreal dpr, const QColor &color)
{
    QPixmap pix(size * dpr);
    pix.setDevicePixelRatio(dpr);
    pix.fill(Qt::transparent);

    int width = qRound(size.width() * dpr);
    int points = peaks.size();
    if (width <= 0 || points == 0)
        return pix;

    QPainter p(&pix);
    p.scale(1.0 / dpr, 1.0 / dpr);
    qreal height = size.height() * dpr;
    qreal center = height / 2.0;

    for (int x = 0; x < width; x++)
    {
        int from = (qint64)x * points / width;
        int to = qMax(from + 1, (int)((qint64)(x + 1) * points / width));
        float peak = 0;
        for (int i = from; i < to && i < points; i++)
            peak = qMax(peak, peaks[i]);
        qreal h = qMax<qreal>(1.0, peak * height / 2.0);
        p.fillRect(QRectF(x, center - h, 1, h * 2), color);
    }

    return pix;
}

void VCAudioTimer::refreshEntries()
{
    QList<Entry> list;
    QSet<quint32> running;
    qint64 now = m_clock.elapsed();

    foreach (Function *f, m_doc->functionsByType(Function::AudioType))
    {
        if (f == NULL || f->isRunning() == false)
            continue;

        Entry entry;
        entry.id = f->id();
        entry.paused = f->isPaused();
        entry.name = f->name();
        Audio *audio = qobject_cast<Audio*>(f);
        if (audio != NULL)
            entry.file = audio->getSourceFileName();
        entry.total = f->totalDuration();

        /* extrapolate between engine ticks, never more than a few ticks ahead */
        quint32 engine = f->elapsed();

        /* until the engine applies a seek, keep showing the requested position */
        if (m_pendingSeeks.contains(entry.id))
        {
            QPair<quint32, qint64> seek = m_pendingSeeks.value(entry.id);
            if (qAbs((qint64)engine - (qint64)seek.first) < 300 || now - seek.second > 500)
                m_pendingSeeks.remove(entry.id);
            else
                engine = seek.first;
        }
        Smoothing s = m_smoothing.value(entry.id, { engine, now, engine });
        if (engine != s.engineElapsed)
        {
            s.engineElapsed = engine;
            s.stamp = now;
        }
        quint32 shown = engine;
        if (entry.paused == false)
            shown += (quint32)qMin<qint64>(now - s.stamp, 60);
        /* avoid tiny steps back when the engine catches up, but follow real jumps (seek) */
        if (shown < s.shown && s.shown - shown < 120)
            shown = s.shown;
        s.shown = shown;
        m_smoothing[entry.id] = s;

        entry.elapsed = entry.total ? qMin(shown, entry.total) : shown;
        running.insert(entry.id);
        list.append(entry);
    }

    /* forget the tracks that are not running anymore */
    foreach (quint32 id, m_smoothing.keys())
    {
        if (running.contains(id) == false)
        {
            m_smoothing.remove(id);
            m_waveCache.remove(id);
        }
    }

    m_entries = list;
}

VCAudioTimer::VCAudioTimer(QWidget* parent, Doc* doc)
    : VCWidget(parent, doc)
    , m_dragID(Function::invalidId())
    , m_dragPosition(0)
{
    /* Set the class name "VCAudioTimer" as the object name as well */
    setObjectName(VCAudioTimer::staticMetaObject.className());

    setType(VCWidget::AudioTimerWidget);
    setCaption(tr("Audio Timer"));
    resize(QSize(360, 90));

    m_clock.start();

    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotUpdate()));
    m_timer->start(IDLE_INTERVAL);
}

VCAudioTimer::~VCAudioTimer()
{
}

void VCAudioTimer::slotUpdate()
{
    refreshEntries();

    /* refresh at display rate only while something is playing */
    bool playing = false;
    QString state;
    foreach (Entry entry, m_entries)
    {
        if (entry.paused == false)
            playing = true;
        state.append(QString("%1|%2|%3|%4|%5;").arg(entry.name).arg(entry.elapsed)
                                               .arg(entry.total).arg(entry.paused)
                                               .arg(trackAnalysis(m_doc, entry.file).ready));
    }

    int interval = playing || m_dragID != Function::invalidId() ? FRAME_INTERVAL : IDLE_INTERVAL;
    if (m_timer->interval() != interval)
        m_timer->setInterval(interval);

    if (isVisible() && state != m_lastState)
    {
        m_lastState = state;
        update();
    }
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCAudioTimer::createCopy(VCWidget* parent) const
{
    Q_ASSERT(parent != NULL);

    VCAudioTimer* timer = new VCAudioTimer(parent, m_doc);
    if (timer->copyFrom(this) == false)
    {
        delete timer;
        timer = NULL;
    }

    return timer;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCAudioTimer::editProperties()
{
    bool ok = false;
    QString text = QInputDialog::getText(NULL, tr("Rename Audio Timer"), tr("Caption:"),
                                         QLineEdit::Normal, caption(), &ok);
    if (ok == true)
        setCaption(text);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCAudioTimer::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCAudioTimer)
    {
        qWarning() << Q_FUNC_INFO << "Audio timer node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown audio timer tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCAudioTimer::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* VC Audio Timer entry */
    doc->writeStartElement(KXMLQLCVCAudioTimer);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* End the <AudioTimer> tag */
    doc->writeEndElement();

    return true;
}

/****************************************************************************
 * Drawing
 ****************************************************************************/

void VCAudioTimer::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setFont(font());

    QColor fg = palette().color(foregroundRole());
    QRect area = rect().adjusted(4, 2, -4, -2);
    const QList<Entry> &list = m_entries;
    m_buttons.clear();
    m_seekAreas.clear();

    if (list.isEmpty())
    {
        QColor dim = fg;
        dim.setAlpha(120);
        painter.setPen(dim);
        painter.drawText(area, Qt::AlignCenter | Qt::TextWordWrap, caption());
    }
    else
    {
        QFontMetrics fm(font());
        int barHeight = qMax(4, fm.height() / 4);
        int rowHeight = qMax(area.height() / list.count(), fm.height() + barHeight + 4);
        int y = area.top();

        foreach (Entry entry, list)
        {
            AudioAnalysis analysis = trackAnalysis(m_doc, entry.file);

            QRect textRect(area.left(), y, area.width(), fm.height());
            QString times;
            if (entry.total)
                times = QString("%1 / %2   -%3").arg(msToString(entry.elapsed),
                                                     msToString(entry.total),
                                                     msToString(entry.total - entry.elapsed));
            else
                times = msToString(entry.elapsed);

            if (analysis.ready && analysis.bpm > 0)
                times.prepend(QString("%1 BPM   ").arg(analysis.bpm, 0, 'f', 1));

            /* Play/Pause and Stop buttons */
            int btnSize = fm.height();
            QRect playRect(textRect.left(), textRect.top(), btnSize, btnSize);
            QRect stopRect(playRect.right() + 4, textRect.top(), btnSize, btnSize);
            drawButton(painter, playRect, entry.paused ? PlayButton : PauseButton, fg);
            drawButton(painter, stopRect, StopButton, fg);
            m_buttons.append(ButtonArea(entry.id, PauseButton, playRect));
            m_buttons.append(ButtonArea(entry.id, StopButton, stopRect));

            int timesWidth = fm.horizontalAdvance(times);
            painter.setPen(fg);
            painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, times);
            QRect nameRect = textRect.adjusted(stopRect.right() + 8 - textRect.left(), 0, -(timesWidth + 10), 0);
            painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter,
                             fm.elidedText(entry.name, Qt::ElideRight, nameRect.width()));

            if (entry.total)
            {
                // the last 10 seconds are highlighted in red, a paused track in orange
                bool ending = entry.total - entry.elapsed <= 10000;
                QColor played = entry.paused ? QColor(230, 160, 40) :
                                ending ? QColor(220, 50, 50) : QColor(80, 200, 80);
                QColor groove = fg;
                groove.setAlpha(50);
                double progress = (double)entry.elapsed / entry.total;

                int top = textRect.bottom() + 2;
                int waveHeight = qMax(barHeight, y + rowHeight - top - 2);
                QRect waveRect(area.left(), top, area.width(), waveHeight);
                m_seekAreas.append({ entry.id, waveRect, entry.total });

                /* while dragging, show where the track will be moved to */
                if (entry.id == m_dragID)
                    progress = qMin(1.0, (double)m_dragPosition / entry.total);

                painter.setPen(Qt::NoPen);
                if (analysis.ready && waveRect.height() > barHeight)
                {
                    QColor unplayed = fg;
                    unplayed.setAlpha(90);

                    /* render the waveform only when the track, size or colors change */
                    WaveCache &cache = m_waveCache[entry.id];
                    qreal dpr = devicePixelRatioF();
                    if (cache.file != entry.file || cache.size != waveRect.size() ||
                        cache.unplayedColor != unplayed || cache.unplayed.devicePixelRatio() != dpr)
                    {
                        cache.file = entry.file;
                        cache.size = waveRect.size();
                        cache.unplayedColor = unplayed;
                        cache.unplayed = renderWaveform(analysis.peaks, waveRect.size(), dpr, unplayed);
                        cache.playedColor = QColor();
                    }
                    if (cache.playedColor != played)
                    {
                        cache.playedColor = played;
                        cache.played = renderWaveform(analysis.peaks, waveRect.size(), dpr, played);
                    }

                    /* played part on the left of the playhead, with sub-pixel precision */
                    qreal playedWidth = waveRect.width() * progress;
                    painter.drawPixmap(waveRect.topLeft(), cache.unplayed);
                    painter.drawPixmap(QRectF(waveRect.left(), waveRect.top(), playedWidth, waveRect.height()),
                                       cache.played,
                                       QRectF(0, 0, playedWidth * dpr, waveRect.height() * dpr));

                    /* playhead */
                    painter.fillRect(QRectF(waveRect.left() + playedWidth - 1, waveRect.top(),
                                            2, waveRect.height()), fg);
                }
                else
                {
                    QRect barRect(area.left(), top, area.width(), barHeight);
                    painter.setBrush(groove);
                    painter.drawRect(barRect);
                    painter.setBrush(played);
                    barRect.setWidth((int)(barRect.width() * progress));
                    painter.drawRect(barRect);
                }
            }

            y += rowHeight;
        }
    }

    painter.end();

    VCWidget::paintEvent(e);
}

void VCAudioTimer::drawButton(QPainter &painter, const QRect &r, ButtonType type, const QColor &color)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bg = color;
    bg.setAlpha(40);
    painter.setPen(QPen(color, 1));
    painter.setBrush(bg);
    painter.drawRoundedRect(QRectF(r).adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    QRectF g = QRectF(r).adjusted(r.width() * 0.3, r.height() * 0.28, -r.width() * 0.3, -r.height() * 0.28);

    if (type == PlayButton)
    {
        QPolygonF triangle;
        triangle << g.topLeft() << QPointF(g.right(), g.center().y()) << g.bottomLeft();
        painter.drawPolygon(triangle);
    }
    else if (type == PauseButton)
    {
        qreal w = g.width() / 3;
        painter.drawRect(QRectF(g.left(), g.top(), w, g.height()));
        painter.drawRect(QRectF(g.right() - w, g.top(), w, g.height()));
    }
    else
    {
        painter.drawRect(g);
    }

    painter.restore();
}

void VCAudioTimer::mousePressEvent(QMouseEvent *e)
{
    if (mode() != Doc::Operate || isDisabled())
    {
        VCWidget::mousePressEvent(e);
        return;
    }

    foreach (SeekArea area, m_seekAreas)
    {
        if (area.rect.contains(e->pos()) == false || area.rect.width() <= 0)
            continue;

        m_dragID = area.functionID;
        m_dragPosition = (quint64)(e->pos().x() - area.rect.left()) * area.total / area.rect.width();
        update();
        return;
    }

    foreach (ButtonArea area, m_buttons)
    {
        if (area.rect.contains(e->pos()) == false)
            continue;

        Function *f = m_doc->function(area.functionID);
        if (f == NULL || f->isRunning() == false)
            break;

        if (area.type == StopButton)
            f->stop(FunctionParent::master());
        else
            f->setPause(!f->isPaused());

        update();
        break;
    }
}

void VCAudioTimer::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragID == Function::invalidId())
    {
        VCWidget::mouseMoveEvent(e);
        return;
    }

    foreach (SeekArea area, m_seekAreas)
    {
        if (area.functionID != m_dragID || area.rect.width() <= 0)
            continue;

        int x = qBound(area.rect.left(), e->pos().x(), area.rect.right());
        m_dragPosition = (quint64)(x - area.rect.left()) * area.total / area.rect.width();
        update();
        break;
    }
}

void VCAudioTimer::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_dragID == Function::invalidId())
    {
        VCWidget::mouseReleaseEvent(e);
        return;
    }

    /* the track is moved only once, when the mouse is released */
    Audio *audio = qobject_cast<Audio*>(m_doc->function(m_dragID));
    if (audio != NULL)
    {
        audio->requestSeek(m_dragPosition);
        qint64 now = m_clock.elapsed();
        m_pendingSeeks[m_dragID] = qMakePair(m_dragPosition, now);
        m_smoothing[m_dragID] = { m_dragPosition, now, m_dragPosition };
    }

    m_dragID = Function::invalidId();
    update();
}
