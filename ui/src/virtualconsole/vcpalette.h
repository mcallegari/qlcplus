/*
  Q Light Controller Plus
  vcpalette.h

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

#ifndef VCPALETTE_H
#define VCPALETTE_H

#include <QSharedPointer>
#include <QMutex>
#include <QHash>
#include <QMap>
#include <QSet>

#include "dmxsource.h"
#include "vcwidget.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class GenericFader;
class QGridLayout;
class QToolButton;
class Doc;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCPalette QStringLiteral("FixturePalette")

/**
 * Console-style palettes: the operator selects some fixtures, then presses
 * a palette to apply it to them. A palette is a Scene: each selected fixture
 * takes the values of a fixture of the same model found in the Scene or,
 * when the Scene has none, the values of channels of the same type.
 * Only the channels contained in the palette are changed, and they override
 * any running function until they are released.
 */
class VCPalette final : public VCWidget, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCPalette)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCPalette(QWidget* parent, Doc* doc);
    ~VCPalette();

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    VCWidget* createCopy(VCWidget* parent) const override;

protected:
    bool copyFrom(const VCWidget* widget) override;

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    QList<quint32> fixtures() const;
    void setFixtures(const QList<quint32> &ids);

    QList<quint32> groups() const;
    void setGroups(const QList<quint32> &ids);

    /** Palettes are Scene function IDs */
    QList<quint32> palettes() const;
    void setPalettes(const QList<quint32> &ids);

    int columns() const;
    void setColumns(int columns);

    /** Rebuild all the buttons from the current configuration */
    Q_INVOKABLE void updateButtons();

private:
    QList<quint32> m_fixtures;
    QList<quint32> m_groups;
    QList<quint32> m_palettes;
    int m_columns;

    /*********************************************************************
     * Buttons
     *********************************************************************/
private:
    QToolButton *createButton(const QString &text, bool checkable);
    void updateSelectionButtons();
    /** Color shown on a palette button, taken from the RGB values of its Scene */
    QColor paletteColor(quint32 sceneID) const;

private slots:
    void slotFixtureToggled(bool checked);
    void slotGroupClicked();
    void slotPaletteClicked();
    void slotSelectAll();
    void slotSelectNone();
    void slotInvertSelection();
    void slotRelease();
    void slotReleaseAll();
    void slotFunctionRemoved(quint32 id);
    void slotFixtureRemoved(quint32 id);

private:
    QGridLayout *m_grid;
    QHash<QToolButton*, quint32> m_fixtureButtons;
    QHash<QToolButton*, quint32> m_groupButtons;
    QHash<QToolButton*, quint32> m_paletteButtons;
    QSet<quint32> m_selection;
    /** Last fixture clicked, start of a Shift+click range */
    quint32 m_lastFixture;

    /*********************************************************************
     * Palette values
     *********************************************************************/
private:
    /** Calculate the values a palette sets on a fixture: channel -> value */
    QMap<quint32, uchar> paletteValues(quint32 sceneID, quint32 fixtureID) const;

    /** Values currently applied: fixture ID -> (channel -> value) */
    QMap<quint32, QMap<quint32, uchar> > m_values;
    bool m_valuesChanged;
    QMutex m_valuesMutex;

    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;

public:
    /** @reimp */
    void writeDMX(MasterTimer *timer, QList<Universe*> universes) override;

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    void editProperties() override;

    /*****************************************************************************
     * External input
     *****************************************************************************/
    /** @reimp */
    void updateFeedback() override { }

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(QXmlStreamReader &root) override;
    bool saveXML(QXmlStreamWriter *doc) override;

    /*********************************************************************
     * Web access / mode
     *********************************************************************/
protected slots:
    void slotModeChanged(Doc::Mode mode) override;
};

/** @} */

#endif
