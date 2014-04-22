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
#include <QProcess>
#include <QNetworkInterface>

#include "webaccess.h"

#include "vcaudiotriggers.h"
#include "virtualconsole.h"
#include "inputoutputmap.h"
#include "commonjscss.h"
#include "vcsoloframe.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "qlcconfig.h"
#include "webaccess.h"
#include "vccuelist.h"
#include "mongoose.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "function.h"
#include "vclabel.h"
#include "vcframe.h"
#include "qlcfile.h"
#include "chaser.h"
#include "doc.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
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
#else
 #include "audiorenderer_qt.h"
 #include "audiocapture_qt.h"
#endif

#define AUTOSTART_PROJECT_NAME "autostart.qxw"
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
   #define IFACES_SYSTEM_FILE "/etc/network/interfaces"
#endif

WebAccess* s_instance = NULL;

static int event_handler(struct mg_connection *conn, enum mg_event ev)
{
    if (ev == MG_REQUEST)
    {
        if (conn->is_websocket)
            return s_instance->websocketDataHandler(conn);

        return s_instance->beginRequestHandler(conn);
    }
    else if (ev == MG_WS_HANDSHAKE)
    {
        if (conn->is_websocket)
        {
            s_instance->websocketDataHandler(conn);
            return MG_FALSE;
        }
    }
    else if (ev == MG_AUTH)
    {
        return MG_TRUE;
    }
    else
    {
        return MG_FALSE;
    }

    return MG_FALSE;
}

WebAccess::WebAccess(Doc *doc, VirtualConsole *vcInstance, QObject *parent) :
    QThread(parent)
  , m_doc(doc)
  , m_vc(vcInstance)
  , m_server(NULL)
  , m_conn(NULL)
  , m_running(false)
{
    Q_ASSERT(s_instance == NULL);
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(m_vc != NULL);

    s_instance = this;

    m_server = mg_create_server(NULL, event_handler);
    mg_set_option(m_server, "listening_port", "9999");
    start();

    connect(m_vc, SIGNAL(loaded()),
            this, SLOT(slotVCLoaded()));
}

WebAccess::~WebAccess()
{
    m_running = false;
    wait();
    mg_destroy_server(&m_server);
    m_interfaces.clear();
}

void WebAccess::run()
{
    m_running = true;
    while (isRunning())
    {
        mg_poll_server(m_server, 500);
    }

}

QString WebAccess::loadXMLPost(mg_connection *conn, QString &filename)
{
    const char *data;
    int data_len;
    char vname[1024], fname[1024];
    QString XMLdata = "";

    if (conn != NULL &&
        mg_parse_multipart(conn->content, conn->content_len,
                           vname, sizeof(vname),
                           fname, sizeof(fname),
                           &data, &data_len) > 0)
    {
        XMLdata = QString(data);
        XMLdata.truncate(data_len);
        filename = QString(fname);
        qDebug() << "Filename:" << filename;
    }

    return XMLdata;
}

// This function will be called by mongoose on every new request.
mg_result WebAccess::beginRequestHandler(mg_connection *conn)
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

  //const struct mg_request_info *ri = mg_get_request_info(conn);
  qDebug() << Q_FUNC_INFO << conn->request_method << conn->uri;

  if (QString(conn->uri) == "/qlcplusWS")
      return MG_FALSE;

  if (QString(conn->uri) == "/loadProject")
  {
      QString prjname;
      QString projectXML = loadXMLPost(conn, prjname);
      qDebug() << "Project XML:\n\n" << projectXML << "\n\n";

      QByteArray postReply =
              QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
              "<script type=\"text/javascript\">\n" WEBSOCKET_JS
              "</script></head><body style=\"background-color: #45484d;\">"
              "<div style=\"position: absolute; width: 100%; height: 30px; top: 50%; background-color: #888888;"
              "text-align: center; font:bold 24px/1.2em sans-serif;\">"
              + tr("Loading project...") +
              "</div></body></html>").toLatin1();
      int post_size = postReply.length();
      mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n\r\n"
                "%s",
                post_size, postReply.data());

      emit loadProject(projectXML);

      return MG_TRUE;
  }
  else if (QString(conn->uri) == "/config")
  {
      content = getConfigHTML();
  }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
  else if (QString(conn->uri) == "/system")
  {
      content = getSystemConfigHTML();
  }
