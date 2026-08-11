/*
  Q Light Controller Plus
  stagewizard_colors.cpp

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

#include "stagewizard.h"

#include "doc.h"
#include "fixture.h"
#include "qlccapability.h"
#include "qlcchannel.h"

#include <QtMath>
#include <QRegularExpression>

#include <algorithm>

// ── Colour-wheel matching ────────────────────────────────────────────────────
//
// Two fixtures rarely describe the same physical gel the same way. A Chauvet
// spot calls its blue "Dark blue" at #00008b; a Martin profile calls the same
// slot "Deep Blue" at #0000ff. Their green is "Green" #00ff00 against "Light
// Green" #80ff00. Matching on the exact hex (or on a quantised hex key, which
// is the same thing with coarser buckets and hard edges) finds only the few
// slots that happen to agree byte-for-byte — white, red, yellow — and drops
// everything else, even though a lighting designer would call them the same
// colour without hesitating.
//
// So matching runs on two channels:
//
//   1. NAME. A vocabulary of canonical colours, each with the words makers
//      actually use for it. "dark blue", "deep blue" and "blue" all reduce to
//      Blue. This is the strong signal: a maker naming a slot "Blue" means it.
//
//   2. COLOUR. When a name says nothing useful (a bare "Color 5", a maker's
//      house name for a gel), fall back to perceptual distance in HSV with hue
//      weighted heavily, so lightness differences between two makers' idea of
//      the same gel don't separate them, but genuinely different hues stay
//      apart.
//
// Both feed one comparison, colorsMatch(), used to intersect wheels.

namespace
{
    /** A canonical colour: the label the VC button gets, a representative
     *  colour for the swatch, and the aliases makers use. Order matters — it
     *  is the order the swatches appear in, and earlier entries win when a
     *  capability name contains more than one alias. */
    struct CanonicalColor
    {
        const char *label;
        int r, g, b;
        const char *aliases;   ///< lowercase, '|' separated, matched as words
    };

    // Ordered white -> warm -> cool, which is roughly how wheels are laid out
    // and how a designer scans them.
    const CanonicalColor kVocabulary[] = {
        { "White",      255, 255, 255, "white|open|no colour|no color" },
        { "Red",        255,   0,   0, "red|deep red" },
        { "Orange",     255, 128,   0, "orange|amber|orange red|red orange" },
        { "Yellow",     255, 255,   0, "yellow|light yellow|pale yellow" },
        { "Green",        0, 255,   0, "green|light green|dark green" },
        { "Cyan",         0, 255, 255, "cyan|turquoise|aqua|aquamarine" },
        { "Light Blue",   0, 128, 255, "light blue|pale blue|sky blue|azure" },
        { "Blue",         0,   0, 255, "blue|dark blue|deep blue|congo" },
        { "Lavender",   204, 102, 255, "lavender|violet|purple|uv" },
        { "Magenta",    255,   0, 255, "magenta|fuchsia" },
        { "Pink",       255,  85, 127, "pink|rose" },
    };
    const int kVocabularyCount = int(sizeof(kVocabulary) / sizeof(kVocabulary[0]));

    /** Colours closer than this are treated as the same gel. Calibrated so
     *  #0000ff vs #00008b (blue vs dark blue) and #00ff00 vs #80ff00 (green vs
     *  light green) match, while blue vs cyan and red vs orange do not. */
    const qreal kSameColorThreshold = 0.22;

    /** Strip the decoration makers append to slot names, so "Color 2 - Deep
     *  Blue - Stepped scrolling" is compared as "deep blue". */
    QString normalizedCapabilityName(const QString &raw)
    {
        QString s = raw.toLower();

        // Drop trailing mode descriptions after a dash: these describe HOW the
        // wheel moves, not which colour is in the gate.
        static const QRegularExpression reMode(
            QStringLiteral("\\s*-\\s*(continuous|stepped|step)\\s+scrolling.*$"));
        s.remove(reMode);

        // Drop slot numbering, both "Color 3 - x" prefixes and "(Color 3)"
        // suffixes, which otherwise make every slot name look distinct.
        static const QRegularExpression reSlotPrefix(
            QStringLiteral("^\\s*colou?r\\s*\\d+\\s*[-:]\\s*"));
        s.remove(reSlotPrefix);
        static const QRegularExpression reSlotSuffix(
            QStringLiteral("\\s*\\(\\s*colou?r\\s*\\d+\\s*\\)"));
        s.remove(reSlotSuffix);

        return s.simplified();
    }
}

