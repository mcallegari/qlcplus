/*
  Q Light Controller Plus
  webaccessauth.h

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

#ifndef WEBACCESSAUTH_H
#define WEBACCESSAUTH_H

#include <QString>
#include <QMap>
#include <QList>

class QHttpRequest;
class QHttpResponse;

enum WebAccessUserLevel
{
    GUEST = 0,
    LOGGED_IN = 1,
    VC_ONLY = 10,
    SIMPLE_DESK_AND_VC = 20,
    SUPER_ADMIN = 100,
    NOT_PROVIDED = 100,
};

struct WebAccessUser
{
    QString username;
    QString passwordHash;
    WebAccessUserLevel level;

    WebAccessUser(
        const QString& _username,
        const QString& _passwordHash,
        WebAccessUserLevel _level
    )
    : username(_username)
    , passwordHash(_passwordHash)
    , level(_level)
    {}

    WebAccessUser()
    : username()
    , passwordHash()
    , level(WebAccessUserLevel::GUEST)
    {}

};

/**
 * This class implements HTTP basic authentication scheme
 * as defined in RFC7617
 */
class WebAccessAuth
{
private:
    QMap<QString, WebAccessUser> m_passwords;
    QString m_realm;
    QString m_passwordsFile;
public:
    WebAccessAuth(const QString& realm);

    /**
     * Loads file with entries in form:
     *     username:passwordHash[:userLevel]
     * where:
     *   - passwordHash is SHA256 hash of user's passsword
     *   - userLevel is an integer
     * 
     * Note: duplicate usernames will be silently skipped
     *       (the last entry in file will be used)
     */
    bool loadPasswordsFile(const QString& filePath);

    /**
     * Saves current contents of password table into
     * file provided in latest invokation of loadPasswordsFile
     */
    bool savePasswordsFile() const;

    /**
     * Note: This function has to be called before any
     *       content is sent, because it adds some headers
     */
    WebAccessUser authenticateRequest(const QHttpRequest* req, QHttpResponse* res) const;

    /**
     * Send HTTP 403 response
     *
     * Note: This function ends the response
     */
    void sendUnauthorizedResponse(QHttpResponse* res) const;

    /**
     * Adds user to password table. If given username already
     * exists __it is replaced__.
     */
    void addUser(const QString& username, const QString& password, WebAccessUserLevel level);

    /**
     * Removes user from password table if it exists.
     */
    void deleteUser(const QString& username);
    
    QList<WebAccessUser> getUsers() const;

private:
    QString hashPassword(const QString& password) const;
    bool hasAtLeastOneAdmin() const;
};

#endif // WEBACCESSAUTH_H