#endif
  else if (QString(conn->uri) == "/loadFixture")
  {
      QString fxName;
      QString fixtureXML = loadXMLPost(conn, fxName);
      qDebug() << "Fixture name:" << fxName;
      qDebug() << "Fixture XML:\n\n" << fixtureXML << "\n\n";

      m_doc->fixtureDefCache()->storeFixtureDef(fxName, fixtureXML);

      QByteArray postReply =
                    QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
                    "<script type=\"text/javascript\">\n"
                    " alert(\"" + tr("Fixture stored and loaded") + "\");"
                    " window.location = \"/config\"\n"
                    "</script></head></html>").toLatin1();
      int post_size = postReply.length();
      mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "Content-Length: %d\r\n\r\n"
                      "%s",
                      post_size, postReply.data());

      return MG_TRUE;
  }
  else if (QString(conn->uri) != "/")
      return MG_TRUE;
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
  return MG_TRUE;
}

mg_result WebAccess::websocketDataHandler(mg_connection *conn)
{
    if (conn == NULL)
        return MG_TRUE;

    m_conn = conn; // store this to send VC loaded async event

    if (conn->content_len == 0)
        return MG_TRUE;

    QString qData = QString(conn->content);
    qData.truncate(conn->content_len);
    qDebug() << "[websocketDataHandler]" << qData;

    QStringList cmdList = qData.split("|");
    if (cmdList.isEmpty())
        return MG_TRUE;

    if(cmdList[0] == "QLC+CMD")
    {
        if (cmdList.count() < 2)
            return MG_FALSE;

        if(cmdList[1] == "opMode")
            emit toggleDocMode();

        return MG_TRUE;
    }
    else if (cmdList[0] == "QLC+IO")
    {
        if (cmdList.count() < 3)
            return MG_FALSE;

        int universe = cmdList[2].toInt();

        if (cmdList[1] == "INPUT")
        {
            m_doc->inputOutputMap()->setInputPatch(universe, cmdList[3], cmdList[4].toUInt());
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "OUTPUT")
        {
            m_doc->inputOutputMap()->setOutputPatch(universe, cmdList[3], cmdList[4].toUInt(), false);
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "FB")
        {
            m_doc->inputOutputMap()->setOutputPatch(universe, cmdList[3], cmdList[4].toUInt(), true);
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "PROFILE")
        {
            InputPatch *inPatch = m_doc->inputOutputMap()->inputPatch(universe);
            if (inPatch != NULL)
            {
                m_doc->inputOutputMap()->setInputPatch(universe, inPatch->pluginName(), inPatch->input(), cmdList[3]);
                m_doc->inputOutputMap()->saveDefaults();
            }
        }
        else if (cmdList[1] == "PASSTHROUGH")
        {
            quint32 uniIdx = cmdList[2].toUInt();
            if (cmdList[3] == "true")
                m_doc->inputOutputMap()->setUniversePassthrough(uniIdx, true);
            else
                m_doc->inputOutputMap()->setUniversePassthrough(uniIdx, false);
            m_doc->inputOutputMap()->saveDefaults();
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

        return MG_TRUE;
    }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    else if(cmdList[0] == "QLC+SYS")
    {
        if (cmdList.at(1) == "NETWORK")
        {
            for (int i = 0; i < m_interfaces.count(); i++)
            {
                if (m_interfaces.at(i).name == cmdList.at(2))
                {
                    m_interfaces[i].enabled = true;
                    if (cmdList.at(3) == "static")
                        m_interfaces[i].isStatic = true;
                    else
                        m_interfaces[i].isStatic = false;
                    m_interfaces[i].address = cmdList.at(4);
                    m_interfaces[i].netmask = cmdList.at(5);
                    m_interfaces[i].gateway = cmdList.at(6);
                    if (m_interfaces[i].isWireless == true)
                    {
                        m_interfaces[i].ssid = cmdList.at(7);
                        m_interfaces[i].wpaPass = cmdList.at(8);
                    }
                    if (writeNetworkFile() == false)
                        qDebug() << "[webaccess] Error writing network configuration file !";
                    QString wsMessage = QString("ALERT|" + tr("Network configuration changed. Reboot to apply the changes."));
                    mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toLatin1().data(), wsMessage.length());
                    return MG_TRUE;
                }
            }
            qDebug() << "[webaccess] Error: interface" << cmdList.at(2) << "not found !";
            return MG_TRUE;
        }
        else if (cmdList.at(1) == "AUTOSTART")
        {
            if (cmdList.count() < 3)
                return MG_FALSE;

            QString asName = QString("%1/%2/%3").arg(getenv("HOME")).arg(USERQLCPLUSDIR).arg(AUTOSTART_PROJECT_NAME);
            if (cmdList.at(2) == "none")
                QFile::remove(asName);
            else
                emit storeAutostartProject(asName);
            QString wsMessage = QString("ALERT|" + tr("Autostart configuration changed"));
            mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toLatin1().data(), wsMessage.length());
            return MG_TRUE;
        }
        else if (cmdList.at(1) == "REBOOT")
        {
            QProcess *rebootProcess = new QProcess();
            rebootProcess->start("reboot", QStringList());
        }
    }
