/*
  Q Light Controller Plus
  webaccess-qml.cpp

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
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <qmath.h>

#include "webaccess-qml.h"
#include "webaccessauth.h"
#include "webaccesssimpledesk.h"
#include "webaccessnetwork.h"
#include "commonjscss.h"
#include "qlcfile.h"
#include "qlcconfig.h"

#include "virtualconsole.h"
#include "vcaudiotriggers.h"
#include "vcanimation.h"
#include "vcspeeddial.h"
#include "vccuelist.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcclock.h"
#include "vcxypad.h"
#include "vcpage.h"

#include "function.h"
#include "chaser.h"
#include "doc.h"
#include "listmodel.h"
#include "simpledesk.h"

#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "qhttpconnection.h"


static QJsonObject fontToJson(const QFont &font)
{
    QJsonObject obj;
    obj["family"] = font.family();
    obj["pixelSize"] = font.pixelSize();
    obj["pointSize"] = font.pointSize();
    obj["bold"] = font.bold();
    obj["italic"] = font.italic();
    obj["weight"] = font.weight();
    return obj;
}

static QJsonObject rectToJson(const QRectF &rect)
{
    QJsonObject obj;
    obj["x"] = rect.x();
    obj["y"] = rect.y();
    obj["w"] = rect.width();
    obj["h"] = rect.height();
    return obj;
}

static QString colorToString(const QColor &color)
{
    if (!color.isValid())
        return QString();
    return color.name();
}

static QJsonObject loadUiStyleJson()
{
    QDir userConfDir = QLCFile::userDirectory(QString(USERQLCPLUSDIR),
                                              QString(USERQLCPLUSDIR),
                                              QStringList());
    const QString stylePath = userConfDir.absolutePath()
            + QDir::separator()
            + QStringLiteral("qlcplusUiStyle.json");
    QFile jsonFile(stylePath);
    if (jsonFile.exists() == false || jsonFile.open(QIODevice::ReadOnly) == false)
        return QJsonObject();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || jsonDoc.isObject() == false)
        return QJsonObject();

    return jsonDoc.object();
}

static QString sliderDisplayValue(VCSlider *slider)
{
    if (slider == nullptr)
        return QString();

    if (slider->valueDisplayStyle() == VCSlider::DMXValue)
        return QString::number(slider->value());

    int p = qFloor(((double(slider->value()) / double(UCHAR_MAX)) * double(100)) + 0.5);
    return QString::number(p);
}

static QString mimeTypeForPath(const QString &path)
{
    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "otf") return "font/otf";
    if (ext == "ttf") return "font/ttf";
    if (ext == "woff") return "font/woff";
    if (ext == "woff2") return "font/woff2";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "css") return "text/css";
    if (ext == "js") return "text/javascript";
    if (ext == "json") return "application/json";
    return "application/octet-stream";
}

static bool isWidgetVisibleForWeb(const VCWidget *widget, const VirtualConsole *vc)
{
    if (widget == nullptr || vc == nullptr)
        return false;
    if (widget->isVisible() == false)
        return false;

    const QObject *obj = widget;
    const VCPage *pageParent = nullptr;
    while (obj != nullptr)
    {
        const VCWidget *childWidget = qobject_cast<const VCWidget *>(obj);
        const VCFrame *frameParent = qobject_cast<const VCFrame *>(obj->parent());
        if (childWidget != nullptr && frameParent != nullptr && frameParent->multiPageMode())
        {
            if (childWidget->page() != frameParent->currentPage())
                return false;
        }

        const VCPage *page = qobject_cast<const VCPage *>(obj);
        if (page != nullptr)
            pageParent = page;

        obj = obj->parent();
    }

    if (pageParent != nullptr)
        return pageParent == vc->page(vc->selectedPage());

    return true;
}
/*
static void logWidgetTree(VCWidget *widget, int depth)
{
    if (widget == nullptr)
        return;

    const QRectF geom = widget->geometry();
    const QString indent(depth * 2, ' ');
    qDebug().noquote() << indent
        + QString("%1 id=%2 page=%3 geom=%4,%5 %6x%7")
              .arg(VCWidget::typeToString(widget->type()))
              .arg(widget->id())
              .arg(widget->page())
              .arg(geom.x())
              .arg(geom.y())
              .arg(geom.width())
              .arg(geom.height());

    VCFrame *frame = qobject_cast<VCFrame *>(widget);
    if (frame == nullptr)
        return;

    const QList<VCWidget *> children = frame->children(false);
    for (VCWidget *child : children)
        logWidgetTree(child, depth + 1);
}
*/
static QString getSimpleDeskQmlHtml(const Doc *doc, const SimpleDesk *sd)
{
    if (doc == nullptr || sd == nullptr)
        return QString();

    int uni = sd->getCurrentUniverseIndex() + 1;
    int page = sd->getCurrentPage();

    QString JScode = "<script src=\"simpledesk-v5.js\"></script>\n";
    JScode += "<script>\n";
    JScode += "var currentUniverse = " + QString::number(uni) + ";\n";
    JScode += "var currentPage = " + QString::number(page) + ";\n";
    JScode += "var channelsPerPage = " + QString::number(sd->getSlidersNumber()) + ";\n";
    JScode += "</script>\n";

    QString CSScode = "<link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"webaccess-v5.css\">\n";
    CSScode += "<link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"simpledesk-v5.css\">\n";

    QString bodyHTML = "<div id=\"app\">\n"
                       "<header class=\"topbar\">\n"
                       "<div class=\"brand\">\n"
                       "<div class=\"brand-title\">" + QObject::tr("Simple Desk") + "</div>\n"
                       "<div class=\"brand-sub\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>\n"
                       "</div>\n"
                       "<div class=\"sd-topbar-center\">\n"
                       "<div class=\"sd-section\">\n"
                       "<div class=\"sd-label\">" + QObject::tr("Universe") + "</div>\n"
                       "<select class=\"sd-select\" id=\"universeSelect\">\n";

    QStringList uniList = doc->inputOutputMap()->universeNames();
    for (int i = 0; i < uniList.count(); i++)
    {
        QString selected = (i + 1 == uni) ? " selected" : "";
        bodyHTML += "<option value=\"" + QString::number(i) + "\"" + selected + ">"
                + uniList.at(i) + "</option>\n";
    }

    bodyHTML += "</select>\n"
                "</div>\n"
                "<button class=\"nav-btn\" id=\"resetUniverseBtn\" type=\"button\">"
                + QObject::tr("Reset universe") + "</button>\n"
                "<div class=\"sd-section\">\n"
                "<div class=\"sd-label\">" + QObject::tr("Faders") + "</div>\n"
                "<select class=\"sd-select\" id=\"fadersSelect\">\n"
                "<option value=\"8\">8</option>\n"
                "<option value=\"16\">16</option>\n"
                "<option value=\"24\">24</option>\n"
                "<option value=\"32\">32</option>\n"
                "<option value=\"48\">48</option>\n"
                "<option value=\"64\">64</option>\n"
                "</select>\n"
                "</div>\n"
                "<div class=\"sd-section\">\n"
                "<div class=\"sd-label\">" + QObject::tr("Page") + "</div>\n"
                "<button class=\"nav-btn\" id=\"pagePrev\" type=\"button\">&#x2039;</button>\n"
                "<div class=\"sd-page-display\" id=\"pageDisplay\">" + QString::number(page) + "</div>\n"
                "<button class=\"nav-btn\" id=\"pageNext\" type=\"button\">&#x203a;</button>\n"
                "</div>\n"
                "</div>\n"
                "<div class=\"topbar-right\">\n"
                "<div class=\"actions\">\n"
                "<a class=\"nav-btn\" href=\"/\">" + QObject::tr("Back") + "</a>\n"
                "<a class=\"nav-btn\" href=\"/keypad.html\">DMX Keypad</a>\n"
                "</div>\n"
                "</div>\n"
                "</header>\n"
                "<main class=\"sd-stage\">\n"
                "<div class=\"sd-sliders\" id=\"slidersContainer\"></div>\n"
                "</main>\n"
                "</div>\n";

    return QString(HTML_HEADER) + JScode + CSScode + "</head>\n<body>\n" + bodyHTML + "</body>\n</html>";
}

