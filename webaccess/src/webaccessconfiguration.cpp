/*
  Q Light Controller Plus
  webaccessconfiguration.h

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

#include <QSettings>

#include "webaccessconfiguration.h"
#include "webaccessauth.h"
#include "audioplugincache.h"
#include "inputoutputmap.h"
#include "commonjscss.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "doc.h"

WebAccessConfiguration::WebAccessConfiguration()
{
}

QString WebAccessConfiguration::getIOConfigHTML(Doc *doc)
{
    QString html = "";
    InputOutputMap *ioMap = doc->inputOutputMap();

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

    for (quint32 i = 0; i < ioMap->universesCount(); i++)
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

QString WebAccessConfiguration::getAudioConfigHTML(Doc *doc)
{
    QString html = "";
    QList<AudioDeviceInfo> devList = doc->audioPluginCache()->audioDevicesList();

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

    foreach (AudioDeviceInfo info, devList)
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

QString WebAccessConfiguration::getUserFixturesConfigHTML()
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

QString WebAccessConfiguration::getPasswordsConfigHTML(WebAccessAuth *auth)
{
    QString html = "";
    html += "<table class=\"hovertable\" id=\"auth-passwords-table\" style=\"width: 100%;\">";
    html +=
        "<tr>"
            "<th>" + tr("Username") + "</th>"
            "<th>" + tr("Password") + "</th>"
            "<th>" + tr("Access level") + "</th>"
            "<th>" + tr("Action") + "</th>"
        "</tr>";

    foreach (WebAccessUser user, auth->getUsers())
    {
        QString username = user.username;
        int level = user.level;

        html += "<tr id=\"auth-row-" + username + "\">";
        html += "<td>" + username + "</td>";
        html += "<td><input type=\"password\" id=\"auth-password-" + username + "\""
                " placeholder=\"" + tr("Leave blank to not change") + "\"></td>";
        html += "<td>";
            html += "<select id=\"auth-level-" + username + "\">";

            html += "<option value=\"" + QString::number(VC_ONLY_LEVEL) + "\"";
            if (level >= VC_ONLY_LEVEL && level < SIMPLE_DESK_AND_VC_LEVEL)
                html += "selected";
            html += ">" + tr("Only Virtual Console") + "</option>";

            html += "<option value=\"" + QString::number(SIMPLE_DESK_AND_VC_LEVEL) + "\"";
            if (level >= SIMPLE_DESK_AND_VC_LEVEL && level < SUPER_ADMIN_LEVEL)
                html += "selected";
            html += ">" + tr("Virtual Console and Simple Desk") + "</option>";

            html += "<option value=\"" + QString::number(SUPER_ADMIN_LEVEL) + "\"";
            if (level >= SUPER_ADMIN_LEVEL)
                html += "selected";
            html += ">" + tr("Everything") + "</option>";

            html += "</select>";
        html += "</td>";
        html += "<td>";
            html += "<button role=\"button\" onclick=\"authChangeUser('" + username + "')\">"
                 + tr("Change") + "</button>";
            html += "<button role=\"button\" onclick=\"authDeleteUser('" + username + "')\">"
                 + tr("Delete user") + "</button>";
        html += "</td>";
        html += "</tr>";
    }

    html += "<tr>";
    html += "<td><input type=\"text\" id=\"auth-new-username\" placeholder=\"" + tr("New username...") + "\"></td>";
    html += "<td><input type=\"password\" id=\"auth-new-password\" placeholder=\"" + tr("New password...") + "\"></td>";
    html += "<td>";
        html += "<select id=\"auth-new-level\">";

        html += "<option value=\"" + QString::number(VC_ONLY_LEVEL) + "\">"
            + tr("Only Virtual Console") + "</option>";
        html += "<option value=\"" + QString::number(SIMPLE_DESK_AND_VC_LEVEL) + "\">"
            + tr("Virtual Console and Simple Desk") + "</option>";
        html += "<option value=\"" + QString::number(SUPER_ADMIN_LEVEL) + "\">"
            + tr("Everything") + "</option>";

        html += "</select>";
    html += "</td>";
    html += "<td>";
        // Script will dynamically add rows with users so it needs to know translations
        html += "<button role=\"button\" onclick=\"authAddUser("
                "'" + tr("Change") + "','" + tr("Delete user") + "'"
                ",'" + tr("Username and password are required fields.") + "'"
                ",'" + tr("New password...") + "'"
                ")\" class=\"authAddUser\">"
                + tr("Add user") + "</button>";
    html += "</td>";
    html += "</tr>";
    html += "</table>";
    html += "<i style=\"color: #CCCCCC\">";
    html += tr("Note: if there isn't at least one user with access level \"Everything\" on the list"
               " authorization will be disabled.");
    html += "</i>";

    return html;
}

QString WebAccessConfiguration::getHTML(Doc *doc, WebAccessAuth *auth)
{
    QString m_JScode = "<script type=\"text/javascript\" src=\"websocket.js\"></script>\n";
    m_JScode += "<script type=\"text/javascript\" src=\"configuration.js\"></script>\n";

    QString m_CSScode =
                 "<style type=\"text/css\" media=\"screen\">\n"
                 "html { height: 100%; background-color: #222; }\n"
                 "body {\n"
                 " margin: 0px;\n"
                 " background: #222;\n"
                 "}\n\n"
                 "form {\n"
                 "position: absolute;\n"
                 "top: -100px;\n"
                 "visibility: hidden;\n"
                 "}\n"
                 "</style>\n"
                 "<link href=\"common.css\" rel=\"stylesheet\" type=\"text/css\" media=\"screen\">\n";

    QString extraButtons = "";
    if (QLCFile::hasWindowManager() == false)
    {
        extraButtons = "<a class=\"button button-blue\" href=\"/system\"><span>" + tr("System") + "</span></a>\n";
    }

    QString bodyHTML = "<form action=\"/loadFixture\" method=\"POST\" enctype=\"multipart/form-data\">\n"
                       "<input id=\"loadTrigger\" type=\"file\" "
                       "onchange=\"document.getElementById('submitTrigger').click();\" name=\"qlcfxi\">\n"
                       "<input id=\"submitTrigger\" type=\"submit\"></form>"

                       "<div class=\"controlBar\">\n"
                       "<a class=\"button button-blue\" href=\"/\"><span>" + tr("Back") + "</span></a>\n" +
                       extraButtons +
                       "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>"
                       "</div>\n";

    // ********************* IO mapping ***********************
    bodyHTML += "<div style=\"margin: 30px 7% 30px 7%; width: 86%;\" >\n";
    bodyHTML += "<div style=\"font-family: verdana,arial,sans-serif; font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("Universes configuration") + "</div><br>\n";
    bodyHTML += getIOConfigHTML(doc);
    bodyHTML += "</div>";

    // ********************* audio devices ********************
    bodyHTML += "<div style=\"margin: 30px 7% 30px 7%;\" >\n";
    bodyHTML += "<div style=\"font-family: verdana,arial,sans-serif; font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("Audio configuration") + "</div><br>\n";
    bodyHTML += getAudioConfigHTML(doc);
    bodyHTML += "</div>";

    // **************** User loaded fixtures ******************

    bodyHTML += "<div style=\"margin: 30px 7% 30px 7%;\" >\n";
    bodyHTML += "<div style=\"font-family: verdana,arial,sans-serif; font-size:20px; text-align:center; color:#CCCCCC;\">";
    bodyHTML += tr("User loaded fixtures") + "</div><br>\n";
    bodyHTML += getUserFixturesConfigHTML();
    bodyHTML += "</div>";

    // ******************* User management ********************
    if (auth)
    {
        bodyHTML += "<div style=\"margin: 30px 7% 30px 7%;\" >\n";
        bodyHTML += "<div style=\"font-family: verdana,arial,sans-serif; font-size:20px; text-align:center; color:#CCCCCC;\">";
        bodyHTML += tr("Authorized users") + "</div><br>\n";
        bodyHTML += getPasswordsConfigHTML(auth);
        bodyHTML += "</div>";
    }

    QString str = HTML_HEADER + m_JScode + m_CSScode + "</head>\n<body>\n" + bodyHTML + "</body>\n</html>";

    return str;
}