#endif
    else if(cmdList[0] == "POLL")
        return MG_TRUE;

    if (qData.contains("|") == false)
        return MG_FALSE;

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
                if (cmdList.count() < 2)
                    return MG_FALSE;

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

    return MG_TRUE;
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
    if (m_frameFound == false)
    {
        m_CSScode += frame->getCSS();
        m_frameFound = true;
    }
    if (frame->isHeaderVisible())
    {
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
    if (m_soloFrameFound == false)
    {
        m_CSScode += frame->getCSS();
        m_soloFrameFound = true;
    }
    if (frame->isHeaderVisible())
    {
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
            "onchange=\"slVchange(" + slID + ");\" ontouchmove=\"slVchange(" + slID + ");\""
            "style=\""
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
                    {
                        if (chaser->fadeInSpeed() == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(chaser->fadeInSpeed()) + "</td>";
                    }
                    break;
                    case Chaser::PerStep:
                    {
                        if (step.fadeIn == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(step.fadeIn) + "</td>";
                    }
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
                    {
                        if (chaser->fadeOutSpeed() == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(chaser->fadeOutSpeed()) + "</td>";
                    }
                    break;
                    case Chaser::PerStep:
                    {
                        if (step.fadeOut == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(step.fadeOut) + "</td>";
                    }
                    break;
                    default:
                    case Chaser::Default:
                        str += "<td></td>";
                }

                switch (chaser->durationMode())
                {
                    case Chaser::Common:
                    {
                        if (chaser->duration() == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(chaser->duration()) + "</td>";
                    }
                    break;
                    case Chaser::PerStep:
                    {
                        if (step.fadeOut == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(step.duration) + "</td>";
                    }
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
            HIDDEN_FORM_CSS
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
            "<span>" + tr("Load project") + "</span></a>\n"

            "<a class=\"button button-blue\" href=\"/config\"><span>" + tr("Configuration") + "</span></a>\n"

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

QString WebAccess::getIOConfigHTML()
{
    QString html = "";
    InputOutputMap *ioMap = m_doc->inputOutputMap();

    QStringList IOplugins = ioMap->inputPluginNames();
    foreach (QString out, ioMap->outputPluginNames())
        if (IOplugins.contains(out) == false)
            IOplugins.append(out);

    QStringList inputLines, outputLines, feedbackLines;
    QStringList profiles = ioMap->profileNames();

    foreach (QString pluginName, IOplugins)
    {
        QStringList inputs = ioMap->pluginInputs(pluginName);
        QStringList outputs = ioMap->pluginOutputs(pluginName);
        bool hasFeedback = ioMap->pluginSupportsFeedback(pluginName);

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

    html += "<table class=\"hovertable\" style=\"width: 100%;\">\n";
    html += "<tr><th>Universe</th><th>Input</th><th>Output</th><th>Feedback</th><th>Profile</th></tr>\n";

    for (quint32 i = 0; i < ioMap->universes(); i++)
    {
        InputPatch* ip = ioMap->inputPatch(i);
        OutputPatch* op = ioMap->outputPatch(i);
        OutputPatch* fp = ioMap->feedbackPatch(i);
        QString uniName = ioMap->getUniverseNameByIndex(i);
        bool uniPass = ioMap->getUniversePassthrough(i);

        QString currentInputPluginName = (ip == NULL)?KInputNone:ip->pluginName();
        quint32 currentInput = (ip == NULL)?QLCChannel::invalid():ip->input();
        QString currentOutputPluginName = (op == NULL)?KOutputNone:op->pluginName();
        quint32 currentOutput = (op == NULL)?QLCChannel::invalid():op->output();
        QString currentFeedbackPluginName = (fp == NULL)?KOutputNone:fp->pluginName();
        quint32 currentFeedback = (fp == NULL)?QLCChannel::invalid():fp->output();
        QString currentProfileName = (ip == NULL)?KInputNone:ip->profileName();

        html += "<tr align=center><td>" + uniName + "</td>\n";
        html += "<td><select onchange=\"ioChanged('INPUT', " + QString::number(i) + ", this.value);\">\n";
        for (int in = 0; in < inputLines.count(); in++)
        {
            QStringList strList = inputLines.at(in).split(",");
            QString selected = "";
            if (currentInputPluginName == strList.at(0) && currentInput == strList.at(2).toUInt())
                selected = "selected";
            html += "<option value=\"" + QString("%1|%2").arg(strList.at(0)).arg(strList.at(2)) + "\" " + selected + ">" +
                    QString("[%1] %2").arg(strList.at(0)).arg(strList.at(1)) + "</option>\n";
        }
        html += "</select></td>\n";
        html += "<td><select onchange=\"ioChanged('OUTPUT', " + QString::number(i) + ", this.value);\">\n";
        for (int in = 0; in < outputLines.count(); in++)
        {
            QStringList strList = outputLines.at(in).split(",");
            QString selected = "";
            if (currentOutputPluginName == strList.at(0) && currentOutput == strList.at(2).toUInt())
                selected = "selected";
            html += "<option value=\"" + QString("%1|%2").arg(strList.at(0)).arg(strList.at(2)) + "\" " + selected + ">" +
                    QString("[%1] %2").arg(strList.at(0)).arg(strList.at(1)) + "</option>\n";
        }
        html += "</select></td>\n";
        html += "<td><select onchange=\"ioChanged('FB', " + QString::number(i) + ", this.value);\">\n";
        for (int in = 0; in < feedbackLines.count(); in++)
        {
            QStringList strList = feedbackLines.at(in).split(",");
            QString selected = "";
            if (currentFeedbackPluginName == strList.at(0) && currentFeedback == strList.at(2).toUInt())
                selected = "selected";
            html += "<option value=\"" + QString("%1|%2").arg(strList.at(0)).arg(strList.at(2)) + "\" " + selected + ">" +
                    QString("[%1] %2").arg(strList.at(0)).arg(strList.at(1)) + "</option>\n";
        }
        html += "</select></td>\n";
        html += "<td><select onchange=\"ioChanged('PROFILE', " + QString::number(i) + ", this.value);\">\n";
        for (int p = 0; p < profiles.count(); p++)
        {
            QString selected = "";
            if (currentProfileName == profiles.at(p))
                selected = "selected";
            html += "<option value=\"" + profiles.at(p) + "\" " + selected + ">" + profiles.at(p) + "</option>\n";
        }
        html += "</select></td>\n";
        html += "<td><label><input type=\"checkbox\" ";
        if (uniPass == true)
            html +="checked=\"checked\"";
        html += " onchange=\"ioChanged('PASSTHROUGH', " + QString::number(i) + ", this.checked);\">";
        html += tr("Passthrough") + "</label></td>\n";

        html += "</tr>\n";
    }
    html += "</table>\n";

    return html;
}

QString WebAccess::getAudioConfigHTML()
{
    QString html = "";
    QList<AudioDeviceInfo> devList;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
 #if defined( __APPLE__) || defined(Q_OS_MAC)
    devList = AudioRendererPortAudio::getDevicesInfo();
 #elif defined(WIN32) || defined(Q_OS_WIN)
    devList = AudioRendererWaveOut::getDevicesInfo();
 #else
    devList = AudioRendererAlsa::getDevicesInfo();
 #endif
#else
    devList = AudioRendererQt::getDevicesInfo();
#endif
    html += "<table class=\"hovertable\" style=\"width: 100%;\">\n";
    html += "<tr><th>Input</th><th>Output</th></tr>\n";
    html += "<tr align=center>";

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
    html += audioInSelect + audioOutSelect + "</tr>\n</table>\n";

    return html;
}

QString WebAccess::getUserFixturesConfigHTML()
{
    QString html = "";
    QDir userFx = QLCFixtureDefCache::userDefinitionDirectory();

    if (userFx.exists() == false || userFx.isReadable() == false)
        return "";

    html += "<table class=\"hovertable\" style=\"width: 100%;\">\n";
    html += "<tr><th>File name</th></tr>\n";

    /* Attempt to read all specified files from the given directory */
    QStringListIterator it(userFx.entryList());
    while (it.hasNext() == true)
    {
        QString path(it.next());

        if (path.toLower().endsWith(".qxf") == true ||
            path.toLower().endsWith(".d4"))
                html += "<tr><td>" + path + "</td></tr>\n";
    }
    html += "</table>\n";
    html += "<br><a class=\"button button-blue\" href=\"javascript:document.getElementById('loadTrigger').click();\">\n"
     "<span>" + tr("Load fixture") + "</span></a>\n";

    return html;
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
            "html { height: 100%; background-color: #111; }\n"
            "body {\n"
            " margin: 0px;\n"
            " background-image: linear-gradient(to bottom, #45484d 0%, #111 100%);\n"
            " background-image: -webkit-linear-gradient(top, #45484d 0%, #111 100%);\n"
            "}\n"
            HIDDEN_FORM_CSS
            CONTROL_BAR_CSS
            BUTTON_BASE_CSS
            BUTTON_SPAN_CSS
            BUTTON_STATE_CSS
            BUTTON_BLUE_CSS
            SWINFO_CSS
            TABLE_CSS
            "</style>\n";

    QString extraButtons = "";
    if (QLCFile::isRaspberry() == true)
    {
        extraButtons = "<a class=\"button button-blue\" href=\"/system\"><span>" + tr("System") + "</span></a>\n";
    }

    QString bodyHTML = "<form action=\"/loadFixture\" method=\"POST\" enctype=\"multipart/form-data\">\n"
                       "<input id=\"loadTrigger\" type=\"file\" "
                       "onchange=\"document.getElementById('submitTrigger').click();\" name=\"qlcfxi\" />\n"
                       "<input id=\"submitTrigger\" type=\"submit\"/></form>"

                       "<div class=\"controlBar\">\n"
                       "<a class=\"button button-blue\" href=\"/\"><span>" + tr("Back") + "</span></a>\n" +
                       extraButtons +
                       "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>"
                       "</div>\n";

    // ********************* IO mapping ***********************
    bodyHTML += "<div style=\"margin: 30px 7% 30px 7%; width: 86%; height: 300px;\" >\n";
    bodyHTML += "<div style=\"font-family: verdana,arial,sans-serif; font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("Universes configuration") + "</div><br>\n";
    bodyHTML += getIOConfigHTML();

    // ********************* audio devices ********************
    bodyHTML += "<div style=\"margin: 30px 7% 30px 7%; width: 86%; height: 300px;\" >\n";
    bodyHTML += "<div style=\"font-family: verdana,arial,sans-serif; font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("Audio configuration") + "</div><br>\n";
    bodyHTML += getAudioConfigHTML();

    // **************** User loaded fixtures ******************

    bodyHTML += "<div style=\"margin: 30px 7% 30px 7%; width: 86%; height: 300px;\" >\n";
    bodyHTML += "<div style=\"font-family: verdana,arial,sans-serif; font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("User loaded fixtures") + "</div><br>\n";
    bodyHTML += getUserFixturesConfigHTML();

    QString str = HTML_HEADER + m_JScode + m_CSScode + "</head>\n<body>\n" + bodyHTML + "</body>\n</html>";

    return str;
}

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
void WebAccess::resetInterface(InterfaceInfo *iface)
{
    iface->name = "";
    iface->isStatic = false;
    iface->isWireless = false;
    iface->address = "";
    iface->gateway = "";
    iface->gateway = "";
    iface->enabled = false;
    iface->ssid = "";
    iface->wpaPass = "";
}

void WebAccess::appendInterface(InterfaceInfo iface)
{
    m_interfaces.append(iface);
}

QString WebAccess::getInterfaceHTML(InterfaceInfo *iface)
{
    QString dhcpChk = iface->isStatic?QString():QString("checked");
    QString staticChk = iface->isStatic?QString("checked"):QString();
    QString visibility = iface->isStatic?QString("visible"):QString("hidden");
    QString html = "<div style=\"margin: 20px 7% 20px 7%; width: 86%;\" >\n";
    html += "<div style=\"font-family: verdana,arial,sans-serif; padding: 5px 7px; font-size:20px; "
            "color:#CCCCCC; background:#222; border-radius: 7px;\">";

    html += tr("Network interface: ") + iface->name + "<br>\n";

    html += "<form style=\"margin: 5px 15px; color:#FFF;\">\n";
    if (iface->isWireless)
    {
        html += tr("Access point name (SSID): ") + "<input type=\"text\" id=\"" +
                iface->name + "SSID\" size=\"15\" value=\"" + iface->ssid + "\"><br>\n";
        html += tr("WPA-PSK Password: ") + "<input type=\"text\" id=\"" +
                iface->name + "WPAPSK\" size=\"15\" value=\"" + iface->wpaPass + "\"><br>\n";
    }
    /** IP mode radio buttons */
    html += "<input type=\"radio\" name=" + iface->name + "NetGroup onclick=\"showStatic('" +
            iface->name + "', false);\" value=\"dhcp\" " + dhcpChk + ">" + tr("Dynamic (DHCP)") + "<br>\n";
    html += "<input type=\"radio\" name=" + iface->name + "NetGroup onclick=\"showStatic('" +
            iface->name + "', true);\" value=\"static\" " + staticChk + ">" + tr("Static") + "<br>\n";

    /** Static IP fields */
    html += "<div id=\"" + iface->name + "StaticFields\" style=\"padding: 5px 30px; visibility:" + visibility + ";\">\n";
    html += tr("IP Address: ") + "<input type=\"text\" id=\"" +
            iface->name + "IPaddr\" size=\"15\" value=\"" + iface->address + "\"><br>\n";
    html += tr("Netmask: ") + "<input type=\"text\" id=\"" +
            iface->name + "Netmask\" size=\"15\" value=\"" + iface->netmask + "\"><br>\n";
    html += tr("Gateway: ") + "<input type=\"text\" size=\"15\" id=\"" +
            iface->name + "Gateway\" value=\"" + iface->gateway + "\"><br>\n";
    html += "</div>\n";
    html += "<input type=\"button\" value=\"" + tr("Apply changes") + "\" onclick=\"applyParams('" + iface->name + "');\" >\n";
    html += "</form></div></div>";

    return html;
}

QString WebAccess::getNetworkHTML()
{
    QStringList systemDevs;
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
        systemDevs.append(interface.name());

    QFile netFile(IFACES_SYSTEM_FILE);
    if (netFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
        return "";

    m_interfaces.clear();
    InterfaceInfo currInterface;
    resetInterface(&currInterface);

    QString html = "";

    QTextStream in(&netFile);
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        line = line.simplified();
        // ignore comments
        if (line.startsWith('#'))
            continue;

        QStringList ifaceRow = line.split(" ");
        if (ifaceRow.count() == 0)
            continue;

        QString keyword = ifaceRow.at(0);
        if (keyword == "iface")
        {
            if (currInterface.isStatic == true)
            {
                html += getInterfaceHTML(&currInterface);
                appendInterface(currInterface);
                resetInterface(&currInterface);
            }

            if (ifaceRow.count() < 4)
                continue;

            currInterface.name = ifaceRow.at(1);
            // remove the interface from the system list
            // the remaining interfaces will be added at the
            // end as disabled interfaces
            systemDevs.removeOne(currInterface.name);

            if (currInterface.name.contains("wlan") || currInterface.name.contains("ra"))
                currInterface.isWireless = true;

            if (ifaceRow.at(3) == "dhcp")
            {
                html += getInterfaceHTML(&currInterface);
                appendInterface(currInterface);
                resetInterface(&currInterface);
            }
            else if (ifaceRow.at(3) == "static")
                currInterface.isStatic = true;
        }
        else if (keyword == "address")
            currInterface.address = ifaceRow.at(1);
        else if (keyword == "netmask")
            currInterface.netmask = ifaceRow.at(1);
        else if (keyword == "gateway")
            currInterface.gateway = ifaceRow.at(1);
        else if (keyword == "wpa-ssid")
            currInterface.ssid = ifaceRow.at(1);
        else if (keyword == "wpa-psk")
            currInterface.wpaPass = ifaceRow.at(1);

        currInterface.enabled = true;
    }

    netFile.close();

    if (currInterface.isStatic == true)
    {
        html += getInterfaceHTML(&currInterface);
        appendInterface(currInterface);
        resetInterface(&currInterface);
    }

    foreach(QString dev, systemDevs)
    {
        currInterface.name = dev;
        currInterface.enabled = false;
        if (currInterface.name.contains("wlan") || currInterface.name.contains("ra"))
            currInterface.isWireless = true;
        html += getInterfaceHTML(&currInterface);
        appendInterface(currInterface);
        resetInterface(&currInterface);
    }

    foreach (InterfaceInfo info, m_interfaces)
    {
        qDebug() << "Interface:" << info.name << "isstatic:" << info.isStatic;
    }

    return html;
}

QString WebAccess::getSystemConfigHTML()
{
    m_JScode = "<script language=\"javascript\" type=\"text/javascript\">\n" WEBSOCKET_JS;
    m_JScode += "function systemCmd(cmd, iface, mode, addr, mask, gw, ssid, wpapsk)\n"
            "{\n"
            " websocket.send(\"QLC+SYS|\" + cmd + \"|\" + iface + \"|\" + mode + \"|\" + addr + \"|\" + mask + \"|\" + gw + \"|\" + ssid + \"|\" + wpapsk);\n"
            "};\n"

            "function showStatic(iface, enable) {\n"
            " var divName = iface + \"StaticFields\";\n"
            " var obj=document.getElementById(divName);\n"
            " if (enable == true)\n"
            "   obj.style.visibility='visible';\n"
            " else\n"
            "   obj.style.visibility='hidden';\n"
            "}\n"

            "function applyParams(iface) {\n"
            " var radioGroup = iface + \"NetGroup\";\n"
            " var radios = document.getElementsByName(radioGroup);\n"
            " var ssidObj = document.getElementById(iface+\"SSID\");\n"
            " var ssidVal = '';\n"
            " if (ssidObj != null) ssidVal = ssidObj.value;\n"
            " var wpapskObj = document.getElementById(iface+\"WPAPSK\");\n"
            " var wpapskVal = '';\n"
            " if (wpapskObj != null) wpapskVal = wpapskObj.value;\n"
            " if (radios[0].checked)\n"
            "   systemCmd(\"NETWORK\", iface, \"dhcp\", '', '', '', ssidVal, wpapskVal);\n"
            " else if (radios[1].checked) {\n"
            "   var addrName=iface+\"IPaddr\";\n"
            "   var maskName=iface+\"Netmask\";\n"
            "   var gwName=iface+\"Gateway\";\n"
            "   systemCmd(\"NETWORK\", iface, \"static\", document.getElementById(addrName).value,"
            " document.getElementById(maskName).value, document.getElementById(gwName).value, ssidVal, wpapskVal);\n"
            " }\n"
            "}\n"

            "function setAutostart() {\n"
            " var radios = document.getElementsByName('autostart');\n"
            " if (radios[0].checked)\n"
            "   websocket.send('QLC+SYS|AUTOSTART|none');\n"
            " else\n"
            "   websocket.send('QLC+SYS|AUTOSTART|current');\n"
            "}\n\n";

    m_JScode += "</script>\n";

    m_CSScode = "<style>\n"
            "html { height: 100%; background-color: #111; }\n"
            "body {\n"
            " margin: 0px;\n"
            " background-image: linear-gradient(to bottom, #45484d 0%, #111 100%);\n"
            " background-image: -webkit-linear-gradient(top, #45484d 0%, #111 100%);\n"
            "}\n"
            CONTROL_BAR_CSS
            BUTTON_BASE_CSS
            BUTTON_SPAN_CSS
            BUTTON_STATE_CSS
            BUTTON_BLUE_CSS
            SWINFO_CSS
            "</style>\n";

    QString bodyHTML = "<div class=\"controlBar\">\n"
                       "<a class=\"button button-blue\" href=\"/\"><span>" + tr("Back") + "</span></a>\n"
                       "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>"
                       "</div>\n";

    bodyHTML += "<div style=\"margin: 15px 7% 0px 7%; width: 86%; font-family: verdana,arial,sans-serif;"
                "font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("Network configuration") + "</div>\n";
    bodyHTML += getNetworkHTML();

    bodyHTML += "<div style=\"margin: 15px 7% 0px 7%; width: 86%; font-family: verdana,arial,sans-serif;"
                "font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("Project autostart") + "</div>\n";
    bodyHTML += "<div style=\"margin: 15px 7% 0px 7%; width: 86%; font-family: verdana,arial,sans-serif;"
                "font-size:18px; padding: 5px 0px; color:#CCCCCC; background:#222; border-radius: 7px;\">";
    bodyHTML += "<form style=\"margin: 5px 15px; color:#FFF;\">\n";
    bodyHTML += "<input type=\"radio\" name=autostart value=\"none\">" + tr("No project") + "\n";
    bodyHTML += "<input type=\"radio\" name=autostart value=\"current\" checked>" + tr("Use current project") + "\n";
    bodyHTML += "<input type=\"button\" value=\"" + tr("Apply changes") + "\" onclick=\"setAutostart();\" >\n";
    bodyHTML += "</form></div>\n";

    bodyHTML += "<div style=\"margin:5px 7%;\">\n";
    bodyHTML += "<a class=\"button button-blue\" href=\"\" onclick=\"javascript:websocket.send('QLC+SYS|REBOOT');\"><span>" + tr("Reboot") + "</span></a>\n";
    bodyHTML += "</div>\n";

    QString str = HTML_HEADER + m_JScode + m_CSScode + "</head>\n<body>\n" + bodyHTML + "</body>\n</html>";

    return str;
}

bool WebAccess::writeNetworkFile()
{
    QFile netFile(IFACES_SYSTEM_FILE);
    if (netFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;

    netFile.write(QString("auto lo\n").toLatin1());
    netFile.write(QString("iface lo inet loopback\n").toLatin1());
    netFile.write(QString("allow-hotplug eth0\n").toLatin1());

    foreach (InterfaceInfo iface, m_interfaces)
    {
        if (iface.enabled == false)
            continue;

        if (iface.isWireless)
            netFile.write((QString("auto %1\n").arg(iface.name)).toLatin1());

        if (iface.isStatic == false)
            netFile.write((QString("iface %1 inet dhcp\n").arg(iface.name)).toLatin1());
        else
        {
            netFile.write((QString("iface %1 inet static\n").arg(iface.name)).toLatin1());
            netFile.write((QString("  address %1\n").arg(iface.address)).toLatin1());
            netFile.write((QString("  netmask %1\n").arg(iface.netmask)).toLatin1());
            netFile.write((QString("  gateway %1\n").arg(iface.gateway)).toLatin1());
        }
        if (iface.isWireless)
        {
            if (iface.ssid.isEmpty() == false)
                netFile.write((QString("  wpa-ssid %1\n").arg(iface.ssid)).toLatin1());
            if (iface.wpaPass.isEmpty() == false)
                netFile.write((QString("  wpa-psk %1\n").arg(iface.wpaPass)).toLatin1());
        }
    }

    netFile.close();

    return true;
}
#endif

void WebAccess::slotVCLoaded()
{
    if (m_conn == NULL)
        return;

    QString wsMessage = QString("URL|/");
    mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toLatin1().data(), wsMessage.length());
}


