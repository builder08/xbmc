/* test_libFLAC - Unit tester for libFLAC
 * Copyright (C) 2002,2003,2004,2005,2006,2007  Josh Coalson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef FLAC__TEST_LIBFLAC_FILE_UTILS_H
#define FLAC__TEST_LIBFLAC_FILE_UTILS_H

/* needed because of off_t */
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "FLAC/format.h"
#include <sys/types.h> /* for off_t */

extern const long file_utils__ogg_serial_number;

FLAC__bool file_utils__generate_flacfile(FLAC__bool is_ogg, const char *output_filename, off_t *output_filesize, unsigned length, const FLAC__StreamMetadata *streaminfo, FLAC__StreamMetadata **metadata, unsigned num_metadata);

#endif
