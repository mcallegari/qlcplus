/*
  Q Light Controller Plus
  webaccessauth.cpp

  Copyright (c) Bartosz Grabias

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
#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QTextStream>

#include "webaccessauth.h"

#include "qhttprequest.h"
#include "qhttpresponse.h"

#define PASSWORD_HASH_ALGORITHM QCryptographicHash::Algorithm::Sha256

WebAccessAuth::WebAccessAuth(const QString& realm)
    : m_passwords()
    , m_realm(realm)
{
}

bool WebAccessAuth::loadPasswordsFile(const QString& filePath)
{
    m_passwordsFile = filePath;

    QFile file(filePath);
    
    if(! file.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::Text))
        return false;
    
    QTextStream stream(&file);
    QString line;

    while(! (line = stream.readLine()).isNull()) 
    {
        auto parts = line.split(':');
        
        if(parts.size() < 2)
        {
            qDebug() << "Skipping invalid line '" << line << "'";
            continue;
        }

        auto username = parts[0];
        auto passwordHash = parts[1];
        int userLevel = (parts.size() >= 3) ? (parts[2].toInt()) : (WebAccessUserLevel::NOT_PROVIDED);

        WebAccessUser user(username, passwordHash, (WebAccessUserLevel)userLevel);

        // Silently overrides duplicate usernames due to unique keys in maps
        this->m_passwords.insert(username, user);
    }

    return true;
}

bool WebAccessAuth::savePasswordsFile() const
{
    if(m_passwordsFile.isEmpty())
        return false;
    
    QFile file(m_passwordsFile);
    
    if(! file.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::Text))
        return false;
    
    QTextStream stream(&file);
    
    for(auto& username : m_passwords.keys())
    {
        auto user = m_passwords.value(username);
        stream << user.username << ':' << user.passwordHash << ':' << (int)user.level << endl;
    }

    return true;
}

WebAccessUser WebAccessAuth::authenticateRequest(const QHttpRequest* req, QHttpResponse* res) const
{
    // Disable authentication when no administrative accounts are proviced
    if(! this->hasAtLeastOneAdmin())
        return WebAccessUser("", "", WebAccessUserLevel::NOT_PROVIDED);
    
    QString header = QString("Basic realm=\"") + m_realm + QString("\"");
    res->setHeader("WWW-Authenticate", header);

    QString auth = req->header("Authorization");
    // Tell the browser that authorization is required
    if(! auth.startsWith("Basic "))
    {
        return WebAccessUser();
    }

    auto authentication = QString(QByteArray::fromBase64(auth.right(auth.size() - 6).toUtf8()));
    int colonIndex = authentication.indexOf(':');
    
    // Disallow empty passwords
    if(colonIndex == -1)
    {
        return WebAccessUser();
    }

    auto username = authentication.left(colonIndex);
    auto passwordHash = this->hashPassword(authentication.mid(colonIndex + 1));

    auto userIterator = m_passwords.find(username);
    if(userIterator == m_passwords.end() || (*userIterator).passwordHash != passwordHash)
    {
        return WebAccessUser();
    }

    return *userIterator;
}

void WebAccessAuth::addUser(const QString& username, const QString& password, WebAccessUserLevel level)
{
    WebAccessUser user(username, this->hashPassword(password), level);
    m_passwords.insert(username, user);
}

bool WebAccessAuth::setUserLevel(const QString& username, WebAccessUserLevel level)
{
    auto userIt = m_passwords.find(username);
    if(userIt == m_passwords.end())
        return false;
    
    (*userIt).level = level;
    m_passwords.insert(username, *userIt);
    return true;
}

void WebAccessAuth::deleteUser(const QString& username)
{
    m_passwords.remove(username);
}

QList<WebAccessUser> WebAccessAuth::getUsers() const
{
    return m_passwords.values();
}

void WebAccessAuth::sendUnauthorizedResponse(QHttpResponse* res) const
{
    const static QByteArray text = QString(
        "<html>"
            "<head>"
                "<meta charset=\"utf-8\">"
                "<title>Unauthorized</title>"
            "</head>"
            "<body>"
                "<h1>401 Unauthorized</h1>"
                "<p>Access to this resource requires proper authorization"
                " and you have failed to authenticate.</p>"
            "</body>"
        "</html>"
    ).toUtf8();

    res->setHeader("Content-Type", "text/html");
    res->setHeader("Content-Length", QString::number(text.size()));
    res->writeHead(401);
    res->end(text);
}

QString WebAccessAuth::hashPassword(const QString& password) const
{
    return QCryptographicHash::hash(password.toUtf8(), PASSWORD_HASH_ALGORITHM).toHex();
}

bool WebAccessAuth::hasAtLeastOneAdmin() const
{
    for(auto& user : m_passwords)
    {
        if(user.level >= WebAccessUserLevel::SUPER_ADMIN)
            return true;
    }

    return false;
}