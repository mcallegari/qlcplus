/*
  Q Light Controller - Unit test
  universeperf_test.cpp

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

#include <QtTest>
#include <QElapsedTimer>
#include <QVector>
#include <QSharedPointer>
#include <QPluginLoader>
#include <QFileInfo>
#include <memory>

#include "universeperf_test.h"

#include "doc.h"
#include "fixture.h"
#include "fadechannel.h"
#include "grandmaster.h"
#include "genericfader.h"
#include "mastertimer.h"
#include "universe.h"
#include "ioplugincache.h"
#include "qlcioplugin.h"
#include "outputpatch.h"

namespace
{
#define PERF_UNIVERSES      150
#define PERF_CHANNELS       400
#define PERF_FADERS         300
#define PERF_WARMUP_TICKS   15
#define PERF_BENCHMARK_TICKS 150
#define PERF_FADE_MS        2000

class TestUniverse : public Universe
{
public:
    using Universe::Universe;
    using Universe::processFaders;
};

struct ArtNetPluginHandle
{
    std::unique_ptr<QPluginLoader> loader;
    QLCIOPlugin *plugin = NULL;
};

QStringList artnetPluginCandidates()
{
    QStringList candidates;
    QStringList pluginDirs;
    const QString envDir = qEnvironmentVariable("QLC_PLUGIN_DIR");
    if (!envDir.isEmpty())
        pluginDirs << envDir;

    pluginDirs << IOPluginCache::systemPluginDirectory().absolutePath();
    pluginDirs << QDir::current().absoluteFilePath(QStringLiteral("plugins/artnet/src"));
    pluginDirs << QDir::current().absoluteFilePath(QStringLiteral("../plugins/artnet/src"));
    pluginDirs << QDir::current().absoluteFilePath(QStringLiteral("build/plugins/artnet/src"));
    pluginDirs << QDir::current().absoluteFilePath(QStringLiteral("../build/plugins/artnet/src"));

#if defined(Q_OS_WIN)
    const QString pluginName = QStringLiteral("artnet.dll");
#elif defined(Q_OS_MACOS)
    const QString pluginName = QStringLiteral("libartnet.dylib");
#else
    const QString pluginName = QStringLiteral("libartnet.so");
#endif

    for (const QString &entry : pluginDirs)
    {
        QFileInfo fi(entry);
        if (fi.isFile())
        {
            candidates << fi.absoluteFilePath();
            continue;
        }

        QDir dir(entry);
        if (!dir.exists())
            continue;

        QString candidate = dir.absoluteFilePath(pluginName);
        if (QFileInfo::exists(candidate))
            candidates << candidate;
    }

    return candidates;
}

QLCIOPlugin *loadArtNetPlugin(Doc &doc, ArtNetPluginHandle &handle)
{
    foreach (const QString &pluginPath, artnetPluginCandidates())
    {
        std::unique_ptr<QPluginLoader> loader(new QPluginLoader(pluginPath));
        QObject *instance = loader->instance();
        if (instance == NULL)
            continue;

        QLCIOPlugin *plugin = qobject_cast<QLCIOPlugin*>(instance);
        if (plugin != NULL && plugin->name() == QStringLiteral("ArtNet"))
        {
            plugin->init();
            handle.loader = std::move(loader);
            handle.plugin = plugin;
            return plugin;
        }

        loader->unload();
    }

    IOPluginCache *cache = doc.ioPluginCache();
    if (cache == NULL)
        return NULL;

    QLCIOPlugin *plugin = cache->plugin(QStringLiteral("ArtNet"));
    if (plugin != NULL)
        return plugin;

    for (const QString &dirPath : artnetPluginCandidates())
    {
        QDir dir = QFileInfo(dirPath).isDir() ? QDir(dirPath) : QFileInfo(dirPath).absoluteDir();
        if (!dir.exists())
            continue;
        cache->load(dir);
        plugin = cache->plugin(QStringLiteral("ArtNet"));
        if (plugin != NULL)
            return plugin;
    }

    return handle.plugin;
}
}

void UniversePerf_Test::benchmarkLargeUniverseMix()
{
    const int universesCount = qBound(1, PERF_UNIVERSES, 256);
    const int channelsCount = qBound(1, PERF_CHANNELS, int(UNIVERSE_SIZE));
    const int fadersCount = qBound(1, PERF_FADERS, 16);
    const int warmupTicks = qBound(0, PERF_WARMUP_TICKS, 10000);
    const int benchmarkTicks = qBound(1, PERF_BENCHMARK_TICKS, 100000);
    const int fadeTimeMs = qBound(1, PERF_FADE_MS, 120000);

    GrandMaster gm(this);
    Doc doc(this, 0);
    ArtNetPluginHandle artnetHandle;
    QObject universeOwner;
    QLCIOPlugin *artnet = loadArtNetPlugin(doc, artnetHandle);
    if (artnet == NULL)
        QSKIP("ArtNet plugin not available. Set QLC_PLUGIN_DIR or build/load ArtNet plugin.");

    QStringList outputs = artnet->outputs();
    int outputLine = outputs.indexOf(QStringLiteral("127.0.0.1"));
    bool loopbackLine = true;
    if (outputLine < 0)
    {
        for (int i = 0; i < outputs.size(); ++i)
        {
            if (outputs.at(i).startsWith(QStringLiteral("127.")))
            {
                outputLine = i;
                break;
            }
        }
    }
    if (outputLine < 0)
    {
        if (outputs.isEmpty())
            QSKIP("ArtNet exposes no output lines on this machine.");
        outputLine = 0;
        loopbackLine = false;
    }

    qInfo().noquote()
        << QStringLiteral("[UniversePerf] artnet_line=%1 line_ip=%2 mode=%3")
              .arg(outputLine)
              .arg(outputs.value(outputLine, QStringLiteral("<none>")))
              .arg(loopbackLine ? QStringLiteral("loopback-line") : QStringLiteral("forced-loopback-destination"));

    QVector<TestUniverse*> universes;
    universes.reserve(universesCount);
    QVector<QVector<QSharedPointer<GenericFader>>> universeFaders;
    universeFaders.resize(universesCount);

    for (int u = 0; u < universesCount; ++u)
    {
        TestUniverse *universe = new TestUniverse(quint32(u), &gm, &universeOwner);
        universes.append(universe);
        universeFaders[u].reserve(fadersCount);
        QVERIFY(universe->setOutputPatch(artnet, quint32(outputLine), 0));
        QVERIFY(universe->outputPatch(0) != NULL);
        universe->outputPatch(0)->setPluginParameter(QStringLiteral("outputIP"), QStringLiteral("127.0.0.1"));
        universe->outputPatch(0)->setPluginParameter(QStringLiteral("transmitMode"), QStringLiteral("Full"));

        for (int f = 0; f < fadersCount; ++f)
        {
            QSharedPointer<GenericFader> fader = universe->requestFader(Universe::FaderPriority(f % 4));
            fader->setName(QStringLiteral("perf_u%1_f%2").arg(u).arg(f));
            fader->setParentFunctionID(quint32(f + 1));
            fader->setBlendMode(Universe::BlendMode(f % 4));
            fader->setHandleSecondary((f % 2) == 0);
            universeFaders[u].append(fader);
        }

        for (int ch = 0; ch < channelsCount; ++ch)
        {
            const bool intensityChannel = (ch % 3) != 0;
            universe->setChannelCapability(ushort(ch), intensityChannel ? QLCChannel::Intensity : QLCChannel::Pan);

            QSharedPointer<GenericFader> fader = universeFaders[u].at(ch % fadersCount);
            const quint32 channel = quint32(ch);
            const quint32 target = quint32((u * 17 + ch * 11) & 0xFF);
            const int channelPattern = ch % 21;
            const int fadeMs = fadeTimeMs + (ch % 5) * MasterTimer::tick();

            fader->updateChannel(&doc, universe, Fixture::invalidId(), channel,
                                 [intensityChannel, target, channelPattern, fadeMs](FadeChannel &fc)
            {
                fc.setStart(0);
                fc.setCurrent(0);
                fc.setElapsed(0);
                fc.setReady(false);
                fc.setFadeTime(uint(fadeMs));

                fc.removeFlag(FadeChannel::Relative);
                fc.removeFlag(FadeChannel::Flashing);
                fc.removeFlag(FadeChannel::Override);
                fc.removeFlag(FadeChannel::ForceLTP);

                if (intensityChannel)
                {
                    fc.addFlag(FadeChannel::HTP);
                    fc.addFlag(FadeChannel::Intensity);
                    fc.removeFlag(FadeChannel::LTP);
                }
                else
                {
                    fc.removeFlag(FadeChannel::HTP);
                    fc.removeFlag(FadeChannel::Intensity);
                    fc.addFlag(FadeChannel::LTP);
                }

                if (channelPattern == 0)
                {
                    fc.addFlag(FadeChannel::Relative);
                    fc.setTarget(quint32(0x7F + ((target % 9) - 4)));
                }
                else if (channelPattern == 1)
                {
                    fc.addFlag(FadeChannel::Override);
                    fc.setTarget(target);
                }
                else if (channelPattern == 2)
                {
                    fc.addFlag(FadeChannel::Flashing);
                    fc.addFlag(FadeChannel::ForceLTP);
                    fc.setTarget(target);
                }
                else
                {
                    fc.setTarget(target);
                }
            });
        }
    }

    for (int tick = 0; tick < warmupTicks; ++tick)
    {
        for (int u = 0; u < universes.size(); ++u)
        {
            if ((tick % 8) == 0)
                universeFaders[u].at(0)->adjustIntensity((tick % 16) ? 0.75 : 1.0);
            universes[u]->processFaders(MasterTimer::tick());
        }
    }

    QElapsedTimer timer;
    timer.start();
    for (int tick = 0; tick < benchmarkTicks; ++tick)
    {
        for (int u = 0; u < universes.size(); ++u)
        {
            if ((tick % 8) == 0)
                universeFaders[u].at(0)->adjustIntensity((tick % 16) ? 0.75 : 1.0);
            universes[u]->processFaders(MasterTimer::tick());
        }
    }
    const qint64 elapsedNs = timer.nsecsElapsed();
    const double elapsedMs = double(elapsedNs) / 1000000.0;

    const double totalUniverseTicks = double(universesCount) * double(benchmarkTicks);
    const double tickAvgMs = elapsedMs / double(benchmarkTicks);
    const double universeTickAvgMs = elapsedMs / totalUniverseTicks;
    const double budgetUsage = (tickAvgMs / double(MasterTimer::tick())) * 100.0;

    qInfo().noquote()
        << QStringLiteral("[UniversePerf] universes=%1 channels=%2 faders=%3 warmup_ticks=%4 benchmark_ticks=%5 fade_ms=%6 elapsed_ns=%7 elapsed_ms=%8 tick_avg_ms=%9 universe_tick_avg_ms=%10 tick_budget_use_pct=%11")
               .arg(universesCount)
               .arg(channelsCount)
               .arg(fadersCount)
               .arg(warmupTicks)
               .arg(benchmarkTicks)
               .arg(fadeTimeMs)
               .arg(elapsedNs)
               .arg(elapsedMs, 0, 'f', 3)
               .arg(tickAvgMs, 0, 'f', 6)
               .arg(universeTickAvgMs, 0, 'f', 6)
               .arg(budgetUsage, 0, 'f', 2);

    QCOMPARE(universes.size(), universesCount);
    for (int u = 0; u < universes.size(); ++u)
        QVERIFY(universes[u]->usedChannels() >= ushort(channelsCount));

    universeFaders.clear();
    universes.clear();
}

QTEST_GUILESS_MAIN(UniversePerf_Test)
