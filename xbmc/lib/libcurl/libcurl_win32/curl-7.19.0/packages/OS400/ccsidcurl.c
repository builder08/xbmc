/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2008, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: ccsidcurl.c,v 1.7 2008-07-07 10:39:46 patrickm Exp $
 *
 ***************************************************************************/

/* CCSID API wrappers for OS/400. */

#include <iconv.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#pragma enum(int)

#include <curl/curl.h>
#include "urldata.h"
#include "url.h"
#include "getinfo.h"
#include "ccsidcurl.h"

#include "os400sys.h"

#ifndef SIZE_MAX
#define SIZE_MAX        ((size_t) ~0)   /* Is unsigned on OS/400. */
#endif


#define ASCII_CCSID     819     /* Use ISO-8859-1 as ASCII. */
#define NOCONV_CCSID    65535   /* No conversion. */
#define ICONV_ID_SIZE   32      /* Size of iconv_open() code identifier. */
#define ICONV_OPEN_ERROR(t)     ((t).return_value == -1)

#define ALLOC_GRANULE   8       /* Alloc. granule for curl_formadd_ccsid(). */


static void
makeOS400IconvCode(char buf[ICONV_ID_SIZE], unsigned int ccsid)

{
  /**
  *** Convert a CCSID to the corresponding IBM iconv_open() character
  ***  code identifier.
  ***  This code is specific to the OS400 implementation of the iconv library.
  ***  CCSID 65535 (no conversion) is replaced by the ASCII CCSID.
  ***  CCSID 0 is interpreted by the OS400 as the job's CCSID.
  **/

  ccsid &= 0xFFFF;

  if (ccsid == NOCONV_CCSID)
    ccsid = ASCII_CCSID;

  memset(buf, 0, ICONV_ID_SIZE);
  curl_msprintf(buf, "IBMCCSID%05u0000000", ccsid);
}


static iconv_t
iconv_open_CCSID(unsigned int ccsidout, unsigned int ccsidin, unsigned int cstr)

{
  char fromcode[ICONV_ID_SIZE];
  char tocode[ICONV_ID_SIZE];

  /**
  ***  Like iconv_open(), but character codes are given as CCSIDs.
  ***  If `cstr' is non-zero, conversion is set up to stop whenever a
  ***   null character is encountered.
  ***  See iconv_open() IBM description in "National Language Support API".
  **/

  makeOS400IconvCode(fromcode, ccsidin);
  makeOS400IconvCode(tocode, ccsidout);
  memset(tocode + 13, 0, sizeof tocode - 13);   /* Dest. code id format. */

  if (cstr)
    fromcode[18] = '1';                         /* Set null-terminator flag. */

  return iconv_open(tocode, fromcode);
}


static int
convert(char * d, size_t dlen, int dccsid,
        const char * s, int slen, int sccsid)

{
  int i;
  iconv_t cd;
  size_t lslen;

  /**
  ***  Convert `sccsid'-coded `slen'-data bytes at `s' into `dccsid'-coded
  ***   data stored in the `dlen'-byte buffer at `d'.
  ***  If `slen' < 0, source string is null-terminated.
  ***  CCSID 65535 (no conversion) is replaced by the ASCII CCSID.
  ***  Return the converted destination byte count, or -1 if error.
  **/

  if (sccsid == 65535)
    sccsid = ASCII_CCSID;

  if (dccsid == 65535)
    dccsid = ASCII_CCSID;

  if (sccsid == dccsid) {
    lslen = slen >= 0? slen: strlen(s) + 1;
    i = lslen < dlen? lslen: dlen;

    if (s != d && i > 0)
      memcpy(d, s, i);

    return i;
    }

  if (slen < 0) {
    lslen = 0;
    cd = iconv_open_CCSID(dccsid, sccsid, 1);
    }
  else {
    lslen = (size_t) slen;
    cd = iconv_open_CCSID(dccsid, sccsid, 0);
    }

  if (ICONV_OPEN_ERROR(cd))
    return -1;

  i = dlen;

  if ((int) iconv(cd, (char * *) &s, &lslen, &d, &dlen) < 0)
    i = -1;
  else
    i -= dlen;

  iconv_close(cd);
  return i;
}


static char *
dynconvert(int dccsid, const char * s, int slen, int sccsid)

