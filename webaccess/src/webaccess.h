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

#include "webaccessbase.h"

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

class QHttpRequest;
class QHttpResponse;
class QHttpConnection;

class WebAccess final : public WebAccessBase
{
    Q_OBJECT
public:
    explicit WebAccess(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance,
                       int portNumber, bool enableAuth, QString passwdFile = QString(),
                       QObject *parent = 0);
    /** Destructor */
    ~WebAccess();

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
    void slotHandleHTTPRequest(QHttpRequest *req, QHttpResponse *resp) override;
    void slotHandleWebSocketRequest(QHttpConnection *conn, QString data) override;
    void slotHandleWebSocketClose(QHttpConnection *conn) override;

    void slotFunctionStarted(quint32 fid) override;
    void slotFunctionStopped(quint32 fid) override;

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
    void slotMatrixColorChanged(int);
    void slotMatrixAnimationValueChanged(QString name);
    void slotMatrixControlKnobValueChanged(int controlID, int value);

    void slotGrandMasterValueChanged(uchar value);

protected:
    QString m_JScode;
    QString m_CSScode;

    void handleProjectLoad(const QByteArray &projectXml) override;
    bool storeFixtureDefinition(const QString &fxName, const QByteArray &fixtureXML) override;
    void handleAutostartProject(const QString &path) override;

signals:
    void toggleDocMode();
    void loadProject(QString xmlData);
    void storeAutostartProject(QString filename);

public slots:

};

#endif // WEBACCESS_H
