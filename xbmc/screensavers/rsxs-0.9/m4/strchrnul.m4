# strchrnul.m4 serial 3
dnl Copyright (C) 2003 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_STRCHRNUL],
[
  dnl Persuade glibc <string.h> to declare strchrnul().
  AC_REQUIRE([AC_GNU_SOURCE])

  AC_REPLACE_FUNCS(strchrnul)
  if test $ac_cv_func_strchrnul = no; then
    gl_PREREQ_STRCHRNUL
  fi
])

# Prerequisites of lib/strchrnul.c.
AC_DEFUN([gl_PREREQ_STRCHRNUL], [:])