{
  char * d;
  char * cp;
  size_t dlen;
  int l;
  int l2;
  static const char nullbyte = 0;

  /* Like convert, but the destination is allocated and returned. */

  dlen = (size_t) (slen < 0? strlen(s): slen) + 1;
  dlen *= MAX_CONV_EXPANSION;           /* Allow some expansion. */
  d = malloc(dlen);

  if (!d)
    return (char *) NULL;

  l = convert(d, dlen, dccsid, s, slen, sccsid);

  if (l < 0) {
    free(d);
    return (char *) NULL;
    }

  if (slen < 0) {
    /* Need to null-terminate even when source length is given.
       Since destination code size is unknown, use a conversion to generate
       terminator. */

    l2 = convert(d + l, dlen - l, dccsid, &nullbyte, -1, ASCII_CCSID);

    if (l2 < 0) {
      free(d);
      return (char *) NULL;
      }

    l += l2;
    }

  if ((size_t) l < dlen) {
    cp = realloc(d, l);         /* Shorten to minimum needed. */

    if (cp)
      d = cp;
    }

  return d;
}


char *
curl_version_ccsid(unsigned int ccsid)

{
  int i;
  char * aversion;
  char * eversion;

  aversion = curl_version();

  if (!aversion)
    return aversion;

  i = strlen(aversion) + 1;
  i *= MAX_CONV_EXPANSION;

  if (!(eversion = Curl_thread_buffer(LK_CURL_VERSION, i)))
    return (char *) NULL;

  if (convert(eversion, i, ccsid, aversion, -1, ASCII_CCSID) < 0)
    return (char *) NULL;

  return eversion;
}


char *
curl_easy_escape_ccsid(CURL * handle, const char * string, int length,
                       unsigned int sccsid, unsigned int dccsid)

{
  char * s;
  char * d;

  if (!string) {
    errno = EINVAL;
    return (char *) NULL;
    }

  s = dynconvert(ASCII_CCSID, s, length? length: -1, sccsid);

  if (!s)
    return (char *) NULL;

  d = curl_easy_escape(handle, s, 0);
  free(s);

  if (!d)
    return (char *) NULL;

  s = dynconvert(dccsid, d, -1, ASCII_CCSID);
  free(d);
  return s;
}


char *
curl_easy_unescape_ccsid(CURL * handle, const char * string, int length,
                         int * outlength,
                         unsigned int sccsid, unsigned int dccsid)

{
  char * s;
  char * d;

  if (!string) {
    errno = EINVAL;
    return (char *) NULL;
    }

  s = dynconvert(ASCII_CCSID, s, length? length: -1, sccsid);

  if (!s)
    return (char *) NULL;

  d = curl_easy_unescape(handle, s, 0, outlength);
  free(s);

  if (!d)
    return (char *) NULL;

  s = dynconvert(dccsid, d, -1, ASCII_CCSID);
  free(d);

  if (s && outlength)
    *outlength = strlen(s);

  return s;
}


struct curl_slist *
curl_slist_append_ccsid(struct curl_slist * list,
                        const char * data, unsigned int ccsid)

{
  char * s;

  s = (char *) NULL;

  if (!data)
    return curl_slist_append(list, data);

  s = dynconvert(ASCII_CCSID, data, -1, ccsid);

  if (!s)
    return (struct curl_slist *) NULL;

  list = curl_slist_append(list, s);
  free(s);
  return list;
}


time_t
curl_getdate_ccsid(const char * p, const time_t * unused, unsigned int ccsid)

{
  char * s;
  time_t t;

  if (!p)
    return curl_getdate(p, unused);

  s = dynconvert(ASCII_CCSID, p, -1, ccsid);

  if (!s)
    return (time_t) -1;

  t = curl_getdate(s, unused);
  free(s);
  return t;
}


static int
convert_version_info_string(const char * * stringp,
                            char * * bufp, int * left, unsigned int ccsid)

{
  int l;

  /* Helper for curl_version_info_ccsid(): convert a string if defined.
     Result is stored in the `*left'-byte buffer at `*bufp'.
     `*bufp' and `*left' are updated accordingly.
     Return 0 if ok, else -1. */

  if (*stringp) {
    l = convert(*bufp, *left, ccsid, *stringp, -1, ASCII_CCSID);

    if (l <= 0)
      return -1;

    *stringp = *bufp;
    *bufp += l;
    *left -= l;
    }

  return 0;
}


