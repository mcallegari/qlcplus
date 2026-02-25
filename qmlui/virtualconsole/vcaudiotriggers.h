/*
  Q Light Controller Plus
  vcaudiotriggers.h

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

#ifndef VCAUDIOTRIGGER_H
#define VCAUDIOTRIGGER_H

#include "vcwidget.h"
#include "treemodel.h"
#include "dmxsource.h"

#define KXMLQLCVCAudioTriggers QStringLiteral("AudioTriggers")

class AudioCapture;
class VirtualConsole;

class VCAudioTriggers : public VCWidget, public DMXSource
{
    Q_OBJECT

    Q_PROPERTY(bool captureEnabled READ captureEnabled WRITE setCaptureEnabled NOTIFY captureEnabledChanged)
    Q_PROPERTY(uchar volumeLevel READ volumeLevel WRITE setVolumeLevel NOTIFY volumeLevelChanged FINAL)
    Q_PROPERTY(int barsNumber READ barsNumber WRITE setBarsNumber NOTIFY barsNumberChanged FINAL)
    Q_PROPERTY(QVariantList audioLevels READ audioLevels NOTIFY audioLevelsChanged)
    Q_PROPERTY(QVariantList barsInfo READ barsInfo NOTIFY barsInfoChanged)

    Q_PROPERTY(QVariant groupsTreeModel READ groupsTreeModel NOTIFY groupsTreeModelChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCAudioTriggers(Doc* doc = nullptr, VirtualConsole *vc = nullptr, QObject *parent = nullptr);
    virtual ~VCAudioTriggers();

    /** @reimp */
    QString defaultCaption() const override;

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page) override;

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent) override;

    /** @reimp */
    QString propertiesResource() const override;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent) const override;

    /** Get/Set the capture enable status of this widget */
    bool captureEnabled() const;
    void setCaptureEnabled(bool enable);

    /** Get/Set the capture volume level */
    uchar volumeLevel() const;
    void setVolumeLevel(uchar level);

    /** Get/Set the number of spectrum bars */
    int barsNumber() const;
    void setBarsNumber(int num);

    QVariantList audioLevels() const;

signals:
    void captureEnabledChanged();
    void volumeLevelChanged();
    void barsNumberChanged();
    void audioLevelsChanged();

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget) override;

private:
    FunctionParent functionParent() const;

private:
    VirtualConsole *m_vc;
    AudioCapture *m_inputCapture;
    bool m_captureEnabled;
    uchar m_volumeLevel;

    QVariantList m_audioLevels;

    /*********************************************************************
     * Spectrum & Volume bars
     *********************************************************************/
public:
    enum BarType
    {
        None = 0,
        DMXBar,
        FunctionBar,
        VCWidgetBar
    };
    Q_ENUM(BarType)

    struct AudioBar
    {
        BarType m_type = BarType::None;
        uchar m_minThreshold = 51; // 20%
        uchar m_maxThreshold = 204; // 80%
        uchar m_value = 0;
        int m_divisor = 1;

        /** List of individual DMX channels when m_type == DMXBar */
        QList<SceneValue> m_dmxChannels;

        /** List of absolute DMX channel addresses when m_type == DMXBar.
          * This is precalculated to speed up writeDMX */
        QList<int> m_absDmxChannels;

        /** ID of the attached Function when m_type == FunctionBar */
        quint32 m_functionId = Function::invalidId();

        /** Reference to an attached Function when m_type == FunctionBar */
        Function *m_function = nullptr;

        /** ID of the attchaed VCWidget when m_type == VCWidgetBar */
        quint32 m_widgetId = VCWidget::invalidId();

        /** Reference to an attached VCWidget when m_type == VCWidgetBar */
        VCWidget *m_widget = nullptr;

        /** Trigger state for beat-based widget actions */
        bool m_tapped = false;
        int m_skippedBeats = 0;
    };

    Q_INVOKABLE void selectBarForEditing(int index);
    QVariantList barsInfo() const;

    Q_INVOKABLE void setBarType(BarType type);
    Q_INVOKABLE void setBarThresholds(uchar minThr, uchar maxThr);
    Q_INVOKABLE void setBarFunction(quint32 functionId);
    Q_INVOKABLE void setBarWidget(quint32 widgetId);
    void setBarDmxChannels(QList<SceneValue>list);

protected slots:
    void slotSpectrumDataChanged(double *spectrumBands, int size, double maxMagnitude, quint32 power);

signals:
    void barsInfoChanged();
    /** Notify the listeners that the fixture tree model has changed */
    void groupsTreeModelChanged();
    /** Notify the listeners that the search filter has changed */
    void searchFilterChanged();

private:
    void updateBarWidgetReference(AudioBar &bar);
    void checkWidgetFunctionality(AudioBar &bar);

private:
    // first bar is always volume
    QVector <AudioBar> m_spectrumBars;

    /** Index of the bar currently being edited.
     *  This is needed to simplify the widget editing */
    int m_selectedBar;

    /*********************************************************************
     * Fixture tree methods
     *********************************************************************/
public:
    /** Returns the data model to display a tree of FixtureGroups/Fixtures */
    QVariant groupsTreeModel();

    /** Get/Set a string to filter Group/Fixture/Channel names */
    QString searchFilter() const;
    void setSearchFilter(QString searchFilter);

    Q_INVOKABLE void applyToSameType(bool enable);

private:
    void updateFixtureTree();

    /** Recursive method to check/uncheck channels for fixtures of the same type */
    void checkFixtureTree(TreeModel *tree, Fixture *sourceFixture, quint32 channelIndex, bool checked);

protected slots:
    void slotTreeDataChanged(TreeModelItem *item, int role, const QVariant &value);

private:
    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_fixtureTree;
    /** A string to filter the displayed tree items */
    QString m_searchFilter;

    /** Flag to apply a channel selection to all
     *  the fixtures of the same type */
    bool m_applyToSameType, m_isUpdating;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** @reimp */
    void updateFeedback() override;

public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value) override;

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, QList<Universe*> universes) override;

private:
    /** Map used to lookup a GenericFader instance for a Universe ID */
    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadBarXML(QXmlStreamReader &root);
    bool saveBarXML(QXmlStreamWriter *doc, int index) const;

    bool loadXML(QXmlStreamReader &root) override;
    bool saveXML(QXmlStreamWriter *doc) const override;
};

#endif
