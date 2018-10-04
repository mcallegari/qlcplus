/*
  Q Light Controller Plus
  utility.js

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      https://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

// Replaces internal "qrc:/*.png" image URIs with web-friendly URIs
function replaceqrc()
{
    var imgs = document.images;
    for (var i = 0; i < imgs.length; i++)
    {
        var src = imgs[i].src;
        imgs[i].src = src.replace("qrc:", "../../icons/png");
    }
}

