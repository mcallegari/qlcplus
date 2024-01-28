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
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QCryptographicHash>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif

#include "webaccessauth.h"
#include "qlcconfig.h"

#include "qhttprequest.h"
#include "qhttpresponse.h"

#define SALT_LENGTH           32
#define DEFAULT_PASSWORD_FILE "web_passwd"

WebAccessAuth::WebAccessAuth(const QString& realm)
    : m_passwords()
    , m_realm(realm)
{
    m_passwordsFile = QString("%1/%2/%3").arg(getenv("HOME")).arg(USERQLCPLUSDIR).arg(DEFAULT_PASSWORD_FILE);
}

bool WebAccessAuth::loadPasswordsFile(const QString& filePath)
{
    if (!filePath.isEmpty())
        m_passwordsFile = filePath;

    QFile file(m_passwordsFile);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    QString line;

    // Line format is as follows:
    // username:passwordHash:userLevel:hashType:salt
    // where everything after passwordHash is optional
    while (!(line = stream.readLine()).isNull())
    {
        QStringList parts = line.split(':');

        if (parts.size() < 2)
        {
            qDebug() << "Skipping invalid line '" << line << "'";
            continue;
        }

        QString username = parts[0];
        QString passwordHash = parts[1];
        int userLevel = (parts.size() >= 3) ? (parts[2].toInt()) : (NOT_PROVIDED_LEVEL);
        QString hashType = (parts.size() >= 4) ? (parts[3]) : (DEFAULT_PASSWORD_HASH_TYPE);
        QString salt = (parts.size() >= 5) ? (parts[4]) : ("");

        WebAccessUser user(
            username,
            passwordHash,
            (WebAccessUserLevel)userLevel,
            hashType,
            salt
        );

        // Silently overrides duplicate usernames due to unique keys in maps
        this->m_passwords.insert(username, user);
    }

    return true;
}

bool WebAccessAuth::savePasswordsFile() const
{
    if (m_passwordsFile.isEmpty())
        return false;

    QFile file(m_passwordsFile);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);

    foreach (QString username, m_passwords.keys())
    {
        WebAccessUser user = m_passwords.value(username);
        stream
            << user.username << ':'
            << user.passwordHash << ':'
            << (int)user.level << ':'
            << user.hashType << ':'
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            << user.passwordSalt << Qt::endl;
#else
            << user.passwordSalt << endl;
#endif
    }

    return true;
}

WebAccessUser WebAccessAuth::authenticateRequest(const QHttpRequest* req, QHttpResponse* res) const
{
    // Disable authentication when no administrative accounts are proviced
    if (!this->hasAtLeastOneAdmin())
        return WebAccessUser(NOT_PROVIDED_LEVEL);

    QString header = QString("Basic realm=\"") + m_realm + QString("\"");
    res->setHeader("WWW-Authenticate", header);

    QString auth = req->header("Authorization");
    // Tell the browser that authorization is required
    if (!auth.startsWith("Basic "))
        return WebAccessUser();

    QString authentication = QString(QByteArray::fromBase64(auth.right(auth.size() - 6).toUtf8()));
    int colonIndex = authentication.indexOf(':');

    // Disallow empty passwords
    if (colonIndex == -1)
        return WebAccessUser();

    QString username = authentication.left(colonIndex);
    QString password = authentication.mid(colonIndex + 1);

    QMap<QString, WebAccessUser>::const_iterator userIterator = m_passwords.find(username);
    if (userIterator == m_passwords.end() || ! this->verifyPassword(password, *userIterator))
        return WebAccessUser();

    return *userIterator;
}

void WebAccessAuth::addUser(const QString& username, const QString& password, WebAccessUserLevel level)
{
    QString salt = this->generateSalt();
    WebAccessUser user(
        username,
        this->hashPassword(DEFAULT_PASSWORD_HASH_TYPE, password, salt),
        level,
        DEFAULT_PASSWORD_HASH_TYPE,
        salt
    );
    m_passwords.insert(username, user);
}

bool WebAccessAuth::setUserLevel(const QString& username, WebAccessUserLevel level)
{
    QMap<QString, WebAccessUser>::iterator userIt = m_passwords.find(username);
    if (userIt == m_passwords.end())
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
    //TODO: Allow for localization of this
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


QString WebAccessAuth::generateSalt() const
{
    QString salt;

    for (int i = 0; i < SALT_LENGTH; i++)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
        int halfByte = qrand() % 16;
#else
        int halfByte = QRandomGenerator::global()->generate() % 16;
#endif
        salt.append(QString::number(halfByte, 16));
    }

    return salt;
}

QString WebAccessAuth::hashPassword(const QString& hashType, const QString& password, const QString& passwordSalt) const
{
    QString passwordWithSalt = password + passwordSalt;
    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha1;

    if (hashType == "sha1")
    {
        algorithm = QCryptographicHash::Sha1;
    }
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
    else if (hashType == "md5")
    {
        algorithm = QCryptographicHash::Md5;
    }
    else if (hashType == "sha256")
    {
        algorithm = QCryptographicHash::Sha256;
    }
#endif // QT_CRYPTOGRAPHICHASH_ONLY_SHA1
    else
    {
        qDebug() << "Unknown password hash algorithm " << hashType << ", defaulting to sha1.";
        algorithm = QCryptographicHash::Sha1;
    }

    return QCryptographicHash::hash(passwordWithSalt.toUtf8(), algorithm).toHex();
}

bool WebAccessAuth::verifyPassword(const QString& password, const WebAccessUser& user) const
{
    return this->hashPassword(user.hashType, password, user.passwordSalt) == user.passwordHash;
}

bool WebAccessAuth::hasAtLeastOneAdmin() const
{
    foreach (WebAccessUser user, m_passwords.values())
    {
        if (user.level >= SUPER_ADMIN_LEVEL)
            return true;
    }

    return false;
}
