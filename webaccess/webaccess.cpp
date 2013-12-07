/*
  Q Light Controller Plus
  webaccess.cpp

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

#include <QDebug>

#include "vcaudiotriggers.h"
#include "virtualconsole.h"
#include "commonjscss.h"
#include "vcsoloframe.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "qlcconfig.h"
#include "webaccess.h"
#include "vccuelist.h"
#include "outputmap.h"
#include "inputmap.h"
#include "mongoose.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "function.h"
#include "vclabel.h"
#include "vcframe.h"
#include "chaser.h"
#include "doc.h"

#if defined( __APPLE__) || defined(Q_OS_MAC)
  #include "audiorenderer_portaudio.h"
  #include "audiocapture_portaudio.h"
#elif defined(WIN32) || defined(Q_OS_WIN)
  #include "audiorenderer_waveout.h"
  #include "audiocapture_wavein.h"
#else
  #include "audiorenderer_alsa.h"
  #include "audiocapture_alsa.h"
#endif

#define POST_DATA_SIZE 1024

WebAccess* s_instance = NULL;

static int begin_request_handler(struct mg_connection *conn)
{
    return s_instance->beginRequestHandler(conn);
}

static void websocket_ready_handler(struct mg_connection *conn)
{
    s_instance->websocketReadyHandler(conn);
}

static int websocket_data_handler(struct mg_connection *conn, int flags,
                                  char *data, size_t data_len)
{
    return s_instance->websocketDataHandler(conn, flags, data, data_len);
}

WebAccess::WebAccess(Doc *doc, VirtualConsole *vcInstance, QObject *parent) :
    QObject(parent)
  , m_doc(doc)
  , m_vc(vcInstance)
{
    Q_ASSERT(s_instance == NULL);
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(m_vc != NULL);

    s_instance = this;

    // List of options. Last element must be NULL.
    const char *options[] = {"listening_ports", "9999", NULL};

    // Prepare callbacks structure. We have only one callback, the rest are NULL.
    memset(&m_callbacks, 0, sizeof(m_callbacks));
    m_callbacks.begin_request = begin_request_handler;
    m_callbacks.websocket_ready = websocket_ready_handler;
    m_callbacks.websocket_data = websocket_data_handler;

    // Start the web server.
    m_ctx = mg_start(&m_callbacks, NULL, options);
}

WebAccess::~WebAccess()
{
    mg_stop(m_ctx);
}

// This function will be called by mongoose on every new request.
int WebAccess::beginRequestHandler(mg_connection *conn)
{
  m_genericFound = false;
  m_buttonFound = false;
  m_frameFound = false;
  m_soloFrameFound = false;
  m_labelFound = false;
  m_cueListFound = false;
  m_sliderFound = false;
  m_knobFound = false;
  m_xyPadFound = false;
  m_speedDialFound = false;
  m_audioTriggersFound = false;

  QString content;

  const struct mg_request_info *ri = mg_get_request_info(conn);
  qDebug() << Q_FUNC_INFO << ri->request_method << ri->uri;

  if (QString(ri->uri) == "/qlcplusWS")
      return 0;

  if (QString(ri->uri) == "/loadProject")
  {
      char post_data[POST_DATA_SIZE];
      QString projectXML = "";
      bool done = false;

      while(!done)
      {
          int read = mg_read(conn, post_data, sizeof(post_data));

          qDebug() << "POST: received: " << read << "bytes";

          QString recv(post_data);

          if (read < POST_DATA_SIZE)
          {
              recv.truncate(read);
              done = true;
          }
          projectXML += recv;
      }

      projectXML.remove(0, projectXML.indexOf("\n\r") + 2);
      projectXML.truncate(projectXML.indexOf("\n\r"));
      qDebug() << "Project XML:\n\n" << projectXML << "\n\n";

      QByteArray postReply =
              QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
              "<script type=\"text/javascript\">\n"
              " window.location = \"/\"\n"
              "</script></head></html>").toLatin1();
      int post_size = postReply.length();
      mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n\r\n"
                "%s",
                post_size, postReply.data());

      emit loadProject(projectXML);

      return 1;
  }
  else if (QString(ri->uri) == "/config")
  {
      content = getConfigHTML();
  }
  else if (QString(ri->uri) != "/")
      return 1;
  else
      content = getVCHTML();

  // Prepare the message we're going to send
  int content_length = content.length();
  QByteArray contentArray = content.toLatin1();

  // Send HTTP reply to the client
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s",
            content_length, contentArray.data());

  // Returning non-zero tells mongoose that our function has replied to
  // the client, and mongoose should not send client any more data.
  return 1;
}

void WebAccess::websocketReadyHandler(mg_connection *conn)
{
    qDebug() << Q_FUNC_INFO;
    m_conn = conn;
    static const char *message = "QLC+ is ready";
    mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, message, strlen(message));
}

int WebAccess::websocketDataHandler(mg_connection *conn, int flags, char *data, size_t data_len)
{
    Q_UNUSED(conn)
    Q_UNUSED(flags)

    QString qData = QString(data);
    qData.truncate((int)data_len);
    qDebug() << "[websocketDataHandler]" << qData;

    QStringList cmdList = qData.split("|");
    if (cmdList.isEmpty())
        return 1;

    if(cmdList[0] == "QLC+CMD")
    {
        if (cmdList.count() < 2)
            return 0;

        if(cmdList[1] == "opMode")
            emit toggleDocMode();

        return 1;
    }
    else if (cmdList[0] == "QLC+IO")
    {
        if (cmdList.count() < 2)
            return 0;

        int universe = cmdList[2].toInt();

        if (cmdList[1] == "INPUT")
            m_doc->inputMap()->setPatch(universe, cmdList[3], cmdList[4].toUInt());
        else if (cmdList[1] == "OUTPUT")
            m_doc->outputMap()->setPatch(universe, cmdList[3], cmdList[4].toUInt(), false);
        else if (cmdList[1] == "FB")
            m_doc->outputMap()->setPatch(universe, cmdList[3], cmdList[4].toUInt(), true);
        else if (cmdList[1] == "PROFILE")
        {
            InputPatch *inPatch = m_doc->inputMap()->patch(universe);
            if (inPatch != NULL)
                m_doc->inputMap()->setPatch(universe, inPatch->pluginName(), inPatch->input(), cmdList[3]);
        }
        else if (cmdList[1] == "AUDIOIN")
        {
            QSettings settings;
            if (cmdList[2] == "__qlcplusdefault__")
                settings.remove(SETTINGS_AUDIO_INPUT_DEVICE);
            else
            {
                settings.setValue(SETTINGS_AUDIO_INPUT_DEVICE, cmdList[2]);
                m_doc->destroyAudioCapture();
            }
        }
        else if (cmdList[1] == "AUDIOOUT")
        {
            QSettings settings;
            if (cmdList[2] == "__qlcplusdefault__")
                settings.remove(SETTINGS_AUDIO_OUTPUT_DEVICE);
            else
                settings.setValue(SETTINGS_AUDIO_OUTPUT_DEVICE, cmdList[2]);
        }
        else
            qDebug() << "[webaccess] Command" << cmdList[1] << "not supported !";

        return 1;
    }
    else if(cmdList[0] == "POLL")
        return 1;

    quint32 widgetID = cmdList[0].toUInt();
    VCWidget *widget = m_vc->widget(widgetID);
    uchar value = 0;
    if (cmdList.count() > 1)
        value = (uchar)cmdList[1].toInt();
    if (widget != NULL)
    {
        switch(widget->type())
        {
            case VCWidget::ButtonWidget:
            {
                VCButton *button = qobject_cast<VCButton*>(widget);
                button->pressFunction();
            }
            break;
            case VCWidget::SliderWidget:
            {
                VCSlider *slider = qobject_cast<VCSlider*>(widget);
                slider->setSliderValue(value);
            }
            break;
            case VCWidget::AudioTriggersWidget:
            {
                VCAudioTriggers *triggers = qobject_cast<VCAudioTriggers*>(widget);
                triggers->slotEnableButtonToggled(value ? true : false);
            }
            break;
            case VCWidget::CueListWidget:
            {
                VCCueList *cue = qobject_cast<VCCueList*>(widget);
                if (cmdList[1] == "PLAY")
                    cue->slotPlayback();
                else if (cmdList[1] == "PREV")
                    cue->slotPreviousCue();
                else if (cmdList[1] == "NEXT")
                    cue->slotNextCue();
                else if (cmdList[1] == "STEP")
                    cue->playCueAtIndex(cmdList[2].toInt());
            }
            break;
            default:
            break;
        }
    }

    return 1;
}

QString WebAccess::getWidgetHTML(VCWidget *widget)
{
    if (m_genericFound == false)
    {
        m_CSScode += widget->getCSS();
        m_genericFound = true;
    }

    QString str = "<div class=\"vcwidget\" style=\""
            "left: " + QString::number(widget->x()) + "px; "
            "top: " + QString::number(widget->y()) + "px; "
            "width: " + QString::number(widget->width()) + "px; "
            "height: " + QString::number(widget->height()) + "px; "
            "background-color: " + widget->backgroundColor().name() + ";\">\n";

    str +=  tr("Widget not supported (yet) for web access") + "</div>\n";

    return str;
}

QString WebAccess::getFrameHTML(VCFrame *frame)
{
    QColor border(90, 90, 90);

    QString str = "<div class=\"vcframe\" style=\"left: " + QString::number(frame->x()) +
          "px; top: " + QString::number(frame->y()) + "px; width: " +
           QString::number(frame->width()) +
          "px; height: " + QString::number(frame->height()) + "px; "
          "background-color: " + frame->backgroundColor().name() + "; "
          "border-radius: 4px;\n"
          "border: 1px solid " + border.name() + ";\">\n";
    if (frame->isHeaderVisible())
    {
        if (m_frameFound == false)
        {
            m_CSScode += frame->getCSS();
            m_frameFound = true;
        }
        str += "<div class=\"vcframeHeader\" style=\"color:" +
                frame->foregroundColor().name() + "\">" + frame->caption() + "</div>\n";
    }

    str += getChildrenHTML(frame);
    str += "</div>\n";

    return str;
}

QString WebAccess::getSoloFrameHTML(VCSoloFrame *frame)
{
    QColor border(255, 0, 0);

    QString str = "<div class=\"vcsoloframe\" style=\"left: " + QString::number(frame->x()) +
          "px; top: " + QString::number(frame->y()) + "px; width: " +
           QString::number(frame->width()) +
          "px; height: " + QString::number(frame->height()) + "px; "
          "background-color: " + frame->backgroundColor().name() + "; "
          "border-radius: 4px;\n"
          "border: 1px solid " + border.name() + ";\">\n";
    if (frame->isHeaderVisible())
    {
        if (m_soloFrameFound == false)
        {
            m_CSScode += frame->getCSS();
            m_soloFrameFound = true;
        }
        str += "<div class=\"vcsoloframeHeader\" style=\"color:" +
                frame->foregroundColor().name() + "\">" + frame->caption() + "</div>\n";
    }

    str += getChildrenHTML(frame);
    str += "</div>\n";

    return str;
}

void WebAccess::slotButtonToggled(bool on)
{
    VCButton *btn = (VCButton *)sender();

    QString wsMessage = QString::number(btn->id());
    if (on == true)
        wsMessage.append("|BUTTON|1");
    else
        wsMessage.append("|BUTTON|0");

    mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toLatin1().data(), wsMessage.length());
}

QString WebAccess::getButtonHTML(VCButton *btn)
{
    if (m_buttonFound == false)
    {
        m_JScode += btn->getJS();
        m_CSScode += btn->getCSS();
        m_buttonFound = true;
    }

    QString onCSS = "";
    if (btn->isOn())
        onCSS = "border: 3px solid #00E600;";

    QString str = "<div class=\"vcbutton-wrapper\" style=\""
            "left: " + QString::number(btn->x()) + "px; "
            "top: " + QString::number(btn->y()) + "px;\">\n";
    str +=  "<a class=\"vcbutton\" id=\"" + QString::number(btn->id()) + "\" "
            "href=\"javascript:buttonClick(" + QString::number(btn->id()) + ");\" "
            "style=\""
            "width: " + QString::number(btn->width()) + "px; "
            "height: " + QString::number(btn->height()) + "px; "
            "color: " + btn->foregroundColor().name() + "; "
            "background-color: " + btn->backgroundColor().name() + "; " + onCSS + "\">" +
            btn->caption() + "</a>\n</div>\n";

    connect(btn, SIGNAL(pressedState(bool)),
            this, SLOT(slotButtonToggled(bool)));

    return str;
}

void WebAccess::slotSliderValueChanged(QString val)
{
    VCSlider *slider = (VCSlider *)sender();

    QString wsMessage = QString("%1|SLIDER|%2").arg(slider->id()).arg(val);

    mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toLatin1().data(), wsMessage.length());
}

QString WebAccess::getSliderHTML(VCSlider *slider)
{
    if (m_sliderFound == false)
    {
        m_JScode += slider->getJS();
        m_CSScode += slider->getCSS();
        m_sliderFound = true;
    }

    QString slID = QString::number(slider->id());

    QString str = "<div class=\"vcslider\" style=\""
            "left: " + QString::number(slider->x()) + "px; "
            "top: " + QString::number(slider->y()) + "px; "
            "width: " + QString::number(slider->width()) + "px; "
            "height: " + QString::number(slider->height()) + "px; "
            "background-color: " + slider->backgroundColor().name() + ";\">\n";

    str += "<div id=\"slv" + slID + "\" "
            "class=\"vcslLabel\" style=\"top:0px;\">" +
            QString::number(slider->sliderValue()) + "</div>\n";

    str +=  "<input type=\"range\" class=\"vVertical\" "
            "id=\"" + slID + "\" "
            "onchange=\"slVchange(" + slID + ");\" style=\""
            "width: " + QString::number(slider->height() - 50) + "px; "
            "margin-top: " + QString::number(slider->height() - 50) + "px; "
            "margin-left: " + QString::number(slider->width() / 2) + "px;\" "
            "min=\"0\" max=\"255\" step=\"1\" value=\"" +
            QString::number(slider->sliderValue()) + "\" />\n";

    str += "<div id=\"sln" + slID + "\" "
            "class=\"vcslLabel\" style=\"bottom:0px;\">" +
            slider->caption() + "</div>\n"
            "</div>\n";

    connect(slider, SIGNAL(valueChanged(QString)),
            this, SLOT(slotSliderValueChanged(QString)));
    return str;
}

QString WebAccess::getLabelHTML(VCLabel *label)
{
    if (m_labelFound == false)
    {
        m_CSScode += label->getCSS();
        m_labelFound = true;
    }

    QString str = "<div class=\"vclabel-wrapper\" style=\""
            "left: " + QString::number(label->x()) + "px; "
            "top: " + QString::number(label->y()) + "px;\">\n";
    str +=  "<div class=\"vclabel\" style=\""
            "width: " + QString::number(label->width()) + "px; "
            "height: " + QString::number(label->height()) + "px; "
            "color: " + label->foregroundColor().name() + "; "
            "background-color: " + label->backgroundColor().name() + "\">" +
            label->caption() + "</div>\n</div>\n";

    return str;
}

QString WebAccess::getAudioTriggersHTML(VCAudioTriggers *triggers)
{
    if (m_audioTriggersFound == false)
    {
        m_CSScode += triggers->getCSS();
        m_JScode += triggers->getJS();
        m_audioTriggersFound = true;
    }

    QString str = "<div class=\"vcaudiotriggers\" style=\"left: " + QString::number(triggers->x()) +
          "px; top: " + QString::number(triggers->y()) + "px; width: " +
           QString::number(triggers->width()) +
          "px; height: " + QString::number(triggers->height()) + "px; "
          "background-color: " + triggers->backgroundColor().name() + ";\">\n";

    str += "<div class=\"vcaudioHeader\" style=\"color:" +
            triggers->foregroundColor().name() + "\">" + triggers->caption() + "</div>\n";

    str += "<div class=\"vcatbutton-wrapper\">\n";
    str += "<a  class=\"vcatbutton\" id=\"" + QString::number(triggers->id()) + "\" "
            "href=\"javascript:atButtonClick(" + QString::number(triggers->id()) + ");\" "
            "style=\""
            "width: " + QString::number(triggers->width() - 2) + "px; "
            "height: " + QString::number(triggers->height() - 42) + "px;\">"
            + tr("Enable") + "</a>\n";

    str += "</div></div>\n";

    return str;
}

void WebAccess::slotCueIndexChanged(int idx)
{
    VCCueList *cue = (VCCueList *)sender();

    QString wsMessage = QString("%1|CUE|%2").arg(cue->id()).arg(idx);

    mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toLatin1().data(), wsMessage.length());
}

QString WebAccess::getCueListHTML(VCCueList *cue)
{
    if (m_cueListFound == false)
    {
        m_CSScode += cue->getCSS();
        m_JScode += cue->getJS();
        m_cueListFound = true;
    }

    QString str = "<div id=\"" + QString::number(cue->id()) + "\" "
            "class=\"vccuelist\" style=\"left: " + QString::number(cue->x()) +
            "px; top: " + QString::number(cue->y()) + "px; width: " +
             QString::number(cue->width()) +
            "px; height: " + QString::number(cue->height()) + "px; "
            "background-color: " + cue->backgroundColor().name() + ";\">\n";

    str += "<div style=\"width: 100%; height: " + QString::number(cue->height() - 32) + "px; overflow: scroll;\" >\n";
    str += "<table class=\"hovertable\" style=\"width: 100%;\">\n";
    str += "<tr><th>#</th><th>Name</th><th>Fade In</th><th>Fade Out</th><th>Duration</th><th>Notes</th></tr>\n";
    Chaser *chaser = cue->chaser();
    Doc *doc = m_vc->getDoc();
    if (chaser != NULL)
    {
        for (int i = 0; i < chaser->stepsCount(); i++)
        {
            QString stepID = QString::number(cue->id()) + "_" + QString::number(i);
            str += "<tr id=\"" + stepID + "\" "
                    "onclick=\"enableCue(" + QString::number(cue->id()) + ", " + QString::number(i) + ");\" "
                    "onmouseover=\"this.style.backgroundColor='#CCD9FF';\" "
                    "onmouseout=\"checkMouseOut(" + QString::number(cue->id()) + ", " + QString::number(i) + ");\">\n";
            ChaserStep step = chaser->stepAt(i);
            str += "<td>" + QString::number(i + 1) + "</td>";
            Function* function = doc->function(step.fid);
            if (function != NULL)
            {
                str += "<td>" + function->name() + "</td>";

                switch (chaser->fadeInMode())
                {
                    case Chaser::Common:
                        str += "<td>" + Function::speedToString(chaser->fadeInSpeed()) + "</td>";
                        break;
                    case Chaser::PerStep:
                        str += "<td>" + Function::speedToString(step.fadeIn) + "</td>";
                        break;
                    default:
                    case Chaser::Default:
                        str += "<td></td>";
                }

                //if (step.hold != 0)
                //    str +=  "<td>" + Function::speedToString(step.hold) + "</td>";
                //else str += "<td></td>";

                switch (chaser->fadeOutMode())
                {
                    case Chaser::Common:
                        str += "<td>" + Function::speedToString(chaser->fadeOutSpeed()) + "</td>";
                        break;
                    case Chaser::PerStep:
                        str += "<td>" + Function::speedToString(step.fadeOut) + "</td>";
                        break;
                    default:
                    case Chaser::Default:
                        str += "<td></td>";
                }

                switch (chaser->durationMode())
                {
                    case Chaser::Common:
                        str += "<td>" + Function::speedToString(chaser->duration()) + "</td>";
                        break;
                    case Chaser::PerStep:
                        str += "<td>" + Function::speedToString(step.duration) + "</td>";
                        break;
                    default:
                    case Chaser::Default:
                        str += "<td></td>";
                }

                str += "<td>" + step.note + "</td>\n";
            }
            str += "</td>\n";
        }
    }
    str += "</table>\n";
    str += "</div>\n";

    str += "<a class=\"button button-blue\" style=\"height: 29px; font-size: 24px;\" "
            "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'PLAY');\">\n"
            "<span id=\"play" + QString::number(cue->id()) + "\">Play</span></a>\n";
    str += "<a class=\"button button-blue\" style=\"height: 29px; font-size: 24px;\" "
            "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'PREV');\">\n"
            "<span>Previous</span></a>\n";
    str += "<a class=\"button button-blue\" style=\"height: 29px; font-size: 24px;\" "
            "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'NEXT');\">\n"
            "<span>Next</span></a>\n";
    str += "</div>\n";

    connect(cue, SIGNAL(stepChanged(int)),
            this, SLOT(slotCueIndexChanged(int)));

    return str;
}

QString WebAccess::getChildrenHTML(VCWidget *frame)
{
    if (frame == NULL)
        return QString();

    QString str;

    QList<VCWidget *> chList = frame->findChildren<VCWidget*>();

    qDebug () << "getChildrenHTML: found " << chList.count() << " children";

    foreach (VCWidget *widget, chList)
    {
        if (widget->parentWidget() != frame || widget->isVisible() == false)
            continue;

        switch (widget->type())
        {
            case VCWidget::FrameWidget:
                str += getFrameHTML((VCFrame *)widget);
            break;
            case VCWidget::SoloFrameWidget:
                str += getSoloFrameHTML((VCSoloFrame *)widget);
            break;
            case VCWidget::ButtonWidget:
                str += getButtonHTML((VCButton *)widget);
            break;
            case VCWidget::SliderWidget:
                str += getSliderHTML((VCSlider *)widget);
            break;
            case VCWidget::LabelWidget:
                str += getLabelHTML((VCLabel *)widget);
            break;
            case VCWidget::AudioTriggersWidget:
                str += getAudioTriggersHTML((VCAudioTriggers *)widget);
            break;
            case VCWidget::CueListWidget:
                str += getCueListHTML((VCCueList *)widget);
            break;
            default:
                str += getWidgetHTML(widget);
            break;
        }
    }

    return str;
}

QString WebAccess::getVCHTML()
{
    m_JScode = "<script language=\"javascript\" type=\"text/javascript\">\n" WEBSOCKET_JS;

    m_CSScode = "<style>\n"
            "body { margin: 0px; }\n"

            "form {\n"
            " position: absolute;\n"
            " top: -100px;\n"
            " visibility: hidden;\n"
            "}\n\n"

            CONTROL_BAR_CSS
            BUTTON_BASE_CSS
            BUTTON_SPAN_CSS
            BUTTON_STATE_CSS
            BUTTON_BLUE_CSS
            SWINFO_CSS
            "</style>\n";

    VCFrame *mainFrame = m_vc->contents();
    QSize mfSize = mainFrame->size();
    QString widgetsHTML =
            "<form action=\"/loadProject\" method=\"POST\" enctype=\"multipart/form-data\">\n"
            "<input id=\"loadTrigger\" type=\"file\" "
            "onchange=\"document.getElementById('submitTrigger').click();\" name=\"qlcprj\" />\n"
            "<input id=\"submitTrigger\" type=\"submit\"/></form>"

            "<div class=\"controlBar\">\n"
            "<a class=\"button button-blue\" href=\"javascript:document.getElementById('loadTrigger').click();\">\n"
            "<span>Load project</span></a>\n"

            "<a class=\"button button-blue\" href=\"/config\"><span>Configuration</span></a>\n"

            "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>"
            "</div>\n"
            "<div style=\"position: relative; "
            "width: " + QString::number(mfSize.width()) +
            "px; height: " + QString::number(mfSize.height()) + "px; "
            "background-color: " + mainFrame->backgroundColor().name() + "; \" />\n";

    widgetsHTML += getChildrenHTML(mainFrame);

    m_JScode += "\n</script>\n";

    QString str = HTML_HEADER + m_JScode + m_CSScode + "</head>\n<body>\n" + widgetsHTML + "</body>\n</html>";
    return str;
}

QString WebAccess::getConfigHTML()
{

    m_JScode = "<script language=\"javascript\" type=\"text/javascript\">\n" WEBSOCKET_JS;
    m_JScode += "function ioChanged(cmd, uni, val)\n"
            "{\n"
            " websocket.send(\"QLC+IO|\" + cmd + \"|\" + uni + \"|\" + val);\n"
            "};\n\n";
    m_JScode += "</script>\n";

    m_CSScode = "<style>\n"
            "html { height: 100%; }\n"
            "body {\n"
            " margin: 0px;\n"
            " height: 100%;\n"
            " background: linear-gradient(to bottom, #45484d 0%, #000000 100%);\n"
            " background: -webkit-linear-gradient(top, #45484d 0%, #000000 100%);\n"
            "}\n"
            CONTROL_BAR_CSS
            BUTTON_BASE_CSS
            BUTTON_SPAN_CSS
            BUTTON_STATE_CSS
            BUTTON_BLUE_CSS
            SWINFO_CSS
            TABLE_CSS
            "</style>\n";

    QString bodyHTML = "<div class=\"controlBar\">\n"
                       "<a class=\"button button-blue\" href=\"/\"><span>Back</span></a>\n"
                       "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>"
                       "</div>\n";

    InputMap *inMap = m_doc->inputMap();
    OutputMap *outMap = m_doc->outputMap();

    QStringList IOplugins = inMap->pluginNames();
    foreach (QString out, outMap->pluginNames())
        if (IOplugins.contains(out) == false)
            IOplugins.append(out);

    QStringList inputLines, outputLines, feedbackLines;
    QStringList profiles = inMap->profileNames();

    foreach (QString pluginName, IOplugins)
    {
        QStringList inputs = inMap->pluginInputs(pluginName);
        QStringList outputs = outMap->pluginOutputs(pluginName);
        bool hasFeedback = outMap->pluginSupportsFeedback(pluginName);

        for (int i = 0; i < inputs.count(); i++)
            inputLines.append(QString("%1,%2,%3").arg(pluginName).arg(inputs.at(i)).arg(i));
        for (int i = 0; i < outputs.count(); i++)
        {
            outputLines.append(QString("%1,%2,%3").arg(pluginName).arg(outputs.at(i)).arg(i));
            if (hasFeedback)
                feedbackLines.append(QString("%1,%2,%3").arg(pluginName).arg(outputs.at(i)).arg(i));
        }
    }
    inputLines.prepend("None, None, -1");
    outputLines.prepend("None, None, -1");
    feedbackLines.prepend("None, None, -1");
    profiles.prepend("None");

    bodyHTML += "<div style=\"margin: 30px 7% 30px 7%; width: 86%; height: 300px;\" >\n";
    bodyHTML += "<div style=\"font-family: verdana,arial,sans-serif; font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += "Universes configuration</div><br>\n";
    bodyHTML += "<table class=\"hovertable\" style=\"width: 100%;\">\n";
    bodyHTML += "<tr><th>Universe</th><th>Input</th><th>Output</th><th>Feedback</th><th>Profile</th></tr>\n";

    for (int i = 0; i < 4; i++)
    {
        QString currentInputPluginName = inMap->patch(i)->pluginName();
        quint32 currentInput = inMap->patch(i)->input();
        QString currentOutputPluginName = outMap->patch(i)->pluginName();
        quint32 currentOutput = outMap->patch(i)->output();
        QString currentFeedbackPluginName = outMap->feedbackPatch(i)->pluginName();
        quint32 currentFeedback = outMap->feedbackPatch(i)->output();
        QString currentProfileName = inMap->patch(i)->profileName();

        bodyHTML += "<tr align=center><td>Universe " + QString::number(i+1) + "</td>\n";
        bodyHTML += "<td><select onchange=\"ioChanged('INPUT', " + QString::number(i) + ", this.value);\">\n";
        for (int in = 0; in < inputLines.count(); in++)
        {
            QStringList strList = inputLines.at(in).split(",");
            QString selected = "";
            if (currentInputPluginName == strList.at(0) && currentInput == strList.at(2).toUInt())
                selected = "selected";
            bodyHTML += "<option value=\"" + QString("%1|%2").arg(strList.at(0)).arg(strList.at(2)) + "\" " + selected + ">" +
                    QString("[%1] %2").arg(strList.at(0)).arg(strList.at(1)) + "</option>\n";
        }
        bodyHTML += "</select></td>\n";
        bodyHTML += "<td><select onchange=\"ioChanged('OUTPUT', " + QString::number(i) + ", this.value);\">\n";
        for (int in = 0; in < outputLines.count(); in++)
        {
            QStringList strList = outputLines.at(in).split(",");
            QString selected = "";
            if (currentOutputPluginName == strList.at(0) && currentOutput == strList.at(2).toUInt())
                selected = "selected";
            bodyHTML += "<option value=\"" + QString("%1|%2").arg(strList.at(0)).arg(strList.at(2)) + "\" " + selected + ">" +
                    QString("[%1] %2").arg(strList.at(0)).arg(strList.at(1)) + "</option>\n";
        }
        bodyHTML += "</select></td>\n";
        bodyHTML += "<td><select onchange=\"ioChanged('FB', " + QString::number(i) + ", this.value);\">\n";
        for (int in = 0; in < feedbackLines.count(); in++)
        {
            QStringList strList = feedbackLines.at(in).split(",");
            QString selected = "";
            if (currentFeedbackPluginName == strList.at(0) && currentFeedback == strList.at(2).toUInt())
                selected = "selected";
            bodyHTML += "<option value=\"" + QString("%1|%2").arg(strList.at(0)).arg(strList.at(2)) + "\" " + selected + ">" +
                    QString("[%1] %2").arg(strList.at(0)).arg(strList.at(1)) + "</option>\n";
        }
        bodyHTML += "</select></td>\n";
        bodyHTML += "<td><select onchange=\"ioChanged('PROFILE', " + QString::number(i) + ", this.value);\">\n";
        for (int p = 0; p < profiles.count(); p++)
        {
            QString selected = "";
            if (currentProfileName == profiles.at(p))
                selected = "selected";
            bodyHTML += "<option value=\"" + profiles.at(p) + "\" " + selected + ">" + profiles.at(p) + "</option>\n";
        }
        bodyHTML += "</select></td>\n";

        bodyHTML += "</tr>\n";
    }
    bodyHTML += "</table>\n";

    // ********************* audio devices ********************
    QList<AudioDeviceInfo> devList;

#if defined( __APPLE__) || defined(Q_OS_MAC)
    devList = AudioRendererPortAudio::getDevicesInfo();
#elif defined(WIN32) || defined(Q_OS_WIN)
    devList = AudioRendererWaveOut::getDevicesInfo();
#else
    devList = AudioRendererAlsa::getDevicesInfo();
#endif

    bodyHTML += "<div style=\"margin: 30px 7% 30px 7%; width: 86%; height: 300px;\" >\n";
    bodyHTML += "<div style=\"font-family: verdana,arial,sans-serif; font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += "Audio configuration</div><br>\n";
    bodyHTML += "<table class=\"hovertable\" style=\"width: 100%;\">\n";
    bodyHTML += "<tr><th>Input</th><th>Output</th></tr>\n";
    bodyHTML += "<tr align=center>";

    QString audioInSelect = "<td><select onchange=\"ioChanged('AUDIOIN', this.value);\">\n"
                            "<option value=\"__qlcplusdefault__\">Default device</option>\n";
    QString audioOutSelect = "<td><select onchange=\"ioChanged('AUDIOOUT', this.value);\">\n"
                             "<option value=\"__qlcplusdefault__\">Default device</option>\n";

    QString inputName, outputName;
    QSettings settings;
    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_DEVICE);
    if (var.isValid() == true)
        inputName = var.toString();

    var = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE);
    if (var.isValid() == true)
        outputName = var.toString();

    foreach( AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_INPUT)
            audioInSelect += "<option value=\"" + info.privateName + "\" " +
                             ((info.privateName == inputName)?"selected":"") + ">" +
                             info.deviceName + "</option>\n";
        if (info.capabilities & AUDIO_CAP_OUTPUT)
            audioOutSelect += "<option value=\"" + info.privateName + "\" " +
                    ((info.privateName == outputName)?"selected":"") + ">" +
                    info.deviceName + "</option>\n";
    }
    audioInSelect += "</select></td>\n";
    audioOutSelect += "</select></td>\n";
    bodyHTML += audioInSelect + audioOutSelect + "</tr>\n</table>\n";

    QString str = HTML_HEADER + m_JScode + m_CSScode + "</head>\n<body>\n" + bodyHTML + "</body>\n</html>";

    return str;
}


