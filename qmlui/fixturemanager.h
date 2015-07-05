/*
  Q Light Controller Plus
  fixturemanager.h

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

#ifndef FIXTUREMANAGER_H
#define FIXTUREMANAGER_H

#include <QQmlListProperty>
#include <QQuickView>
#include <QMultiHash>
#include <QObject>
#include <QList>

#include "scenevalue.h"

class Doc;
class Fixture;

class FixtureManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int fixturesCount READ fixturesCount NOTIFY fixturesCountChanged)
    Q_PROPERTY(QQmlListProperty<Fixture> fixtures READ fixtures)
    Q_PROPERTY(QVariantList goboChannels READ goboChannels NOTIFY goboChannelsChanged)
    Q_PROPERTY(QVariantList colorWheelChannels READ colorWheelChannels NOTIFY colorWheelChannelsChanged)

public:
    FixtureManager(QQuickView *view, Doc *doc, QObject *parent = 0);

    Q_INVOKABLE quint32 invalidFixture();
    Q_INVOKABLE quint32 fixtureForAddress(quint32 index);
    Q_INVOKABLE bool addFixture(QString manuf, QString model, QString mode, QString name,
                                int uniIdx, int address, int channels, int quantity, quint32 gap,
                                qreal xPos, qreal yPos);
    Q_INVOKABLE QString channelIcon(quint32 fxID, quint32 chIdx);

    Q_INVOKABLE void setChannelValue(quint32 fixtureID, quint32 channelIndex, quint8 value);
    Q_INVOKABLE void setIntensityValue(quint8 value);
    Q_INVOKABLE void setColorValue(quint8 red, quint8 green, quint8 blue,
                                   quint8 white, quint8 amber, quint8 uv);
    Q_INVOKABLE void setPresetValue(int index, quint8 value);

    /**
     * @brief setFixtureCapabilities
     * @param fxID the Fixture unique ID
     * @param enable used to increment/decrement the UI tools counters
     * @return A multihash containg the fixture capabilities by channel type
     */
    QMultiHash<int, SceneValue> setFixtureCapabilities(quint32 fxID, bool enable);

    /** Returns the number of fixtures currently loaded in the project */
    int fixturesCount();

    /** Returns a QML-readable list of references to Fixture classes */
    QQmlListProperty<Fixture> fixtures();

    /** Returns the names of the currently selected fixtures with gobo channels.
     *  The names are in the format: Product - Channel name */
    QVariantList goboChannels();

    /** Returns the names of the currently selected fixtures with color wheel channels.
     *  The names are in the format: Product - Channel name */
    QVariantList colorWheelChannels();

    /** Returns the list of QLCCapability in QVariant format for
     *  the channel cached at the given index */
    Q_INVOKABLE QVariantList presetCapabilities(int index);

signals:
    void docLoaded();
    void fixturesCountChanged();
    void newFixtureCreated(quint32 fxID, qreal x, qreal y);
    void channelValueChanged(quint32 fixtureID, quint32 channelIndex, quint8 value);
    void channelTypeValueChanged(int type, quint8 value);

    /** Notify the listeners that a color has been picked in the ColorTool.
     *  It emits all the possible components: RGB, White, Amber and UV */
    void colorChanged(QColor rgb, QColor wauv);

    /** Notify the listeners that a preset value has been picked.
     *  To uniquely identify which preset channel has changed, a reference
     *  to a cached QLCChannel is emitted as well */
    void presetChanged(const QLCChannel *channel, quint8 value);

    /** Notify the listeners that the list of fixtures with gobo channels has changed */
    void goboChannelsChanged();

    /** Notify the listeners that the list of fixtures with color wheel channels has changed */
    void colorWheelChannelsChanged();

private:
    /** Generic method that returns the names of the cached channels for
     *  the required $group */
    QVariantList presetsChannels(QLCChannel::Group group);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** List of the current Fixtures in Doc */
    QList<Fixture *> m_fixtureList;
    /** Keep a map of references to the available preset channels and a related Fixture ID */
    QMap<const QLCChannel *, quint32>m_presetsCache;
};

#endif // FIXTUREMANAGER_H