curl_version_info_data *
curl_version_info_ccsid(CURLversion stamp, unsigned int ccsid)

{
  curl_version_info_data * p;
  char * cp;
  int n;
  int nproto;
  int i;
  curl_version_info_data * id;

  /* The assertion below is possible, because although the second operand
     is an enum member, the first is a #define. In that case, the OS/400 C
     compiler seems to compare string values after substitution. */

#if CURLVERSION_NOW != CURLVERSION_FOURTH
#error curl_version_info_data structure has changed: upgrade this procedure too.
#endif

  /* If caller has been compiled with a new version, error. */

  if (stamp > CURLVERSION_NOW)
    return (curl_version_info_data *) NULL;

  p = curl_version_info(stamp);

  if (!p)
    return p;

  /* Measure thread space needed. */

  n = 0;
  nproto = 0;

  if (p->protocols) {
    while (p->protocols[nproto])
      n += strlen(p->protocols[nproto++]);

    n += nproto++;
    }

  if (p->version)
    n += strlen(p->version) + 1;

  if (p->host)
    n += strlen(p->host) + 1;

  if (p->ssl_version)
    n += strlen(p->ssl_version) + 1;

  if (p->libz_version)
    n += strlen(p->libz_version) + 1;

  if (p->ares)
    n += strlen(p->ares) + 1;

  if (p->libidn)
    n += strlen(p->libidn) + 1;

  if (p->libssh_version)
    n += strlen(p->libssh_version) + 1;

  /* Allocate thread space. */

  n *= MAX_CONV_EXPANSION;

  if (nproto)
    n += nproto * sizeof(const char *);

  cp = Curl_thread_buffer(LK_VERSION_INFO_DATA, n);
  id = (curl_version_info_data *) Curl_thread_buffer(LK_VERSION_INFO,
                                                     sizeof *id);

  if (!id || !cp)
    return (curl_version_info_data *) NULL;

  /* Copy data and convert strings. */

  memcpy((char *) id, (char *) p, sizeof *p);

  if (id->protocols) {
    id->protocols = (const char * const *) cp;
    i = nproto * sizeof id->protocols[0];
    memcpy(cp, (char *) p->protocols, i);
    cp += i;
    n -= i;

    for (i = 0; id->protocols[i]; i++)
      if (convert_version_info_string(((const char * *) id->protocols) + i,
                                      &cp, &n, ccsid))
        return (curl_version_info_data *) NULL;
    }

  if (convert_version_info_string(&id->version, &cp, &n, ccsid))
    return (curl_version_info_data *) NULL;

  if (convert_version_info_string(&id->host, &cp, &n, ccsid))
    return (curl_version_info_data *) NULL;

  if (convert_version_info_string(&id->ssl_version, &cp, &n, ccsid))
    return (curl_version_info_data *) NULL;

  if (convert_version_info_string(&id->libz_version, &cp, &n, ccsid))
    return (curl_version_info_data *) NULL;

  if (convert_version_info_string(&id->ares, &cp, &n, ccsid))
    return (curl_version_info_data *) NULL;

  if (convert_version_info_string(&id->libidn, &cp, &n, ccsid))
    return (curl_version_info_data *) NULL;

  if (convert_version_info_string(&id->libssh_version, &cp, &n, ccsid))
    return (curl_version_info_data *) NULL;

  return id;
}


const char *
curl_easy_strerror_ccsid(CURLcode error, unsigned int ccsid)

{
  int i;
  const char * s;
  char * buf;

  s = curl_easy_strerror(error);

  if (!s)
    return s;

  i = MAX_CONV_EXPANSION * (strlen(s) + 1);

  if (!(buf = Curl_thread_buffer(LK_EASY_STRERROR, i)))
    return (const char *) NULL;

  if (convert(buf, i, ccsid, s, -1, ASCII_CCSID) < 0)
    return (const char *) NULL;

  return (const char *) buf;
}


const char *
curl_share_strerror_ccsid(CURLSHcode error, unsigned int ccsid)

