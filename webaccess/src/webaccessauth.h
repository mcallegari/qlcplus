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

class QHttpRequest;
class QHttpResponse;

/**
 * This class implements HTTP basic authentication scheme
 * as defined in RFC7617
 */
class WebAccessAuth
{
private:
    QMap<QString, QString> m_passwords;
    QString m_realm;
    QString m_passwordsFile;
public:
    WebAccessAuth(const QString& realm);

    /**
     * Loads file with entries in form:
     *     username:passwordHash
     * where passwordHash is SHA256 hash of user's passsword
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
     * 
     * Note: if this function returns false the request
     *       is served by this function
     */
    bool authenticateRequest(const QHttpRequest* req, QHttpResponse* res) const;

    /**
     * Adds user to password table. If given username already
     * exists __it is replaced__.
     */
    void addUser(const QString& username, const QString& password);

    /**
     * Removes user from password table if it exists.
     */
    void deleteUser(const QString& username);

private:
    void sendUnauthorizedResponse(QHttpResponse* res) const;
    QString hashPassword(const QString& password) const;
};

#endif // WEBACCESSAUTH_H
