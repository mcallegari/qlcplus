/*
  Q Light Controller Plus
  webaccess-qml.h

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

#ifndef WEBACCESS_QML_H
#define WEBACCESS_QML_H

#include <QSet>

#include "webaccessbase.h"

class VirtualConsole;
class SimpleDesk;
class VCWidget;
class VCFrame;
class VCButton;
class VCSlider;
class VCLabel;
class VCCueList;
class VCAudioTriggers;
class VCClock;
class VCAnimation;
class VCSpeedDial;
class VCXYPad;
class Doc;

class QHttpRequest;
class QHttpResponse;
class QHttpConnection;

class QJsonObject;

class WebAccessQml : public WebAccessBase
{
    Q_OBJECT
public:
    explicit WebAccessQml(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance,
                          int portNumber, bool enableAuth, QString passwdFile = QString(),
                          QObject *parent = nullptr);
    ~WebAccessQml();

signals:
    void loadProject(QByteArray xmlData);
    void storeAutostartProject(QString fileName);

protected slots:
    void slotHandleHTTPRequest(QHttpRequest *req, QHttpResponse *resp);
    void slotHandleWebSocketRequest(QHttpConnection *conn, QString data);
    void slotHandleWebSocketClose(QHttpConnection *conn);

    void slotFunctionStarted(quint32 fid);
    void slotFunctionStopped(quint32 fid);

    void slotDocLoaded();
    void slotSelectedPageChanged(int page);

    void slotButtonStateChanged(int state);
    void slotButtonDisableStateChanged(bool disable);
    void slotLabelDisableStateChanged(bool disable);
    void slotSliderValueChanged(int value);
    void slotSliderDisableStateChanged(bool disable);
    void slotSliderOverrideChanged();
    void slotAudioTriggersToggled();
    void slotWidgetDisableStateChanged(bool disable);
    void slotCueIndexChanged(int idx);
    void slotCuePlaybackStateChanged();
    void slotCueSideFaderLevelChanged();
    void slotCueDisableStateChanged(bool disable);
    void slotFramePageChanged(int pageNum);
    void slotFrameDisableStateChanged(bool disable);
    void slotMatrixFaderChanged();
    void slotMatrixColorsChanged();
    void slotMatrixAlgorithmChanged();
    void slotXYPadPositionChanged();
    void slotSpeedDialTimeChanged();
    void slotSpeedDialFactorChanged();
    void slotClockTimeChanged(int time);
    void slotGrandMasterValueChanged(uchar value);

protected:
    QString webFilePath(const QString &relativePath) const override;
    void sendMatrixState(VCAnimation *animation);
    void handleAutostartProject(const QString &path) override;

    QByteArray getVCJson();
    QJsonObject baseWidgetToJson(VCWidget *widget);
    QJsonObject widgetToJson(VCWidget *widget);
    QJsonObject frameToJson(VCFrame *frame);
    void collectWidgets(VCFrame *frame, QList<VCWidget *> &list, bool recursive = true);

    void setupWidgetConnections(VCWidget *widget);
    QString widgetBackgroundImagePath(VCWidget *widget) const;

protected:
    QSet<quint32> m_connectedWidgets;
};

#endif // WEBACCESS_QML_H
