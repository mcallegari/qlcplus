/*
  Q Light Controller Plus
  channeledit.cpp

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

#include <QQmlEngine>
#include <algorithm>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QVector>

#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "colorfilters.h"
#include "channeledit.h"

ChannelEdit::ChannelEdit(QLCChannel *channel, QObject *parent)
    : QObject(parent)
    , m_channel(channel)
{
    if (m_channel->capabilities().count() == 0)
    {
        QLCCapability *cap = new QLCCapability(0, UCHAR_MAX);
        QQmlEngine::setObjectOwnership(cap, QQmlEngine::CppOwnership);
        cap->setWarning(QLCCapability::EmptyName);
        m_channel->addCapability(cap);
    }

    connect(m_channel, SIGNAL(presetChanged()), this, SLOT(setupPreset()));
    connect(m_channel, SIGNAL(nameChanged()), this, SIGNAL(channelChanged()));
    connect(m_channel, SIGNAL(defaultValueChanged()), this, SIGNAL(channelChanged()));
    connect(m_channel, SIGNAL(controlByteChanged()), this, SIGNAL(channelChanged()));

    updateCapabilities();
}

ChannelEdit::~ChannelEdit()
{
}

QString ChannelEdit::normalizeWords(const QString &input) const
{
    QString normalized = input.toLower();
    normalized.replace(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")), QStringLiteral(" "));
    return normalized.simplified();
}

bool ChannelEdit::isEntirelyLowercase(const QString &text) const
{
    bool hasLetters = false;

    for (const QChar &ch : text)
    {
        if (!ch.isLetter())
            continue;

        hasLetters = true;
        if (ch != ch.toLower())
            return false;
    }

    return hasLetters;
}

QString ChannelEdit::titleCaseWords(const QString &text) const
{
    QString title = text;
    bool wordStart = true;

    for (int i = 0; i < title.length(); i++)
    {
        QChar ch = title.at(i);
        if (ch.isLetterOrNumber())
        {
            if (wordStart && ch.isLetter())
                title[i] = ch.toUpper();
            wordStart = false;
        }
        else
        {
            wordStart = true;
        }
    }

    return title;
}

int ChannelEdit::fuzzyScore(const QString &a, const QString &b) const
{
    if (a == b)
        return 1000;

    const int maxLen = qMax(a.length(), b.length());
    if (maxLen == 0)
        return 1000;

    QVector<int> prev(b.length() + 1, 0);
    QVector<int> curr(b.length() + 1, 0);

    for (int j = 0; j <= b.length(); j++)
        prev[j] = j;

    for (int i = 1; i <= a.length(); i++)
    {
        curr[0] = i;
        for (int j = 1; j <= b.length(); j++)
        {
            const int cost = (a.at(i - 1) == b.at(j - 1)) ? 0 : 1;
            curr[j] = qMin(qMin(curr[j - 1] + 1, prev[j] + 1), prev[j - 1] + cost);
        }
        prev = curr;
    }

    const int dist = prev[b.length()];
    return ((maxLen - dist) * 1000) / maxLen;
}

QList<QColor> ChannelEdit::detectNamedColors(const QString &description, const QVariantList &namedColors) const
{
    QList<QColor> detected;
    if (namedColors.isEmpty())
        return detected;

    struct ColorEntry
    {
        QString name;
        QString normalized;
        QString compact;
        QColor rgb;
    };
    QVector<ColorEntry> colors;
    colors.reserve(namedColors.count());

    for (const QVariant &entryVar : namedColors)
    {
        const QVariantMap entry = entryVar.toMap();
        const QString name = entry.value(QStringLiteral("name")).toString().trimmed();
        const QColor rgb = entry.value(QStringLiteral("rgb")).value<QColor>();
        if (name.isEmpty() || !rgb.isValid())
            continue;

        const QString normalizedName = normalizeWords(name);
        if (normalizedName.isEmpty())
            continue;

        QString compact = normalizedName;
        compact.remove(QLatin1Char(' '));
        colors.append({ name, normalizedName, compact, rgb });
    }

    const QString normalizedDesc = normalizeWords(description);
    if (normalizedDesc.isEmpty() || colors.isEmpty())
        return detected;

    QSet<QString> selectedNames;
    QString remainingDesc = normalizedDesc;

    struct ExactMatch
    {
        int startPos;
        int endPos;
        int len;
        int colorIndex;
    };
    QList<ExactMatch> exactMatches;

    for (int i = 0; i < colors.count(); i++)
    {
        QString pattern = QRegularExpression::escape(colors.at(i).normalized);
        pattern.replace(QStringLiteral("\\ "), QStringLiteral("\\s+"));
        QRegularExpression re(QStringLiteral("\\b%1\\b").arg(pattern));

        auto matches = re.globalMatch(remainingDesc);
        while (matches.hasNext())
        {
            const auto match = matches.next();
            exactMatches.append({ int(match.capturedStart()), int(match.capturedEnd()),
                                  int(colors.at(i).normalized.length()), i });
        }
    }

    std::sort(exactMatches.begin(), exactMatches.end(), [](const ExactMatch &a, const ExactMatch &b) {
        if (a.startPos != b.startPos)
            return a.startPos < b.startPos;
        return a.len > b.len;
    });

    QList<QPair<int, int> > occupiedRanges;
    for (const ExactMatch &match : exactMatches)
    {
        bool overlaps = false;
        for (const auto &range : occupiedRanges)
        {
            if (match.startPos < range.second && match.endPos > range.first)
            {
                overlaps = true;
                break;
            }
        }
        if (overlaps)
            continue;

        const ColorEntry &color = colors.at(match.colorIndex);
        if (selectedNames.contains(color.name))
            continue;

        selectedNames.insert(color.name);
        detected.append(color.rgb);
        occupiedRanges.append(qMakePair(match.startPos, match.endPos));
        remainingDesc.replace(match.startPos, match.endPos - match.startPos,
                              QString(match.endPos - match.startPos, QLatin1Char(' ')));

        if (detected.count() >= 2)
            return detected;
    }

    const QString unmatchedDesc = remainingDesc.simplified();
    if (unmatchedDesc.isEmpty())
        return detected;

    QStringList chunks;
    const QStringList tokens = unmatchedDesc.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    chunks.reserve(tokens.count() * 2);
    for (int i = 0; i < tokens.count(); i++)
    {
        chunks.append(tokens.at(i));
        if (i + 1 < tokens.count())
            chunks.append(tokens.at(i) + QLatin1Char(' ') + tokens.at(i + 1));
    }

    // Simple fuzzy fallback on 1-2 word chunks
    for (const QString &chunk : chunks)
    {
        QString compactChunk = chunk;
        compactChunk.remove(QLatin1Char(' '));
        if (compactChunk.length() < 3)
            continue;
        const bool chunkHasDigits = chunk.contains(QRegularExpression(QStringLiteral("\\d")));

        int bestIndex = -1;
        int bestScore = 0;
        for (int i = 0; i < colors.count(); i++)
        {
            if (selectedNames.contains(colors.at(i).name))
                continue;
            if (!chunkHasDigits && colors.at(i).normalized.contains(QRegularExpression(QStringLiteral("\\s\\d+$"))))
                continue;

            const int score = fuzzyScore(compactChunk, colors.at(i).compact);
            if (score > bestScore)
            {
                bestScore = score;
                bestIndex = i;
            }
        }
        if (bestIndex < 0 || bestScore < 850)
            continue;

        const ColorEntry &color = colors.at(bestIndex);
        selectedNames.insert(color.name);
        detected.append(color.rgb);
        if (detected.count() >= 2)
            break;
    }

    return detected;
}

QLCChannel *ChannelEdit::channel()
{
    return m_channel;
}

QVariantList ChannelEdit::channelPresetList() const
{
    QVariantList list;

    QVariantMap custom;
    custom.insert("mIcon", "qrc:/edit.svg");
    custom.insert("mLabel", tr("Custom"));
    custom.insert("mValue", 0);
    list.append(custom);

    for (int i = QLCChannel::Custom + 1; i < QLCChannel::LastPreset; i++)
    {
        QLCChannel ch;
        ch.setPreset(QLCChannel::Preset(i));
        QVariantMap chMap;
        chMap.insert("mIcon", ch.getIconNameFromGroup(ch.group(), true));
        chMap.insert("mLabel", ch.name() + " (" + ch.presetToString(QLCChannel::Preset(i)) + ")");
        chMap.insert("mValue", i);
        list.append(chMap);
    }

    return list;
}

QVariantList ChannelEdit::capabilityPresetList() const
{
    QVariantList list;

    QVariantMap custom;
    //custom.insert("mIcon", "qrc:/edit.svg");
    custom.insert("mLabel", tr("Custom"));
    custom.insert("mValue", 0);
    list.append(custom);

    for (int i = QLCCapability::Custom + 1; i < QLCCapability::LastPreset; i++)
    {
        QLCCapability cap;
        QVariantMap capMap;
        cap.setPreset(QLCCapability::Preset(i));
        capMap.insert("mLabel", cap.presetToString(QLCCapability::Preset(i)));
        capMap.insert("mValue", i);
        list.append(capMap);
    }

    return list;
}

int ChannelEdit::group() const
{
    QLCChannel::Group grp = m_channel->group();

    if (grp == QLCChannel::Intensity)
        return int(m_channel->colour());
    else
        return int(grp);
}

void ChannelEdit::setGroup(int group)
{
    if (group > QLCChannel::Nothing && group < QLCChannel::NoGroup)
    {
        m_channel->setColour(QLCChannel::PrimaryColour(group));
        m_channel->setGroup(QLCChannel::Intensity);
    }
    else
    {
        m_channel->setColour(QLCChannel::NoColour);
        m_channel->setGroup(QLCChannel::Group(group));
    }

    emit groupChanged();
    emit channelChanged();
}

QVariantList ChannelEdit::channelTypeList() const
{
    QVariantList list;

    for (QString &grp : QLCChannel::groupList())
    {
        QLCChannel ch;
        ch.setGroup(QLCChannel::stringToGroup(grp));

        QVariantMap chMap;
        chMap.insert("mIcon", ch.getIconNameFromGroup(ch.group(), true));
        chMap.insert("mLabel", ch.groupToString(ch.group()));
        chMap.insert("mValue", ch.group());
        list.append(chMap);

        if (ch.group() == QLCChannel::Intensity)
        {
            for (QString &color : QLCChannel::colourList())
            {
                QLCChannel cc;
                cc.setGroup(QLCChannel::Intensity);
                cc.setColour(QLCChannel::stringToColour(color));

                QVariantMap chMap;
                chMap.insert("mIcon", cc.getIconNameFromGroup(ch.group(), true));
                chMap.insert("mLabel", cc.colourToString(cc.colour()));
                chMap.insert("mValue", cc.colour());
                list.append(chMap);
            }
        }
    }

    return list;
}

void ChannelEdit::updateCapabilities()
{
    m_capabilities.clear();

    for (QLCCapability *cap : m_channel->capabilities())
    {
        QVariantMap capMap;
        capMap.insert("cRef", QVariant::fromValue(cap));
        m_capabilities.append(capMap);
    }

    emit capabilitiesChanged();
}

void ChannelEdit::setupPreset()
{
    if (m_channel->preset() == QLCChannel::Custom)
    {
        emit channelChanged();
        return;
    }

    for (QLCCapability *cap : m_channel->capabilities())
        m_channel->removeCapability(cap);

    m_channel->addPresetCapability();

    updateCapabilities();

    emit channelChanged();
}

QVariantList ChannelEdit::capabilities() const
{
    return m_capabilities;
}

QLCCapability *ChannelEdit::addNewCapability()
{
    int min = 0;
    if (m_channel->capabilities().count())
    {
        QLCCapability *last = m_channel->capabilities().last();
        min = last->max() + 1;
    }
    QLCCapability *cap = new QLCCapability(min, UCHAR_MAX);
    QQmlEngine::setObjectOwnership(cap, QQmlEngine::CppOwnership);
    cap->setWarning(QLCCapability::EmptyName);
    if (m_channel->addCapability(cap))
    {
        updateCapabilities();
    }
    else
    {
        delete cap;
        return nullptr;
    }

    return cap;
}

QLCCapability *ChannelEdit::addCapability(int min, int max, QString name)
{
    QLCCapability *cap = new QLCCapability(min, max);
    QQmlEngine::setObjectOwnership(cap, QQmlEngine::CppOwnership);
    cap->setName(name);

    if (m_channel->addCapability(cap))
    {
        updateCapabilities();
    }
    else
    {
        delete cap;
        return nullptr;
    }

    return cap;
}

bool ChannelEdit::checkAvailability(int startAddress, int amount)
{
    int endAddress = startAddress + amount - 1;
    for (QLCCapability *cap : m_channel->capabilities())
    {
        if (cap->max() >= startAddress && cap->min() <= endAddress)
            return false;
    }

    return true;
}

void ChannelEdit::removeCapabilityAtIndex(int index)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return;

    if (m_channel->removeCapability(caps[index]))
        updateCapabilities();
}

int ChannelEdit::getCapabilityPresetAtIndex(int index)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return 0;

    return caps.at(index)->preset();
}

void ChannelEdit::setCapabilityPresetAtIndex(int index, int preset)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return;

    QLCCapability *cap = caps.at(index);
    cap->setPreset(QLCCapability::Preset(preset));
    emit channelChanged();
}

int ChannelEdit::getCapabilityPresetType(int index)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return QLCCapability::None;

    return caps.at(index)->presetType();
}

QString ChannelEdit::getCapabilityPresetUnits(int index)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return QString();

    return caps.at(index)->presetUnits();
}

QVariant ChannelEdit::getCapabilityValueAt(int index, int vIndex)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return QVariant();

    return caps.at(index)->resource(vIndex);
}

void ChannelEdit::setCapabilityValueAt(int index, int vIndex, QVariant value)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return;

    caps.at(index)->setResource(vIndex, value);
    emit channelChanged();
}

void ChannelEdit::checkCapabilities()
{
    QVector<bool>allocation;
    allocation.fill(false, 256);

    QListIterator <QLCCapability*> it(m_channel->capabilities());
    while (it.hasNext() == true)
    {
        QLCCapability *cap = it.next();
        cap->setWarning(QLCCapability::NoWarning);
        if (cap->name().isEmpty())
            cap->setWarning(QLCCapability::EmptyName);

        for (int i = cap->min(); i <= cap->max(); i++)
        {
            if (allocation[i] == true)
                cap->setWarning(QLCCapability::Overlapping);
            else
                allocation[i] = true;
        }
    }
}

void ChannelEdit::autoPatchColorCapabilities()
{
    if (m_channel == nullptr || m_channel->group() != QLCChannel::Colour)
        return;

    QVariantList namedColors;
    QDir filtersDir = QLCFile::systemDirectory(QString(COLORFILTERSDIR), QString(KExtColorFilters));
    QString namedRgbPath = filtersDir.filePath(QStringLiteral("namedrgb.qxcf"));
    if (!QFileInfo::exists(namedRgbPath))
    {
        QStringList matches = filtersDir.entryList(QStringList() << QStringLiteral("*namedrgb*.qxcf"),
                                                   QDir::Files | QDir::Readable);
        if (!matches.isEmpty())
            namedRgbPath = filtersDir.filePath(matches.first());
    }
    if (QFileInfo::exists(namedRgbPath))
    {
        ColorFilters namedRGB;
        if (namedRGB.loadXML(namedRgbPath) == QFile::NoError)
            namedColors = namedRGB.filtersList();
    }

    bool changed = false;

    for (QLCCapability *cap : m_channel->capabilities())
    {
        if (cap == nullptr)
            continue;

        if (isEntirelyLowercase(cap->name()))
        {
            cap->setName(titleCaseWords(cap->name()));
            changed = true;
        }

        const QList<QColor> colors = detectNamedColors(cap->name(), namedColors);
        if (colors.count() >= 2)
        {
            cap->setPreset(QLCCapability::ColorDoubleMacro);
            cap->setResource(0, colors.at(0));
            cap->setResource(1, colors.at(1));
            changed = true;
        }
        else if (colors.count() == 1)
        {
            cap->setPreset(QLCCapability::ColorMacro);
            cap->setResource(0, colors.at(0));
            changed = true;
        }
    }

    if (changed)
        emit channelChanged();
}
