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
        int colonIndex = line.indexOf(':');

        if(colonIndex == -1)
        {
            qDebug() << "Skipping invalid line '" << line << "'";
            continue;
        }
        
        auto username = line.left(colonIndex);
        auto passwordHash = line.mid(colonIndex + 1);

        // Silently overrides duplicate usernames due to unique keys in maps
        this->m_passwords.insert(username, passwordHash);
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
        stream << username << ':' << m_passwords.value(username) << endl;
    }

    return true;
}

bool WebAccessAuth::authenticateRequest(const QHttpRequest* req, QHttpResponse* res) const
{
    // Disable authentication when no passwords were added
    if(this->m_passwords.empty())
        return true;
    
    QString header = QString("Basic realm=\"") + m_realm + QString("\"");
    res->setHeader("WWW-Authenticate", header);

    QString auth = req->header("Authorization");
    // Tell the browser that authorization is required
    if(! auth.startsWith("Basic "))
    {
        this->sendUnauthorizedResponse(res);
        return false;
    }

    auto authentication = QString(QByteArray::fromBase64(auth.right(auth.size() - 6).toUtf8()));
    int colonIndex = authentication.indexOf(':');
    
    // Disallow empty passwords
    if(colonIndex == -1)
    {
        this->sendUnauthorizedResponse(res);
        return false;
    }

    auto username = authentication.left(colonIndex);
    auto passwordHash = this->hashPassword(authentication.mid(colonIndex + 1));

    auto passwordHashIterator = m_passwords.find(username);
    if(passwordHashIterator == m_passwords.end() || *passwordHashIterator != passwordHash)
    {
        this->sendUnauthorizedResponse(res);
        return false;
    }

    return true;
}

void WebAccessAuth::addUser(const QString& username, const QString& password)
{
    m_passwords.insert(username, this->hashPassword(password));
}

void WebAccessAuth::deleteUser(const QString& username)
{
    m_passwords.remove(username);
}

QList<QString> WebAccessAuth::getUsernames() const
{
    return m_passwords.keys();
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
                "and you have failed to authenticate.</p>"
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