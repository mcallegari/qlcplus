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

#define DEFAULT_PASSWORD_HASH_TYPE "sha256"

class QHttpRequest;
class QHttpResponse;

enum WebAccessUserLevel
{
    GUEST_LEVEL = 0,
    LOGGED_IN_LEVEL = 1,
    VC_ONLY_LEVEL = 10,
    SIMPLE_DESK_AND_VC_LEVEL = 20,
    SUPER_ADMIN_LEVEL = 100,
    NOT_PROVIDED_LEVEL = 100,
};

struct WebAccessUser
{
    QString username;
    QString passwordHash;
    WebAccessUserLevel level;
    QString hashType;
    QString passwordSalt;

    WebAccessUser(
        const QString& _username,
        const QString& _passwordHash,
        WebAccessUserLevel _level,
        const QString& _hashType,
        const QString& _passwordSalt
    )
    : username(_username)
    , passwordHash(_passwordHash)
    , level(_level)
    , hashType(_hashType)
    , passwordSalt(_passwordSalt)
    {}

    WebAccessUser(
        WebAccessUserLevel _level
    )
    : username("")
    , passwordHash("")
    , level(_level)
    , hashType(DEFAULT_PASSWORD_HASH_TYPE)
    , passwordSalt("")
    {}

    WebAccessUser()
    : username()
    , passwordHash()
    , level(GUEST_LEVEL)
    , hashType(DEFAULT_PASSWORD_HASH_TYPE)
    , passwordSalt("")
    {}

};

/**
 * This class implements HTTP basic authentication scheme
 * as defined in RFC7617
 */
class WebAccessAuth
{

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
     * Sets user access level
     *
     * Returns true when user with given username exists
     */
    bool setUserLevel(const QString& username, WebAccessUserLevel level);

    /**
     * Removes user from password table if it exists.
     */
    void deleteUser(const QString& username);

    QList<WebAccessUser> getUsers() const;

private:
    QString generateSalt() const;
    QString hashPassword(const QString& hashType, const QString& password, const QString& passwordSalt) const;
    bool verifyPassword(const QString& password, const WebAccessUser& user) const;
    bool hasAtLeastOneAdmin() const;

private:
    QMap<QString, WebAccessUser> m_passwords;
    QString m_realm;
    QString m_passwordsFile;
};

#endif // WEBACCESSAUTH_H
