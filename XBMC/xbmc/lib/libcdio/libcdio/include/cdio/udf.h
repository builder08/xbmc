/*  
    $Id: udf.h,v 1.21 2006/04/16 02:34:10 rocky Exp $
    Copyright (C) 2005, 2006 Rocky Bernstein <rocky@panix.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*!
 * \file udf.h 
 *
 * \brief The top-level interface header for libudf: UDF filesystem
 * library; applications include this.
 *
*/

#ifndef UDF_H
#define UDF_H 

#include <cdio/cdio.h>
#include <cdio/ecma_167.h>
#include <cdio/posix.h>

typedef uint16_t partition_num_t;

/** Opaque structures. */
typedef struct udf_s udf_t; 
typedef struct udf_file_s udf_file_t;
typedef struct udf_dirent_s udf_dirent_t;

/**
   Imagine the below a \#define'd value rather than distinct values of
   an enum.
*/
typedef enum {
  UDF_BLOCKSIZE       = 2048
} udf_enum1_t; 

/** This variable is trickery to force the above enum symbol value to
    be recorded in debug symbol tables. It is used to allow one refer
    to above enumeration values in a debugger and debugger
    expressions */
extern udf_enum1_t debug_udf_enum1;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /*!
    Close UDF and free resources associated with p_udf.
  */
  bool udf_close (udf_t *p_udf);
  
  /*!  
    Seek to a position i_start and then read i_blocks. Number of
    blocks read is returned. One normally expects the return to be
    equal to i_blocks.
  */

  driver_return_code_t udf_read_sectors (const udf_t *p_udf, void *ptr, 
					 lsn_t i_start,  long int i_blocks);

  /*!
    Open an UDF for reading. Maybe in the future we will have
    a mode. NULL is returned on error.
    
    Caller must free result - use udf_close for that.
  */
  udf_t *udf_open (const char *psz_path);
  
  /*!
    Return the partition number of the the opened udf handle. -1 
    Is returned if we have an error.
  */
  int16_t udf_get_part_number(const udf_t *p_udf);

  /*!
    Get the root in p_udf. If b_any_partition is false then
    the root must be in the given partition.
    NULL is returned if the partition is not found or a root is not found or
    there is on error.

    Caller must free result - use udf_file_free for that.
  */
  udf_dirent_t *udf_get_root (udf_t *p_udf, bool b_any_partition, 
			      partition_num_t i_partition);
  
  /**
   * Gets the Volume Identifier string, in 8bit unicode (latin-1)
   * psz_volid, place to put the string
   * i_volid_size, size of the buffer volid points to
   * returns the size of buffer needed for all data
   */
  int udf_get_volume_id(udf_t *p_udf, /*out*/ char *psz_volid,  
			unsigned int i_volid);
  
  /**
   * Gets the Volume Set Identifier, as a 128-byte dstring (not decoded)
   * WARNING This is not a null terminated string
   * volsetid, place to put the data
   * volsetid_size, size of the buffer volsetid points to 
   * the buffer should be >=128 bytes to store the whole volumesetidentifier
   * returns the size of the available volsetid information (128)
   * or 0 on error
   */
  int udf_get_volumeset_id(udf_t *p_udf, /*out*/ uint8_t *volsetid,
			   unsigned int i_volsetid);
  
  /*!
    Return a file pointer matching pzz_name. 
  */
  udf_dirent_t *udf_fopen(udf_dirent_t *p_udf_root, const char *psz_name);
  
  /*! udf_mode_string - fill in string PSZ_STR with an ls-style ASCII
    representation of the i_mode. PSZ_STR is returned.

    10 characters are stored in PSZ_STR; a terminating null byte is added.
    The characters stored in PSZ_STR are:
    
    0	File type.  'd' for directory, 'c' for character
	special, 'b' for block special, 'm' for multiplex,
	'l' for symbolic link, 's' for socket, 'p' for fifo,
	'-' for regular, '?' for any other file type

    1	'r' if the owner may read, '-' otherwise.

    2	'w' if the owner may write, '-' otherwise.

    3	'x' if the owner may execute, 's' if the file is
	set-user-id, '-' otherwise.
	'S' if the file is set-user-id, but the execute
	bit isn't set.

    4	'r' if group members may read, '-' otherwise.

    5	'w' if group members may write, '-' otherwise.

    6	'x' if group members may execute, 's' if the file is
	set-group-id, '-' otherwise.
	'S' if it is set-group-id but not executable.

    7	'r' if any user may read, '-' otherwise.

    8	'w' if any user may write, '-' otherwise.

    9	'x' if any user may execute, 't' if the file is "sticky"
	(will be retained in swap space after execution), '-'
	otherwise.
	'T' if the file is sticky but not executable.  */

  char *udf_mode_string (mode_t i_mode, char *psz_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#include <cdio/udf_time.h>
#include <cdio/udf_file.h>

#endif /*UDF_H*/
