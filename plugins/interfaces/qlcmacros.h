/*
  Q Light Controller
  qlcmacros.h

  Copyright (C) Heikki Junnila

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

#ifndef QLCMACROS_H
#define QLCMACROS_H

/*****************************************************************************
 * Utils
 *****************************************************************************/

/** Win32 needs these in order to be able to link to dynamic libraries */
#ifdef QLC_EXPORT
#  define QLC_DECLSPEC Q_DECL_EXPORT
#else
#  define QLC_DECLSPEC Q_DECL_IMPORT
#endif

#ifdef CLAMP
#undef CLAMP
#endif
/**
 * Ensure that x is between the limits set by low and high.
 * If low is greater than high the result is undefined.
 *
 * This is copied from GLib sources
 *
 * @param x The value to clamp
 * @param low The minimum allowed value
 * @param high The maximum allowed value
 * @return The value of x clamped to the range between low and high
 */
#define CLAMP(x, low, high) \
    (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifdef MAX
#undef MAX
#endif
/**
 * Return the bigger value of the two given values
 *
 * @param x The first value to compare
 * @param y The second value to compare
 * @return The bigger one of the given values
 */
#define MAX(x, y) ((x < y) ? y : x)

#ifdef MIN
#undef MIN
#endif
/**
 * Return the smaller value of the two given values
 *
 * @param x The first value to compare
 * @param y The second value to compare
 * @return The smaller one of the given values
 */
#define MIN(x, y) ((x < y) ? x : y)

#ifdef SCALE
#undef SCALE
#endif
/**
 * Scale a value within a source range to an equal value within the
 * destination range. I.e. 5 on a scale of 0 - 10 would be 10 on a scale
 * of 0 - 20.
 */
#define SCALE(x, src_min, src_max, dest_min, dest_max) \
      dest_min + ((x - src_min) * ((dest_max - dest_min) / (src_max - src_min)))


#define MS_PER_SECOND (1000)                //! Milliseconds in a second
#define MS_PER_MINUTE (60 * MS_PER_SECOND)  //! Milliseconds in a minute
#define MS_PER_HOUR   (60 * MS_PER_MINUTE)  //! Milliseconds in an hour

#endif