WebAccessQml::WebAccessQml(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance,
                           int portNumber, bool enableAuth, QString passwdFile, QObject *parent)
    : WebAccessBase(doc, vcInstance, sdInstance, portNumber, enableAuth, passwdFile, parent)
{
    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotDocLoaded()));

    connect(m_doc->inputOutputMap(), SIGNAL(grandMasterValueChanged(uchar)),
            this, SLOT(slotGrandMasterValueChanged(uchar)));

    connect(m_vc, SIGNAL(selectedPageChanged(int)),
            this, SLOT(slotSelectedPageChanged(int)));
}

WebAccessQml::~WebAccessQml()
{
}

void WebAccessQml::slotHandleHTTPRequest(QHttpRequest *req, QHttpResponse *resp)
{
    WebAccessUser user;

    if (!authenticateRequest(req, resp, user))
        return;

    QString reqUrl = req->url().path();
    QString content;

    qDebug() << Q_FUNC_INFO << req->methodString() << req->url();

    if (reqUrl == "/vc.json")
    {
        if (!requireAuthLevel(resp, user, VC_ONLY_LEVEL))
            return;
        QByteArray json = getVCJson();
        resp->setHeader("Content-Type", "application/json");
        resp->setHeader("Content-Length", QString::number(json.size()));
        resp->writeHead(200);
        resp->end(json);
        return;
    }
    else if (reqUrl.startsWith("/qrc/"))
    {
        QString qrcPath = ":/" + reqUrl.mid(5);
        if (sendFile(resp, qrcPath, mimeTypeForPath(qrcPath)))
            return;
    }
    else if (reqUrl == "/simpleDesk")
    {
        if (!requireAuthLevel(resp, user, SIMPLE_DESK_AND_VC_LEVEL))
            return;
        content = getSimpleDeskQmlHtml(m_doc, m_sd);
        sendHtmlResponse(resp, content);
        return;
    }
    CommonRequestResult commonResult = handleCommonHTTPRequest(req, resp, user, reqUrl, content);
    if (commonResult == CommonRequestResult::Handled)
        return;
    if (commonResult == CommonRequestResult::ContentReady)
    {
        sendHtmlResponse(resp, content);
        return;
    }

    if (serveWebFile(resp, "/webaccess-v5.html", "text/html"))
        return;
    content = QString(HTML_HEADER) + "</head><body>Missing webaccess-v5.html</body></html>";
    sendHtmlResponse(resp, content);
}

void WebAccessQml::handleProjectLoad(const QByteArray &projectXml)
{
    emit loadProject(projectXml);
}

bool WebAccessQml::storeFixtureDefinition(const QString &fxName, const QByteArray &fixtureXML)
{
    QString fxPath = QString("%1/%2/%3").arg(getenv("HOME")).arg(USERQLCPLUSDIR).arg(fxName);
    QFile fxFile(fxPath);
    if (fxFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        fxFile.write(fixtureXML);
        fxFile.close();
        return true;
    }

    qWarning() << Q_FUNC_INFO << "Unable to save file" << fxPath;
    return false;
}

