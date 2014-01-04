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
#include "mongoose.h"

class VirtualConsole;
class VCAudioTriggers;
class VCSoloFrame;
class VCCueList;
class VCWidget;
class VCButton;
class VCSlider;
class VCLabel;
class VCFrame;
class Doc;

class WebAccess : public QObject
{
    Q_OBJECT
public:
    explicit WebAccess(Doc *doc, VirtualConsole *vcInstance, QObject *parent = 0);
    /** Destructor */
    ~WebAccess();

    int beginRequestHandler(struct mg_connection *conn);
    void websocketReadyHandler(struct mg_connection *conn);
    int websocketDataHandler(struct mg_connection *conn, int flags,
                               char *data, size_t data_len);

private:
    QString loadXMLPost(struct mg_connection *conn, QString &filename);
    QString getWidgetHTML(VCWidget *widget);
    QString getFrameHTML(VCFrame *frame);
    QString getSoloFrameHTML(VCSoloFrame *frame);
    QString getButtonHTML(VCButton *btn);
    QString getSliderHTML(VCSlider *slider);
    QString getLabelHTML(VCLabel *label);
    QString getAudioTriggersHTML(VCAudioTriggers *triggers);
    QString getCueListHTML(VCCueList *cue);

    QString getChildrenHTML(VCWidget *frame);
    QString getVCHTML();

    QString getIOConfigHTML();
    QString getAudioConfigHTML();
    QString getUserFixturesConfigHTML();
    QString getConfigHTML();

protected slots:
    void slotVCLoaded();
    void slotButtonToggled(bool on);
    void slotSliderValueChanged(QString val);
    void slotCueIndexChanged(int idx);

protected:
    QString m_JScode;
    QString m_CSScode;

    bool m_genericFound;
    bool m_buttonFound;
    bool m_frameFound;
    bool m_soloFrameFound;
    bool m_labelFound;
    bool m_cueListFound;
    bool m_sliderFound;
    bool m_knobFound;
    bool m_xyPadFound;
    bool m_speedDialFound;
    bool m_audioTriggersFound;

protected:
    Doc *m_doc;
    VirtualConsole *m_vc;

    struct mg_context *m_ctx;
    struct mg_connection *m_conn;
    struct mg_callbacks m_callbacks;

signals:
    void toggleDocMode();
    void loadProject(QString xmlData);
    
public slots:
    
};

#endif // WEBACCESS_H
