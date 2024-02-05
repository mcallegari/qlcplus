/*
  Q Light Controller Plus
  editorview.h

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

#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QQuickView>

#include "physicaledit.h"
#include "qlcchannel.h"

class QLCFixtureDef;
class ChannelEdit;
class ListModel;
class ModeEdit;

class EditorView : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool id READ id CONSTANT)
    Q_PROPERTY(bool isModified READ isModified NOTIFY hasChanged)
    Q_PROPERTY(bool isUser READ isUser CONSTANT)
    Q_PROPERTY(QString fileName READ fileName CONSTANT)

    Q_PROPERTY(int productType READ productType WRITE setProductType NOTIFY productTypeChanged)
    Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer NOTIFY manufacturerChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged)

    Q_PROPERTY(PhysicalEdit *globalPhysical READ globalPhysical CONSTANT)
    Q_PROPERTY(QVariant channels READ channels NOTIFY channelsChanged)

    Q_PROPERTY(QVariant modes READ modes NOTIFY modesChanged)

public:
    EditorView(QQuickView *view, int id, QLCFixtureDef *fixtureDef, QObject *parent = nullptr);
    ~EditorView();

    /** Get the unique ID of this editor */
    int id() const;

    /** Get the fixture definition reference being edited */
    QLCFixtureDef *fixtureDefinition();

    /** Get if the definition is user or system */
    bool isUser() const;

    /** Get/Set the fixture type */
    int productType() const;
    void setProductType(int type);

    /** Get/Set the fixture manufacturer */
    QString manufacturer() const;
    void setManufacturer(QString manufacturer);

    /** Get/Set the fixture model */
    QString model() const;
    void setModel(QString model);

    /** Get/Set the definition author */
    QString author() const;
    void setAuthor(QString author);

    /** Get an editor reference for the
     *  global physical properties */
    PhysicalEdit *globalPhysical();

signals:
    void hasChanged();
    void productTypeChanged(int type);
    void manufacturerChanged(QString manufacturer);
    void modelChanged(QString model);
    void authorChanged(QString author);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Unique ID of this editor */
    int m_id;
    /** Reference to the definition being edited */
    QLCFixtureDef *m_fixtureDef;
    /** Reference to the global physical properties */
    PhysicalEdit *m_globalPhy;

    /************************************************************************
     * Channels
     ************************************************************************/
public:
    enum CompositeChannelTypes
    {
        RGBChannel = QLCChannel::Nothing + 100,
        RGBWChannel,
        RGBAWChannel
    };
    Q_ENUM(CompositeChannelTypes)

    /** Get a list of all the available channels in the definition */
    QVariant channels() const;

    /** Request a channel editor.
     *  If $name is empty, a new channel is added */
    Q_INVOKABLE ChannelEdit *requestChannelEditor(QString name);

    Q_INVOKABLE void addPresetChannel(QString name, int group);

    /** Delete the given $channel from the definition */
    Q_INVOKABLE bool deleteChannel(QLCChannel *channel);

    Q_INVOKABLE bool deleteChannels(QVariantList channels);

private:
    void updateChannelList();

private:
    /** Reference to a channel list usable in QML */
    ListModel *m_channelList;

    /** Reference to a channel editor */
    ChannelEdit *m_channelEdit;

signals:
    void channelsChanged();

    /************************************************************************
     * Modes
     ************************************************************************/
public:
    /** Get a list of all the available modes in the definition */
    QVariant modes() const;

    /** Request a mode editor.
     *  If name is empty, a new mode is added */
    Q_INVOKABLE ModeEdit *requestModeEditor(QString name);

private:
    void updateModeList();

protected slots:
    void modeNameChanged();

private:
    /** Reference to a mode list usable in QML */
    ListModel *m_modeList;

    /** Reference to a mode editor */
    ModeEdit *m_modeEdit;

signals:
    void modesChanged();

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    Q_INVOKABLE QString save();
    Q_INVOKABLE QString saveAs(QString path);

    QString fileName();
    void setFilenameFromModel();

    /** Get the definition modification flag */
    bool isModified() const;

private:
    QString checkFixture();

protected slots:
    void setModified(bool modified = true);

private:
    /** The definition file name */
    QString m_fileName;
    /** Definition modification flag */
    bool m_isModified;
};

#endif // EDITORVIEW_H
