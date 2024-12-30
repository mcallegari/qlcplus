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

#include <QObject>

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
class WebAccessNetwork;
#endif

class WebAccessAuth;

class VCAudioTriggers;
class VirtualConsole;
class VCSoloFrame;
class SimpleDesk;
class VCCueList;
class VCButton;
class VCWidget;
class VCSlider;
class VCLabel;
class VCFrame;
class VCClock;
class VCMatrix;
class Doc;

class QHttpServer;
class QHttpRequest;
class QHttpResponse;
class QHttpConnection;

class WebAccess : public QObject
{
    Q_OBJECT
public:
    explicit WebAccess(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance,
                       int portNumber, bool enableAuth, QString passwdFile = QString(),
                       QObject *parent = 0);
    /** Destructor */
    ~WebAccess();

private:
    bool sendFile(QHttpResponse *response, QString filename, QString contentType);
    void sendWebSocketMessage(const QString &message);

    QString getWidgetBackgroundImage(VCWidget *widget);
    QString getWidgetHTML(VCWidget *widget);
    QString getFrameHTML(VCFrame *frame);
    QString getSoloFrameHTML(VCSoloFrame *frame);
    QString getButtonHTML(VCButton *btn);
    QString getSliderHTML(VCSlider *slider);
    QString getLabelHTML(VCLabel *label);
    QString getAudioTriggersHTML(VCAudioTriggers *triggers);
    QString getCueListHTML(VCCueList *cue);
    QString getClockHTML(VCClock *clock);
    QString getMatrixHTML(VCMatrix *matrix);
    QString getGrandMasterSliderHTML();

    QString getChildrenHTML(VCWidget *frame, int pagesNum, int currentPageIdx);
    QString getVCHTML();

    QString getSimpleDeskHTML();

protected slots:
    void slotHandleHTTPRequest(QHttpRequest *req, QHttpResponse *resp);
    void slotHandleWebSocketRequest(QHttpConnection *conn, QString data);
    void slotHandleWebSocketClose(QHttpConnection *conn);

    void slotFunctionStarted(quint32 fid);
    void slotFunctionStopped(quint32 fid);

    void slotVCLoaded();
    void slotButtonStateChanged(int state);
    void slotButtonDisableStateChanged(bool disable);
    void slotLabelDisableStateChanged(bool disable);
    void slotSliderValueChanged(QString val);
    void slotSliderDisableStateChanged(bool disable);
    void slotAudioTriggersToggled(bool toggle);
    void slotCueIndexChanged(int idx);
    void slotCueStepNoteChanged(int idx, QString note);
    void slotCueProgressStateChanged();
    void slotCueShowSideFaderPanel();
    void slotCueSideFaderValueChanged();
    void slotCuePlaybackStateChanged();
    void slotCueDisableStateChanged(bool disable);
    void slotClockTimeChanged(quint32 time);
    void slotClockDisableStateChanged(bool disable);
    void slotFramePageChanged(int pageNum);
    void slotFrameDisableStateChanged(bool disable);
    void slotMatrixSliderValueChanged(int value);
    void slotMatrixColor1Changed();
    void slotMatrixColor2Changed();
    void slotMatrixColor3Changed();
    void slotMatrixColor4Changed();
    void slotMatrixColor5Changed();
    void slotMatrixAnimationValueChanged(QString name);
    void slotMatrixControlKnobValueChanged(int controlID, int value);

    void slotGrandMasterValueChanged(uchar value);

protected:
    QString m_JScode;
    QString m_CSScode;

protected:
    Doc *m_doc;
    VirtualConsole *m_vc;
    SimpleDesk *m_sd;
    WebAccessAuth *m_auth;
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    WebAccessNetwork *m_netConfig;
#endif

    QHttpServer *m_httpServer;
    QList<QHttpConnection *> m_webSocketsList;

    bool m_pendingProjectLoaded;

signals:
    void toggleDocMode();
    void loadProject(QString xmlData);
    void storeAutostartProject(QString filename);

public slots:

};

#endif // WEBACCESS_H
