/*
  Q Light Controller Plus - Unit test
  resource_paths.h

  Copyright (c) Jano Svitok

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

#ifndef RESOURCE_PATHS_H
#define RESOURCE_PATHS_H

#ifdef USE_CTEST
    #define INTERNAL_FIXTUREDIR "../../resources/fixtures/"
    #define INTERNAL_PROFILEDIR "../../resources/inputprofiles/"
    #define INTERNAL_SCRIPTDIR "../../resources/rgbscripts/"
#else
    #define INTERNAL_FIXTUREDIR "../../../resources/fixtures/"
    #define INTERNAL_PROFILEDIR "../../../resources/inputprofiles/"
    #define INTERNAL_SCRIPTDIR "../../../resources/rgbscripts/"
#endif

#endif