void WebAccessQml::slotHandleWebSocketRequest(QHttpConnection *conn, QString data)
{
    if (conn == nullptr)
        return;

    WebAccessUser *user = static_cast<WebAccessUser*>(conn->userData);

    QStringList cmdList = data.split("|");
    if (cmdList.isEmpty())
        return;

    if (cmdList[0] == "QLC+CMD")
    {
        return;
    }
    else if (cmdList[0] == "VC_PAGE")
    {
        if (m_auth && user && user->level < VC_ONLY_LEVEL)
            return;
        if (cmdList.count() > 1)
            m_vc->setSelectedPage(cmdList[1].toInt());
        return;
    }
    if (handleCommonWebSocketCommand(conn, user, cmdList, "[webaccess-v5]", true))
        return;
    else if (cmdList[0] == "QLC+API")
    {
        if (m_auth && user && user->level < VC_ONLY_LEVEL)
            return;

        if (cmdList.count() < 2)
            return;

        QString apiCmd = cmdList[1];
        QString wsAPIMessage = QString("QLC+API|%1|").arg(apiCmd);

        if (apiCmd == "isProjectLoaded")
        {
            wsAPIMessage.append(m_pendingProjectLoaded ? "true" : "false");
        }
        else if (apiCmd == "getFunctionsNumber")
        {
            wsAPIMessage.append(QString::number(m_doc->functions().count()));
        }
        else if (apiCmd == "getFunctionsList")
        {
            foreach (Function *f, m_doc->functions())
                wsAPIMessage.append(QString("%1|%2|").arg(f->id()).arg(f->name()));
            wsAPIMessage.truncate(wsAPIMessage.length() - 1);
        }
        else if (apiCmd == "getFunctionType")
        {
            if (cmdList.count() < 3)
                return;

            quint32 fID = cmdList[2].toUInt();
            Function *f = m_doc->function(fID);
            if (f != nullptr)
                wsAPIMessage.append(f->typeString());
            else
                wsAPIMessage.append(Function::typeToString(Function::Undefined));
        }
        else if (apiCmd == "getFunctionStatus")
        {
            if (cmdList.count() < 3)
                return;

            quint32 fID = cmdList[2].toUInt();
            Function *f = m_doc->function(fID);
            if (f != nullptr)
                wsAPIMessage.append(f->isRunning() ? "Running" : "Stopped");
            else
                wsAPIMessage.append(Function::typeToString(Function::Undefined));
        }
        else if (apiCmd == "setFunctionStatus")
        {
            if (cmdList.count() < 4)
                return;

            quint32 fID = cmdList[2].toUInt();
            quint32 newStatus = cmdList[3].toUInt();
            Function *f = m_doc->function(fID);

            if (f != nullptr)
            {
                if (!f->isRunning() && newStatus)
                    f->start(m_doc->masterTimer(), FunctionParent::master());
                else if (f->isRunning() && !newStatus)
                    f->stop(FunctionParent::master());
            }
            return;
        }
        else if (apiCmd == "getWidgetsNumber")
        {
            QList<VCWidget *> widgets;
            for (int i = 0; i < m_vc->pagesCount(); i++)
                collectWidgets(m_vc->page(i), widgets, true);
            wsAPIMessage.append(QString::number(widgets.count()));
        }
        else if (apiCmd == "getWidgetsList")
        {
            QList<VCWidget *> widgets;
            for (int i = 0; i < m_vc->pagesCount(); i++)
                collectWidgets(m_vc->page(i), widgets, true);

            foreach (VCWidget *widget, widgets)
                wsAPIMessage.append(QString("%1|%2|").arg(widget->id()).arg(widget->caption()));
            wsAPIMessage.truncate(wsAPIMessage.length() - 1);
        }
        else if (apiCmd == "getWidgetType")
        {
            if (cmdList.count() < 3)
                return;

            quint32 wID = cmdList[2].toUInt();
            VCWidget *widget = m_vc->widget(wID);
            if (widget != nullptr)
                wsAPIMessage.append(QString("%1|%2").arg(wID).arg(VCWidget::typeToString(widget->type())));
            else
                wsAPIMessage.append(QString("%1|%2").arg(wID).arg(VCWidget::typeToString(VCWidget::UnknownWidget)));
        }
        else if (apiCmd == "getWidgetFunction")
        {
            if (cmdList.count() < 3)
                return;

            quint32 wID = cmdList[2].toUInt();
            VCWidget *widget = m_vc->widget(wID);

            wsAPIMessage.append(QString("%1|").arg(wID));

            quint32 fID = 0;

            if (widget != nullptr)
            {
                switch (widget->type())
                {
                    case VCWidget::ButtonWidget:
                    {
                        VCButton *button = qobject_cast<VCButton*>(widget);
                        if (button != nullptr && button->functionID() != Function::invalidId())
                            fID = button->functionID();
                    }
                    break;
                    case VCWidget::CueListWidget:
                    {
                        VCCueList *cue = qobject_cast<VCCueList*>(widget);
                        if (cue != nullptr && cue->chaserID() != Function::invalidId())
                            fID = cue->chaserID();
                    }
                    break;
                    case VCWidget::SliderWidget:
                    {
                        VCSlider *slider = qobject_cast<VCSlider*>(widget);
                        if (slider != nullptr && slider->controlledFunction() != Function::invalidId())
                            fID = slider->controlledFunction();
                    }
                    break;
                    default:
                        break;
                }
            }

            Function *f = (fID != 0) ? m_doc->function(fID) : nullptr;
            if (f != nullptr)
                wsAPIMessage.append(QString("%1|%2|%3").arg(f->id()).arg(f->typeString()).arg(f->name()));
            else
                wsAPIMessage.append(QString("0|%1|").arg(Function::typeToString(Function::Undefined)));
        }
        else if (apiCmd == "getWidgetStatus")
        {
            if (cmdList.count() < 3)
                return;

            quint32 wID = cmdList[2].toUInt();
            VCWidget *widget = m_vc->widget(wID);
            if (widget != nullptr)
            {
                wsAPIMessage.append(QString("%1|").arg(wID));

                switch (widget->type())
                {
                    case VCWidget::ButtonWidget:
                    {
                        VCButton *button = qobject_cast<VCButton*>(widget);
                        if (button->state() == VCButton::Active)
                            wsAPIMessage.append("255");
                        else if (button->state() == VCButton::Monitoring)
                            wsAPIMessage.append("127");
                        else
                            wsAPIMessage.append("0");
                    }
                    break;
                    case VCWidget::SliderWidget:
                    {
                        VCSlider *slider = qobject_cast<VCSlider*>(widget);
                        wsAPIMessage.append(QString::number(slider->value()));
                    }
                    break;
                    case VCWidget::CueListWidget:
                    {
                        VCCueList *cue = qobject_cast<VCCueList*>(widget);
                        if (cue->playbackStatus() == VCCueList::Playing)
                            wsAPIMessage.append(QString("PLAY|%1").arg(cue->playbackIndex()));
                        else
                            wsAPIMessage.append("STOP");
                    }
                    break;
                    case VCWidget::AnimationWidget:
                    {
                        VCAnimation *animation = qobject_cast<VCAnimation*>(widget);
                        wsAPIMessage.append(QString::number(animation->faderLevel()));
                    }
                    break;
                    default:
                        wsAPIMessage.append("0");
                    break;
                }
            }
        }
        else if (apiCmd == "getChannelsValues")
        {
            if (m_auth && user && user->level < SIMPLE_DESK_AND_VC_LEVEL)
                return;

            if (cmdList.count() < 4)
                return;

            quint32 universe = cmdList[2].toUInt() - 1;
            int startAddr = cmdList[3].toInt() - 1;
            int count = 1;
            if (cmdList.count() == 5)
                count = cmdList[4].toInt();

            wsAPIMessage.append(WebAccessSimpleDesk::getChannelsMessage(m_doc, m_sd, universe, startAddr, count));
        }
        else if (apiCmd == "sdResetChannel")
        {
            if (m_auth && user && user->level < SIMPLE_DESK_AND_VC_LEVEL)
                return;

            if (cmdList.count() < 3)
                return;

            quint32 chNum = cmdList[2].toUInt() - 1;
            m_sd->resetAbsoluteChannel(chNum);
            wsAPIMessage = "QLC+API|getChannelsValues|";
            wsAPIMessage.append(WebAccessSimpleDesk::getChannelsMessage(
                                m_doc, m_sd, m_sd->getCurrentUniverseIndex(),
                                (m_sd->getCurrentPage() - 1) * m_sd->getSlidersNumber(),
                                m_sd->getSlidersNumber()));
        }
        else if (apiCmd == "sdResetUniverse")
        {
            if (m_auth && user && user->level < SIMPLE_DESK_AND_VC_LEVEL)
                return;

            if (cmdList.count() < 3)
                return;

            quint32 universeIndex = cmdList[2].toUInt() - 1;
            m_sd->resetUniverse(universeIndex);
            wsAPIMessage = "QLC+API|getChannelsValues|";
            wsAPIMessage.append(WebAccessSimpleDesk::getChannelsMessage(
                                m_doc, m_sd, m_sd->getCurrentUniverseIndex(),
                                0, m_sd->getSlidersNumber()));
        }

        conn->webSocketWrite(wsAPIMessage);
        return;
    }
    else if (cmdList[0] == "CH")
    {
        if (m_auth && user && user->level < SIMPLE_DESK_AND_VC_LEVEL)
            return;

        if (cmdList.count() < 3)
            return;

        uint absAddress = cmdList[1].toInt() - 1;
        int value = cmdList[2].toInt();
        m_sd->setAbsoluteChannelValue(absAddress, uchar(value));
        return;
    }
    else if (cmdList[0] == "GM_VALUE")
    {
        uchar value = cmdList[1].toInt();
        m_doc->inputOutputMap()->setGrandMasterValue(value);
        return;
    }
    else if (cmdList[0] == "POLL")
        return;

    if (!data.contains("|"))
        return;

    if (m_auth && user && user->level < VC_ONLY_LEVEL)
        return;

    quint32 widgetID = cmdList[0].toUInt();
    VCWidget *widget = m_vc->widget(widgetID);
    uchar value = 0;
    if (cmdList.count() > 1)
        value = (uchar)cmdList[1].toInt();

    if (widget == nullptr)
        return;

    switch (widget->type())
    {
        case VCWidget::ButtonWidget:
        {
            VCButton *button = qobject_cast<VCButton*>(widget);
            if (button != nullptr)
                button->requestStateChange(value > 0);
        }
        break;
        case VCWidget::SliderWidget:
        {
            VCSlider *slider = qobject_cast<VCSlider*>(widget);
            if (slider != nullptr)
            {
                if (cmdList.count() > 1 && cmdList[1] == "SLIDER_OVERRIDE")
                {
                    bool enable = cmdList.count() > 2 ? (cmdList[2].toInt() > 0) : false;
                    slider->setIsOverriding(enable);
                }
                else
                {
                    slider->setValue(value, true, true);
                }
            }
        }
        break;
        case VCWidget::AudioTriggersWidget:
        {
            VCAudioTriggers *triggers = qobject_cast<VCAudioTriggers*>(widget);
            if (triggers != nullptr)
            {
                if (cmdList.count() > 2 && cmdList[1] == "AUDIO_VOLUME")
                {
                    int volume = cmdList[2].toInt();
                    if (volume < 0)
                        volume = 0;
                    else if (volume > 100)
                        volume = 100;
                    triggers->setVolumeLevel(uchar(volume));
                }
                else
                {
                    bool ok = false;
                    int enabledValue = cmdList.count() > 1 ? cmdList[1].toInt(&ok) : 0;
                    if (ok)
                        triggers->setCaptureEnabled(enabledValue > 0);
                }
            }
        }
        break;
        case VCWidget::CueListWidget:
        {
            if (cmdList.count() < 2)
                return;

            VCCueList *cue = qobject_cast<VCCueList*>(widget);
            if (cue == nullptr)
                return;

            if (cmdList[1] == "PLAY")
                cue->playClicked();
            else if (cmdList[1] == "STOP")
                cue->stopClicked();
            else if (cmdList[1] == "PREV")
                cue->previousClicked();
            else if (cmdList[1] == "NEXT")
                cue->nextClicked();
            else if (cmdList[1] == "STEP" && cmdList.count() > 2)
                cue->setPlaybackIndex(cmdList[2].toInt());
            else if (cmdList[1] == "CUE_STEP_NOTE" && cmdList.count() > 3)
                cue->setStepNote(cmdList[2].toInt(), cmdList[3]);
            else if (cmdList[1] == "CUE_SIDECHANGE" && cmdList.count() > 2)
                cue->setSideFaderLevel(cmdList[2].toInt());
        }
        break;
        case VCWidget::FrameWidget:
        case VCWidget::SoloFrameWidget:
        {
            if (cmdList.count() < 2)
                return;

            VCFrame *frame = qobject_cast<VCFrame*>(widget);
            if (frame == nullptr)
                return;

            if (cmdList[1] == "NEXT_PG")
            {
                int nextPage = frame->currentPage() + 1;
                if (nextPage >= frame->totalPagesNumber())
                    nextPage = frame->pagesLoop() ? 0 : frame->currentPage();
                frame->setCurrentPage(nextPage);
            }
            else if (cmdList[1] == "PREV_PG")
            {
                int prevPage = frame->currentPage() - 1;
                if (prevPage < 0)
                    prevPage = frame->pagesLoop() ? frame->totalPagesNumber() - 1 : frame->currentPage();
                frame->setCurrentPage(prevPage);
            }
            else if (cmdList[1] == "PAGE" && cmdList.count() > 2)
                frame->setCurrentPage(cmdList[2].toInt());
            else if (cmdList[1] == "FRAME_DISABLE" && cmdList.count() > 2)
                frame->setDisabled(cmdList[2] == "1");
            else if (cmdList[1] == "COLLAPSE" && cmdList.count() > 2)
                frame->setCollapsed(cmdList[2].toInt() == 1);
        }
        break;
        case VCWidget::AnimationWidget:
        {
            if (cmdList.count() < 2)
                return;

            VCAnimation *animation = qobject_cast<VCAnimation*>(widget);
            if (animation == nullptr)
                return;

            if (cmdList[1] == "MATRIX_SLIDER" && cmdList.count() > 2)
                animation->setFaderLevel(cmdList[2].toInt());
            else if (cmdList[1] == "MATRIX_COLOR_1" && cmdList.count() > 2)
                animation->setColor1(QColor(cmdList[2]));
            else if (cmdList[1] == "MATRIX_COLOR_2" && cmdList.count() > 2)
                animation->setColor2(QColor(cmdList[2]));
            else if (cmdList[1] == "MATRIX_COLOR_3" && cmdList.count() > 2)
                animation->setColor3(QColor(cmdList[2]));
            else if (cmdList[1] == "MATRIX_COLOR_4" && cmdList.count() > 2)
                animation->setColor4(QColor(cmdList[2]));
            else if (cmdList[1] == "MATRIX_COLOR_5" && cmdList.count() > 2)
                animation->setColor5(QColor(cmdList[2]));
            else if (cmdList[1] == "MATRIX_COMBO" && cmdList.count() > 2)
                animation->setAlgorithmIndex(cmdList[2].toInt());
        }
        break;
        case VCWidget::XYPadWidget:
        {
            VCXYPad *xypad = qobject_cast<VCXYPad*>(widget);
            if (xypad == nullptr || cmdList.count() < 2)
                return;

            if (cmdList[1] == "XYPAD")
            {
                if (cmdList.count() < 4)
                    return;
                qreal x = cmdList[2].toDouble();
                qreal y = cmdList[3].toDouble();
                xypad->setCurrentPosition(QPointF(x, y));
            }
            else if (cmdList[1] == "XYPAD_RANGE_H")
            {
                if (cmdList.count() < 4)
                    return;
                qreal minVal = cmdList[2].toDouble();
                qreal maxVal = cmdList[3].toDouble();
                xypad->setHorizontalRange(QPointF(minVal, maxVal));
            }
            else if (cmdList[1] == "XYPAD_RANGE_V")
            {
                if (cmdList.count() < 4)
                    return;
                qreal minVal = cmdList[2].toDouble();
                qreal maxVal = cmdList[3].toDouble();
                xypad->setVerticalRange(QPointF(minVal, maxVal));
            }
        }
        break;
        case VCWidget::SpeedWidget:
        {
            if (cmdList.count() < 2)
                return;

            VCSpeedDial *dial = qobject_cast<VCSpeedDial*>(widget);
            if (dial == nullptr)
                return;

            if (cmdList[1] == "SPEED_UP")
                dial->increaseSpeedFactor();
            else if (cmdList[1] == "SPEED_DOWN")
                dial->decreaseSpeedFactor();
            else if (cmdList[1] == "SPEED_APPLY")
                dial->applyFunctionsTime();
            else if (cmdList[1] == "SPEED_TIME" && cmdList.count() > 2)
                dial->setCurrentTime(cmdList[2].toUInt());
            else if (cmdList[1] == "SPEED_FACTOR" && cmdList.count() > 2)
                dial->setCurrentFactor(static_cast<VCSpeedDial::SpeedMultiplier>(cmdList[2].toInt()));
        }
        break;
        default:
        break;
    }
}

