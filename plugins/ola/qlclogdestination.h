/*
  Q Light Controller
  qlclogdestination.h

  Copyright (c) Heikki Junnila
                Simon Newton

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    void Write(log_level level, const string &log_line);
private:
    static const string PREFIX;
};

} // ola
#endif
