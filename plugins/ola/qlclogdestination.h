/*
  Q Light Controller
  qlclogdestination.h

  Copyright (c) Heikki Junnila
                Simon Newton

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

#ifndef QLCLOGDESTINATION_H
#define QLCLOGDESTINATION_H

#include <string>
#include <ola/Logging.h>

namespace ola {

// A LogDestination that uses the q{Debug,Warning,Critical} functions
class QLCLogDestination : public LogDestination
{
public:
    void Write(log_level level, const std::string &log_line);
private:
    static const std::string PREFIX;
};

} // ola
#endif