QString StageWizard::canonicalColorName(const QString &capabilityName,
                                        const QColor &color)
{
    const QString name = normalizedCapabilityName(capabilityName);

    // A name that describes a transition ("white to red") names two colours and
    // therefore none: the gate holds a blend the wheel is passing through.
    if (name.contains(QStringLiteral(" to ")))
        return QString();

    // ── 1. By name ──────────────────────────────────────────────────────────
    // Aliases are matched as whole words, longest first within an entry, so
    // "dark blue" is preferred over the bare "blue" it contains, and a slot
    // called "Greenish" doesn't match "green".
    if (!name.isEmpty())
    {
        // Longest alias wins ACROSS the whole vocabulary, not merely within one
        // entry. "Orange red" contains both "orange" and "red"; the longer,
        // more specific "orange red"-style match must decide, otherwise the
        // answer depends on which vocabulary entry happens to come first.
        QString bestLabel;
        int bestLen = 0;
        for (int i = 0; i < kVocabularyCount; i++)
        {
            const QStringList aliases =
                QString::fromLatin1(kVocabulary[i].aliases).split('|');
            for (const QString &alias : aliases)
            {
                if (alias.length() <= bestLen)
                    continue;
                QRegularExpression re(QStringLiteral("\\b%1\\b")
                                      .arg(QRegularExpression::escape(alias)));
                if (re.match(name).hasMatch())
                {
                    bestLabel = QString::fromLatin1(kVocabulary[i].label);
                    bestLen = alias.length();
                }
            }
        }
        if (!bestLabel.isEmpty())
            return bestLabel;
    }

    // ── 2. By colour ────────────────────────────────────────────────────────
    // No usable name (a bare "Color 5", or a house name like "Congo"): take the
    // nearest vocabulary entry, but only when it is genuinely close, so an
    // off-vocabulary gel (CTO correction, UV) stays unmatched rather than being
    // forced onto whichever entry happens to be least far away.
    if (color.isValid())
    {
        int best = -1;
        qreal bestDist = kSameColorThreshold;
        for (int i = 0; i < kVocabularyCount; i++)
        {
            qreal d = colorDistance(color, QColor(kVocabulary[i].r,
                                                  kVocabulary[i].g,
                                                  kVocabulary[i].b));
            if (d < bestDist)
            {
                bestDist = d;
                best = i;
            }
        }
        if (best >= 0)
            return QString::fromLatin1(kVocabulary[best].label);
    }

    return QString();
}

qreal StageWizard::colorDistance(const QColor &a, const QColor &b)
{
    if (!a.isValid() || !b.isValid())
        return 1.0;

    // float, not qreal: Qt 6 narrowed QColor::getHsvF() to float* (it took
    // qreal* in Qt 5). The values are promoted to qreal for the maths below.
    float ahf, asf, avf, bhf, bsf, bvf;
    a.getHsvF(&ahf, &asf, &avf);
    b.getHsvF(&bhf, &bsf, &bvf);

    const qreal ah = ahf, as = asf, av = avf;
    const qreal bh = bhf, bs = bsf, bv = bvf;

    // An achromatic colour (black, white, any grey) has no meaningful hue —
    // QColor reports -1. Compare those on brightness alone, so white matches
    // white regardless of what hue the other side claims.
    const bool aGrey = (as < 0.15) || ah < 0.0;
    const bool bGrey = (bs < 0.15) || bh < 0.0;
    if (aGrey || bGrey)
    {
        if (aGrey != bGrey)
            return 1.0;                    // grey vs a saturated colour
        return qAbs(av - bv);              // both grey: brightness only
    }

    // Hue is circular: 0.98 and 0.02 are adjacent, not opposite.
    qreal dh = qAbs(ah - bh);
    if (dh > 0.5)
        dh = 1.0 - dh;

    // Hue dominates. Saturation and value differ wildly between makers
    // describing the same gel — one writes #0000ff, another #00008b — so they
    // contribute, but far less. Weights are normalised to keep the result 0..1.
    const qreal wH = 1.0, wS = 0.25, wV = 0.15;
    qreal d = (wH * (dh * 2.0) + wS * qAbs(as - bs) + wV * qAbs(av - bv))
              / (wH + wS + wV);
    return qBound(0.0, d, 1.0);
}

QList<StageWizard::WheelColor> StageWizard::wheelColorsOf(quint32 fixtureID) const
{
    QList<WheelColor> result;

    Fixture *fx = m_doc->fixture(fixtureID);
    if (fx == nullptr)
        return result;

    for (quint32 ch = 0; ch < fx->channels(); ++ch)
    {
        const QLCChannel *c = fx->channel(ch);
        if (c == nullptr || c->group() != QLCChannel::Colour)
            continue;

        for (const QLCCapability *cap : c->capabilities())
        {
            // Only single solid colours. ColorDoubleMacro is a split gel or a
            // crossfade between two slots, not a colour you can select.
            if (cap->preset() != QLCCapability::ColorMacro)
                continue;

            QColor col = cap->resource(0).value<QColor>();
            if (!col.isValid())
                continue;

            WheelColor wc;
            wc.fixtureID = fixtureID;
            wc.channel   = ch;
            // Mid-range: a slot spanning 8..15 is safely selected at 11, away
            // from the boundaries where a fixture may start scrolling.
            wc.value     = uchar((cap->min() + cap->max()) / 2);
            wc.name      = cap->name();
            wc.color     = col;
            result.append(wc);
        }
        break;  // only the first colour wheel
    }

    return result;
}

