/*
  Q Light Controller Plus
  configuration.js

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

var websocket;

function ioChanged(cmd, uni, val)
{
  websocket.send("QLC+IO|" + cmd + "|" + uni + "|" + val);
}

function authChangeUser(username)
{
  var newPasswordElement = document.getElementById("auth-password-" + username);
  var newLevelElement = document.getElementById("auth-level-" + username);

  if(newPasswordElement.value) {
    websocket.send("QLC+AUTH|ADD_USER|" + username + "|" + newPasswordElement.value + "|" + newLevelElement.value);
  } else {
    websocket.send("QLC+AUTH|SET_USER_LEVEL|" + username + "|" + newLevelElement.value);
  }

  newPasswordElement.value = "";
}

function authDeleteUser(username)
{
  var passwordRow = document.getElementById("auth-row-" + username);
  if(confirm("Do you really want to remove \"" + username + "\" from list?")) {
    websocket.send("QLC+AUTH|DEL_USER|" + username);
    passwordRow.parentNode.removeChild(passwordRow);
  }
}

function authAddUser(trChangeUser, trDeleteUser, trFieldsRequired, trNewPasswordPlaceholder)
{
  var usernameElement = document.getElementById("auth-new-username");
  var passwordElement = document.getElementById("auth-new-password");
  var levelElement = document.getElementById("auth-new-level");
  var username = usernameElement.value;
  var level = levelElement.value;

  if(! username || ! passwordElement.value) {
    return alert(trFieldsRequired);
  }

  websocket.send("QLC+AUTH|ADD_USER|" + username + "|" + passwordElement.value + "|" + level);

  var tableElement = document.getElementById("auth-passwords-table");
  var rowCount = tableElement.rows.length;
  var row = tableElement.insertRow(rowCount - 1);
  row.id = "auth-row-" + username;

  var usernameCell = row.insertCell(0);
  var passwordCell = row.insertCell(1);
  var levelCell = row.insertCell(2);
  var actionsCell  = row.insertCell(3);

  usernameCell.innerText = username;

  var passwordInput = document.createElement("input");
  passwordInput.type = "password";
  passwordInput.id = "auth-password-" + username;
  passwordInput.placeholder = trNewPasswordPlaceholder;

  passwordCell.appendChild(passwordInput);

  var levelInput = document.createElement("select");
  levelInput.id = "auth-level-" + username;
  var levels = levelElement.getElementsByTagName("option");
  for(var l of levels) {
    var option = document.createElement("option");
    option.value = l.value;
    option.innerText = l.innerText;
    option.selected = (l.value === level);

    levelInput.appendChild(option);
  }

  levelCell.appendChild(levelInput);

  var changeUserButton = document.createElement("button");
  changeUserButton.onclick = function() { authChangeUser(username); };
  changeUserButton.innerText = trChangeUser;

  actionsCell.appendChild(changeUserButton);

  var deleteUserButton = document.createElement("button");
  deleteUserButton.onclick = function() { authDeleteUser(username); };
  deleteUserButton.innerText = trDeleteUser;

  actionsCell.appendChild(deleteUserButton);

  // Reset new user form
  usernameElement.value = "";
  passwordElement.value = "";
}
