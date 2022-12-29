/*
  Q Light Controller
  channelsgroup.h

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

#ifndef CHANNELSGROUP_H
#define CHANNELSGROUP_H

#include <QObject>

#include "qlcinputsource.h"
#include "scenevalue.h"

class QXmlStreamReader;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCChannelsGroup QString("ChannelsGroup")

class ChannelsGroup : public QObject
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** Create a new ChannelsGroup with empty/invalid values */
    ChannelsGroup(Doc *doc);

    /** Copy constructor */
    ChannelsGroup(Doc* doc, const ChannelsGroup* chg);

    /** destroy this ChannelsGroup */
    ~ChannelsGroup();

protected:
    Doc * m_doc;

    void init();

signals:
    /** Emitted whenever a channels group's properties are changed */
    void changed(quint32 id);

public slots:
    void slotFixtureRemoved(quint32 fixtureId);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    static bool loader(QXmlStreamReader &xmlDoc, Doc* doc);

    bool saveXML(QXmlStreamWriter *doc);

    bool loadXML(QXmlStreamReader &xmlDoc);

    /************************************************************************
     * ID
     ************************************************************************/
public:
    /** Set a channels group's id (only Doc is allowed to do this!) */
    void setId(quint32 id);

    /** Get a channels group's unique id */
    quint32 id() const;

    /** Get an invalid channels group id */
    static quint32 invalidId();

private:
    quint32 m_id;

    /************************************************************************
     * Name
     ************************************************************************/
public:
    /** Set the name of a channels group */
    void setName(const QString& name);

    /** Get the name of a channels group */
    QString name() const;

    /************************************************************************
     * Channels
     ************************************************************************/
    /** Empty the current values of this channels group */
    void resetChannels();

    /** Add a channel to this channels group */
    bool addChannel(quint32 fxid, quint32 channel);

    /** Returns the current list of channels of this group */
    QList <SceneValue> getChannels() const;

    /*********************************************************************
     * Status
     *********************************************************************/
public:
    /**
     * Get the channels group instance's status info for Fixture Manager
     *
     * @return A sort-of HTML-RTF-gibberish for Fixture Manager
     */
    QString status(Doc *doc) const;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /**
     * Set external input $source to listen to.
     *
     * @param source The input source to set
     */
    void setInputSource(QSharedPointer<QLCInputSource> const& source);

    /**
     * Get an assigned external input source
     *
     * @param id The id of the source to get
     */
    QSharedPointer<QLCInputSource> const& inputSource() const;

protected slots:
    /**
     * Slot that receives external input data
     *
     * @param universe Input universe
     * @param channel Input channel
     * @param value New value for universe & value
     */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

signals:
    void valueChanged(quint32 channel, uchar value);

private:
    QString m_name;

    uchar m_masterValue;
    QList <SceneValue> m_channels;

    QSharedPointer<QLCInputSource> m_input;
};

/** @} */

#endif