QList<StageWizard::CommonWheelColor>
StageWizard::commonWheelColors(const QList<quint32> &fixtureIDs) const
{
    // Wheels, one list per fixture that has one. Fixtures without a colour
    // wheel (a plain wash in the same group) are not counted as dissenters —
    // they simply don't take part.
    QList<QList<WheelColor>> wheels;
    for (quint32 fxID : fixtureIDs)
    {
        QList<WheelColor> w = wheelColorsOf(fxID);
        if (!w.isEmpty())
            wheels.append(w);
    }

    QList<CommonWheelColor> result;
    if (wheels.isEmpty())
        return result;

    // Two slots are the same colour when the vocabulary agrees on their name,
    // or — for slots the vocabulary doesn't cover — when they are perceptually
    // close. The second clause is what carries a maker's house colours across
    // models that happen to use the same gel.
    auto sameColor = [](const WheelColor &a, const WheelColor &b)
    {
        const QString an = canonicalColorName(a.name, a.color);
        const QString bn = canonicalColorName(b.name, b.color);
        if (!an.isEmpty() && !bn.isEmpty())
            return an == bn;
        if (an.isEmpty() != bn.isEmpty())
            return false;   // one is vocabulary, the other isn't
        return colorDistance(a.color, b.color) < kSameColorThreshold;
    };

    // Walk the FIRST wheel and keep the slots every other wheel also has. Using
    // one wheel as the reference is enough: a colour missing from it can't be
    // common to all of them anyway.
    const QList<WheelColor> &reference = wheels.first();
    QStringList takenLabels;    // so two slots of the same colour yield one button

    for (const WheelColor &ref : reference)
    {
        // A wheel often repeats a colour (continuous and stepped ranges for the
        // same gel). Emit it once — checked before any work is done for it.
        const QString label = canonicalColorName(ref.name, ref.color);
        const QString dedupKey = label.isEmpty() ? ref.color.name() : label;
        if (takenLabels.contains(dedupKey))
            continue;

        CommonWheelColor common;
        common.label = label;
        common.color = ref.color;
        common.members.append(ref);

        bool onEveryWheel = true;
        for (int w = 1; w < wheels.count(); w++)
        {
            const WheelColor *match = nullptr;
            qreal bestDist = 2.0;

            // Best match on this wheel, not merely the first acceptable one:
            // where a wheel carries both "Blue" and "Light Blue", the closer of
            // the two should be the one that pairs up.
            for (const WheelColor &cand : wheels.at(w))
            {
                if (!sameColor(ref, cand))
                    continue;
                qreal d = colorDistance(ref.color, cand.color);
                if (d < bestDist)
                {
                    bestDist = d;
                    match = &cand;
                }
            }

            if (match == nullptr)
            {
                onEveryWheel = false;
                break;
            }
            common.members.append(*match);
        }

        if (!onEveryWheel)
            continue;

        // Prefer the vocabulary's colour for the swatch: two makers' hexes for
        // the same gel differ, and the canonical one keeps page 0 consistent.
        if (!common.label.isEmpty())
        {
            for (int i = 0; i < kVocabularyCount; i++)
            {
                if (common.label == QString::fromLatin1(kVocabulary[i].label))
                {
                    common.color = QColor(kVocabulary[i].r, kVocabulary[i].g,
                                          kVocabulary[i].b);
                    break;
                }
            }
        }
        else
        {
            // Off-vocabulary: name it after the reference wheel's own label,
            // tidied up, so the button says something rather than nothing.
            common.label = ref.name;
        }

        takenLabels.append(dedupKey);
        result.append(common);
    }

    // Vocabulary order, with off-vocabulary colours last, so the strip reads
    // the same way on every page regardless of how the wheels are laid out.
    auto rank = [](const CommonWheelColor &c)
    {
        for (int i = 0; i < kVocabularyCount; i++)
            if (c.label == QString::fromLatin1(kVocabulary[i].label))
                return i;
        return kVocabularyCount;
    };
    std::stable_sort(result.begin(), result.end(),
                     [&](const CommonWheelColor &a, const CommonWheelColor &b)
                     {
                         return rank(a) < rank(b);
                     });

    return result;
}
