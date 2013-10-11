/*
  Q Light Controller
  qlcmacros.h

  Copyright (C) Heikki Junnila

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