void WebAccessQml::slotHandleWebSocketClose(QHttpConnection *conn)
{
    qDebug() << "Websocket Connection closed";
    if (conn->userData)
    {
        WebAccessUser* user = static_cast<WebAccessUser*>(conn->userData);
        delete user;
        conn->userData = nullptr;
    }
    conn->deleteLater();

    m_webSocketsList.removeOne(conn);
}

void WebAccessQml::slotFunctionStarted(quint32 fid)
{
    QString wsMessage = QString("FUNCTION|%1|Running").arg(fid);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotFunctionStopped(quint32 fid)
{
    QString wsMessage = QString("FUNCTION|%1|Stopped").arg(fid);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotDocLoaded()
{
    m_pendingProjectLoaded = true;
    m_connectedWidgets.clear();
}

void WebAccessQml::slotSelectedPageChanged(int page)
{
    sendWebSocketMessage(QString("VC_PAGE|%1").arg(page));
}

QString WebAccessQml::webFilePath(const QString &relativePath) const
{
    QString basePath = QLCFile::systemDirectory(WEBFILESDIR).path();
    QString fullPath = QString("%1%2%3").arg(basePath).arg(QDir::separator()).arg(relativePath);
    if (QFile::exists(fullPath))
        return fullPath;

    QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates;
    candidates << QDir::cleanPath(QString("%1/../webaccess/res/%2").arg(appDir).arg(relativePath));
    candidates << QDir::cleanPath(QString("%1/webaccess/res/%2").arg(QDir::currentPath()).arg(relativePath));

    for (const QString &path : candidates)
    {
        if (QFile::exists(path))
            return path;
    }

    return fullPath;
}

void WebAccessQml::sendMatrixState(const VCAnimation *animation) const
{
    if (animation == nullptr)
        return;

    QString wsMessage = QString("%1|MATRIX_STATE|%2|%3|%4|%5|%6|%7|%8")
            .arg(animation->id())
            .arg(animation->faderLevel())
            .arg(animation->algorithmIndex())
            .arg(colorToString(animation->getColor1()))
            .arg(colorToString(animation->getColor2()))
            .arg(colorToString(animation->getColor3()))
            .arg(colorToString(animation->getColor4()))
            .arg(colorToString(animation->getColor5()));
    sendWebSocketMessage(wsMessage);
}

QString WebAccessQml::widgetBackgroundImagePath(const VCWidget *widget) const
{
    if (widget == nullptr || widget->backgroundImage().isEmpty())
        return QString();

    QString imgPath = widget->backgroundImage();
#if defined(WIN32) || defined(Q_OS_WIN)
    if (imgPath.contains(':'))
    {
        imgPath.prepend('/');
        imgPath.replace(':', '/');
    }
#endif
    return imgPath;
}

QJsonObject WebAccessQml::baseWidgetToJson(const VCWidget *widget)
{
    QJsonObject obj;
    if (widget == nullptr)
        return obj;

    setupWidgetConnections(widget);

    obj["id"] = int(widget->id());
    obj["type"] = VCWidget::typeToString(widget->type());
    obj["typeId"] = widget->type();
    obj["page"] = widget->page();
    obj["caption"] = widget->caption();
    obj["geometry"] = rectToJson(widget->geometry());
    obj["visible"] = widget->isVisible();
    obj["disabled"] = widget->isDisabled();
    obj["bgColor"] = colorToString(widget->backgroundColor());
    obj["fgColor"] = colorToString(widget->foregroundColor());
    obj["bgImage"] = widgetBackgroundImagePath(widget);
    obj["font"] = fontToJson(widget->font());

    return obj;
}

QJsonObject WebAccessQml::frameToJson(const VCFrame *frame)
{
    QJsonObject obj = baseWidgetToJson(frame);
    obj["showHeader"] = frame->showHeader();
    obj["showEnable"] = frame->showEnable();
    obj["isCollapsed"] = frame->isCollapsed();
    obj["multiPageMode"] = frame->multiPageMode();
    obj["pagesLoop"] = frame->pagesLoop();
    obj["currentPage"] = frame->currentPage();
    obj["totalPages"] = frame->totalPagesNumber();
    QJsonArray labels;
    for (const QString &label : frame->pageLabels())
        labels.append(label);
    obj["pageLabels"] = labels;

    QJsonArray children;
    QList<VCWidget *> childList = frame->children(false);
    for (const VCWidget *child : childList)
        children.append(widgetToJson(child));
    obj["children"] = children;

    return obj;
}

QJsonObject WebAccessQml::widgetToJson(const VCWidget *widget)
{
    QJsonObject obj = baseWidgetToJson(widget);
    if (widget == nullptr)
        return obj;

    switch (widget->type())
    {
        case VCWidget::ButtonWidget:
        {
            const VCButton *button = qobject_cast<const VCButton*>(widget);
            obj["state"] = button->state();
            obj["actionType"] = button->actionType();
            obj["functionId"] = int(button->functionID());
        }
        break;
        case VCWidget::SliderWidget:
        {
            const VCSlider *slider = qobject_cast<const VCSlider*>(widget);
            obj["value"] = slider->value();
            obj["rangeLow"] = slider->rangeLowLimit();
            obj["rangeHigh"] = slider->rangeHighLimit();
            obj["sliderMode"] = VCSlider::sliderModeToString(slider->sliderMode());
            obj["widgetStyle"] = slider->widgetStyleToString(slider->widgetStyle());
            obj["valueDisplay"] = VCSlider::valueDisplayStyleToString(slider->valueDisplayStyle());
            obj["inverted"] = slider->invertedAppearance();
            obj["monitor"] = slider->monitorEnabled();
            obj["isOverriding"] = slider->isOverriding();
        }
        break;
        case VCWidget::XYPadWidget:
        {
            const VCXYPad *xypad = qobject_cast<const VCXYPad*>(widget);
            QPointF pos = xypad->currentPosition();
            QJsonObject posObj;
            posObj["x"] = pos.x();
            posObj["y"] = pos.y();
            obj["position"] = posObj;
            QJsonObject hRange;
            hRange["min"] = xypad->horizontalRange().x();
            hRange["max"] = xypad->horizontalRange().y();
            obj["horizontalRange"] = hRange;
            QJsonObject vRange;
            vRange["min"] = xypad->verticalRange().x();
            vRange["max"] = xypad->verticalRange().y();
            obj["verticalRange"] = vRange;
            obj["invertedAppearance"] = xypad->invertedAppearance();
            obj["displayMode"] = int(xypad->displayMode());
        }
        break;
        case VCWidget::FrameWidget:
        case VCWidget::SoloFrameWidget:
            return frameToJson(qobject_cast<const VCFrame*>(widget));
        case VCWidget::CueListWidget:
        {
            const VCCueList *cue = qobject_cast<const VCCueList*>(widget);
            obj["chaserId"] = int(cue->chaserID());
            obj["nextPrevBehavior"] = int(cue->nextPrevBehavior());
            obj["playbackLayout"] = int(cue->playbackLayout());
            obj["sideFaderMode"] = int(cue->sideFaderMode());
            obj["sideFaderLevel"] = cue->sideFaderLevel();
            obj["primaryTop"] = cue->primaryTop();
            obj["nextStepIndex"] = cue->nextStepIndex();
            obj["playbackStatus"] = int(cue->playbackStatus());
            obj["playbackIndex"] = cue->playbackIndex();

            QJsonArray steps;
            QVariant listVar = cue->stepsList();
            ListModel *model = qobject_cast<ListModel*>(listVar.value<QObject*>());
            if (model != nullptr)
            {
                for (int i = 0; i < model->rowCount(); i++)
                {
                    QVariant itemVar = model->itemAt(i);
                    if (itemVar.canConvert<QVariantMap>())
                    {
                        QVariantMap map = itemVar.toMap();
                        quint32 funcId = map.value("funcID").toUInt();
                        Function *func = m_doc->function(funcId);
                        if (func != nullptr)
                        {
                            map.insert("funcName", func->name());
                            map.insert("funcType", func->typeString());
                        }
                        QJsonObject stepObj = QJsonObject::fromVariantMap(map);
                        steps.append(stepObj);
                    }
                }
            }
            obj["steps"] = steps;
        }
        break;
        case VCWidget::AudioTriggersWidget:
        {
            const VCAudioTriggers *triggers = qobject_cast<const VCAudioTriggers*>(widget);
            obj["enabled"] = triggers->captureEnabled();
            obj["volume"] = triggers->volumeLevel();
            obj["bars"] = triggers->barsNumber();
        }
        break;
        case VCWidget::ClockWidget:
        {
            const VCClock *clock = qobject_cast<const VCClock*>(widget);
            obj["clockType"] = int(clock->clockType());
            obj["currentTime"] = clock->currentTime();
            obj["targetTime"] = clock->targetTime();
        }
        break;
        case VCWidget::AnimationWidget:
        {
            const VCAnimation *animation = qobject_cast<const VCAnimation*>(widget);
            obj["visibilityMask"] = int(animation->visibilityMask());
            obj["functionId"] = int(animation->functionID());
            obj["faderLevel"] = animation->faderLevel();
            obj["instantChanges"] = animation->instantChanges();
            obj["color1"] = colorToString(animation->getColor1());
            obj["color2"] = colorToString(animation->getColor2());
            obj["color3"] = colorToString(animation->getColor3());
            obj["color4"] = colorToString(animation->getColor4());
            obj["color5"] = colorToString(animation->getColor5());
            obj["algorithmIndex"] = animation->algorithmIndex();
            QJsonArray algos;
            for (const QString &name : animation->algorithms())
                algos.append(name);
            obj["algorithms"] = algos;
        }
        break;
        case VCWidget::SpeedWidget:
        {
            const VCSpeedDial *dial = qobject_cast<const VCSpeedDial*>(widget);
            obj["visibilityMask"] = int(dial->visibilityMask());
            obj["timeMin"] = int(dial->timeMinimumValue());
            obj["timeMax"] = int(dial->timeMaximumValue());
            obj["currentTime"] = int(dial->currentTime());
            obj["resetOnDialChange"] = dial->resetOnDialChange();
            obj["currentFactor"] = int(dial->currentFactor());
            obj["presetsList"] = QJsonArray::fromVariantList(dial->presetsList());
        }
        break;
        default:
            break;
    }

    return obj;
}

void WebAccessQml::collectWidgets(const VCFrame *frame, QList<VCWidget *> &list, bool recursive) const
{
    if (frame == nullptr)
        return;

    QList<VCWidget *> children = frame->children(false);
    for (VCWidget *child : children)
    {
        list.append(child);
        if (recursive)
        {
            VCFrame *childFrame = qobject_cast<VCFrame *>(child);
            if (childFrame != nullptr)
                collectWidgets(childFrame, list, recursive);
        }
    }
}

QByteArray WebAccessQml::getVCJson()
{
    QJsonObject root;
    root["version"] = 1;
    QJsonObject appObj;
    appObj["name"] = QString(APPNAME);
    appObj["version"] = QString(APPVERSION);
    root["app"] = appObj;
    root["pixelDensity"] = m_vc->pixelDensity();
    root["selectedPage"] = m_vc->selectedPage();
    QJsonObject uiStyle = loadUiStyleJson();
    if (uiStyle.isEmpty() == false)
        root["uiStyle"] = uiStyle;

    QJsonArray pages;
    for (int i = 0; i < m_vc->pagesCount(); i++)
    {
        VCFrame *page = m_vc->page(i);
        QJsonObject pageObj = frameToJson(page);
        pageObj["index"] = i;
        pages.append(pageObj);
    }
    root["pages"] = pages;

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

void WebAccessQml::setupWidgetConnections(const VCWidget *widget)
{
    if (widget == nullptr)
        return;

    if (m_connectedWidgets.contains(widget->id()))
        return;

    m_connectedWidgets.insert(widget->id());

    switch (widget->type())
    {
        case VCWidget::ButtonWidget:
        {
            const VCButton *button = qobject_cast<const VCButton*>(widget);
            connect(button, SIGNAL(stateChanged(int)),
                    this, SLOT(slotButtonStateChanged(int)));
            connect(button, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotButtonDisableStateChanged(bool)));
        }
        break;
        case VCWidget::LabelWidget:
        {
            const VCLabel *label = qobject_cast<const VCLabel*>(widget);
            connect(label, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotLabelDisableStateChanged(bool)));
        }
        break;
        case VCWidget::SliderWidget:
        {
            const VCSlider *slider = qobject_cast<const VCSlider*>(widget);
            connect(slider, SIGNAL(valueChanged(int)),
                    this, SLOT(slotSliderValueChanged(int)));
            connect(slider, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotSliderDisableStateChanged(bool)));
            connect(slider, SIGNAL(isOverridingChanged()),
                    this, SLOT(slotSliderOverrideChanged()));
        }
        break;
        case VCWidget::AudioTriggersWidget:
        {
            const VCAudioTriggers *triggers = qobject_cast<const VCAudioTriggers*>(widget);
            connect(triggers, SIGNAL(captureEnabledChanged()),
                    this, SLOT(slotAudioTriggersToggled()));
            connect(triggers, SIGNAL(volumeLevelChanged()),
                    this, SLOT(slotAudioTriggersVolumeChanged()));
            connect(triggers, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotWidgetDisableStateChanged(bool)));
        }
        break;
        case VCWidget::CueListWidget:
        {
            const VCCueList *cue = qobject_cast<const VCCueList*>(widget);
            connect(cue, SIGNAL(playbackIndexChanged(int)),
                    this, SLOT(slotCueIndexChanged(int)));
            connect(cue, SIGNAL(playbackStatusChanged()),
                    this, SLOT(slotCuePlaybackStateChanged()));
            connect(cue, SIGNAL(sideFaderLevelChanged()),
                    this, SLOT(slotCueSideFaderLevelChanged()));
            connect(cue, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotCueDisableStateChanged(bool)));
        }
        break;
        case VCWidget::FrameWidget:
        case VCWidget::SoloFrameWidget:
        {
            const VCFrame *frame = qobject_cast<const VCFrame*>(widget);
            connect(frame, SIGNAL(currentPageChanged(int)),
                    this, SLOT(slotFramePageChanged(int)));
            connect(frame, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotFrameDisableStateChanged(bool)));

            QList<VCWidget *> children = frame->children(false);
            for (VCWidget *child : children)
                setupWidgetConnections(child);
        }
        break;
        case VCWidget::AnimationWidget:
        {
            const VCAnimation *animation = qobject_cast<const VCAnimation*>(widget);
            connect(animation, SIGNAL(faderLevelChanged()),
                    this, SLOT(slotMatrixFaderChanged()));
            connect(animation, SIGNAL(color1Changed()),
                    this, SLOT(slotMatrixColorsChanged()));
            connect(animation, SIGNAL(color2Changed()),
                    this, SLOT(slotMatrixColorsChanged()));
            connect(animation, SIGNAL(color3Changed()),
                    this, SLOT(slotMatrixColorsChanged()));
            connect(animation, SIGNAL(color4Changed()),
                    this, SLOT(slotMatrixColorsChanged()));
            connect(animation, SIGNAL(color5Changed()),
                    this, SLOT(slotMatrixColorsChanged()));
            connect(animation, SIGNAL(algorithmIndexChanged()),
                    this, SLOT(slotMatrixAlgorithmChanged()));
            connect(animation, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotWidgetDisableStateChanged(bool)));
        }
        break;
        case VCWidget::XYPadWidget:
        {
            const VCXYPad *xypad = qobject_cast<const VCXYPad*>(widget);
            connect(xypad, SIGNAL(currentPositionChanged()),
                    this, SLOT(slotXYPadPositionChanged()));
            connect(xypad, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotWidgetDisableStateChanged(bool)));
        }
        break;
        case VCWidget::SpeedWidget:
        {
            const VCSpeedDial *dial = qobject_cast<const VCSpeedDial*>(widget);
            connect(dial, SIGNAL(currentTimeChanged()),
                    this, SLOT(slotSpeedDialTimeChanged()));
            connect(dial, SIGNAL(currentFactorChanged()),
                    this, SLOT(slotSpeedDialFactorChanged()));
            connect(dial, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotWidgetDisableStateChanged(bool)));
        }
        break;
        case VCWidget::ClockWidget:
        {
            const VCClock *clock = qobject_cast<const VCClock*>(widget);
            connect(clock, SIGNAL(currentTimeChanged(int)),
                    this, SLOT(slotClockTimeChanged(int)));
            connect(clock, SIGNAL(disabledStateChanged(bool)),
                    this, SLOT(slotWidgetDisableStateChanged(bool)));
        }
        break;
        default:
            break;
    }
}