{
  int i;
  const char * s;
  char * buf;

  s = curl_share_strerror(error);

  if (!s)
    return s;

  i = MAX_CONV_EXPANSION * (strlen(s) + 1);

  if (!(buf = Curl_thread_buffer(LK_SHARE_STRERROR, i)))
    return (const char *) NULL;

  if (convert(buf, i, ccsid, s, -1, ASCII_CCSID) < 0)
    return (const char *) NULL;

  return (const char *) buf;
}


const char *
curl_multi_strerror_ccsid(CURLMcode error, unsigned int ccsid)

{
  int i;
  const char * s;
  char * buf;

  s = curl_multi_strerror(error);

  if (!s)
    return s;

  i = MAX_CONV_EXPANSION * (strlen(s) + 1);

  if (!(buf = Curl_thread_buffer(LK_MULTI_STRERROR, i)))
    return (const char *) NULL;

  if (convert(buf, i, ccsid, s, -1, ASCII_CCSID) < 0)
    return (const char *) NULL;

  return (const char *) buf;
}


CURLcode
curl_easy_getinfo_ccsid(CURL * curl, CURLINFO info, ...)

{
  va_list arg;
  void * paramp;
  CURLcode ret;
  unsigned int ccsid;
  char * * cpp;
  char * s;
  char * d;
  struct SessionHandle * data;

  /* WARNING: unlike curl_easy_get_info(), the strings returned by this
     procedure have to be free'ed. */

  data = (struct SessionHandle *) curl;
  va_start(arg, info);
  paramp = va_arg(arg, void *);
  ret = Curl_getinfo(data, info, paramp);

  if (ret != CURLE_OK || ((int) info & CURLINFO_TYPEMASK) != CURLINFO_STRING) {
    va_end(arg);
    return ret;
    }

  ccsid = va_arg(arg, unsigned int);
  va_end(arg);
  cpp = (char * *) paramp;
  s = *cpp;

  if (!s)
    return ret;

  d = dynconvert(ccsid, s, -1, ASCII_CCSID);
  *cpp = d;

  if (!d)
    return CURLE_OUT_OF_MEMORY;

  return ret;
}


static int
Curl_is_formadd_string(CURLformoption option)

{
  switch (option) {

    case CURLFORM_FILENAME:
    case CURLFORM_CONTENTTYPE:
    case CURLFORM_BUFFER:
    case CURLFORM_FILE:
    case CURLFORM_FILECONTENT:
    case CURLFORM_COPYCONTENTS:
    case CURLFORM_COPYNAME:
      return 1;
    }

  return 0;
}


static void
Curl_formadd_release_local(struct curl_forms * forms, int nargs, int skip)

{
  while (nargs--)
    if (nargs != skip)
      if (Curl_is_formadd_string(forms[nargs].option))
        if (forms[nargs].value)
          free((char *) forms[nargs].value);

  free((char *) forms);
}


static int
Curl_formadd_convert(struct curl_forms * forms,
                     int formx, int lengthx, unsigned int ccsid)

{
  int l;
  char * cp;
  char * cp2;

  if (formx < 0 || !forms[formx].value)
    return 0;

  if (lengthx >= 0)
    l = (int) forms[lengthx].value;
  else
    l = strlen(forms[formx].value) + 1;

  cp = malloc(MAX_CONV_EXPANSION * l);

  if (!cp)
    return -1;

  l = convert(cp, MAX_CONV_EXPANSION * l, ASCII_CCSID,
              forms[formx].value, l, ccsid);

  if (l < 0) {
    free(cp);
    return -1;
    }

  cp2 = realloc(cp, l);                 /* Shorten buffer to the string size. */

  if (cp2)
    cp = cp2;

  forms[formx].value = cp;

  if (lengthx >= 0)
    forms[lengthx].value = (char *) l;  /* Update to length after conversion. */

  return l;
}


CURLFORMcode
curl_formadd_ccsid(struct curl_httppost * * httppost,
                   struct curl_httppost * * last_post, ...)

{
  va_list arg;
  CURLformoption option;
  CURLFORMcode result;
  struct curl_forms * forms;
  struct curl_forms * lforms;
  struct curl_forms * tforms;
  unsigned int lformlen;
  const char * value;
  unsigned int ccsid;
  int nargs;
  int namex;
  int namelengthx;
  int contentx;
  int lengthx;
  unsigned int contentccsid;
  unsigned int nameccsid;

