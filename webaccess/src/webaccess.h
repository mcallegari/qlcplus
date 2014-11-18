/*
  Q Light Controller Plus
  webaccess.h

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

#ifndef WEBACCESS_H
#define WEBACCESS_H

#include <QThread>
#include "mongoose.h"

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
class WebAccessNetwork;
#endif

class VCAudioTriggers;
class VirtualConsole;
class VCSoloFrame;
class SimpleDesk;
class VCCueList;
class VCWidget;
class VCButton;
class VCSlider;
class VCLabel;
class VCFrame;
class Doc;

class WebAccess : public QThread
{
    Q_OBJECT
public:
    explicit WebAccess(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance, QObject *parent = 0);
    /** Destructor */
    ~WebAccess();

    mg_result beginRequestHandler(struct mg_connection *conn);
    mg_result websocketDataHandler(struct mg_connection *conn);
    mg_result closeHandler(struct mg_connection* conn);

private:
    QString loadXMLPost(struct mg_connection *conn, QString &filename);
    bool sendFile(struct mg_connection *conn, QString filename, QString contentType);
    QString getWidgetHTML(VCWidget *widget);
    QString getFrameHTML(VCFrame *frame);
    QString getSoloFrameHTML(VCSoloFrame *frame);
    QString getButtonHTML(VCButton *btn);
    QString getSliderHTML(VCSlider *slider);
    QString getLabelHTML(VCLabel *label);
    QString getAudioTriggersHTML(VCAudioTriggers *triggers);
    QString getCueListHTML(VCCueList *cue);

    QString getChildrenHTML(VCWidget *frame, int pagesNum, int currentPageIdx);
    QString getVCHTML();

    QString getSimpleDeskHTML();

private:
    /** Input data thread worker method */
    virtual void run();

protected slots:
    void slotVCLoaded();
    void slotButtonToggled(bool on);
    void slotSliderValueChanged(QString val);
    void slotCueIndexChanged(int idx);
    void slotFramePageChanged(int pageNum);

protected:
    QString m_JScode;
    QString m_CSScode;

protected:
    Doc *m_doc;
    VirtualConsole *m_vc;
    SimpleDesk *m_sd;
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    WebAccessNetwork *m_netConfig;
#endif

    struct mg_server *m_server;
    struct mg_connection *m_conn;

    bool m_running;
    bool m_pendingProjectLoaded;

signals:
    void toggleDocMode();
    void loadProject(QString xmlData);
    void storeAutostartProject(QString filename);

public slots:

};

#endif // WEBACCESS_H