void WebAccessQml::slotButtonStateChanged(int state)
{
    VCButton *btn = qobject_cast<VCButton *>(sender());
    if (btn == nullptr)
        return;

    QString wsMessage = QString("%1|BUTTON|%2").arg(btn->id()).arg(state == VCButton::Active ? 255 :
                                                                  state == VCButton::Monitoring ? 127 : 0);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotButtonDisableStateChanged(bool disable)
{
    VCButton *btn = qobject_cast<VCButton *>(sender());
    if (btn == nullptr)
        return;

    QString wsMessage = QString("%1|BUTTON_DISABLE|%2").arg(btn->id()).arg(disable);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotLabelDisableStateChanged(bool disable)
{
    VCLabel *lbl = qobject_cast<VCLabel *>(sender());
    if (lbl == nullptr)
        return;

    QString wsMessage = QString("%1|LABEL_DISABLE|%2").arg(lbl->id()).arg(disable);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotSliderValueChanged(int value)
{
    VCSlider *slider = qobject_cast<VCSlider *>(sender());
    if (slider == nullptr)
        return;

    QString wsMessage = QString("%1|SLIDER|%2|%3").arg(slider->id()).arg(value).arg(sliderDisplayValue(slider));
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotSliderDisableStateChanged(bool disable)
{
    VCSlider *slider = qobject_cast<VCSlider *>(sender());
    if (slider == nullptr)
        return;

    QString wsMessage = QString("%1|SLIDER_DISABLE|%2").arg(slider->id()).arg(disable);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotSliderOverrideChanged()
{
    VCSlider *slider = qobject_cast<VCSlider *>(sender());
    if (slider == nullptr)
        return;

    QString wsMessage = QString("%1|SLIDER_OVERRIDE|%2").arg(slider->id()).arg(slider->isOverriding());
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotAudioTriggersToggled()
{
    VCAudioTriggers *triggers = qobject_cast<VCAudioTriggers *>(sender());
    if (triggers == nullptr)
        return;

    QString wsMessage = QString("%1|AUDIOTRIGGERS|%2").arg(triggers->id()).arg(triggers->captureEnabled());
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotAudioTriggersVolumeChanged()
{
    VCAudioTriggers *triggers = qobject_cast<VCAudioTriggers *>(sender());
    if (triggers == nullptr)
        return;

    QString wsMessage = QString("%1|AUDIO_VOLUME|%2").arg(triggers->id()).arg(triggers->volumeLevel());
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotWidgetDisableStateChanged(bool disable)
{
    VCWidget *widget = qobject_cast<VCWidget *>(sender());
    if (widget == nullptr)
        return;

    QString wsMessage = QString("%1|WIDGET_DISABLE|%2").arg(widget->id()).arg(disable);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotCueIndexChanged(int idx)
{
    VCCueList *cue = qobject_cast<VCCueList *>(sender());
    if (cue == nullptr)
        return;

    QString wsMessage = QString("%1|CUE|%2").arg(cue->id()).arg(idx);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotCuePlaybackStateChanged()
{
    VCCueList *cue = qobject_cast<VCCueList *>(sender());
    if (cue == nullptr)
        return;

    QString wsMessage = QString("%1|CUE_STATE|%2|%3|%4|%5|%6")
            .arg(cue->id())
            .arg(int(cue->playbackStatus()))
            .arg(cue->playbackIndex())
            .arg(cue->nextStepIndex())
            .arg(cue->sideFaderLevel())
            .arg(cue->primaryTop());
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotCueSideFaderLevelChanged()
{
    VCCueList *cue = qobject_cast<VCCueList *>(sender());
    if (cue == nullptr)
        return;

    QString wsMessage = QString("%1|CUE_SIDE|%2|%3").arg(cue->id()).arg(cue->sideFaderLevel()).arg(cue->primaryTop());
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotCueDisableStateChanged(bool disable)
{
    VCCueList *cue = qobject_cast<VCCueList *>(sender());
    if (cue == nullptr)
        return;

    QString wsMessage = QString("%1|CUE_DISABLE|%2").arg(cue->id()).arg(disable);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotFramePageChanged(int pageNum)
{
    VCFrame *frame = qobject_cast<VCFrame *>(sender());
    if (frame == nullptr)
        return;

    QString wsMessage = QString("%1|FRAME|%2").arg(frame->id()).arg(pageNum);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotFrameDisableStateChanged(bool disable)
{
    VCFrame *frame = qobject_cast<VCFrame *>(sender());
    if (frame == nullptr)
        return;

    QString wsMessage = QString("%1|FRAME_DISABLE|%2").arg(frame->id()).arg(disable);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotMatrixFaderChanged()
{
    VCAnimation *animation = qobject_cast<VCAnimation *>(sender());
    if (animation == nullptr)
        return;

    sendMatrixState(animation);
}

void WebAccessQml::slotMatrixColorsChanged()
{
    VCAnimation *animation = qobject_cast<VCAnimation *>(sender());
    if (animation == nullptr)
        return;

    sendMatrixState(animation);
}

void WebAccessQml::slotMatrixAlgorithmChanged()
{
    VCAnimation *animation = qobject_cast<VCAnimation *>(sender());
    if (animation == nullptr)
        return;

    sendMatrixState(animation);
}

void WebAccessQml::slotXYPadPositionChanged()
{
    VCXYPad *xypad = qobject_cast<VCXYPad *>(sender());
    if (xypad == nullptr)
        return;

    QPointF pos = xypad->currentPosition();
    QString wsMessage = QString("%1|XYPAD|%2|%3").arg(xypad->id()).arg(pos.x()).arg(pos.y());
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotSpeedDialTimeChanged()
{
    VCSpeedDial *dial = qobject_cast<VCSpeedDial *>(sender());
    if (dial == nullptr)
        return;

    QString wsMessage = QString("%1|SPEED_STATE|%2|%3")
            .arg(dial->id())
            .arg(dial->currentTime())
            .arg(int(dial->currentFactor()));
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotSpeedDialFactorChanged()
{
    VCSpeedDial *dial = qobject_cast<VCSpeedDial *>(sender());
    if (dial == nullptr)
        return;

    QString wsMessage = QString("%1|SPEED_STATE|%2|%3")
            .arg(dial->id())
            .arg(dial->currentTime())
            .arg(int(dial->currentFactor()));
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotClockTimeChanged(int time)
{
    VCClock *clock = qobject_cast<VCClock *>(sender());
    if (clock == nullptr)
        return;
    if (isWidgetVisibleForWeb(clock, m_vc) == false)
        return;

    QString wsMessage = QString("%1|CLOCK|%2").arg(clock->id()).arg(time);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::slotGrandMasterValueChanged(uchar value)
{
    int p = qFloor(((double(value) / double(UCHAR_MAX)) * double(100)) + 0.5);
    QString gmDisplayValue = QString("%1%").arg(p, 2, 10, QChar('0'));
    QString wsMessage = QString("GM_VALUE|%1|%2").arg(value).arg(gmDisplayValue);
    sendWebSocketMessage(wsMessage);
}

void WebAccessQml::handleAutostartProject(const QString &path)
{
    emit storeAutostartProject(path);
}