  /* A single curl_formadd() call cannot be splitted in several calls to deal
     with all parameters: the original parameters are thus copied to a local
     curl_forms array and converted to ASCII when needed.
     CURLFORM_PTRNAME is processed as if it were CURLFORM_COPYNAME.
     CURLFORM_COPYNAME and CURLFORM_NAMELENGTH occurrence order in
     parameters is not defined; for this reason, the actual conversion is
     delayed to the end of parameter processing. The same applies to
     CURLFORM_COPYCONTENTS/CURLFORM_CONTENTSLENGTH, but these may appear
     several times in the parameter list; the problem resides here in knowing
     which CURLFORM_CONTENTSLENGTH applies to which CURLFORM_COPYCONTENTS and
     when we can be sure to have both info for conversion: end of parameter
     list is such a point, but CURLFORM_CONTENTTYPE is also used here as a
     natural separator between content data definitions; this seems to be
     in accordance with FormAdd() behavior. */

  /* Allocate the local curl_forms array. */

  lformlen = ALLOC_GRANULE;
  lforms = (struct curl_forms *) malloc(lformlen * sizeof * lforms);

  if (!lforms)
    return CURL_FORMADD_MEMORY;

  /* Process the arguments, copying them into local array, latching conversion
     indexes and converting when needed. */

  result = CURL_FORMADD_OK;
  nargs = 0;
  contentx = -1;
  lengthx = -1;
  namex = -1;
  namelengthx = -1;
  forms = (struct curl_forms *) NULL;
  va_start(arg, last_post);

  for (;;) {
    /* Make sure there is still room for an item in local array. */

    if (nargs >= lformlen) {
      lformlen += ALLOC_GRANULE;
      tforms = (struct curl_forms *) realloc((char *) lforms,
                                             lformlen * sizeof *lforms);

      if (!tforms) {
        result = CURL_FORMADD_MEMORY;
        break;
        }

      lforms = tforms;
      }

    /* Get next option. */

    if (forms) {
      /* Get option from array. */

      option = forms->option;
      value = forms->value;
      forms++;
      }
    else {
      /* Get option from arguments. */

      option = va_arg(arg, CURLformoption);

      if (option == CURLFORM_END)
        break;
      }

    /* Dispatch by option. */

    switch (option) {

    case CURLFORM_END:
      forms = (struct curl_forms *) NULL;       /* Leave array mode. */
      continue;

    case CURLFORM_ARRAY:
      if (!forms) {
        forms = va_arg(arg, struct curl_forms *);
        continue;
        }

      result = CURL_FORMADD_ILLEGAL_ARRAY;
      break;

    case CURLFORM_COPYNAME:
      option = CURLFORM_PTRNAME;                /* Static for now. */

    case CURLFORM_PTRNAME:
      if (namex >= 0)
        result = CURL_FORMADD_OPTION_TWICE;

      namex = nargs;

      if (!forms) {
        value = va_arg(arg, char *);
        nameccsid = (unsigned int) va_arg(arg, long);
        }
      else {
        nameccsid = (unsigned int) forms->value;
        forms++;
        }

      break;

    case CURLFORM_COPYCONTENTS:
      if (contentx >= 0)
        result = CURL_FORMADD_OPTION_TWICE;

      contentx = nargs;

      if (!forms) {
        value = va_arg(arg, char *);
        contentccsid = (unsigned int) va_arg(arg, long);
        }
      else {
        contentccsid = (unsigned int) forms->value;
        forms++;
        }

      break;

    case CURLFORM_PTRCONTENTS:
    case CURLFORM_BUFFERPTR:
      if (!forms)
        value = va_arg(arg, char *);            /* No conversion. */

      break;

    case CURLFORM_CONTENTSLENGTH:
      lengthx = nargs;

      if (!forms)
        value = (char *) va_arg(arg, long);

      break;

    case CURLFORM_NAMELENGTH:
      namelengthx = nargs;

      if (!forms)
        value = (char *) va_arg(arg, long);

      break;

    case CURLFORM_BUFFERLENGTH:
      if (!forms)
        value = (char *) va_arg(arg, long);

      break;

    case CURLFORM_CONTENTHEADER:
      if (!forms)
        value = (char *) va_arg(arg, struct curl_slist *);

      break;

    case CURLFORM_STREAM:
      if (!forms)
        value = (char *) va_arg(arg, void *);

      break;

    case CURLFORM_CONTENTTYPE:
      /* If a previous content has been encountered, convert it now. */

      if (Curl_formadd_convert(lforms, contentx, lengthx, contentccsid) < 0) {
        result = CURL_FORMADD_MEMORY;
        break;
        }

      contentx = -1;
      lengthx = -1;
      /* Fall into default. */

    default:
      /* Must be a convertible string. */

      if (!Curl_is_formadd_string(option)) {
        result = CURL_FORMADD_UNKNOWN_OPTION;
        break;
        }

      if (!forms) {
        value = va_arg(arg, char *);
        ccsid = (unsigned int) va_arg(arg, long);
        }
      else {
        ccsid = (unsigned int) forms->value;
        forms++;
        }

      /* Do the conversion. */

      lforms[nargs].value = value;

      if (Curl_formadd_convert(lforms, nargs, -1, ccsid) < 0) {
        result = CURL_FORMADD_MEMORY;
        break;
        }

      value = lforms[nargs].value;
      }

    if (result != CURL_FORMADD_OK)
      break;

    lforms[nargs].value = value;
    lforms[nargs++].option = option;
    }

  va_end(arg);

  /* Convert the name and the last content, now that we know their lengths. */

  if (result == CURL_FORMADD_OK && namex >= 0) {
    if (Curl_formadd_convert(lforms, namex, namelengthx, nameccsid) < 0)
      result = CURL_FORMADD_MEMORY;
    else
      lforms[namex].option = CURLFORM_COPYNAME;         /* Force copy. */
    }

  if (result == CURL_FORMADD_OK) {
    if (Curl_formadd_convert(lforms, contentx, lengthx, contentccsid) < 0)
      result = CURL_FORMADD_MEMORY;
    else
      contentx = -1;
    }

  /* Do the formadd with our converted parameters. */

  if (result == CURL_FORMADD_OK) {
    lforms[nargs].option = CURLFORM_END;
    result = curl_formadd(httppost, last_post,
                          CURLFORM_ARRAY, lforms, CURLFORM_END);
    }

  /* Terminate. */

  Curl_formadd_release_local(lforms, nargs, contentx);
  return result;
}


typedef struct {
  curl_formget_callback append;
  void *                arg;
  unsigned int          ccsid;
}   cfcdata;


static size_t
Curl_formget_callback_ccsid(void * arg, const char * buf, size_t len)

{
  cfcdata * p;
  char * b;
  int l;
  size_t ret;

  p = (cfcdata *) arg;

  if ((long) len <= 0)
    return (*p->append)(p->arg, buf, len);

  b = malloc(MAX_CONV_EXPANSION * len);

  if (!b)
    return (size_t) -1;

  l = convert(b, MAX_CONV_EXPANSION * len, p->ccsid, buf, len, ASCII_CCSID);

  if (l < 0) {
    free(b);
    return (size_t) -1;
    }

  ret = (*p->append)(p->arg, b, l);
  free(b);
  return ret == l? len: -1;
}


int
curl_formget_ccsid(struct curl_httppost * form, void * arg,
                   curl_formget_callback append, unsigned int ccsid)

{
  cfcdata lcfc;

  lcfc.append = append;
  lcfc.arg = arg;
  lcfc.ccsid = ccsid;
  return curl_formget(form, (void *) &lcfc, Curl_formget_callback_ccsid);
}


CURLcode
curl_easy_setopt_ccsid(CURL * curl, CURLoption tag, ...)

{
  CURLcode result;
  va_list arg;
  struct SessionHandle * data;
  char * s;
  char * cp;
  unsigned int ccsid;
  size_t len;
  curl_off_t pfsize;
  static char testwarn = 1;

  /* Warns if this procedure has not been updated when the dupstring enum
     changes.
     We (try to) do it only once: there is no need to issue several times
     the same message; but since threadsafeness is not handled here,
     this may occur (and we don't care!). */

  if (testwarn) {
    testwarn = 0;

    if ((int) STRING_LAST != (int) STRING_SSL_ISSUERCERT + 1)
      curl_mfprintf(stderr,
       "*** WARNING: curl_easy_setopt_ccsid() should be reworked ***\n");
    }

  data = (struct SessionHandle *) curl;
  va_start(arg, tag);

  switch (tag) {

  case CURLOPT_CAINFO:
  case CURLOPT_CAPATH:
  case CURLOPT_COOKIE:
  case CURLOPT_COOKIEFILE:
  case CURLOPT_COOKIEJAR:
  case CURLOPT_COOKIELIST:
  case CURLOPT_CUSTOMREQUEST:
  case CURLOPT_EGDSOCKET:
  case CURLOPT_ENCODING:
  case CURLOPT_FTPPORT:
  case CURLOPT_FTP_ACCOUNT:
  case CURLOPT_FTP_ALTERNATIVE_TO_USER:
  case CURLOPT_INTERFACE:
  case CURLOPT_KEYPASSWD:
  case CURLOPT_KRBLEVEL:
  case CURLOPT_NETRC_FILE:
  case CURLOPT_PROXY:
  case CURLOPT_PROXYUSERPWD:
  case CURLOPT_RANDOM_FILE:
  case CURLOPT_RANGE:
  case CURLOPT_REFERER:
  case CURLOPT_SSH_PRIVATE_KEYFILE:
  case CURLOPT_SSH_PUBLIC_KEYFILE:
  case CURLOPT_SSLCERT:
  case CURLOPT_SSLCERTTYPE:
  case CURLOPT_SSLENGINE:
  case CURLOPT_SSLKEY:
  case CURLOPT_SSLKEYTYPE:
  case CURLOPT_SSL_CIPHER_LIST:
  case CURLOPT_URL:
  case CURLOPT_USERAGENT:
  case CURLOPT_USERPWD:
  case CURLOPT_SSH_HOST_PUBLIC_KEY_MD5:
  case CURLOPT_CRLFILE:
  case CURLOPT_ISSUERCERT:
    s = va_arg(arg, char *);
    ccsid = va_arg(arg, unsigned int);

    if (s) {
      s = dynconvert(ASCII_CCSID, s, -1, ccsid);

      if (!s) {
        result = CURLE_OUT_OF_MEMORY;
        break;
        }
      }

    result = curl_easy_setopt(curl, tag, s);

    if (s)
      free(s);

    break;

  case CURLOPT_COPYPOSTFIELDS:
    /* Special case: byte count may have been given by CURLOPT_POSTFIELDSIZE
       prior to this call. In this case, convert the given byte count and
       replace the length according to the conversion result. */
    s = va_arg(arg, char *);
    ccsid = va_arg(arg, unsigned int);

    pfsize = data->set.postfieldsize;

    if (!s || !pfsize || ccsid == NOCONV_CCSID || ccsid == ASCII_CCSID) {
      result = curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, s);
      break;
      }

    if (pfsize == -1) {
      /* Data is null-terminated. */
      s = dynconvert(ASCII_CCSID, s, -1, ccsid);

      if (!s) {
        result = CURLE_OUT_OF_MEMORY;
        break;
        }
      }
    else {
      /* Data length specified. */

      if (pfsize < 0 || pfsize > SIZE_MAX) {
        result = CURLE_OUT_OF_MEMORY;
        break;
        }

      len = pfsize;
      pfsize = len * MAX_CONV_EXPANSION;

      if (pfsize > SIZE_MAX)
        pfsize = SIZE_MAX;

      cp = malloc(pfsize);

      if (!cp) {
        result = CURLE_OUT_OF_MEMORY;
        break;
        }

      pfsize = convert(cp, pfsize, ASCII_CCSID, s, len, ccsid);

      if (pfsize < 0) {
        free(cp);
        result = CURLE_OUT_OF_MEMORY;
        break;
        }

      data->set.postfieldsize = pfsize;         /* Replace data size. */
      s = cp;
      }

    result = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, s);
    data->set.str[STRING_COPYPOSTFIELDS] = s;   /* Give to library. */
    break;

  case CURLOPT_ERRORBUFFER:                     /* This is an output buffer. */
  default:
    result = Curl_setopt(data, tag, arg);
    break;
    }

  va_end(arg);
  return result;
}


char *
curl_form_long_value(long value)

{
  /* ILE/RPG cannot cast an integer to a pointer. This procedure does it. */

  return (char *) value;
}
