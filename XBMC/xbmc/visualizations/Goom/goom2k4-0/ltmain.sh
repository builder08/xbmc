# Generated from ltmain.m4sh.

# ltmain.sh (GNU libtool) 2.2.4
# Written by Gordon Matzigkeit <gord@gnu.ai.mit.edu>, 1996

# Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2003, 2004, 2005, 2006, 2007 2008 Free Software Foundation, Inc.
# This is free software; see the source for copying conditions.  There is NO
# warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# GNU Libtool is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# As a special exception to the GNU General Public License,
# if you distribute this file as part of a program or library that
# is built using GNU Libtool, you may include this file under the
# same distribution terms that you use for the rest of that program.
#
# GNU Libtool is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Libtool; see the file COPYING.  If not, a copy
# can be downloaded from http://www.gnu.org/licenses/gpl.html,
# or obtained by writing to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# Usage: $progname [OPTION]... [MODE-ARG]...
#
# Provide generalized library-building support services.
#
#     --config             show all configuration variables
#     --debug              enable verbose shell tracing
# -n, --dry-run            display commands without modifying any files
#     --features           display basic configuration information and exit
#     --mode=MODE          use operation mode MODE
#     --preserve-dup-deps  don't remove duplicate dependency libraries
#     --quiet, --silent    don't print informational messages
#     --tag=TAG            use configuration variables from tag TAG
# -v, --verbose            print informational messages (default)
#     --version            print version information
# -h, --help               print short or long help message
#
# MODE must be one of the following:
#
#       clean              remove files from the build directory
#       compile            compile a source file into a libtool object
#       execute            automatically set library path, then run a program
#       finish             complete the installation of libtool libraries
#       install            install libraries or executables
#       link               create a library or an executable
#       uninstall          remove libraries from an installed directory
#
# MODE-ARGS vary depending on the MODE.
# Try `$progname --help --mode=MODE' for a more detailed description of MODE.
#
# When reporting a bug, please describe a test case to reproduce it and
# include the following information:
#
#       host-triplet:	$host
#       shell:		$SHELL
#       compiler:		$LTCC
#       compiler flags:		$LTCFLAGS
#       linker:		$LD (gnu? $with_gnu_ld)
#       $progname:		(GNU libtool) 2.2.4 Debian-2.2.4-0ubuntu4
#       automake:		$automake_version
#       autoconf:		$autoconf_version
#
# Report bugs to <bug-libtool@gnu.org>.

PROGRAM=ltmain.sh
PACKAGE=libtool
VERSION="2.2.4 Debian-2.2.4-0ubuntu4"
TIMESTAMP=""
package_revision=1.2976

# Be Bourne compatible
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then
  emulate sh
  NULLCMD=:
  # Zsh 3.x and 4.x performs word splitting on ${1+"$@"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '${1+"$@"}'='"$@"'
  setopt NO_GLOB_SUBST
else
  case `(set -o) 2>/dev/null` in *posix*) set -o posix;; esac
fi
BIN_SH=xpg4; export BIN_SH # for Tru64
DUALCASE=1; export DUALCASE # for MKS sh

# NLS nuisances: We save the old values to restore during execute mode.
# Only set LANG and LC_ALL to C if already set.
# These must not be set unconditionally because not all systems understand
# e.g. LANG=C (notably SCO).
lt_user_locale=
lt_safe_locale=
for lt_var in LANG LANGUAGE LC_ALL LC_CTYPE LC_COLLATE LC_MESSAGES
do
  eval "if test \"\${$lt_var+set}\" = set; then
          save_$lt_var=\$$lt_var
          $lt_var=C
	  export $lt_var
	  lt_user_locale=\"$lt_var=\\\$save_\$lt_var; \$lt_user_locale\"
	  lt_safe_locale=\"$lt_var=C; \$lt_safe_locale\"
	fi"
done

$lt_unset CDPATH





: ${CP="cp -f"}
: ${ECHO="echo"}
: ${EGREP="/bin/grep -E"}
: ${FGREP="/bin/grep -F"}
: ${GREP="/bin/grep"}
: ${LN_S="ln -s"}
: ${MAKE="make"}
: ${MKDIR="mkdir"}
: ${MV="mv -f"}
: ${RM="rm -f"}
: ${SED="/bin/sed"}
: ${SHELL="${CONFIG_SHELL-/bin/sh}"}
: ${Xsed="$SED -e 1s/^X//"}

# Global variables:
EXIT_SUCCESS=0
EXIT_FAILURE=1
EXIT_MISMATCH=63  # $? = 63 is used to indicate version mismatch to missing.
EXIT_SKIP=77	  # $? = 77 is used to indicate a skipped test to automake.

exit_status=$EXIT_SUCCESS

# Make sure IFS has a sensible default
lt_nl='
'
IFS=" 	$lt_nl"

dirname="s,/[^/]*$,,"
basename="s,^.*/,,"

# func_dirname_and_basename file append nondir_replacement
# perform func_basename and func_dirname in a single function
# call:
#   dirname:  Compute the dirname of FILE.  If nonempty,
#             add APPEND to the result, otherwise set result
#             to NONDIR_REPLACEMENT.
#             value returned in "$func_dirname_result"
#   basename: Compute filename of FILE.
#             value retuned in "$func_basename_result"
# Implementation must be kept synchronized with func_dirname
# and func_basename. For efficiency, we do not delegate to
# those functions but instead duplicate the functionality here.
func_dirname_and_basename ()
{
  # Extract subdirectory from the argument.
  func_dirname_result=`$ECHO "X${1}" | $Xsed -e "$dirname"`
  if test "X$func_dirname_result" = "X${1}"; then
    func_dirname_result="${3}"
  else
    func_dirname_result="$func_dirname_result${2}"
  fi
  func_basename_result=`$ECHO "X${1}" | $Xsed -e "$basename"`
}

# Generated shell functions inserted here.

# Work around backward compatibility issue on IRIX 6.5. On IRIX 6.4+, sh
# is ksh but when the shell is invoked as "sh" and the current value of
# the _XPG environment variable is not equal to 1 (one), the special
# positional parameter $0, within a function call, is the name of the
# function.
progpath="$0"

# The name of this program:
# In the unlikely event $progname began with a '-', it would play havoc with
# func_echo (imagine progname=-n), so we prepend ./ in that case:
func_dirname_and_basename "$progpath"
progname=$func_basename_result
case $progname in
  -*) progname=./$progname ;;
esac

# Make sure we have an absolute path for reexecution:
case $progpath in
  [\\/]*|[A-Za-z]:\\*) ;;
  *[\\/]*)
     progdir=$func_dirname_result
     progdir=`cd "$progdir" && pwd`
     progpath="$progdir/$progname"
     ;;
  *)
     save_IFS="$IFS"
     IFS=:
     for progdir in $PATH; do
       IFS="$save_IFS"
       test -x "$progdir/$progname" && break
     done
     IFS="$save_IFS"
     test -n "$progdir" || progdir=`pwd`
     progpath="$progdir/$progname"
     ;;
esac

# Sed substitution that helps us do robust quoting.  It backslashifies
# metacharacters that are still active within double-quoted strings.
Xsed="${SED}"' -e 1s/^X//'
sed_quote_subst='s/\([`"$\\]\)/\\\1/g'

# Same as above, but do not quote variable references.
double_quote_subst='s/\(["`\\]\)/\\\1/g'

# Re-`\' parameter expansions in output of double_quote_subst that were
# `\'-ed in input to the same.  If an odd number of `\' preceded a '$'
# in input to double_quote_subst, that '$' was protected from expansion.
# Since each input `\' is now two `\'s, look for any number of runs of
# four `\'s followed by two `\'s and then a '$'.  `\' that '$'.
bs='\\'
bs2='\\\\'
bs4='\\\\\\\\'
dollar='\$'
sed_double_backslash="\
  s/$bs4/&\\
/g
  s/^$bs2$dollar/$bs&/
  s/\\([^$bs]\\)$bs2$dollar/\\1$bs2$bs$dollar/g
  s/\n//g"

# Standard options:
opt_dry_run=false
opt_help=false
opt_quiet=false
opt_verbose=false
opt_warning=:

# func_echo arg...
# Echo program name prefixed message, along with the current mode
# name if it has been set yet.
func_echo ()
{
    $ECHO "$progname${mode+: }$mode: $*"
}

# func_verbose arg...
# Echo program name prefixed message in verbose mode only.
func_verbose ()
{
    $opt_verbose && func_echo ${1+"$@"}

    # A bug in bash halts the script if the last line of a function
    # fails when set -e is in force, so we need another command to
    # work around that:
    :
}

# func_error arg...
# Echo program name prefixed message to standard error.
func_error ()
{
    $ECHO "$progname${mode+: }$mode: "${1+"$@"} 1>&2
}

# func_warning arg...
# Echo program name prefixed warning message to standard error.
func_warning ()
{
    $opt_warning && $ECHO "$progname${mode+: }$mode: warning: "${1+"$@"} 1>&2

    # bash bug again:
    :
}

# func_fatal_error arg...
# Echo program name prefixed message to standard error, and exit.
func_fatal_error ()
{
    func_error ${1+"$@"}
    exit $EXIT_FAILURE
}

# func_fatal_help arg...
# Echo program name prefixed message to standard error, followed by
# a help hint, and exit.
func_fatal_help ()
{
    func_error ${1+"$@"}
    func_fatal_error "$help"
}
help="Try \`$progname --help' for more information."  ## default


# func_grep expression filename
# Check whether EXPRESSION matches any line of FILENAME, without output.
func_grep ()
{
    $GREP "$1" "$2" >/dev/null 2>&1
}


# func_mkdir_p directory-path
# Make sure the entire path to DIRECTORY-PATH is available.
func_mkdir_p ()
{
    my_directory_path="$1"
    my_dir_list=

    if test -n "$my_directory_path" && test "$opt_dry_run" != ":"; then

      # Protect directory names starting with `-'
      case $my_directory_path in
        -*) my_directory_path="./$my_directory_path" ;;
      esac

      # While some portion of DIR does not yet exist...
      while test ! -d "$my_directory_path"; do
        # ...make a list in topmost first order.  Use a colon delimited
	# list incase some portion of path contains whitespace.
        my_dir_list="$my_directory_path:$my_dir_list"

        # If the last portion added has no slash in it, the list is done
        case $my_directory_path in */*) ;; *) break ;; esac

        # ...otherwise throw away the child directory and loop
        my_directory_path=`$ECHO "X$my_directory_path" | $Xsed -e "$dirname"`
      done
      my_dir_list=`$ECHO "X$my_dir_list" | $Xsed -e 's,:*$,,'`

      save_mkdir_p_IFS="$IFS"; IFS=':'
      for my_dir in $my_dir_list; do
	IFS="$save_mkdir_p_IFS"
        # mkdir can fail with a `File exist' error if two processes
        # try to create one of the directories concurrently.  Don't
        # stop in that case!
        $MKDIR "$my_dir" 2>/dev/null || :
      done
      IFS="$save_mkdir_p_IFS"

      # Bail out if we (or some other process) failed to create a directory.
      test -d "$my_directory_path" || \
        func_fatal_error "Failed to create \`$1'"
    fi
}


# func_mktempdir [string]
# Make a temporary directory that won't clash with other running
# libtool processes, and avoids race conditions if possible.  If
# given, STRING is the basename for that directory.
func_mktempdir ()
{
    my_template="${TMPDIR-/tmp}/${1-$progname}"

    if test "$opt_dry_run" = ":"; then
      # Return a directory name, but don't create it in dry-run mode
      my_tmpdir="${my_template}-$$"
    else

      # If mktemp works, use that first and foremost
      my_tmpdir=`mktemp -d "${my_template}-XXXXXXXX" 2>/dev/null`

      if test ! -d "$my_tmpdir"; then
        # Failing that, at least try and use $RANDOM to avoid a race
        my_tmpdir="${my_template}-${RANDOM-0}$$"

        save_mktempdir_umask=`umask`
        umask 0077
        $MKDIR "$my_tmpdir"
        umask $save_mktempdir_umask
      fi

      # If we're not in dry-run mode, bomb out on failure
      test -d "$my_tmpdir" || \
        func_fatal_error "cannot create temporary directory \`$my_tmpdir'"
    fi

    $ECHO "X$my_tmpdir" | $Xsed
}


# func_quote_for_eval arg
# Aesthetically quote ARG to be evaled later.
# This function returns two values: FUNC_QUOTE_FOR_EVAL_RESULT
# is double-quoted, suitable for a subsequent eval, whereas
# FUNC_QUOTE_FOR_EVAL_UNQUOTED_RESULT has merely all characters
# which are still active within double quotes backslashified.
func_quote_for_eval ()
{
    case $1 in
      *[\\\`\"\$]*)
	func_quote_for_eval_unquoted_result=`$ECHO "X$1" | $Xsed -e "$sed_quote_subst"` ;;
      *)
        func_quote_for_eval_unquoted_result="$1" ;;
    esac

    case $func_quote_for_eval_unquoted_result in
      # Double-quote args containing shell metacharacters to delay
      # word splitting, command substitution and and variable
      # expansion for a subsequent eval.
      # Many Bourne shells cannot handle close brackets correctly
      # in scan sets, so we specify it separately.
      *[\[\~\#\^\&\*\(\)\{\}\|\;\<\>\?\'\ \	]*|*]*|"")
        func_quote_for_eval_result="\"$func_quote_for_eval_unquoted_result\""
        ;;
      *)
        func_quote_for_eval_result="$func_quote_for_eval_unquoted_result"
    esac
}


# func_quote_for_expand arg
# Aesthetically quote ARG to be evaled later; same as above,
# but do not quote variable references.
func_quote_for_expand ()
{
    case $1 in
      *[\\\`\"]*)
	my_arg=`$ECHO "X$1" | $Xsed \
	    -e "$double_quote_subst" -e "$sed_double_backslash"` ;;
      *)
        my_arg="$1" ;;
    esac

    case $my_arg in
      # Double-quote args containing shell metacharacters to delay
      # word splitting and command substitution for a subsequent eval.
      # Many Bourne shells cannot handle close brackets correctly
      # in scan sets, so we specify it separately.
      *[\[\~\#\^\&\*\(\)\{\}\|\;\<\>\?\'\ \	]*|*]*|"")
        my_arg="\"$my_arg\""
        ;;
    esac

    func_quote_for_expand_result="$my_arg"
}


# func_show_eval cmd [fail_exp]
# Unless opt_silent is true, then output CMD.  Then, if opt_dryrun is
# not true, evaluate CMD.  If the evaluation of CMD fails, and FAIL_EXP
# is given, then evaluate it.
func_show_eval ()
{
    my_cmd="$1"
    my_fail_exp="${2-:}"

    ${opt_silent-false} || {
      func_quote_for_expand "$my_cmd"
      eval "func_echo $func_quote_for_expand_result"
    }

    if ${opt_dry_run-false}; then :; else
      eval "$my_cmd"
      my_status=$?
      if test "$my_status" -eq 0; then :; else
	eval "(exit $my_status); $my_fail_exp"
      fi
    fi
}


# func_show_eval_locale cmd [fail_exp]
# Unless opt_silent is true, then output CMD.  Then, if opt_dryrun is
# not true, evaluate CMD.  If the evaluation of CMD fails, and FAIL_EXP
# is given, then evaluate it.  Use the saved locale for evaluation.
func_show_eval_locale ()
{
    my_cmd="$1"
    my_fail_exp="${2-:}"

    ${opt_silent-false} || {
      func_quote_for_expand "$my_cmd"
      eval "func_echo $func_quote_for_expand_result"
    }

    if ${opt_dry_run-false}; then :; else
      eval "$lt_user_locale
	    $my_cmd"
      my_status=$?
      eval "$lt_safe_locale"
      if test "$my_status" -eq 0; then :; else
	eval "(exit $my_status); $my_fail_exp"
      fi
    fi
}





# func_version
# Echo version message to standard output and exit.
func_version ()
{
    $SED -n '/^# '$PROGRAM' (GNU /,/# warranty; / {
        s/^# //
	s/^# *$//
        s/\((C)\)[ 0-9,-]*\( [1-9][0-9]*\)/\1\2/
        p
     }' < "$progpath"
     exit $?
}

# func_usage
# Echo short help message to standard output and exit.
func_usage ()
{
    $SED -n '/^# Usage:/,/# -h/ {
        s/^# //
	s/^# *$//
	s/\$progname/'$progname'/
	p
    }' < "$progpath"
    $ECHO
    $ECHO "run \`$progname --help | more' for full usage"
    exit $?
}

# func_help
# Echo long help message to standard output and exit.
func_help ()
{
    $SED -n '/^# Usage:/,/# Report bugs to/ {
        s/^# //
	s/^# *$//
	s*\$progname*'$progname'*
	s*\$host*'"$host"'*
	s*\$SHELL*'"$SHELL"'*
	s*\$LTCC*'"$LTCC"'*
	s*\$LTCFLAGS*'"$LTCFLAGS"'*
	s*\$LD*'"$LD"'*
	s/\$with_gnu_ld/'"$with_gnu_ld"'/
	s/\$automake_version/'"`(automake --version) 2>/dev/null |$SED 1q`"'/
	s/\$autoconf_version/'"`(autoconf --version) 2>/dev/null |$SED 1q`"'/
	p
     }' < "$progpath"
    exit $?
}

# func_missing_arg argname
# Echo program name prefixed message to standard error and set global
# exit_cmd.
func_missing_arg ()
{
    func_error "missing argument for $1"
    exit_cmd=exit
}

exit_cmd=:





# Check that we have a working $ECHO.
if test "X$1" = X--no-reexec; then
  # Discard the --no-reexec flag, and continue.
  shift
elif test "X$1" = X--fallback-echo; then
  # Avoid inline document here, it may be left over
  :
elif test "X`{ $ECHO '\t'; } 2>/dev/null`" = 'X\t'; then
  # Yippee, $ECHO works!
  :
else
  # Restart under the correct shell, and then maybe $ECHO will work.
  exec $SHELL "$progpath" --no-reexec ${1+"$@"}
fi

if test "X$1" = X--fallback-echo; then
  # used as fallback echo
  shift
  cat <<EOF
$*
EOF
  exit $EXIT_SUCCESS
fi

magic="%%%MAGIC variable%%%"
magic_exe="%%%MAGIC EXE variable%%%"

# Global variables.
# $mode is unset
nonopt=
execute_dlfiles=
preserve_args=
lo2o="s/\\.lo\$/.${objext}/"
o2lo="s/\\.${objext}\$/.lo/"
extracted_archives=
extracted_serial=0

opt_dry_run=false
opt_duplicate_deps=false
opt_silent=false
opt_debug=:

# If this variable is set in any of the actions, the command in it
# will be execed at the end.  This prevents here-documents from being
# left over by shells.
exec_cmd=

# func_fatal_configuration arg...
# Echo program name prefixed message to standard error, followed by
# a configuration failure hint, and exit.
func_fatal_configuration ()
{
    func_error ${1+"$@"}
    func_error "See the $PACKAGE documentation for more information."
    func_fatal_error "Fatal configuration error."
}


# func_config
# Display the configuration for all the tags in this script.
func_config ()
{
    re_begincf='^# ### BEGIN LIBTOOL'
    re_endcf='^# ### END LIBTOOL'

    # Default configuration.
    $SED "1,/$re_begincf CONFIG/d;/$re_endcf CONFIG/,\$d" < "$progpath"

    # Now print the configurations for the tags.
    for tagname in $taglist; do
      $SED -n "/$re_begincf TAG CONFIG: $tagname\$/,/$re_endcf TAG CONFIG: $tagname\$/p" < "$progpath"
    done

    exit $?
}

# func_features
# Display the features supported by this script.
func_features ()
{
    $ECHO "host: $host"
    if test "$build_libtool_libs" = yes; then
      $ECHO "enable shared libraries"
    else
      $ECHO "disable shared libraries"
    fi
    if test "$build_old_libs" = yes; then
      $ECHO "enable static libraries"
    else
      $ECHO "disable static libraries"
    fi

    exit $?
}

# func_enable_tag tagname
# Verify that TAGNAME is valid, and either flag an error and exit, or
# enable the TAGNAME tag.  We also add TAGNAME to the global $taglist
# variable here.
func_enable_tag ()
{
  # Global variable:
  tagname="$1"

  re_begincf="^# ### BEGIN LIBTOOL TAG CONFIG: $tagname\$"
  re_endcf="^# ### END LIBTOOL TAG CONFIG: $tagname\$"
  sed_extractcf="/$re_begincf/,/$re_endcf/p"

  # Validate tagname.
  case $tagname in
    *[!-_A-Za-z0-9,/]*)
      func_fatal_error "invalid tag name: $tagname"
      ;;
  esac

  # Don't test for the "default" C tag, as we know it's
  # there but not specially marked.
  case $tagname in
    CC) ;;
    *)
      if $GREP "$re_begincf" "$progpath" >/dev/null 2>&1; then
	taglist="$taglist $tagname"

	# Evaluate the configuration.  Be careful to quote the path
	# and the sed script, to avoid splitting on whitespace, but
	# also don't use non-portable quotes within backquotes within
	# quotes we have to do it in 2 steps:
	extractedcf=`$SED -n -e "$sed_extractcf" < "$progpath"`
	eval "$extractedcf"
      else
	func_error "ignoring unknown tag $tagname"
      fi
      ;;
  esac
}

# Parse options once, thoroughly.  This comes as soon as possible in
# the script to make things like `libtool --version' happen quickly.
{

  # Shorthand for --mode=foo, only valid as the first argument
  case $1 in
  clean|clea|cle|cl)
    shift; set dummy --mode clean ${1+"$@"}; shift
    ;;
  compile|compil|compi|comp|com|co|c)
    shift; set dummy --mode compile ${1+"$@"}; shift
    ;;
  execute|execut|execu|exec|exe|ex|e)
    shift; set dummy --mode execute ${1+"$@"}; shift
    ;;
  finish|finis|fini|fin|fi|f)
    shift; set dummy --mode finish ${1+"$@"}; shift
    ;;
  install|instal|insta|inst|ins|in|i)
    shift; set dummy --mode install ${1+"$@"}; shift
    ;;
  link|lin|li|l)
    shift; set dummy --mode link ${1+"$@"}; shift
    ;;
  uninstall|uninstal|uninsta|uninst|unins|unin|uni|un|u)
    shift; set dummy --mode uninstall ${1+"$@"}; shift
    ;;
  esac

  # Parse non-mode specific arguments:
  while test "$#" -gt 0; do
    opt="$1"
    shift

    case $opt in
      --config)		func_config					;;

      --debug)		preserve_args="$preserve_args $opt"
			func_echo "enabling shell trace mode"
			opt_debug='set -x'
			$opt_debug
			;;

      -dlopen)		test "$#" -eq 0 && func_missing_arg "$opt" && break
			execute_dlfiles="$execute_dlfiles $1"
			shift
			;;

      --dry-run | -n)	opt_dry_run=:					;;
      --features)       func_features					;;
      --finish)		mode="finish"					;;

      --mode)		test "$#" -eq 0 && func_missing_arg "$opt" && break
			case $1 in
			  # Valid mode arguments:
			  clean)	;;
			  compile)	;;
			  execute)	;;
			  finish)	;;
			  install)	;;
			  link)		;;
			  relink)	;;
			  uninstall)	;;

			  # Catch anything else as an error
			  *) func_error "invalid argument for $opt"
			     exit_cmd=exit
			     break
			     ;;
		        esac

			mode="$1"
			shift
			;;

      --preserve-dup-deps)
			opt_duplicate_deps=:				;;

      --quiet|--silent)	preserve_args="$preserve_args $opt"
			opt_silent=:
			;;

      --verbose| -v)	preserve_args="$preserve_args $opt"
			opt_silent=false
			;;

      --tag)		test "$#" -eq 0 && func_missing_arg "$opt" && break
			preserve_args="$preserve_args $opt $1"
			func_enable_tag "$1"	# tagname is set here
			shift
			;;

      # Separate optargs to long options:
      -dlopen=*|--mode=*|--tag=*)
			func_opt_split "$opt"
			set dummy "$func_opt_split_opt" "$func_opt_split_arg" ${1+"$@"}
			shift
			;;

      -\?|-h)		func_usage					;;
      --help)		opt_help=:					;;
      --version)	func_version					;;

      -*)		func_fatal_help "unrecognized option \`$opt'"	;;

      *)		nonopt="$opt"
			break
			;;
    esac
  done


  case $host in
    *cygwin* | *mingw* | *pw32*)
      # don't eliminate duplications in $postdeps and $predeps
      opt_duplicate_compiler_generated_deps=:
      ;;
    *)
      opt_duplicate_compiler_generated_deps=$opt_duplicate_deps
      ;;
  esac

  # Having warned about all mis-specified options, bail out if
  # anything was wrong.
  $exit_cmd $EXIT_FAILURE
}

# func_check_version_match
# Ensure that we are using m4 macros, and libtool script from the same
# release of libtool.
func_check_version_match ()
{
  if test "$package_revision" != "$macro_revision"; then
    if test "$VERSION" != "$macro_version"; then
      if test -z "$macro_version"; then
        cat >&2 <<_LT_EOF
$progname: Version mismatch error.  This is $PACKAGE $VERSION, but the
$progname: definition of this LT_INIT comes from an older release.
$progname: You should recreate aclocal.m4 with macros from $PACKAGE $VERSION
$progname: and run autoconf again.
_LT_EOF
      else
        cat >&2 <<_LT_EOF
$progname: Version mismatch error.  This is $PACKAGE $VERSION, but the
$progname: definition of this LT_INIT comes from $PACKAGE $macro_version.
$progname: You should recreate aclocal.m4 with macros from $PACKAGE $VERSION
$progname: and run autoconf again.
_LT_EOF
      fi
    else
      cat >&2 <<_LT_EOF
$progname: Version mismatch error.  This is $PACKAGE $VERSION, revision $package_revision,
$progname: but the definition of this LT_INIT comes from revision $macro_revision.
$progname: You should recreate aclocal.m4 with macros from revision $package_revision
$progname: of $PACKAGE $VERSION and run autoconf again.
_LT_EOF
    fi

    exit $EXIT_MISMATCH
  fi
}


## ----------- ##
##    Main.    ##
## ----------- ##

$opt_help || {
  # Sanity checks first:
  func_check_version_match

  if test "$build_libtool_libs" != yes && test "$build_old_libs" != yes; then
    func_fatal_configuration "not configured to build any kind of library"
  fi

  test -z "$mode" && func_fatal_error "error: you must specify a MODE."


  # Darwin sucks
  eval std_shrext=\"$shrext_cmds\"


  # Only execute mode is allowed to have -dlopen flags.
  if test -n "$execute_dlfiles" && test "$mode" != execute; then
    func_error "unrecognized option \`-dlopen'"
    $ECHO "$help" 1>&2
    exit $EXIT_FAILURE
  fi

  # Change the help message to a mode-specific one.
  generic_help="$help"
  help="Try \`$progname --help --mode=$mode' for more information."
}


# func_lalib_p file
# True iff FILE is a libtool `.la' library or `.lo' object file.
# This function is only a basic sanity check; it will hardly flush out
# determined imposters.
func_lalib_p ()
{
    $SED -e 4q "$1" 2>/dev/null \
      | $GREP "^# Generated by .*$PACKAGE" > /dev/null 2>&1
}

# func_lalib_unsafe_p file
# True iff FILE is a libtool `.la' library or `.lo' object file.
# This function implements the same check as func_lalib_p without
# resorting to external programs.  To this end, it redirects stdin and
# closes it afterwards, without saving the original file descriptor.
# As a safety measure, use it only where a negative result would be
# fatal anyway.  Works if `file' does not exist.
func_lalib_unsafe_p ()
{
    lalib_p=no
    if test -r "$1" && exec 5<&0 <"$1"; then
	for lalib_p_l in 1 2 3 4
	do
	    read lalib_p_line
	    case "$lalib_p_line" in
		\#\ Generated\ by\ *$PACKAGE* ) lalib_p=yes; break;;
	    esac
	done
	exec 0<&5 5<&-
    fi
    test "$lalib_p" = yes
}

# func_ltwrapper_script_p file
# True iff FILE is a libtool wrapper script
# This function is only a basic sanity check; it will hardly flush out
# determined imposters.
func_ltwrapper_script_p ()
{
    func_lalib_p "$1"
}

# func_ltwrapper_executable_p file
# True iff FILE is a libtool wrapper executable
# This function is only a basic sanity check; it will hardly flush out
# determined imposters.
func_ltwrapper_executable_p ()
{
    func_ltwrapper_exec_suffix=
    case $1 in
    *.exe) ;;
    *) func_ltwrapper_exec_suffix=.exe ;;
    esac
    $GREP "$magic_exe" "$1$func_ltwrapper_exec_suffix" >/dev/null 2>&1
}

# func_ltwrapper_scriptname file
# Assumes file is an ltwrapper_executable
# uses $file to determine the appropriate filename for a
# temporary ltwrapper_script.
func_ltwrapper_scriptname ()
{
    func_ltwrapper_scriptname_result=""
    if func_ltwrapper_executable_p "$1"; then
	func_dirname_and_basename "$1" "" "."
	func_stripname '' '.exe' "$func_basename_result"
	func_ltwrapper_scriptname_result="$func_dirname_result/$objdir/${func_stripname_result}_ltshwrapper"
    fi
}

# func_ltwrapper_p file
# True iff FILE is a libtool wrapper script or wrapper executable
# This function is only a basic sanity check; it will hardly flush out
# determined imposters.
func_ltwrapper_p ()
{
    func_ltwrapper_script_p "$1" || func_ltwrapper_executable_p "$1"
}


# func_execute_cmds commands fail_cmd
# Execute tilde-delimited COMMANDS.
# If FAIL_CMD is given, eval that upon failure.
# FAIL_CMD may read-access the current command in variable CMD!
func_execute_cmds ()
{
    $opt_debug
    save_ifs=$IFS; IFS='~'
    for cmd in $1; do
      IFS=$save_ifs
      eval cmd=\"$cmd\"
      func_show_eval "$cmd" "${2-:}"
    done
    IFS=$save_ifs
}


# func_source file
# Source FILE, adding directory component if necessary.
# Note that it is not necessary on cygwin/mingw to append a dot to
# FILE even if both FILE and FILE.exe exist: automatic-append-.exe
# behavior happens only for exec(3), not for open(2)!  Also, sourcing
# `FILE.' does not work on cygwin managed mounts.
func_source ()
{
    $opt_debug
    case $1 in
    */* | *\\*)	. "$1" ;;
    *)		. "./$1" ;;
    esac
}


# func_infer_tag arg
# Infer tagged configuration to use if any are available and
# if one wasn't chosen via the "--tag" command line option.
# Only attempt this if the compiler in the base compile
# command doesn't match the default compiler.
# arg is usually of the form 'gcc ...'
func_infer_tag ()
{
    $opt_debug
    if test -n "$available_tags" && test -z "$tagname"; then
      CC_quoted=
      for arg in $CC; do
        func_quote_for_eval "$arg"
	CC_quoted="$CC_quoted $func_quote_for_eval_result"
      done
      case $@ in
      # Blanks in the command may have been stripped by the calling shell,
      # but not from the CC environment variable when configure was run.
      " $CC "* | "$CC "* | " `$ECHO $CC` "* | "`$ECHO $CC` "* | " $CC_quoted"* | "$CC_quoted "* | " `$ECHO $CC_quoted` "* | "`$ECHO $CC_quoted` "*) ;;
      # Blanks at the start of $base_compile will cause this to fail
      # if we don't check for them as well.
      *)
	for z in $available_tags; do
	  if $GREP "^# ### BEGIN LIBTOOL TAG CONFIG: $z$" < "$progpath" > /dev/null; then
	    # Evaluate the configuration.
	    eval "`${SED} -n -e '/^# ### BEGIN LIBTOOL TAG CONFIG: '$z'$/,/^# ### END LIBTOOL TAG CONFIG: '$z'$/p' < $progpath`"
	    CC_quoted=
	    for arg in $CC; do
	      # Double-quote args containing other shell metacharacters.
	      func_quote_for_eval "$arg"
	      CC_quoted="$CC_quoted $func_quote_for_eval_result"
	    done
	    case "$@ " in
	      " $CC "* | "$CC "* | " `$ECHO $CC` "* | "`$ECHO $CC` "* | " $CC_quoted"* | "$CC_quoted "* | " `$ECHO $CC_quoted` "* | "`$ECHO $CC_quoted` "*)
	      # The compiler in the base compile command matches
	      # the one in the tagged configuration.
	      # Assume this is the tagged configuration we want.
	      tagname=$z
	      break
	      ;;
	    esac
	  fi
	done
	# If $tagname still isn't set, then no tagged configuration
	# was found and let the user know that the "--tag" command
	# line option must be used.
	if test -z "$tagname"; then
	  func_echo "unable to infer tagged configuration"
	  func_fatal_error "specify a tag with \`--tag'"
#	else
#	  func_verbose "using $tagname tagged configuration"
	fi
	;;
      esac
    fi
}



# func_write_libtool_object output_name pic_name nonpic_name
# Create a libtool object file (analogous to a ".la" file),
# but don't create it if we're doing a dry run.
func_write_libtool_object ()
{
    write_libobj=${1}
    if test "$build_libtool_libs" = yes; then
      write_lobj=\'${2}\'
    else
      write_lobj=none
    fi

    if test "$build_old_libs" = yes; then
      write_oldobj=\'${3}\'
    else
      write_oldobj=none
    fi

    $opt_dry_run || {
      cat >${write_libobj}T <<EOF
# $write_libobj - a libtool object file
# Generated by $PROGRAM (GNU $PACKAGE$TIMESTAMP) $VERSION
#
# Please DO NOT delete this file!
# It is necessary for linking the library.

# Name of the PIC object.
pic_object=$write_lobj

# Name of the non-PIC object
non_pic_object=$write_oldobj

EOF
      $MV "${write_libobj}T" "${write_libobj}"
    }
}

# func_mode_compile arg...
func_mode_compile ()
{
    $opt_debug
    # Get the compilation command and the source file.
    base_compile=
    srcfile="$nonopt"  #  always keep a non-empty value in "srcfile"
    suppress_opt=yes
    suppress_output=
    arg_mode=normal
    libobj=
    later=
    pie_flag=

    for arg
    do
      case $arg_mode in
      arg  )
	# do not "continue".  Instead, add this to base_compile
	lastarg="$arg"
	arg_mode=normal
	;;

      target )
	libobj="$arg"
	arg_mode=normal
	continue
	;;

      normal )
	# Accept any command-line options.
	case $arg in
	-o)
	  test -n "$libobj" && \
	    func_fatal_error "you cannot specify \`-o' more than once"
	  arg_mode=target
	  continue
	  ;;

	-pie | -fpie | -fPIE)
          pie_flag="$pie_flag $arg"
	  continue
	  ;;

	-shared | -static | -prefer-pic | -prefer-non-pic)
	  later="$later $arg"
	  continue
	  ;;

	-no-suppress)
	  suppress_opt=no
	  continue
	  ;;

	-Xcompiler)
	  arg_mode=arg  #  the next one goes into the "base_compile" arg list
	  continue      #  The current "srcfile" will either be retained or
	  ;;            #  replaced later.  I would guess that would be a bug.

	-Wc,*)
	  func_stripname '-Wc,' '' "$arg"
	  args=$func_stripname_result
	  lastarg=
	  save_ifs="$IFS"; IFS=','
	  for arg in $args; do
	    IFS="$save_ifs"
	    func_quote_for_eval "$arg"
	    lastarg="$lastarg $func_quote_for_eval_result"
	  done
	  IFS="$save_ifs"
	  func_stripname ' ' '' "$lastarg"
	  lastarg=$func_stripname_result

	  # Add the arguments to base_compile.
	  base_compile="$base_compile $lastarg"
	  continue
	  ;;

	*)
	  # Accept the current argument as the source file.
	  # The previous "srcfile" becomes the current argument.
	  #
	  lastarg="$srcfile"
	  srcfile="$arg"
	  ;;
	esac  #  case $arg
	;;
      esac    #  case $arg_mode

      # Aesthetically quote the previous argument.
      func_quote_for_eval "$lastarg"
      base_compile="$base_compile $func_quote_for_eval_result"
    done # for arg

    case $arg_mode in
    arg)
      func_fatal_error "you must specify an argument for -Xcompile"
      ;;
    target)
      func_fatal_error "you must specify a target with \`-o'"
      ;;
    *)
      # Get the name of the library object.
      test -z "$libobj" && {
	func_basename "$srcfile"
	libobj="$func_basename_result"
      }
      ;;
    esac

    # Recognize several different file suffixes.
    # If the user specifies -o file.o, it is replaced with file.lo
    case $libobj in
    *.[cCFSifmso] | \
    *.ada | *.adb | *.ads | *.asm | \
    *.c++ | *.cc | *.ii | *.class | *.cpp | *.cxx | \
    *.[fF][09]? | *.for | *.java | *.obj | *.sx)
      func_xform "$libobj"
      libobj=$func_xform_result
      ;;
    esac

    case $libobj in
    *.lo) func_lo2o "$libobj"; obj=$func_lo2o_result ;;
    *)
      func_fatal_error "cannot determine name of library object from \`$libobj'"
      ;;
    esac

    func_infer_tag $base_compile

    for arg in $later; do
      case $arg in
      -shared)
	test "$build_libtool_libs" != yes && \
	  func_fatal_configuration "can not build a shared library"
	build_old_libs=no
	continue
	;;

      -static)
	build_libtool_libs=no
	build_old_libs=yes
	continue
	;;

      -prefer-pic)
	pic_mode=yes
	continue
	;;

      -prefer-non-pic)
	pic_mode=no
	continue
	;;
      esac
    done

    func_quote_for_eval "$libobj"
    test "X$libobj" != "X$func_quote_for_eval_result" \
      && $ECHO "X$libobj" | $GREP '[]~#^*{};<>?"'"'"'	 &()|`$[]' \
      && func_warning "libobj name \`$libobj' may not contain shell special characters."
    func_dirname_and_basename "$obj" "/" ""
    objname="$func_basename_result"
    xdir="$func_dirname_result"
    lobj=${xdir}$objdir/$objname

    test -z "$base_compile" && \
      func_fatal_help "you must specify a compilation command"

    # Delete any leftover library objects.
    if test "$build_old_libs" = yes; then
      removelist="$obj $lobj $libobj ${libobj}T"
    else
      removelist="$lobj $libobj ${libobj}T"
    fi

    # On Cygwin there's no "real" PIC flag so we must build both object types
    case $host_os in
    cygwin* | mingw* | pw32* | os2*)
      pic_mode=default
      ;;
    esac
    if test "$pic_mode" = no && test "$deplibs_check_method" != pass_all; then
      # non-PIC code in shared libraries is not supported
      pic_mode=default
    fi

    # Calculate the filename of the output object if compiler does
    # not support -o with -c
    if test "$compiler_c_o" = no; then
      output_obj=`$ECHO "X$srcfile" | $Xsed -e 's%^.*/%%' -e 's%\.[^.]*$%%'`.${objext}
      lockfile="$output_obj.lock"
    else
      output_obj=
      need_locks=no
      lockfile=
    fi

    # Lock this critical section if it is needed
    # We use this script file to make the link, it avoids creating a new file
    if test "$need_locks" = yes; then
      until $opt_dry_run || ln "$progpath" "$lockfile" 2>/dev/null; do
	func_echo "Waiting for $lockfile to be removed"
	sleep 2
      done
    elif test "$need_locks" = warn; then
      if test -f "$lockfile"; then
	$ECHO "\
*** ERROR, $lockfile exists and contains:
`cat $lockfile 2>/dev/null`

This indicates that another process is trying to use the same
temporary object file, and libtool could not work around it because
your compiler does not support \`-c' and \`-o' together.  If you
repeat this compilation, it may succeed, by chance, but you had better
avoid parallel builds (make -j) in this platform, or get a better
compiler."

	$opt_dry_run || $RM $removelist
	exit $EXIT_FAILURE
      fi
      removelist="$removelist $output_obj"
      $ECHO "$srcfile" > "$lockfile"
    fi

    $opt_dry_run || $RM $removelist
    removelist="$removelist $lockfile"
    trap '$opt_dry_run || $RM $removelist; exit $EXIT_FAILURE' 1 2 15

    if test -n "$fix_srcfile_path"; then
      eval srcfile=\"$fix_srcfile_path\"
    fi
    func_quote_for_eval "$srcfile"
    qsrcfile=$func_quote_for_eval_result

    # Only build a PIC object if we are building libtool libraries.
    if test "$build_libtool_libs" = yes; then
      # Without this assignment, base_compile gets emptied.
      fbsd_hideous_sh_bug=$base_compile

      if test "$pic_mode" != no; then
	command="$base_compile $qsrcfile $pic_flag"
      else
	# Don't build PIC code
	command="$base_compile $qsrcfile"
      fi

      func_mkdir_p "$xdir$objdir"

      if test -z "$output_obj"; then
	# Place PIC objects in $objdir
	command="$command -o $lobj"
      fi

      func_show_eval_locale "$command"	\
          'test -n "$output_obj" && $RM $removelist; exit $EXIT_FAILURE'

      if test "$need_locks" = warn &&
	 test "X`cat $lockfile 2>/dev/null`" != "X$srcfile"; then
	$ECHO "\
*** ERROR, $lockfile contains:
`cat $lockfile 2>/dev/null`

but it should contain:
$srcfile

This indicates that another process is trying to use the same
temporary object file, and libtool could not work around it because
your compiler does not support \`-c' and \`-o' together.  If you
repeat this compilation, it may succeed, by chance, but you had better
avoid parallel builds (make -j) in this platform, or get a better
compiler."

	$opt_dry_run || $RM $removelist
	exit $EXIT_FAILURE
      fi

      # Just move the object if needed, then go on to compile the next one
      if test -n "$output_obj" && test "X$output_obj" != "X$lobj"; then
	func_show_eval '$MV "$output_obj" "$lobj"' \
	  'error=$?; $opt_dry_run || $RM $removelist; exit $error'
      fi

      # Allow error messages only from the first compilation.
      if test "$suppress_opt" = yes; then
	suppress_output=' >/dev/null 2>&1'
      fi
    fi

    # Only build a position-dependent object if we build old libraries.
    if test "$build_old_libs" = yes; then
      if test "$pic_mode" != yes; then
	# Don't build PIC code
	command="$base_compile $qsrcfile$pie_flag"
      else
	command="$base_compile $qsrcfile $pic_flag"
      fi
      if test "$compiler_c_o" = yes; then
	command="$command -o $obj"
      fi

      # Suppress compiler output if we already did a PIC compilation.
      command="$command$suppress_output"
      func_show_eval_locale "$command" \
        '$opt_dry_run || $RM $removelist; exit $EXIT_FAILURE'

      if test "$need_locks" = warn &&
	 test "X`cat $lockfile 2>/dev/null`" != "X$srcfile"; then
	$ECHO "\
*** ERROR, $lockfile contains:
`cat $lockfile 2>/dev/null`

but it should contain:
$srcfile

This indicates that another process is trying to use the same
temporary object file, and libtool could not work around it because
your compiler does not support \`-c' and \`-o' together.  If you
repeat this compilation, it may succeed, by chance, but you had better
avoid parallel builds (make -j) in this platform, or get a better
compiler."

	$opt_dry_run || $RM $removelist
	exit $EXIT_FAILURE
      fi

      # Just move the object if needed
      if test -n "$output_obj" && test "X$output_obj" != "X$obj"; then
	func_show_eval '$MV "$output_obj" "$obj"' \
	  'error=$?; $opt_dry_run || $RM $removelist; exit $error'
      fi
    fi

    $opt_dry_run || {
      func_write_libtool_object "$libobj" "$objdir/$objname" "$objname"

      # Unlock the critical section if it was locked
      if test "$need_locks" != no; then
	removelist=$lockfile
        $RM "$lockfile"
      fi
    }

    exit $EXIT_SUCCESS
}

$opt_help || {
test "$mode" = compile && func_mode_compile ${1+"$@"}
}

func_mode_help ()
{
    # We need to display help for each of the modes.
    case $mode in
      "")
        # Generic help is extracted from the usage comments
        # at the start of this file.
        func_help
        ;;

      clean)
        $ECHO \
"Usage: $progname [OPTION]... --mode=clean RM [RM-OPTION]... FILE...

Remove files from the build directory.

RM is the name of the program to use to delete files associated with each FILE
(typically \`/bin/rm').  RM-OPTIONS are options (such as \`-f') to be passed
to RM.

If FILE is a libtool library, object or program, all the files associated
with it are deleted. Otherwise, only FILE itself is deleted using RM."
        ;;

      compile)
      $ECHO \
"Usage: $progname [OPTION]... --mode=compile COMPILE-COMMAND... SOURCEFILE

Compile a source file into a libtool library object.

This mode accepts the following additional options:

  -o OUTPUT-FILE    set the output file name to OUTPUT-FILE
  -no-suppress      do not suppress compiler output for multiple passes
  -prefer-pic       try to building PIC objects only
  -prefer-non-pic   try to building non-PIC objects only
  -shared           do not build a \`.o' file suitable for static linking
  -static           only build a \`.o' file suitable for static linking

COMPILE-COMMAND is a command to be used in creating a \`standard' object file
from the given SOURCEFILE.

The output file name is determined by removing the directory component from
SOURCEFILE, then substituting the C source code suffix \`.c' with the
library object suffix, \`.lo'."
        ;;

      execute)
        $ECHO \
"Usage: $progname [OPTION]... --mode=execute COMMAND [ARGS]...

Automatically set library path, then run a program.

This mode accepts the following additional options:

  -dlopen FILE      add the directory containing FILE to the library path

This mode sets the library path environment variable according to \`-dlopen'
flags.

If any of the ARGS are libtool executable wrappers, then they are translated
into their corresponding uninstalled binary, and any of their required library
directories are added to the library path.

Then, COMMAND is executed, with ARGS as arguments."
        ;;

      finish)
        $ECHO \
"Usage: $progname [OPTION]... --mode=finish [LIBDIR]...

Complete the installation of libtool libraries.

Each LIBDIR is a directory that contains libtool libraries.

The commands that this mode executes may require superuser privileges.  Use
the \`--dry-run' option if you just want to see what would be executed."
        ;;

      install)
        $ECHO \
"Usage: $progname [OPTION]... --mode=install INSTALL-COMMAND...

Install executables or libraries.

INSTALL-COMMAND is the installation command.  The first component should be
either the \`install' or \`cp' program.

The following components of INSTALL-COMMAND are treated specially:

  -inst-prefix PREFIX-DIR  Use PREFIX-DIR as a staging area for installation

The rest of the components are interpreted as arguments to that command (only
BSD-compatible install options are recognized)."
        ;;

      link)
        $ECHO \
"Usage: $progname [OPTION]... --mode=link LINK-COMMAND...

Link object files or libraries together to form another library, or to
create an executable program.

LINK-COMMAND is a command using the C compiler that you would use to create
a program from several object files.

The following components of LINK-COMMAND are treated specially:

  -all-static       do not do any dynamic linking at all
  -avoid-version    do not add a version suffix if possible
  -dlopen FILE      \`-dlpreopen' FILE if it cannot be dlopened at runtime
  -dlpreopen FILE   link in FILE and add its symbols to lt_preloaded_symbols
  -export-dynamic   allow symbols from OUTPUT-FILE to be resolved with dlsym(3)
  -export-symbols SYMFILE
                    try to export only the symbols listed in SYMFILE
  -export-symbols-regex REGEX
                    try to export only the symbols matching REGEX
  -LLIBDIR          search LIBDIR for required installed libraries
  -lNAME            OUTPUT-FILE requires the installed library libNAME
  -module           build a library that can dlopened
  -no-fast-install  disable the fast-install mode
  -no-install       link a not-installable executable
  -no-undefined     declare that a library does not refer to external symbols
  -o OUTPUT-FILE    create OUTPUT-FILE from the specified objects
  -objectlist FILE  Use a list of object files found in FILE to specify objects
  -precious-files-regex REGEX
                    don't remove output files matching REGEX
  -release RELEASE  specify package release information
  -rpath LIBDIR     the created library will eventually be installed in LIBDIR
  -R[ ]LIBDIR       add LIBDIR to the runtime path of programs and libraries
  -shared           only do dynamic linking of libtool libraries
  -shrext SUFFIX    override the standard shared library file extension
  -static           do not do any dynamic linking of uninstalled libtool libraries
  -static-libtool-libs
                    do not do any dynamic linking of libtool libraries
  -version-info CURRENT[:REVISION[:AGE]]
                    specify library version info [each variable defaults to 0]
  -weak LIBNAME     declare that the target provides the LIBNAME interface

All other options (arguments beginning with \`-') are ignored.

Every other argument is treated as a filename.  Files ending in \`.la' are
treated as uninstalled libtool libraries, other files are standard or library
object files.

If the OUTPUT-FILE ends in \`.la', then a libtool library is created,
only library objects (\`.lo' files) may be specified, and \`-rpath' is
required, except when creating a convenience library.

If OUTPUT-FILE ends in \`.a' or \`.lib', then a standard library is created
using \`ar' and \`ranlib', or on Windows using \`lib'.

If OUTPUT-FILE ends in \`.lo' or \`.${objext}', then a reloadable object file
is created, otherwise an executable program is created."
        ;;

      uninstall)
        $ECHO \
"Usage: $progname [OPTION]... --mode=uninstall RM [RM-OPTION]... FILE...

Remove libraries from an installation directory.

RM is the name of the program to use to delete files associated with each FILE
(typically \`/bin/rm').  RM-OPTIONS are options (such as \`-f') to be passed
to RM.

If FILE is a libtool library, all the files associated with it are deleted.
Otherwise, only FILE itself is deleted using RM."
        ;;

      *)
        func_fatal_help "invalid operation mode \`$mode'"
        ;;
    esac

    $ECHO
    $ECHO "Try \`$progname --help' for more information about other modes."

    exit $?
}

  # Now that we've collected a possible --mode arg, show help if necessary
  $opt_help && func_mode_help


# func_mode_execute arg...
func_mode_execute ()
{
    $opt_debug
    # The first argument is the command name.
    cmd="$nonopt"
    test -z "$cmd" && \
      func_fatal_help "you must specify a COMMAND"

    # Handle -dlopen flags immediately.
    for file in $execute_dlfiles; do
      test -f "$file" \
	|| func_fatal_help "\`$file' is not a file"

      dir=
      case $file in
      *.la)
	# Check to see that this really is a libtool archive.
	func_lalib_unsafe_p "$file" \
	  || func_fatal_help "\`$lib' is not a valid libtool archive"

	# Read the libtool library.
	dlname=
	library_names=
	func_source "$file"

	# Skip this library if it cannot be dlopened.
	if test -z "$dlname"; then
	  # Warn if it was a shared library.
	  test -n "$library_names" && \
	    func_warning "\`$file' was not linked with \`-export-dynamic'"
	  continue
	fi

	func_dirname "$file" "" "."
	dir="$func_dirname_result"

	if test -f "$dir/$objdir/$dlname"; then
	  dir="$dir/$objdir"
	else
	  if test ! -f "$dir/$dlname"; then
	    func_fatal_error "cannot find \`$dlname' in \`$dir' or \`$dir/$objdir'"
	  fi
	fi
	;;

      *.lo)
	# Just add the directory containing the .lo file.
	func_dirname "$file" "" "."
	dir="$func_dirname_result"
	;;

      *)
	func_warning "\`-dlopen' is ignored for non-libtool libraries and objects"
	continue
	;;
      esac

      # Get the absolute pathname.
      absdir=`cd "$dir" && pwd`
      test -n "$absdir" && dir="$absdir"

      # Now add the directory to shlibpath_var.
      if eval "test -z \"\$$shlibpath_var\""; then
	eval "$shlibpath_var=\"\$dir\""
      else
	eval "$shlibpath_var=\"\$dir:\$$shlibpath_var\""
      fi
    done

    # This variable tells wrapper scripts just to set shlibpath_var
    # rather than running their programs.
    libtool_execute_magic="$magic"

    # Check if any of the arguments is a wrapper script.
    args=
    for file
    do
      case $file in
      -*) ;;
      *)
	# Do a test to see if this is really a libtool program.
	if func_ltwrapper_script_p "$file"; then
	  func_source "$file"
	  # Transform arg to wrapped name.
	  file="$progdir/$program"
	elif func_ltwrapper_executable_p "$file"; then
	  func_ltwrapper_scriptname "$file"
	  func_source "$func_ltwrapper_scriptname_result"
	  # Transform arg to wrapped name.
	  file="$progdir/$program"
	fi
	;;
      esac
      # Quote arguments (to preserve shell metacharacters).
      func_quote_for_eval "$file"
      args="$args $func_quote_for_eval_result"
    done

    if test "X$opt_dry_run" = Xfalse; then
      if test -n "$shlibpath_var"; then
	# Export the shlibpath_var.
	eval "export $shlibpath_var"
      fi

      # Restore saved environment variables
      for lt_var in LANG LANGUAGE LC_ALL LC_CTYPE LC_COLLATE LC_MESSAGES
      do
	eval "if test \"\${save_$lt_var+set}\" = set; then
                $lt_var=\$save_$lt_var; export $lt_var
	      else
		$lt_unset $lt_var
	      fi"
      done

      # Now prepare to actually exec the command.
      exec_cmd="\$cmd$args"
    else
      # Display what would be done.
      if test -n "$shlibpath_var"; then
	eval "\$ECHO \"\$shlibpath_var=\$$shlibpath_var\""
	$ECHO "export $shlibpath_var"
      fi
      $ECHO "$cmd$args"
      exit $EXIT_SUCCESS
    fi
}

test "$mode" = execute && func_mode_execute ${1+"$@"}


# func_mode_finish arg...
func_mode_finish ()
{
    $opt_debug
    libdirs="$nonopt"
    admincmds=

    if test -n "$finish_cmds$finish_eval" && test -n "$libdirs"; then
      for dir
      do
	libdirs="$libdirs $dir"
      done

      for libdir in $libdirs; do
	if test -n "$finish_cmds"; then
	  # Do each command in the finish commands.
	  func_execute_cmds "$finish_cmds" 'admincmds="$admincmds
'"$cmd"'"'
	fi
	if test -n "$finish_eval"; then
	  # Do the single finish_eval.
	  eval cmds=\"$finish_eval\"
	  $opt_dry_run || eval "$cmds" || admincmds="$admincmds
       $cmds"
	fi
      done
    fi

    # Exit here if they wanted silent mode.
    $opt_silent && exit $EXIT_SUCCESS

    $ECHO "X----------------------------------------------------------------------" | $Xsed
    $ECHO "Libraries have been installed in:"
    for libdir in $libdirs; do
      $ECHO "   $libdir"
    done
    $ECHO
    $ECHO "If you ever happen to want to link against installed libraries"
    $ECHO "in a given directory, LIBDIR, you must either use libtool, and"
    $ECHO "specify the full pathname of the library, or use the \`-LLIBDIR'"
    $ECHO "flag during linking and do at least one of the following:"
    if test -n "$shlibpath_var"; then
      $ECHO "   - add LIBDIR to the \`$shlibpath_var' environment variable"
      $ECHO "     during execution"
    fi
    if test -n "$runpath_var"; then
      $ECHO "   - add LIBDIR to the \`$runpath_var' environment variable"
      $ECHO "     during linking"
    fi
    if test -n "$hardcode_libdir_flag_spec"; then
      libdir=LIBDIR
      eval flag=\"$hardcode_libdir_flag_spec\"

      $ECHO "   - use the \`$flag' linker flag"
    fi
    if test -n "$admincmds"; then
      $ECHO "   - have your system administrator run these commands:$admincmds"
    fi
    if test -f /etc/ld.so.conf; then
      $ECHO "   - have your system administrator add LIBDIR to \`/etc/ld.so.conf'"
    fi
    $ECHO

    $ECHO "See any operating system documentation about shared libraries for"
    case $host in
      solaris2.[6789]|solaris2.1[0-9])
        $ECHO "more information, such as the ld(1), crle(1) and ld.so(8) manual"
	$ECHO "pages."
	;;
      *)
        $ECHO "more information, such as the ld(1) and ld.so(8) manual pages."
        ;;
    esac
    $ECHO "X----------------------------------------------------------------------" | $Xsed
    exit $EXIT_SUCCESS
}

test "$mode" = finish && func_mode_finish ${1+"$@"}


# func_mode_install arg...
func_mode_install ()
{
    $opt_debug
    # There may be an optional sh(1) argument at the beginning of
    # install_prog (especially on Windows NT).
    if test "$nonopt" = "$SHELL" || test "$nonopt" = /bin/sh ||
       # Allow the use of GNU shtool's install command.
       $ECHO "X$nonopt" | $GREP shtool >/dev/null; then
      # Aesthetically quote it.
      func_quote_for_eval "$nonopt"
      install_prog="$func_quote_for_eval_result "
      arg=$1
      shift
    else
      install_prog=
      arg=$nonopt
    fi

    # The real first argument should be the name of the installation program.
    # Aesthetically quote it.
    func_quote_for_eval "$arg"
    install_prog="$install_prog$func_quote_for_eval_result"

    # We need to accept at least all the BSD install flags.
    dest=
    files=
    opts=
    prev=
    install_type=
    isdir=no
    stripme=
    for arg
    do
      if test -n "$dest"; then
	files="$files $dest"
	dest=$arg
	continue
      fi

      case $arg in
      -d) isdir=yes ;;
      -f)
	case " $install_prog " in
	*[\\\ /]cp\ *) ;;
	*) prev=$arg ;;
	esac
	;;
      -g | -m | -o)
	prev=$arg
	;;
      -s)
	stripme=" -s"
	continue
	;;
      -*)
	;;
      *)
	# If the previous option needed an argument, then skip it.
	if test -n "$prev"; then
	  prev=
	else
	  dest=$arg
	  continue
	fi
	;;
      esac

      # Aesthetically quote the argument.
      func_quote_for_eval "$arg"
      install_prog="$install_prog $func_quote_for_eval_result"
    done

    test -z "$install_prog" && \
      func_fatal_help "you must specify an install program"

    test -n "$prev" && \
      func_fatal_help "the \`$prev' option requires an argument"

    if test -z "$files"; then
      if test -z "$dest"; then
	func_fatal_help "no file or destination specified"
      else
	func_fatal_help "you must specify a destination"
      fi
    fi

    # Strip any trailing slash from the destination.
    func_stripname '' '/' "$dest"
    dest=$func_stripname_result

    # Check to see that the destination is a directory.
    test -d "$dest" && isdir=yes
    if test "$isdir" = yes; then
      destdir="$dest"
      destname=
    else
      func_dirname_and_basename "$dest" "" "."
      destdir="$func_dirname_result"
      destname="$func_basename_result"

      # Not a directory, so check to see that there is only one file specified.
      set dummy $files; shift
      test "$#" -gt 1 && \
	func_fatal_help "\`$dest' is not a directory"
    fi
    case $destdir in
    [\\/]* | [A-Za-z]:[\\/]*) ;;
    *)
      for file in $files; do
	case $file in
	*.lo) ;;
	*)
	  func_fatal_help "\`$destdir' must be an absolute directory name"
	  ;;
	esac
      done
      ;;
    esac

    # This variable tells wrapper scripts just to set variables rather
    # than running their programs.
    libtool_install_magic="$magic"

    staticlibs=
    future_libdirs=
    current_libdirs=
    for file in $files; do

      # Do each installation.
      case $file in
      *.$libext)
	# Do the static libraries later.
	staticlibs="$staticlibs $file"
	;;

      *.la)
	# Check to see that this really is a libtool archive.
	func_lalib_unsafe_p "$file" \
	  || func_fatal_help "\`$file' is not a valid libtool archive"

	library_names=
	old_library=
	relink_command=
	func_source "$file"

	# Add the libdir to current_libdirs if it is the destination.
	if test "X$destdir" = "X$libdir"; then
	  case "$current_libdirs " in
	  *" $libdir "*) ;;
	  *) current_libdirs="$current_libdirs $libdir" ;;
	  esac
	else
	  # Note the libdir as a future libdir.
	  case "$future_libdirs " in
	  *" $libdir "*) ;;
	  *) future_libdirs="$future_libdirs $libdir" ;;
	  esac
	fi

	func_dirname "$file" "/" ""
	dir="$func_dirname_result"
	dir="$dir$objdir"

	if test -n "$relink_command"; then
	  # Determine the prefix the user has applied to our future dir.
	  inst_prefix_dir=`$ECHO "X$destdir" | $Xsed -e "s%$libdir\$%%"`

	  # Don't allow the user to place us outside of our expected
	  # location b/c this prevents finding dependent libraries that
	  # are installed to the same prefix.
	  # At present, this check doesn't affect windows .dll's that
	  # are installed into $libdir/../bin (currently, that works fine)
	  # but it's something to keep an eye on.
	  test "$inst_prefix_dir" = "$destdir" && \
	    func_fatal_error "error: cannot install \`$file' to a directory not ending in $libdir"

	  if test -n "$inst_prefix_dir"; then
	    # Stick the inst_prefix_dir data into the link command.
	    relink_command=`$ECHO "X$relink_command" | $Xsed -e "s%@inst_prefix_dir@%-inst-prefix-dir $inst_prefix_dir%"`
	  else
	    relink_command=`$ECHO "X$relink_command" | $Xsed -e "s%@inst_prefix_dir@%%"`
	  fi

	  func_warning "relinking \`$file'"
	  func_show_eval "$relink_command" \
	    'func_fatal_error "error: relink \`$file'\'' with the above command before installing it"'
	fi

	# See the names of the shared library.
	set dummy $library_names; shift
	if test -n "$1"; then
	  realname="$1"
	  shift

	  srcname="$realname"
	  test -n "$relink_command" && srcname="$realname"T

	  # Install the shared library and build the symlinks.
	  func_show_eval "$install_prog $dir/$srcname $destdir/$realname" \
	      'exit $?'
	  tstripme="$stripme"
	  case $host_os in
	  cygwin* | mingw* | pw32*)
	    case $realname in
	    *.dll.a)
	      tstripme=""
	      ;;
	    esac
	    ;;
	  esac
	  if test -n "$tstripme" && test -n "$striplib"; then
	    func_show_eval "$striplib $destdir/$realname" 'exit $?'
	  fi

	  if test "$#" -gt 0; then
	    # Delete the old symlinks, and create new ones.
	    # Try `ln -sf' first, because the `ln' binary might depend on
	    # the symlink we replace!  Solaris /bin/ln does not understand -f,
	    # so we also need to try rm && ln -s.
	    for linkname
	    do
	      test "$linkname" != "$realname" \
		&& func_show_eval "(cd $destdir && { $LN_S -f $realname $linkname || { $RM $linkname && $LN_S $realname $linkname; }; })"
	    done
	  fi

	  # Do each command in the postinstall commands.
	  lib="$destdir/$realname"
	  func_execute_cmds "$postinstall_cmds" 'exit $?'
	fi

	# Install the pseudo-library for information purposes.
	func_basename "$file"
	name="$func_basename_result"
	instname="$dir/$name"i
	func_show_eval "$install_prog $instname $destdir/$name" 'exit $?'

	# Maybe install the static library, too.
	test -n "$old_library" && staticlibs="$staticlibs $dir/$old_library"
	;;

      *.lo)
	# Install (i.e. copy) a libtool object.

	# Figure out destination file name, if it wasn't already specified.
	if test -n "$destname"; then
	  destfile="$destdir/$destname"
	else
	  func_basename "$file"
	  destfile="$func_basename_result"
	  destfile="$destdir/$destfile"
	fi

	# Deduce the name of the destination old-style object file.
	case $destfile in
	*.lo)
	  func_lo2o "$destfile"
	  staticdest=$func_lo2o_result
	  ;;
	*.$objext)
	  staticdest="$destfile"
	  destfile=
	  ;;
	*)
	  func_fatal_help "cannot copy a libtool object to \`$destfile'"
	  ;;
	esac

	# Install the libtool object if requested.
	test -n "$destfile" && \
	  func_show_eval "$install_prog $file $destfile" 'exit $?'

	# Install the old object if enabled.
	if test "$build_old_libs" = yes; then
	  # Deduce the name of the old-style object file.
	  func_lo2o "$file"
	  staticobj=$func_lo2o_result
	  func_show_eval "$install_prog \$staticobj \$staticdest" 'exit $?'
	fi
	exit $EXIT_SUCCESS
	;;

      *)
	# Figure out destination file name, if it wasn't already specified.
	if test -n "$destname"; then
	  destfile="$destdir/$destname"
	else
	  func_basename "$file"
	  destfile="$func_basename_result"
	  destfile="$destdir/$destfile"
	fi

	# If the file is missing, and there is a .exe on the end, strip it
	# because it is most likely a libtool script we actually want to
	# install
	stripped_ext=""
	case $file in
	  *.exe)
	    if test ! -f "$file"; then
	      func_stripname '' '.exe' "$file"
	      file=$func_stripname_result
	      stripped_ext=".exe"
	    fi
	    ;;
	esac

	# Do a test to see if this is really a libtool program.
	case $host in
	*cygwin*|*mingw*)
	    if func_ltwrapper_executable_p "$file"; then
	      func_ltwrapper_scriptname "$file"
	      wrapper=$func_ltwrapper_scriptname_result
	    else
	      func_stripname '' '.exe' "$file"
	      wrapper=$func_stripname_result
	    fi
	    ;;
	*)
	    wrapper=$file
	    ;;
	esac
	if func_ltwrapper_script_p "$wrapper"; then
	  notinst_deplibs=
	  relink_command=

	  func_source "$wrapper"

	  # Check the variables that should have been set.
	  test -z "$generated_by_libtool_version" && \
	    func_fatal_error "invalid libtool wrapper script \`$wrapper'"

	  finalize=yes
	  for lib in $notinst_deplibs; do
	    # Check to see that each library is installed.
	    libdir=
	    if test -f "$lib"; then
	      func_source "$lib"
	    fi
	    libfile="$libdir/"`$ECHO "X$lib" | $Xsed -e 's%^.*/%%g'` ### testsuite: skip nested quoting test
	    if test -n "$libdir" && test ! -f "$libfile"; then
	      func_warning "\`$lib' has not been installed in \`$libdir'"
	      finalize=no
	    fi
	  done

	  relink_command=
	  func_source "$wrapper"

	  outputname=
	  if test "$fast_install" = no && test -n "$relink_command"; then
	    $opt_dry_run || {
	      if test "$finalize" = yes; then
	        tmpdir=`func_mktempdir`
		func_basename "$file$stripped_ext"
		file="$func_basename_result"
	        outputname="$tmpdir/$file"
	        # Replace the output file specification.
	        relink_command=`$ECHO "X$relink_command" | $Xsed -e 's%@OUTPUT@%'"$outputname"'%g'`

	        $opt_silent || {
	          func_quote_for_expand "$relink_command"
		  eval "func_echo $func_quote_for_expand_result"
	        }
	        if eval "$relink_command"; then :
	          else
		  func_error "error: relink \`$file' with the above command before installing it"
		  $opt_dry_run || ${RM}r "$tmpdir"
		  continue
	        fi
	        file="$outputname"
	      else
	        func_warning "cannot relink \`$file'"
	      fi
	    }
	  else
	    # Install the binary that we compiled earlier.
	    file=`$ECHO "X$file$stripped_ext" | $Xsed -e "s%\([^/]*\)$%$objdir/\1%"`
	  fi
	fi

	# remove .exe since cygwin /usr/bin/install will append another
	# one anyway
	case $install_prog,$host in
	*/usr/bin/install*,*cygwin*)
	  case $file:$destfile in
	  *.exe:*.exe)
	    # this is ok
	    ;;
	  *.exe:*)
	    destfile=$destfile.exe
	    ;;
	  *:*.exe)
	    func_stripname '' '.exe' "$destfile"
	    destfile=$func_stripname_result
	    ;;
	  esac
	  ;;
	esac
	func_show_eval "$install_prog\$stripme \$file \$destfile" 'exit $?'
	$opt_dry_run || if test -n "$outputname"; then
	  ${RM}r "$tmpdir"
	fi
	;;
      esac
    done

    for file in $staticlibs; do
      func_basename "$file"
      name="$func_basename_result"

      # Set up the ranlib parameters.
      oldlib="$destdir/$name"

      func_show_eval "$install_prog \$file \$oldlib" 'exit $?'

      if test -n "$stripme" && test -n "$old_striplib"; then
	func_show_eval "$old_striplib $oldlib" 'exit $?'
      fi

      # Do each command in the postinstall commands.
      func_execute_cmds "$old_postinstall_cmds" 'exit $?'
    done

    test -n "$future_libdirs" && \
      func_warning "remember to run \`$progname --finish$future_libdirs'"

    if test -n "$current_libdirs"; then
      # Maybe just do a dry run.
      $opt_dry_run && current_libdirs=" -n$current_libdirs"
      exec_cmd='$SHELL $progpath $preserve_args --finish$current_libdirs'
    else
      exit $EXIT_SUCCESS
    fi
}

test "$mode" = install && func_mode_install ${1+"$@"}


# func_generate_dlsyms outputname originator pic_p
# Extract symbols from dlprefiles and create ${outputname}S.o with
# a dlpreopen symbol table.
func_generate_dlsyms ()
{
    $opt_debug
    my_outputname="$1"
    my_originator="$2"
    my_pic_p="${3-no}"
    my_prefix=`$ECHO "$my_originator" | sed 's%[^a-zA-Z0-9]%_%g'`
    my_dlsyms=

    if test -n "$dlfiles$dlprefiles" || test "$dlself" != no; then
      if test -n "$NM" && test -n "$global_symbol_pipe"; then
	my_dlsyms="${my_outputname}S.c"
      else
	func_error "not configured to extract global symbols from dlpreopened files"
      fi
    fi

    if test -n "$my_dlsyms"; then
      case $my_dlsyms in
      "") ;;
      *.c)
	# Discover the nlist of each of the dlfiles.
	nlist="$output_objdir/${my_outputname}.nm"

	func_show_eval "$RM $nlist ${nlist}S ${nlist}T"

	# Parse the name list into a source file.
	func_verbose "creating $output_objdir/$my_dlsyms"

	$opt_dry_run || $ECHO > "$output_objdir/$my_dlsyms" "\
/* $my_dlsyms - symbol resolution table for \`$my_outputname' dlsym emulation. */
/* Generated by $PROGRAM (GNU $PACKAGE$TIMESTAMP) $VERSION */

#ifdef __cplusplus
extern \"C\" {
#endif

/* External symbol declarations for the compiler. */\
"

	if test "$dlself" = yes; then
	  func_verbose "generating symbol list for \`$output'"

	  $opt_dry_run || echo ': @PROGRAM@ ' > "$nlist"

	  # Add our own program objects to the symbol list.
	  progfiles=`$ECHO "X$objs$old_deplibs" | $SP2NL | $Xsed -e "$lo2o" | $NL2SP`
	  for progfile in $progfiles; do
	    func_verbose "extracting global C symbols from \`$progfile'"
	    $opt_dry_run || eval "$NM $progfile | $global_symbol_pipe >> '$nlist'"
	  done

	  if test -n "$exclude_expsyms"; then
	    $opt_dry_run || {
	      eval '$EGREP -v " ($exclude_expsyms)$" "$nlist" > "$nlist"T'
	      eval '$MV "$nlist"T "$nlist"'
	    }
	  fi

	  if test -n "$export_symbols_regex"; then
	    $opt_dry_run || {
	      eval '$EGREP -e "$export_symbols_regex" "$nlist" > "$nlist"T'
	      eval '$MV "$nlist"T "$nlist"'
	    }
	  fi

	  # Prepare the list of exported symbols
	  if test -z "$export_symbols"; then
	    export_symbols="$output_objdir/$outputname.exp"
	    $opt_dry_run || {
	      $RM $export_symbols
	      eval "${SED} -n -e '/^: @PROGRAM@ $/d' -e 's/^.* \(.*\)$/\1/p' "'< "$nlist" > "$export_symbols"'
	      case $host in
	      *cygwin* | *mingw* )
                eval "echo EXPORTS "'> "$output_objdir/$outputname.def"'
                eval 'cat "$export_symbols" >> "$output_objdir/$outputname.def"'
	        ;;
	      esac
	    }
	  else
	    $opt_dry_run || {
	      eval "${SED} -e 's/\([].[*^$]\)/\\\\\1/g' -e 's/^/ /' -e 's/$/$/'"' < "$export_symbols" > "$output_objdir/$outputname.exp"'
	      eval '$GREP -f "$output_objdir/$outputname.exp" < "$nlist" > "$nlist"T'
	      eval '$MV "$nlist"T "$nlist"'
	      case $host in
	        *cygwin | *mingw* )
	          eval "echo EXPORTS "'> "$output_objdir/$outputname.def"'
	          eval 'cat "$nlist" >> "$output_objdir/$outputname.def"'
	          ;;
	      esac
	    }
	  fi
	fi

	for dlprefile in $dlprefiles; do
	  func_verbose "extracting global C symbols from \`$dlprefile'"
	  func_basename "$dlprefile"
	  name="$func_basename_result"
	  $opt_dry_run || {
	    eval '$ECHO ": $name " >> "$nlist"'
	    eval "$NM $dlprefile 2>/dev/null | $global_symbol_pipe >> '$nlist'"
	  }
	done

	$opt_dry_run || {
	  # Make sure we have at least an empty file.
	  test -f "$nlist" || : > "$nlist"

	  if test -n "$exclude_expsyms"; then
	    $EGREP -v " ($exclude_expsyms)$" "$nlist" > "$nlist"T
	    $MV "$nlist"T "$nlist"
	  fi

	  # Try sorting and uniquifying the output.
	  if $GREP -v "^: " < "$nlist" |
	      if sort -k 3 </dev/null >/dev/null 2>&1; then
		sort -k 3
	      else
		sort +2
	      fi |
	      uniq > "$nlist"S; then
	    :
	  else
	    $GREP -v "^: " < "$nlist" > "$nlist"S
	  fi

	  if test -f "$nlist"S; then
	    eval "$global_symbol_to_cdecl"' < "$nlist"S >> "$output_objdir/$my_dlsyms"'
	  else
	    $ECHO '/* NONE */' >> "$output_objdir/$my_dlsyms"
	  fi

	  $ECHO >> "$output_objdir/$my_dlsyms" "\

/* The mapping between symbol names and symbols.  */
typedef struct {
  const char *name;
  void *address;
} lt_dlsymlist;
"
	  case $host in
	  *cygwin* | *mingw* )
	    $ECHO >> "$output_objdir/$my_dlsyms" "\
/* DATA imports from DLLs on WIN32 con't be const, because
   runtime relocations are performed -- see ld's documentation
   on pseudo-relocs.  */"
	    lt_dlsym_const= ;;
	  *osf5*)
	    echo >> "$output_objdir/$my_dlsyms" "\
/* This system does not cope well with relocations in const data */"
	    lt_dlsym_const= ;;
	  *)
	    lt_dlsym_const=const ;;
	  esac

	  $ECHO >> "$output_objdir/$my_dlsyms" "\
extern $lt_dlsym_const lt_dlsymlist
lt_${my_prefix}_LTX_preloaded_symbols[];
$lt_dlsym_const lt_dlsymlist
lt_${my_prefix}_LTX_preloaded_symbols[] =
{\
  { \"$my_originator\", (void *) 0 },"

	  case $need_lib_prefix in
	  no)
	    eval "$global_symbol_to_c_name_address" < "$nlist" >> "$output_objdir/$my_dlsyms"
	    ;;
	  *)
	    eval "$global_symbol_to_c_name_address_lib_prefix" < "$nlist" >> "$output_objdir/$my_dlsyms"
	    ;;
	  esac
	  $ECHO >> "$output_objdir/$my_dlsyms" "\
  {0, (void *) 0}
};

/* This works around a problem in FreeBSD linker */
#ifdef FREEBSD_WORKAROUND
static const void *lt_preloaded_setup() {
  return lt_${my_prefix}_LTX_preloaded_symbols;
}
#endif

#ifdef __cplusplus
}
#endif\
"
	} # !$opt_dry_run

	pic_flag_for_symtable=
	case "$compile_command " in
	*" -static "*) ;;
	*)
	  case $host in
	  # compiling the symbol table file with pic_flag works around
	  # a FreeBSD bug that causes programs to crash when -lm is
	  # linked before any other PIC object.  But we must not use
	  # pic_flag when linking with -static.  The problem exists in
	  # FreeBSD 2.2.6 and is fixed in FreeBSD 3.1.
	  *-*-freebsd2*|*-*-freebsd3.0*|*-*-freebsdelf3.0*)
	    pic_flag_for_symtable=" $pic_flag -DFREEBSD_WORKAROUND" ;;
	  *-*-hpux*)
	    pic_flag_for_symtable=" $pic_flag"  ;;
	  *)
	    if test "X$my_pic_p" != Xno; then
	      pic_flag_for_symtable=" $pic_flag"
	    fi
	    ;;
	  esac
	  ;;
	esac
	symtab_cflags=
	for arg in $LTCFLAGS; do
	  case $arg in
	  -pie | -fpie | -fPIE) ;;
	  *) symtab_cflags="$symtab_cflags $arg" ;;
	  esac
	done

	# Now compile the dynamic symbol file.
	func_show_eval '(cd $output_objdir && $LTCC$symtab_cflags -c$no_builtin_flag$pic_flag_for_symtable "$my_dlsyms")' 'exit $?'

	# Clean up the generated files.
	func_show_eval '$RM "$output_objdir/$my_dlsyms" "$nlist" "${nlist}S" "${nlist}T"'

	# Transform the symbol file into the correct name.
	symfileobj="$output_objdir/${my_outputname}S.$objext"
	case $host in
	*cygwin* | *mingw* )
	  if test -f "$output_objdir/$my_outputname.def"; then
	    compile_command=`$ECHO "X$compile_command" | $Xsed -e "s%@SYMFILE@%$output_objdir/$my_outputname.def $symfileobj%"`
	    finalize_command=`$ECHO "X$finalize_command" | $Xsed -e "s%@SYMFILE@%$output_objdir/$my_outputname.def $symfileobj%"`
	  else
	    compile_command=`$ECHO "X$compile_command" | $Xsed -e "s%@SYMFILE@%$symfileobj%"`
	    finalize_command=`$ECHO "X$finalize_command" | $Xsed -e "s%@SYMFILE@%$symfileobj%"`
	  fi
	  ;;
	*)
	  compile_command=`$ECHO "X$compile_command" | $Xsed -e "s%@SYMFILE@%$symfileobj%"`
	  finalize_command=`$ECHO "X$finalize_command" | $Xsed -e "s%@SYMFILE@%$symfileobj%"`
	  ;;
	esac
	;;
      *)
	func_fatal_error "unknown suffix for \`$my_dlsyms'"
	;;
      esac
    else
      # We keep going just in case the user didn't refer to
      # lt_preloaded_symbols.  The linker will fail if global_symbol_pipe
      # really was required.

      # Nullify the symbol file.
      compile_command=`$ECHO "X$compile_command" | $Xsed -e "s% @SYMFILE@%%"`
      finalize_command=`$ECHO "X$finalize_command" | $Xsed -e "s% @SYMFILE@%%"`
    fi
}

# func_win32_libid arg
# return the library type of file 'arg'
#
# Need a lot of goo to handle *both* DLLs and import libs
# Has to be a shell function in order to 'eat' the argument
# that is supplied when $file_magic_command is called.
func_win32_libid ()
{
  $opt_debug
  win32_libid_type="unknown"
  win32_fileres=`file -L $1 2>/dev/null`
  case $win32_fileres in
  *ar\ archive\ import\ library*) # definitely import
    win32_libid_type="x86 archive import"
    ;;
  *ar\ archive*) # could be an import, or static
    if eval $OBJDUMP -f $1 | $SED -e '10q' 2>/dev/null |
       $EGREP 'file format pe-i386(.*architecture: i386)?' >/dev/null ; then
      win32_nmres=`eval $NM -f posix -A $1 |
	$SED -n -e '
	    1,100{
		/ I /{
		    s,.*,import,
		    p
		    q
		}
	    }'`
      case $win32_nmres in
      import*)  win32_libid_type="x86 archive import";;
      *)        win32_libid_type="x86 archive static";;
      esac
    fi
    ;;
  *DLL*)
    win32_libid_type="x86 DLL"
    ;;
  *executable*) # but shell scripts are "executable" too...
    case $win32_fileres in
    *MS\ Windows\ PE\ Intel*)
      win32_libid_type="x86 DLL"
      ;;
    esac
    ;;
  esac
  $ECHO "$win32_libid_type"
}



# func_extract_an_archive dir oldlib
func_extract_an_archive ()
{
    $opt_debug
    f_ex_an_ar_dir="$1"; shift
    f_ex_an_ar_oldlib="$1"
    func_show_eval "(cd \$f_ex_an_ar_dir && $AR x \"\$f_ex_an_ar_oldlib\")" 'exit $?'
    if ($AR t "$f_ex_an_ar_oldlib" | sort | sort -uc >/dev/null 2>&1); then
     :
    else
      func_fatal_error "object name conflicts in archive: $f_ex_an_ar_dir/$f_ex_an_ar_oldlib"
    fi
}


# func_extract_archives gentop oldlib ...
func_extract_archives ()
{
    $opt_debug
    my_gentop="$1"; shift
    my_oldlibs=${1+"$@"}
    my_oldobjs=""
    my_xlib=""
    my_xabs=""
    my_xdir=""

    for my_xlib in $my_oldlibs; do
      # Extract the objects.
      case $my_xlib in
	[\\/]* | [A-Za-z]:[\\/]*) my_xabs="$my_xlib" ;;
	*) my_xabs=`pwd`"/$my_xlib" ;;
      esac
      func_basename "$my_xlib"
      my_xlib="$func_basename_result"
      my_xlib_u=$my_xlib
      while :; do
        case " $extracted_archives " in
	*" $my_xlib_u "*)
	  func_arith $extracted_serial + 1
	  extracted_serial=$func_arith_result
	  my_xlib_u=lt$extracted_serial-$my_xlib ;;
	*) break ;;
	esac
      done
      extracted_archives="$extracted_archives $my_xlib_u"
      my_xdir="$my_gentop/$my_xlib_u"

      func_mkdir_p "$my_xdir"

      case $host in
      *-darwin*)
	func_verbose "Extracting $my_xabs"
	# Do not bother doing anything if just a dry run
	$opt_dry_run || {
	  darwin_orig_dir=`pwd`
	  cd $my_xdir || exit $?
	  darwin_archive=$my_xabs
	  darwin_curdir=`pwd`
	  darwin_base_archive=`basename "$darwin_archive"`
	  darwin_arches=`$LIPO -info "$darwin_archive" 2>/dev/null | $GREP Architectures 2>/dev/null || true`
	  if test -n "$darwin_arches"; then
	    darwin_arches=`$ECHO "$darwin_arches" | $SED -e 's/.*are://'`
	    darwin_arch=
	    func_verbose "$darwin_base_archive has multiple architectures $darwin_arches"
	    for darwin_arch in  $darwin_arches ; do
	      func_mkdir_p "unfat-$$/${darwin_base_archive}-${darwin_arch}"
	      $LIPO -thin $darwin_arch -output "unfat-$$/${darwin_base_archive}-${darwin_arch}/${darwin_base_archive}" "${darwin_archive}"
	      cd "unfat-$$/${darwin_base_archive}-${darwin_arch}"
	      func_extract_an_archive "`pwd`" "${darwin_base_archive}"
	      cd "$darwin_curdir"
	      $RM "unfat-$$/${darwin_base_archive}-${darwin_arch}/${darwin_base_archive}"
	    done # $darwin_arches
            ## Okay now we've a bunch of thin objects, gotta fatten them up :)
	    darwin_filelist=`find unfat-$$ -type f -name \*.o -print -o -name \*.lo -print | $SED -e "$basename" | sort -u`
	    darwin_file=
	    darwin_files=
	    for darwin_file in $darwin_filelist; do
	      darwin_files=`find unfat-$$ -name $darwin_file -print | $NL2SP`
	      $LIPO -create -output "$darwin_file" $darwin_files
	    done # $darwin_filelist
	    $RM -rf unfat-$$
	    cd "$darwin_orig_dir"
	  else
	    cd $darwin_orig_dir
	    func_extract_an_archive "$my_xdir" "$my_xabs"
	  fi # $darwin_arches
	} # !$opt_dry_run
	;;
      *)
        func_extract_an_archive "$my_xdir" "$my_xabs"
	;;
      esac
      my_oldobjs="$my_oldobjs "`find $my_xdir -name \*.$objext -print -o -name \*.lo -print | $NL2SP`
    done

    func_extract_archives_result="$my_oldobjs"
}



# func_emit_wrapper arg
#
# emit a libtool wrapper script on stdout
# don't directly open a file because we may want to
# incorporate the script contents within a cygwin/mingw
# wrapper executable.  Must ONLY be called from within
# func_mode_link because it depends on a number of variable
# set therein.
#
# arg is the value that the WRAPPER_SCRIPT_BELONGS_IN_OBJDIR
# variable will take.  If 'yes', then the emitted script
# will assume that the directory in which it is stored is
# the '.lib' directory.  This is a cygwin/mingw-specific
# behavior.
func_emit_wrapper ()
{
	func_emit_wrapper_arg1=no
	if test -n "$1" ; then
	  func_emit_wrapper_arg1=$1
	fi

	$ECHO "\
#! $SHELL

# $output - temporary wrapper script for $objdir/$outputname
# Generated by $PROGRAM (GNU $PACKAGE$TIMESTAMP) $VERSION
#
# The $output program cannot be directly executed until all the libtool
# libraries that it depends on are installed.
#
# This wrapper script should never be moved out of the build directory.
# If it is, it will not operate correctly.

# Sed substitution that helps us do robust quoting.  It backslashifies
# metacharacters that are still active within double-quoted strings.
Xsed='${SED} -e 1s/^X//'
sed_quote_subst='$sed_quote_subst'

# Be Bourne compatible
if test -n \"\${ZSH_VERSION+set}\" && (emulate sh) >/dev/null 2>&1; then
  emulate sh
  NULLCMD=:
  # Zsh 3.x and 4.x performs word splitting on \${1+\"\$@\"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '\${1+\"\$@\"}'='\"\$@\"'
  setopt NO_GLOB_SUBST
else
  case \`(set -o) 2>/dev/null\` in *posix*) set -o posix;; esac
fi
BIN_SH=xpg4; export BIN_SH # for Tru64
DUALCASE=1; export DUALCASE # for MKS sh

# The HP-UX ksh and POSIX shell print the target directory to stdout
# if CDPATH is set.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

relink_command=\"$relink_command\"

# This environment variable determines our operation mode.
if test \"\$libtool_install_magic\" = \"$magic\"; then
  # install mode needs the following variables:
  generated_by_libtool_version='$macro_version'
  notinst_deplibs='$notinst_deplibs'
else
  # When we are sourced in execute mode, \$file and \$ECHO are already set.
  if test \"\$libtool_execute_magic\" != \"$magic\"; then
    ECHO=\"$qecho\"
    file=\"\$0\"
    # Make sure echo works.
    if test \"X\$1\" = X--no-reexec; then
      # Discard the --no-reexec flag, and continue.
      shift
    elif test \"X\`{ \$ECHO '\t'; } 2>/dev/null\`\" = 'X\t'; then
      # Yippee, \$ECHO works!
      :
    else
      # Restart under the correct shell, and then maybe \$ECHO will work.
      exec $SHELL \"\$0\" --no-reexec \${1+\"\$@\"}
    fi
  fi\
"
	$ECHO "\

  # Find the directory that this script lives in.
  thisdir=\`\$ECHO \"X\$file\" | \$Xsed -e 's%/[^/]*$%%'\`
  test \"x\$thisdir\" = \"x\$file\" && thisdir=.

  # Follow symbolic links until we get to the real thisdir.
  file=\`ls -ld \"\$file\" | ${SED} -n 's/.*-> //p'\`
  while test -n \"\$file\"; do
    destdir=\`\$ECHO \"X\$file\" | \$Xsed -e 's%/[^/]*\$%%'\`

    # If there was a directory component, then change thisdir.
    if test \"x\$destdir\" != \"x\$file\"; then
      case \"\$destdir\" in
      [\\\\/]* | [A-Za-z]:[\\\\/]*) thisdir=\"\$destdir\" ;;
      *) thisdir=\"\$thisdir/\$destdir\" ;;
      esac
    fi

    file=\`\$ECHO \"X\$file\" | \$Xsed -e 's%^.*/%%'\`
    file=\`ls -ld \"\$thisdir/\$file\" | ${SED} -n 's/.*-> //p'\`
  done

  # Usually 'no', except on cygwin/mingw when embedded into
  # the cwrapper.
  WRAPPER_SCRIPT_BELONGS_IN_OBJDIR=$func_emit_wrapper_arg1
  if test \"\$WRAPPER_SCRIPT_BELONGS_IN_OBJDIR\" = \"yes\"; then
    # special case for '.'
    if test \"\$thisdir\" = \".\"; then
      thisdir=\`pwd\`
    fi
    # remove .libs from thisdir
    case \"\$thisdir\" in
    *[\\\\/]$objdir ) thisdir=\`\$ECHO \"X\$thisdir\" | \$Xsed -e 's%[\\\\/][^\\\\/]*$%%'\` ;;
    $objdir )   thisdir=. ;;
    esac
  fi

  # Try to get the absolute directory name.
  absdir=\`cd \"\$thisdir\" && pwd\`
  test -n \"\$absdir\" && thisdir=\"\$absdir\"
"

	if test "$fast_install" = yes; then
	  $ECHO "\
  program=lt-'$outputname'$exeext
  progdir=\"\$thisdir/$objdir\"

  if test ! -f \"\$progdir/\$program\" ||
     { file=\`ls -1dt \"\$progdir/\$program\" \"\$progdir/../\$program\" 2>/dev/null | ${SED} 1q\`; \\
       test \"X\$file\" != \"X\$progdir/\$program\"; }; then

    file=\"\$\$-\$program\"

    if test ! -d \"\$progdir\"; then
      $MKDIR \"\$progdir\"
    else
      $RM \"\$progdir/\$file\"
    fi"

	  $ECHO "\

    # relink executable if necessary
    if test -n \"\$relink_command\"; then
      if relink_command_output=\`eval \$relink_command 2>&1\`; then :
      else
	$ECHO \"\$relink_command_output\" >&2
	$RM \"\$progdir/\$file\"
	exit 1
      fi
    fi

    $MV \"\$progdir/\$file\" \"\$progdir/\$program\" 2>/dev/null ||
    { $RM \"\$progdir/\$program\";
      $MV \"\$progdir/\$file\" \"\$progdir/\$program\"; }
    $RM \"\$progdir/\$file\"
  fi"
	else
	  $ECHO "\
  program='$outputname'
  progdir=\"\$thisdir/$objdir\"
"
	fi

	$ECHO "\

  if test -f \"\$progdir/\$program\"; then"

	# Export our shlibpath_var if we have one.
	if test "$shlibpath_overrides_runpath" = yes && test -n "$shlibpath_var" && test -n "$temp_rpath"; then
	  $ECHO "\
    # Add our own library path to $shlibpath_var
    $shlibpath_var=\"$temp_rpath\$$shlibpath_var\"

    # Some systems cannot cope with colon-terminated $shlibpath_var
    # The second colon is a workaround for a bug in BeOS R4 sed
    $shlibpath_var=\`\$ECHO \"X\$$shlibpath_var\" | \$Xsed -e 's/::*\$//'\`

    export $shlibpath_var
"
	fi

	# fixup the dll searchpath if we need to.
	if test -n "$dllsearchpath"; then
	  $ECHO "\
    # Add the dll search path components to the executable PATH
    PATH=$dllsearchpath:\$PATH
"
	fi

	$ECHO "\
    if test \"\$libtool_execute_magic\" != \"$magic\"; then
      # Run the actual program with our arguments.
"
	case $host in
	# Backslashes separate directories on plain windows
	*-*-mingw | *-*-os2*)
	  $ECHO "\
      exec \"\$progdir\\\\\$program\" \${1+\"\$@\"}
"
	  ;;

	*)
	  $ECHO "\
      exec \"\$progdir/\$program\" \${1+\"\$@\"}
"
	  ;;
	esac
	$ECHO "\
      \$ECHO \"\$0: cannot exec \$program \$*\" 1>&2
      exit 1
    fi
  else
    # The program doesn't exist.
    \$ECHO \"\$0: error: \\\`\$progdir/\$program' does not exist\" 1>&2
    \$ECHO \"This script is just a wrapper for \$program.\" 1>&2
    $ECHO \"See the $PACKAGE documentation for more information.\" 1>&2
    exit 1
  fi
fi\
"
}
# end: func_emit_wrapper

# func_emit_cwrapperexe_src
# emit the source code for a wrapper executable on stdout
# Must ONLY be called from within func_mode_link because
# it depends on a number of variable set therein.
func_emit_cwrapperexe_src ()
{
	cat <<EOF

/* $cwrappersource - temporary wrapper executable for $objdir/$outputname
   Generated by $PROGRAM (GNU $PACKAGE$TIMESTAMP) $VERSION

   The $output program cannot be directly executed until all the libtool
   libraries that it depends on are installed.

   This wrapper executable should never be moved out of the build directory.
   If it is, it will not operate correctly.

   Currently, it simply execs the wrapper *script* "$SHELL $output",
   but could eventually absorb all of the scripts functionality and
   exec $objdir/$outputname directly.
*/
EOF
	    cat <<"EOF"
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
# include <direct.h>
# include <process.h>
# include <io.h>
# define setmode _setmode
#else
# include <unistd.h>
# include <stdint.h>
# ifdef __CYGWIN__
#  include <io.h>
# endif
#endif
#include <malloc.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#if defined(PATH_MAX)
# define LT_PATHMAX PATH_MAX
#elif defined(MAXPATHLEN)
# define LT_PATHMAX MAXPATHLEN
#else
# define LT_PATHMAX 1024
#endif

#ifndef S_IXOTH
# define S_IXOTH 0
#endif
#ifndef S_IXGRP
# define S_IXGRP 0
#endif

#ifdef _MSC_VER
# define S_IXUSR _S_IEXEC
# define stat _stat
# ifndef _INTPTR_T_DEFINED
#  define intptr_t int
# endif
#endif

#ifndef DIR_SEPARATOR
# define DIR_SEPARATOR '/'
# define PATH_SEPARATOR ':'
#endif

#if defined (_WIN32) || defined (__MSDOS__) || defined (__DJGPP__) || \
  defined (__OS2__)
# define HAVE_DOS_BASED_FILE_SYSTEM
# define FOPEN_WB "wb"
# ifndef DIR_SEPARATOR_2
#  define DIR_SEPARATOR_2 '\\'
# endif
# ifndef PATH_SEPARATOR_2
#  define PATH_SEPARATOR_2 ';'
# endif
#endif

#ifndef DIR_SEPARATOR_2
# define IS_DIR_SEPARATOR(ch) ((ch) == DIR_SEPARATOR)
#else /* DIR_SEPARATOR_2 */
# define IS_DIR_SEPARATOR(ch) \
	(((ch) == DIR_SEPARATOR) || ((ch) == DIR_SEPARATOR_2))
#endif /* DIR_SEPARATOR_2 */

#ifndef PATH_SEPARATOR_2
# define IS_PATH_SEPARATOR(ch) ((ch) == PATH_SEPARATOR)
#else /* PATH_SEPARATOR_2 */
# define IS_PATH_SEPARATOR(ch) ((ch) == PATH_SEPARATOR_2)
#endif /* PATH_SEPARATOR_2 */

#ifdef __CYGWIN__
# define FOPEN_WB "wb"
#endif

#ifndef FOPEN_WB
# define FOPEN_WB "w"
#endif
#ifndef _O_BINARY
# define _O_BINARY 0
#endif

#define XMALLOC(type, num)      ((type *) xmalloc ((num) * sizeof(type)))
#define XFREE(stale) do { \
  if (stale) { free ((void *) stale); stale = 0; } \
} while (0)

#undef LTWRAPPER_DEBUGPRINTF
#if defined DEBUGWRAPPER
# define LTWRAPPER_DEBUGPRINTF(args) ltwrapper_debugprintf args
static void
ltwrapper_debugprintf (const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    (void) vfprintf (stderr, fmt, args);
    va_end (args);
}
#else
# define LTWRAPPER_DEBUGPRINTF(args)
#endif

const char *program_name = NULL;

void *xmalloc (size_t num);
char *xstrdup (const char *string);
const char *base_name (const char *name);
char *find_executable (const char *wrapper);
char *chase_symlinks (const char *pathspec);
int make_executable (const char *path);
int check_executable (const char *path);
char *strendzap (char *str, const char *pat);
void lt_fatal (const char *message, ...);

static const char *script_text =
EOF

	    func_emit_wrapper yes |
	        $SED -e 's/\([\\"]\)/\\\1/g' \
	             -e 's/^/  "/' -e 's/$/\\n"/'
	    echo ";"

	    cat <<EOF
const char * MAGIC_EXE = "$magic_exe";

int
main (int argc, char *argv[])
{
  char **newargz;
  char *tmp_pathspec;
  char *actual_cwrapper_path;
  char *shwrapper_name;
  intptr_t rval = 127;
  FILE *shwrapper;

  const char *dumpscript_opt = "--lt-dump-script";
  int i;

  program_name = (char *) xstrdup (base_name (argv[0]));
  LTWRAPPER_DEBUGPRINTF (("(main) argv[0]      : %s\n", argv[0]));
  LTWRAPPER_DEBUGPRINTF (("(main) program_name : %s\n", program_name));

  /* very simple arg parsing; don't want to rely on getopt */
  for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], dumpscript_opt) == 0)
	{
EOF
	    case "$host" in
	      *mingw* | *cygwin* )
		# make stdout use "unix" line endings
		echo "          setmode(1,_O_BINARY);"
		;;
	      esac

	    cat <<EOF
	  printf ("%s", script_text);
	  return 0;
	}
    }

  newargz = XMALLOC (char *, argc + 2);
EOF

	    if test -n "$TARGETSHELL" ; then
	      # no path translation at all
	      lt_newargv0=$TARGETSHELL
	    else
	      case "$host" in
		*mingw* )
		  # awkward: cmd appends spaces to result
		  lt_sed_strip_trailing_spaces="s/[ ]*\$//"
		  lt_newargv0=`( cmd //c echo $SHELL | $SED -e "$lt_sed_strip_trailing_spaces" ) 2>/dev/null || echo $SHELL`
		  case $lt_newargv0 in
		    *.exe | *.EXE) ;;
		    *) lt_newargv0=$lt_newargv0.exe ;;
		  esac
		  ;;
		* ) lt_newargv0=$SHELL ;;
	      esac
	    fi

		cat <<EOF
  newargz[0] = (char *) xstrdup ("$lt_newargv0");
EOF

	    cat <<"EOF"
  tmp_pathspec = find_executable (argv[0]);
  if (tmp_pathspec == NULL)
    lt_fatal ("Couldn't find %s", argv[0]);
  LTWRAPPER_DEBUGPRINTF (("(main) found exe (before symlink chase) at : %s\n",
			  tmp_pathspec));

  actual_cwrapper_path = chase_symlinks (tmp_pathspec);
  LTWRAPPER_DEBUGPRINTF (("(main) found exe (after symlink chase) at : %s\n",
			  actual_cwrapper_path));
  XFREE (tmp_pathspec);

  shwrapper_name = (char *) xstrdup (base_name (actual_cwrapper_path));
  strendzap (actual_cwrapper_path, shwrapper_name);

  /* shwrapper_name transforms */
  strendzap (shwrapper_name, ".exe");
  tmp_pathspec = XMALLOC (char, (strlen (shwrapper_name) +
				 strlen ("_ltshwrapperTMP") + 1));
  strcpy (tmp_pathspec, shwrapper_name);
  strcat (tmp_pathspec, "_ltshwrapperTMP");
  XFREE (shwrapper_name);
  shwrapper_name = tmp_pathspec;
  tmp_pathspec = 0;
  LTWRAPPER_DEBUGPRINTF (("(main) libtool shell wrapper name: %s\n",
			  shwrapper_name));
EOF

	    cat <<EOF
  newargz[1] =
    XMALLOC (char, (strlen (actual_cwrapper_path) +
		    strlen ("$objdir") + 1 + strlen (shwrapper_name) + 1));
  strcpy (newargz[1], actual_cwrapper_path);
  strcat (newargz[1], "$objdir");
  strcat (newargz[1], "/");
  strcat (newargz[1], shwrapper_name);
EOF


	    case $host_os in
	      mingw*)
	    cat <<"EOF"
  {
    char* p;
    while ((p = strchr (newargz[1], '\\')) != NULL)
      {
	*p = '/';
      }
  }
EOF
	    ;;
	    esac

	    cat <<"EOF"
  XFREE (shwrapper_name);
  XFREE (actual_cwrapper_path);

  /* always write in binary mode */
  if ((shwrapper = fopen (newargz[1], FOPEN_WB)) == 0)
    {
      lt_fatal ("Could not open %s for writing", newargz[1]);
    }
  fprintf (shwrapper, "%s", script_text);
  fclose (shwrapper);

  make_executable (newargz[1]);

  for (i = 1; i < argc; i++)
    newargz[i + 1] = xstrdup (argv[i]);
  newargz[argc + 1] = NULL;

  for (i = 0; i < argc + 1; i++)
    {
      LTWRAPPER_DEBUGPRINTF (("(main) newargz[%d]   : %s\n", i, newargz[i]));
    }

EOF

	    case $host_os in
	      mingw*)
		cat <<EOF
  /* execv doesn't actually work on mingw as expected on unix */
  rval = _spawnv (_P_WAIT, "$lt_newargv0", (const char * const *) newargz);
  if (rval == -1)
    {
      /* failed to start process */
      LTWRAPPER_DEBUGPRINTF (("(main) failed to launch target \"$lt_newargv0\": errno = %d\n", errno));
      return 127;
    }
  return rval;
}
EOF
		;;
	      *)
		cat <<EOF
  execv ("$lt_newargv0", newargz);
  return rval; /* =127, but avoids unused variable warning */
}
EOF
		;;
	    esac

	    cat <<"EOF"

void *
xmalloc (size_t num)
{
  void *p = (void *) malloc (num);
  if (!p)
    lt_fatal ("Memory exhausted");

  return p;
}

char *
xstrdup (const char *string)
{
  return string ? strcpy ((char *) xmalloc (strlen (string) + 1),
			  string) : NULL;
}

const char *
base_name (const char *name)
{
  const char *base;

#if defined (HAVE_DOS_BASED_FILE_SYSTEM)
  /* Skip over the disk name in MSDOS pathnames. */
  if (isalpha ((unsigned char) name[0]) && name[1] == ':')
    name += 2;
#endif

  for (base = name; *name; name++)
    if (IS_DIR_SEPARATOR (*name))
      base = name + 1;
  return base;
}

int
check_executable (const char *path)
{
  struct stat st;

  LTWRAPPER_DEBUGPRINTF (("(check_executable)  : %s\n",
			  path ? (*path ? path : "EMPTY!") : "NULL!"));
  if ((!path) || (!*path))
    return 0;

  if ((stat (path, &st) >= 0)
      && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
    return 1;
  else
    return 0;
}

int
make_executable (const char *path)
{
  int rval = 0;
  struct stat st;

  LTWRAPPER_DEBUGPRINTF (("(make_executable)   : %s\n",
			  path ? (*path ? path : "EMPTY!") : "NULL!"));
  if ((!path) || (!*path))
    return 0;

  if (stat (path, &st) >= 0)
    {
      rval = chmod (path, st.st_mode | S_IXOTH | S_IXGRP | S_IXUSR);
    }
  return rval;
}

/* Searches for the full path of the wrapper.  Returns
   newly allocated full path name if found, NULL otherwise
   Does not chase symlinks, even on platforms that support them.
*/
char *
find_executable (const char *wrapper)
{
  int has_slash = 0;
  const char *p;
  const char *p_next;
  /* static buffer for getcwd */
  char tmp[LT_PATHMAX + 1];
  int tmp_len;
  char *concat_name;

  LTWRAPPER_DEBUGPRINTF (("(find_executable)   : %s\n",
			  wrapper ? (*wrapper ? wrapper : "EMPTY!") : "NULL!"));

  if ((wrapper == NULL) || (*wrapper == '\0'))
    return NULL;

  /* Absolute path? */
#if defined (HAVE_DOS_BASED_FILE_SYSTEM)
  if (isalpha ((unsigned char) wrapper[0]) && wrapper[1] == ':')
    {
      concat_name = xstrdup (wrapper);
      if (check_executable (concat_name))
	return concat_name;
      XFREE (concat_name);
    }
  else
    {
#endif
      if (IS_DIR_SEPARATOR (wrapper[0]))
	{
	  concat_name = xstrdup (wrapper);
	  if (check_executable (concat_name))
	    return concat_name;
	  XFREE (concat_name);
	}
#if defined (HAVE_DOS_BASED_FILE_SYSTEM)
    }
#endif

  for (p = wrapper; *p; p++)
    if (*p == '/')
      {
	has_slash = 1;
	break;
      }
  if (!has_slash)
    {
      /* no slashes; search PATH */
      const char *path = getenv ("PATH");
      if (path != NULL)
	{
	  for (p = path; *p; p = p_next)
	    {
	      const char *q;
	      size_t p_len;
	      for (q = p; *q; q++)
		if (IS_PATH_SEPARATOR (*q))
		  break;
	      p_len = q - p;
	      p_next = (*q == '\0' ? q : q + 1);
	      if (p_len == 0)
		{
		  /* empty path: current directory */
		  if (getcwd (tmp, LT_PATHMAX) == NULL)
		    lt_fatal ("getcwd failed");
		  tmp_len = strlen (tmp);
		  concat_name =
		    XMALLOC (char, tmp_len + 1 + strlen (wrapper) + 1);
		  memcpy (concat_name, tmp, tmp_len);
		  concat_name[tmp_len] = '/';
		  strcpy (concat_name + tmp_len + 1, wrapper);
		}
	      else
		{
		  concat_name =
		    XMALLOC (char, p_len + 1 + strlen (wrapper) + 1);
		  memcpy (concat_name, p, p_len);
		  concat_name[p_len] = '/';
		  strcpy (concat_name + p_len + 1, wrapper);
		}
	      if (check_executable (concat_name))
		return concat_name;
	      XFREE (concat_name);
	    }
	}
      /* not found in PATH; assume curdir */
    }
  /* Relative path | not found in path: prepend cwd */
  if (getcwd (tmp, LT_PATHMAX) == NULL)
    lt_fatal ("getcwd failed");
  tmp_len = strlen (tmp);
  concat_name = XMALLOC (char, tmp_len + 1 + strlen (wrapper) + 1);
  memcpy (concat_name, tmp, tmp_len);
  concat_name[tmp_len] = '/';
  strcpy (concat_name + tmp_len + 1, wrapper);

  if (check_executable (concat_name))
    return concat_name;
  XFREE (concat_name);
  return NULL;
}

char *
chase_symlinks (const char *pathspec)
{
#ifndef S_ISLNK
  return xstrdup (pathspec);
#else
  char buf[LT_PATHMAX];
  struct stat s;
  char *tmp_pathspec = xstrdup (pathspec);
  char *p;
  int has_symlinks = 0;
  while (strlen (tmp_pathspec) && !has_symlinks)
    {
      LTWRAPPER_DEBUGPRINTF (("checking path component for symlinks: %s\n",
			      tmp_pathspec));
      if (lstat (tmp_pathspec, &s) == 0)
	{
	  if (S_ISLNK (s.st_mode) != 0)
	    {
	      has_symlinks = 1;
	      break;
	    }

	  /* search backwards for last DIR_SEPARATOR */
	  p = tmp_pathspec + strlen (tmp_pathspec) - 1;
	  while ((p > tmp_pathspec) && (!IS_DIR_SEPARATOR (*p)))
	    p--;
	  if ((p == tmp_pathspec) && (!IS_DIR_SEPARATOR (*p)))
	    {
	      /* no more DIR_SEPARATORS left */
	      break;
	    }
	  *p = '\0';
	}
      else
	{
	  char *errstr = strerror (errno);
	  lt_fatal ("Error accessing file %s (%s)", tmp_pathspec, errstr);
	}
    }
  XFREE (tmp_pathspec);

  if (!has_symlinks)
    {
      return xstrdup (pathspec);
    }

  tmp_pathspec = realpath (pathspec, buf);
  if (tmp_pathspec == 0)
    {
      lt_fatal ("Could not follow symlinks for %s", pathspec);
    }
  return xstrdup (tmp_pathspec);
#endif
}

char *
strendzap (char *str, const char *pat)
{
  size_t len, patlen;

  assert (str != NULL);
  assert (pat != NULL);

  len = strlen (str);
  patlen = strlen (pat);

  if (patlen <= len)
    {
      str += len - patlen;
      if (strcmp (str, pat) == 0)
	*str = '\0';
    }
  return str;
}

static void
lt_error_core (int exit_status, const char *mode,
	       const char *message, va_list ap)
{
  fprintf (stderr, "%s: %s: ", program_name, mode);
  vfprintf (stderr, message, ap);
  fprintf (stderr, ".\n");

  if (exit_status >= 0)
    exit (exit_status);
}

void
lt_fatal (const char *message, ...)
{
  va_list ap;
  va_start (ap, message);
  lt_error_core (EXIT_FAILURE, "FATAL", message, ap);
  va_end (ap);
}
EOF
}
# end: func_emit_cwrapperexe_src

# func_mode_link arg...
func_mode_link ()
{
    $opt_debug
    case $host in
    *-*-cygwin* | *-*-mingw* | *-*-pw32* | *-*-os2*)
      # It is impossible to link a dll without this setting, and
      # we shouldn't force the makefile maintainer to figure out
      # which system we are compiling for in order to pass an extra
      # flag for every libtool invocation.
      # allow_undefined=no

      # FIXME: Unfortunately, there are problems with the above when trying
      # to make a dll which has undefined symbols, in which case not
      # even a static library is built.  For now, we need to specify
      # -no-undefined on the libtool link line when we can be certain
      # that all symbols are satisfied, otherwise we get a static library.
      allow_undefined=yes
      ;;
    *)
      allow_undefined=yes
      ;;
    esac
    libtool_args=$nonopt
    base_compile="$nonopt $@"
    compile_command=$nonopt
    finalize_command=$nonopt

    compile_rpath=
    finalize_rpath=
    compile_shlibpath=
    finalize_shlibpath=
    convenience=
    old_convenience=
    deplibs=
    old_deplibs=
    compiler_flags=
    linker_flags=
    dllsearchpath=
    lib_search_path=`pwd`
    inst_prefix_dir=
    new_inherited_linker_flags=

    avoid_version=no
    dlfiles=
    dlprefiles=
    dlself=no
    export_dynamic=no
    export_symbols=
    export_symbols_regex=
    generated=
    libobjs=
    ltlibs=
    module=no
    no_install=no
    objs=
    non_pic_objects=
    precious_files_regex=
    prefer_static_libs=no
    preload=no
    prev=
    prevarg=
    release=
    rpath=
    xrpath=
    perm_rpath=
    temp_rpath=
    thread_safe=no
    vinfo=
    vinfo_number=no
    weak_libs=
    single_module="${wl}-single_module"
    func_infer_tag $base_compile

    # We need to know -static, to get the right output filenames.
    for arg
    do
      case $arg in
      -shared)
	test "$build_libtool_libs" != yes && \
	  func_fatal_configuration "can not build a shared library"
	build_old_libs=no
	break
	;;
      -all-static | -static | -static-libtool-libs)
	case $arg in
	-all-static)
	  if test "$build_libtool_libs" = yes && test -z "$link_static_flag"; then
	    func_warning "complete static linking is impossible in this configuration"
	  fi
	  if test -n "$link_static_flag"; then
	    dlopen_self=$dlopen_self_static
	  fi
	  prefer_static_libs=yes
	  ;;
	-static)
	  if test -z "$pic_flag" && test -n "$link_static_flag"; then
	    dlopen_self=$dlopen_self_static
	  fi
	  prefer_static_libs=built
	  ;;
	-static-libtool-libs)
	  if test -z "$pic_flag" && test -n "$link_static_flag"; then
	    dlopen_self=$dlopen_self_static
	  fi
	  prefer_static_libs=yes
	  ;;
	esac
	build_libtool_libs=no
	build_old_libs=yes
	break
	;;
      esac
    done

    # See if our shared archives depend on static archives.
    test -n "$old_archive_from_new_cmds" && build_old_libs=yes

    # Go through the arguments, transforming them on the way.
    while test "$#" -gt 0; do
      arg="$1"
      shift
      func_quote_for_eval "$arg"
      qarg=$func_quote_for_eval_unquoted_result
      func_append libtool_args " $func_quote_for_eval_result"

      # If the previous option needs an argument, assign it.
      if test -n "$prev"; then
	case $prev in
	output)
	  func_append compile_command " @OUTPUT@"
	  func_append finalize_command " @OUTPUT@"
	  ;;
	esac

	case $prev in
	dlfiles|dlprefiles)
	  if test "$preload" = no; then
	    # Add the symbol object into the linking commands.
	    func_append compile_command " @SYMFILE@"
	    func_append finalize_command " @SYMFILE@"
	    preload=yes
	  fi
	  case $arg in
	  *.la | *.lo) ;;  # We handle these cases below.
	  force)
	    if test "$dlself" = no; then
	      dlself=needless
	      export_dynamic=yes
	    fi
	    prev=
	    continue
	    ;;
	  self)
	    if test "$prev" = dlprefiles; then
	      dlself=yes
	    elif test "$prev" = dlfiles && test "$dlopen_self" != yes; then
	      dlself=yes
	    else
	      dlself=needless
	      export_dynamic=yes
	    fi
	    prev=
	    continue
	    ;;
	  *)
	    if test "$prev" = dlfiles; then
	      dlfiles="$dlfiles $arg"
	    else
	      dlprefiles="$dlprefiles $arg"
	    fi
	    prev=
	    continue
	    ;;
	  esac
	  ;;
	expsyms)
	  export_symbols="$arg"
	  test -f "$arg" \
	    || func_fatal_error "symbol file \`$arg' does not exist"
	  prev=
	  continue
	  ;;
	expsyms_regex)
	  export_symbols_regex="$arg"
	  prev=
	  continue
	  ;;
	framework)
	  case $host in
	    *-*-darwin*)
	      case "$deplibs " in
		*" $qarg.ltframework "*) ;;
		*) deplibs="$deplibs $qarg.ltframework" # this is fixed later
		   ;;
	      esac
	      ;;
	  esac
	  prev=
	  continue
	  ;;
	inst_prefix)
	  inst_prefix_dir="$arg"
	  prev=
	  continue
	  ;;
	objectlist)
	  if test -f "$arg"; then
	    save_arg=$arg
	    moreargs=
	    for fil in `cat "$save_arg"`
	    do
#	      moreargs="$moreargs $fil"
	      arg=$fil
	      # A libtool-controlled object.

	      # Check to see that this really is a libtool object.
	      if func_lalib_unsafe_p "$arg"; then
		pic_object=
		non_pic_object=

		# Read the .lo file
		func_source "$arg"

		if test -z "$pic_object" ||
		   test -z "$non_pic_object" ||
		   test "$pic_object" = none &&
		   test "$non_pic_object" = none; then
		  func_fatal_error "cannot find name of object for \`$arg'"
		fi

		# Extract subdirectory from the argument.
		func_dirname "$arg" "/" ""
		xdir="$func_dirname_result"

		if test "$pic_object" != none; then
		  # Prepend the subdirectory the object is found in.
		  pic_object="$xdir$pic_object"

		  if test "$prev" = dlfiles; then
		    if test "$build_libtool_libs" = yes && test "$dlopen_support" = yes; then
		      dlfiles="$dlfiles $pic_object"
		      prev=
		      continue
		    else
		      # If libtool objects are unsupported, then we need to preload.
		      prev=dlprefiles
		    fi
		  fi

		  # CHECK ME:  I think I busted this.  -Ossama
		  if test "$prev" = dlprefiles; then
		    # Preload the old-style object.
		    dlprefiles="$dlprefiles $pic_object"
		    prev=
		  fi

		  # A PIC object.
		  func_append libobjs " $pic_object"
		  arg="$pic_object"
		fi

		# Non-PIC object.
		if test "$non_pic_object" != none; then
		  # Prepend the subdirectory the object is found in.
		  non_pic_object="$xdir$non_pic_object"

		  # A standard non-PIC object
		  func_append non_pic_objects " $non_pic_object"
		  if test -z "$pic_object" || test "$pic_object" = none ; then
		    arg="$non_pic_object"
		  fi
		else
		  # If the PIC object exists, use it instead.
		  # $xdir was prepended to $pic_object above.
		  non_pic_object="$pic_object"
		  func_append non_pic_objects " $non_pic_object"
		fi
	      else
		# Only an error if not doing a dry-run.
		if $opt_dry_run; then
		  # Extract subdirectory from the argument.
		  func_dirname "$arg" "/" ""
		  xdir="$func_dirname_result"

		  func_lo2o "$arg"
		  pic_object=$xdir$objdir/$func_lo2o_result
		  non_pic_object=$xdir$func_lo2o_result
		  func_append libobjs " $pic_object"
		  func_append non_pic_objects " $non_pic_object"
	        else
		  func_fatal_error "\`$arg' is not a valid libtool object"
		fi
	      fi
	    done
	  else
	    func_fatal_error "link input file \`$arg' does not exist"
	  fi
	  arg=$save_arg
	  prev=
	  continue
	  ;;
	precious_regex)
	  precious_files_regex="$arg"
	  prev=
	  continue
	  ;;
	release)
	  release="-$arg"
	  prev=
	  continue
	  ;;
	rpath | xrpath)
	  # We need an absolute path.
	  case $arg in
	  [\\/]* | [A-Za-z]:[\\/]*) ;;
	  *)
	    func_fatal_error "only absolute run-paths are allowed"
	    ;;
	  esac
	  if test "$prev" = rpath; then
	    case "$rpath " in
	    *" $arg "*) ;;
	    *) rpath="$rpath $arg" ;;
	    esac
	  else
	    case "$xrpath " in
	    *" $arg "*) ;;
	    *) xrpath="$xrpath $arg" ;;
	    esac
	  fi
	  prev=
	  continue
	  ;;
	shrext)
	  shrext_cmds="$arg"
	  prev=
	  continue
	  ;;
	weak)
	  weak_libs="$weak_libs $arg"
	  prev=
	  continue
	  ;;
	xcclinker)
	  linker_flags="$linker_flags $qarg"
	  compiler_flags="$compiler_flags $qarg"
	  prev=
	  func_append compile_command " $qarg"
	  func_append finalize_command " $qarg"
	  continue
	  ;;
	xcompiler)
	  compiler_flags="$compiler_flags $qarg"
	  prev=
	  func_append compile_command " $qarg"
	  func_append finalize_command " $qarg"
	  continue
	  ;;
	xlinker)
	  linker_flags="$linker_flags $qarg"
	  compiler_flags="$compiler_flags $wl$qarg"
	  prev=
	  func_append compile_command " $wl$qarg"
	  func_append finalize_command " $wl$qarg"
	  continue
	  ;;
	*)
	  eval "$prev=\"\$arg\""
	  prev=
	  continue
	  ;;
	esac
      fi # test -n "$prev"

      prevarg="$arg"

      case $arg in
      -all-static)
	if test -n "$link_static_flag"; then
	  # See comment for -static flag below, for more details.
	  func_append compile_command " $link_static_flag"
	  func_append finalize_command " $link_static_flag"
	fi
	continue
	;;

      -allow-undefined)
	# FIXME: remove this flag sometime in the future.
	func_fatal_error "\`-allow-undefined' must not be used because it is the default"
	;;

      -avoid-version)
	avoid_version=yes
	continue
	;;

      -dlopen)
	prev=dlfiles
	continue
	;;

      -dlpreopen)
	prev=dlprefiles
	continue
	;;

      -export-dynamic)
	export_dynamic=yes
	continue
	;;

      -export-symbols | -export-symbols-regex)
	if test -n "$export_symbols" || test -n "$export_symbols_regex"; then
	  func_fatal_error "more than one -exported-symbols argument is not allowed"
	fi
	if test "X$arg" = "X-export-symbols"; then
	  prev=expsyms
	else
	  prev=expsyms_regex
	fi
	continue
	;;

      -framework)
	prev=framework
	continue
	;;

      -inst-prefix-dir)
	prev=inst_prefix
	continue
	;;

      # The native IRIX linker understands -LANG:*, -LIST:* and -LNO:*
      # so, if we see these flags be careful not to treat them like -L
      -L[A-Z][A-Z]*:*)
	case $with_gcc/$host in
	no/*-*-irix* | /*-*-irix*)
	  func_append compile_command " $arg"
	  func_append finalize_command " $arg"
	  ;;
	esac
	continue
	;;

      -L*)
	func_stripname '-L' '' "$arg"
	dir=$func_stripname_result
	# We need an absolute path.
	case $dir in
	[\\/]* | [A-Za-z]:[\\/]*) ;;
	*)
	  absdir=`cd "$dir" && pwd`
	  test -z "$absdir" && \
	    func_fatal_error "cannot determine absolute directory name of \`$dir'"
	  dir="$absdir"
	  ;;
	esac
	case "$deplibs " in
	*" -L$dir "*) ;;
	*)
	  deplibs="$deplibs -L$dir"
	  lib_search_path="$lib_search_path $dir"
	  ;;
	esac
	case $host in
	*-*-cygwin* | *-*-mingw* | *-*-pw32* | *-*-os2*)
	  testbindir=`$ECHO "X$dir" | $Xsed -e 's*/lib$*/bin*'`
	  case :$dllsearchpath: in
	  *":$dir:"*) ;;
	  *) dllsearchpath="$dllsearchpath:$dir";;
	  esac
	  case :$dllsearchpath: in
	  *":$testbindir:"*) ;;
	  *) dllsearchpath="$dllsearchpath:$testbindir";;
	  esac
	  ;;
	esac
	continue
	;;

      -l*)
	if test "X$arg" = "X-lc" || test "X$arg" = "X-lm"; then
	  case $host in
	  *-*-cygwin* | *-*-mingw* | *-*-pw32* | *-*-beos*)
	    # These systems don't actually have a C or math library (as such)
	    continue
	    ;;
	  *-*-os2*)
	    # These systems don't actually have a C library (as such)
	    test "X$arg" = "X-lc" && continue
	    ;;
	  *-*-openbsd* | *-*-freebsd* | *-*-dragonfly*)
	    # Do not include libc due to us having libc/libc_r.
	    test "X$arg" = "X-lc" && continue
	    ;;
	  *-*-rhapsody* | *-*-darwin1.[012])
	    # Rhapsody C and math libraries are in the System framework
	    deplibs="$deplibs System.ltframework"
	    continue
	    ;;
	  *-*-sco3.2v5* | *-*-sco5v6*)
	    # Causes problems with __ctype
	    test "X$arg" = "X-lc" && continue
	    ;;
	  *-*-sysv4.2uw2* | *-*-sysv5* | *-*-unixware* | *-*-OpenUNIX*)
	    # Compiler inserts libc in the correct place for threads to work
	    test "X$arg" = "X-lc" && continue
	    ;;
	  esac
	elif test "X$arg" = "X-lc_r"; then
	 case $host in
	 *-*-openbsd* | *-*-freebsd* | *-*-dragonfly*)
	   # Do not include libc_r directly, use -pthread flag.
	   continue
	   ;;
	 esac
	fi
	deplibs="$deplibs $arg"
	continue
	;;

      -module)
	module=yes
	continue
	;;

      # Tru64 UNIX uses -model [arg] to determine the layout of C++
      # classes, name mangling, and exception handling.
      # Darwin uses the -arch flag to determine output architecture.
      -model|-arch|-isysroot)
	compiler_flags="$compiler_flags $arg"
	func_append compile_command " $arg"
	func_append finalize_command " $arg"
	prev=xcompiler
	continue
	;;

      -mt|-mthreads|-kthread|-Kthread|-pthread|-pthreads|--thread-safe|-threads)
	compiler_flags="$compiler_flags $arg"
	func_append compile_command " $arg"
	func_append finalize_command " $arg"
	case "$new_inherited_linker_flags " in
	    *" $arg "*) ;;
	    * ) new_inherited_linker_flags="$new_inherited_linker_flags $arg" ;;
	esac
	continue
	;;

      -multi_module)
	single_module="${wl}-multi_module"
	continue
	;;

      -no-fast-install)
	fast_install=no
	continue
	;;

      -no-install)
	case $host in
	*-*-cygwin* | *-*-mingw* | *-*-pw32* | *-*-os2* | *-*-darwin*)
	  # The PATH hackery in wrapper scripts is required on Windows
	  # and Darwin in order for the loader to find any dlls it needs.
	  func_warning "\`-no-install' is ignored for $host"
	  func_warning "assuming \`-no-fast-install' instead"
	  fast_install=no
	  ;;
	*) no_install=yes ;;
	esac
	continue
	;;

      -no-undefined)
	allow_undefined=no
	continue
	;;

      -objectlist)
	prev=objectlist
	continue
	;;

      -o) prev=output ;;

      -precious-files-regex)
	prev=precious_regex
	continue
	;;

      -release)
	prev=release
	continue
	;;

      -rpath)
	prev=rpath
	continue
	;;

      -R)
	prev=xrpath
	continue
	;;

      -R*)
	func_stripname '-R' '' "$arg"
	dir=$func_stripname_result
	# We need an absolute path.
	case $dir in
	[\\/]* | [A-Za-z]:[\\/]*) ;;
	*)
	  func_fatal_error "only absolute run-paths are allowed"
	  ;;
	esac
	case "$xrpath " in
	*" $dir "*) ;;
	*) xrpath="$xrpath $dir" ;;
	esac
	continue
	;;

      -shared)
	# The effects of -shared are defined in a previous loop.
	continue
	;;

      -shrext)
	prev=shrext
	continue
	;;

      -static | -static-libtool-libs)
	# The effects of -static are defined in a previous loop.
	# We used to do the same as -all-static on platforms that
	# didn't have a PIC flag, but the assumption that the effects
	# would be equivalent was wrong.  It would break on at least
	# Digital Unix and AIX.
	continue
	;;

      -thread-safe)
	thread_safe=yes
	continue
	;;

      -version-info)
	prev=vinfo
	continue
	;;

      -version-number)
	prev=vinfo
	vinfo_number=yes
	continue
	;;

      -weak)
        prev=weak
	continue
	;;

      -Wc,*)
	func_stripname '-Wc,' '' "$arg"
	args=$func_stripname_result
	arg=
	save_ifs="$IFS"; IFS=','
	for flag in $args; do
	  IFS="$save_ifs"
          func_quote_for_eval "$flag"
	  arg="$arg $wl$func_quote_for_eval_result"
	  compiler_flags="$compiler_flags $func_quote_for_eval_result"
	done
	IFS="$save_ifs"
	func_stripname ' ' '' "$arg"
	arg=$func_stripname_result
	;;

      -Wl,*)
	func_stripname '-Wl,' '' "$arg"
	args=$func_stripname_result
	arg=
	save_ifs="$IFS"; IFS=','
	for flag in $args; do
	  IFS="$save_ifs"
          func_quote_for_eval "$flag"
	  arg="$arg $wl$func_quote_for_eval_result"
	  compiler_flags="$compiler_flags $wl$func_quote_for_eval_result"
	  linker_flags="$linker_flags $func_quote_for_eval_result"
	done
	IFS="$save_ifs"
	func_stripname ' ' '' "$arg"
	arg=$func_stripname_result
	;;

      -Xcompiler)
	prev=xcompiler
	continue
	;;

      -Xlinker)
	prev=xlinker
	continue
	;;

      -XCClinker)
	prev=xcclinker
	continue
	;;

      # -msg_* for osf cc
      -msg_*)
	func_quote_for_eval "$arg"
	arg="$func_quote_for_eval_result"
	;;

      # -64, -mips[0-9] enable 64-bit mode on the SGI compiler
      # -r[0-9][0-9]* specifies the processor on the SGI compiler
      # -xarch=*, -xtarget=* enable 64-bit mode on the Sun compiler
      # +DA*, +DD* enable 64-bit mode on the HP compiler
      # -q* pass through compiler args for the IBM compiler
      # -m*, -t[45]*, -txscale* pass through architecture-specific
      # compiler args for GCC
      # -F/path gives path to uninstalled frameworks, gcc on darwin
      # -p, -pg, --coverage, -fprofile-* pass through profiling flag for GCC
      # @file GCC response files
      -64|-mips[0-9]|-r[0-9][0-9]*|-xarch=*|-xtarget=*|+DA*|+DD*|-q*|-m*| \
      -t[45]*|-txscale*|-p|-pg|--coverage|-fprofile-*|-F*|@*)
        func_quote_for_eval "$arg"
	arg="$func_quote_for_eval_result"
        func_append compile_command " $arg"
        func_append finalize_command " $arg"
        compiler_flags="$compiler_flags $arg"
        continue
        ;;

      # Some other compiler flag.
      -* | +*)
        func_quote_for_eval "$arg"
	arg="$func_quote_for_eval_result"
	;;

      *.$objext)
	# A standard object.
	objs="$objs $arg"
	;;

      *.lo)
	# A libtool-controlled object.

	# Check to see that this really is a libtool object.
	if func_lalib_unsafe_p "$arg"; then
	  pic_object=
	  non_pic_object=

	  # Read the .lo file
	  func_source "$arg"

	  if test -z "$pic_object" ||
	     test -z "$non_pic_object" ||
	     test "$pic_object" = none &&
	     test "$non_pic_object" = none; then
	    func_fatal_error "cannot find name of object for \`$arg'"
	  fi

	  # Extract subdirectory from the argument.
	  func_dirname "$arg" "/" ""
	  xdir="$func_dirname_result"

	  if test "$pic_object" != none; then
	    # Prepend the subdirectory the object is found in.
	    pic_object="$xdir$pic_object"

	    if test "$prev" = dlfiles; then
	      if test "$build_libtool_libs" = yes && test "$dlopen_support" = yes; then
		dlfiles="$dlfiles $pic_object"
		prev=
		continue
	      else
		# If libtool objects are unsupported, then we need to preload.
		prev=dlprefiles
	      fi
	    fi

	    # CHECK ME:  I think I busted this.  -Ossama
	    if test "$prev" = dlprefiles; then
	      # Preload the old-style object.
	      dlprefiles="$dlprefiles $pic_object"
	      prev=
	    fi

	    # A PIC object.
	    func_append libobjs " $pic_object"
	    arg="$pic_object"
	  fi

	  # Non-PIC object.
	  if test "$non_pic_object" != none; then
	    # Prepend the subdirectory the object is found in.
	    non_pic_object="$xdir$non_pic_object"

	    # A standard non-PIC object
	    func_append non_pic_objects " $non_pic_object"
	    if test -z "$pic_object" || test "$pic_object" = none ; then
	      arg="$non_pic_object"
	    fi
	  else
	    # If the PIC object exists, use it instead.
	    # $xdir was prepended to $pic_object above.
	    non_pic_object="$pic_object"
	    func_append non_pic_objects " $non_pic_object"
	  fi
	else
	  # Only an error if not doing a dry-run.
	  if $opt_dry_run; then
	    # Extract subdirectory from the argument.
	    func_dirname "$arg" "/" ""
	    xdir="$func_dirname_result"

	    func_lo2o "$arg"
	    pic_object=$xdir$objdir/$func_lo2o_result
	    non_pic_object=$xdir$func_lo2o_result
	    func_append libobjs " $pic_object"
	    func_append non_pic_objects " $non_pic_object"
	  else
	    func_fatal_error "\`$arg' is not a valid libtool object"
	  fi
	fi
	;;

      *.$libext)
	# An archive.
	deplibs="$deplibs $arg"
	old_deplibs="$old_deplibs $arg"
	continue
	;;

      *.la)
	# A libtool-controlled library.

	if test "$prev" = dlfiles; then
	  # This library was specified with -dlopen.
	  dlfiles="$dlfiles $arg"
	  prev=
	elif test "$prev" = dlprefiles; then
	  # The library was specified with -dlpreopen.
	  dlprefiles="$dlprefiles $arg"
	  prev=
	else
	  deplibs="$deplibs $arg"
	fi
	continue
	;;

      # Some other compiler argument.
      *)
	# Unknown arguments in both finalize_command and compile_command need
	# to be aesthetically quoted because they are evaled later.
	func_quote_for_eval "$arg"
	arg="$func_quote_for_eval_result"
	;;
      esac # arg

      # Now actually substitute the argument into the commands.
      if test -n "$arg"; then
	func_append compile_command " $arg"
	func_append finalize_command " $arg"
      fi
    done # argument parsing loop

    test -n "$prev" && \
      func_fatal_help "the \`$prevarg' option requires an argument"

    if test "$export_dynamic" = yes && test -n "$export_dynamic_flag_spec"; then
      eval arg=\"$export_dynamic_flag_spec\"
      func_append compile_command " $arg"
      func_append finalize_command " $arg"
    fi

    oldlibs=
    # calculate the name of the file, without its directory
    func_basename "$output"
    outputname="$func_basename_result"
    libobjs_save="$libobjs"

    if test -n "$shlibpath_var"; then
      # get the directories listed in $shlibpath_var
      eval shlib_search_path=\`\$ECHO \"X\${$shlibpath_var}\" \| \$Xsed -e \'s/:/ /g\'\`
    else
      shlib_search_path=
    fi
    eval sys_lib_search_path=\"$sys_lib_search_path_spec\"
    eval sys_lib_dlsearch_path=\"$sys_lib_dlsearch_path_spec\"

    func_dirname "$output" "/" ""
    output_objdir="$func_dirname_result$objdir"
    # Create the object directory.
    func_mkdir_p "$output_objdir"

    # Determine the type of output
    case $output in
    "")
      func_fatal_help "you must specify an output file"
      ;;
    *.$libext) linkmode=oldlib ;;
    *.lo | *.$objext) linkmode=obj ;;
    *.la) linkmode=lib ;;
    *) linkmode=prog ;; # Anything else should be a program.
    esac

    specialdeplibs=

    libs=
    # Find all interdependent deplibs by searching for libraries
    # that are linked more than once (e.g. -la -lb -la)
    for deplib in $deplibs; do
      if $opt_duplicate_deps ; then
	case "$libs " in
	*" $deplib "*) specialdeplibs="$specialdeplibs $deplib" ;;
	esac
      fi
      libs="$libs $deplib"
    done

    if test "$linkmode" = lib; then
      libs="$predeps $libs $compiler_lib_search_path $postdeps"

      # Compute libraries that are listed more than once in $predeps
      # $postdeps and mark them as special (i.e., whose duplicates are
      # not to be eliminated).
      pre_post_deps=
      if $opt_duplicate_compiler_generated_deps; then
	for pre_post_dep in $predeps $postdeps; do
	  case "$pre_post_deps " in
	  *" $pre_post_dep "*) specialdeplibs="$specialdeplibs $pre_post_deps" ;;
	  esac
	  pre_post_deps="$pre_post_deps $pre_post_dep"
	done
      fi
      pre_post_deps=
    fi

    deplibs=
    newdependency_libs=
    newlib_search_path=
    need_relink=no # whether we're linking any uninstalled libtool libraries
    notinst_deplibs= # not-installed libtool libraries
    notinst_path= # paths that contain not-installed libtool libraries

    case $linkmode in
    lib)
	passes="conv dlpreopen link"
	for file in $dlfiles $dlprefiles; do
	  case $file in
	  *.la) ;;
	  *)
	    func_fatal_help "libraries can \`-dlopen' only libtool libraries: $file"
	    ;;
	  esac
	done
	;;
    prog)
	compile_deplibs=
	finalize_deplibs=
	alldeplibs=no
	newdlfiles=
	newdlprefiles=
	passes="conv scan dlopen dlpreopen link"
	;;
    *)  passes="conv"
	;;
    esac

    for pass in $passes; do
      # The preopen pass in lib mode reverses $deplibs; put it back here
      # so that -L comes before libs that need it for instance...
      if test "$linkmode,$pass" = "lib,link"; then
	## FIXME: Find the place where the list is rebuilt in the wrong
	##        order, and fix it there properly
        tmp_deplibs=
	for deplib in $deplibs; do
	  tmp_deplibs="$deplib $tmp_deplibs"
	done
	deplibs="$tmp_deplibs"
      fi

      if test "$linkmode,$pass" = "lib,link" ||
	 test "$linkmode,$pass" = "prog,scan"; then
	libs="$deplibs"
	deplibs=
      fi
      if test "$linkmode" = prog; then
	case $pass in
	dlopen) libs="$dlfiles" ;;
	dlpreopen) libs="$dlprefiles" ;;
	link)
	  libs="$deplibs %DEPLIBS%"
	  test "X$link_all_deplibs" != Xno && libs="$libs $dependency_libs"
	  ;;
	esac
      fi
      if test "$linkmode,$pass" = "lib,dlpreopen"; then
	# Collect and forward deplibs of preopened libtool libs
	for lib in $dlprefiles; do
	  # Ignore non-libtool-libs
	  dependency_libs=
	  case $lib in
	  *.la)	func_source "$lib" ;;
	  esac

	  # Collect preopened libtool deplibs, except any this library
	  # has declared as weak libs
	  for deplib in $dependency_libs; do
            deplib_base=`$ECHO "X$deplib" | $Xsed -e "$basename"`
	    case " $weak_libs " in
	    *" $deplib_base "*) ;;
	    *) deplibs="$deplibs $deplib" ;;
	    esac
	  done
	done
	libs="$dlprefiles"
      fi
      if test "$pass" = dlopen; then
	# Collect dlpreopened libraries
	save_deplibs="$deplibs"
	deplibs=
      fi

      for deplib in $libs; do
	lib=
	found=no
	case $deplib in
	-mt|-mthreads|-kthread|-Kthread|-pthread|-pthreads|--thread-safe|-threads)
	  if test "$linkmode,$pass" = "prog,link"; then
	    compile_deplibs="$deplib $compile_deplibs"
	    finalize_deplibs="$deplib $finalize_deplibs"
	  else
	    compiler_flags="$compiler_flags $deplib"
	    if test "$linkmode" = lib ; then
		case "$new_inherited_linker_flags " in
		    *" $deplib "*) ;;
		    * ) new_inherited_linker_flags="$new_inherited_linker_flags $deplib" ;;
		esac
	    fi
	  fi
	  continue
	  ;;
	-l*)
	  if test "$linkmode" != lib && test "$linkmode" != prog; then
	    func_warning "\`-l' is ignored for archives/objects"
	    continue
	  fi
	  func_stripname '-l' '' "$deplib"
	  name=$func_stripname_result
	  if test "$linkmode" = lib; then
	    searchdirs="$newlib_search_path $lib_search_path $compiler_lib_search_dirs $sys_lib_search_path $shlib_search_path"
	  else
	    searchdirs="$newlib_search_path $lib_search_path $sys_lib_search_path $shlib_search_path"
	  fi
	  for searchdir in $searchdirs; do
	    for search_ext in .la $std_shrext .so .a; do
	      # Search the libtool library
	      lib="$searchdir/lib${name}${search_ext}"
	      if test -f "$lib"; then
		if test "$search_ext" = ".la"; then
		  found=yes
		else
		  found=no
		fi
		break 2
	      fi
	    done
	  done
	  if test "$found" != yes; then
	    # deplib doesn't seem to be a libtool library
	    if test "$linkmode,$pass" = "prog,link"; then
	      compile_deplibs="$deplib $compile_deplibs"
	      finalize_deplibs="$deplib $finalize_deplibs"
	    else
	      deplibs="$deplib $deplibs"
	      test "$linkmode" = lib && newdependency_libs="$deplib $newdependency_libs"
	    fi
	    continue
	  else # deplib is a libtool library
	    # If $allow_libtool_libs_with_static_runtimes && $deplib is a stdlib,
	    # We need to do some special things here, and not later.
	    if test "X$allow_libtool_libs_with_static_runtimes" = "Xyes" ; then
	      case " $predeps $postdeps " in
	      *" $deplib "*)
		if func_lalib_p "$lib"; then
		  library_names=
		  old_library=
		  func_source "$lib"
		  for l in $old_library $library_names; do
		    ll="$l"
		  done
		  if test "X$ll" = "X$old_library" ; then # only static version available
		    found=no
		    func_dirname "$lib" "" "."
		    ladir="$func_dirname_result"
		    lib=$ladir/$old_library
		    if test "$linkmode,$pass" = "prog,link"; then
		      compile_deplibs="$deplib $compile_deplibs"
		      finalize_deplibs="$deplib $finalize_deplibs"
		    else
		      deplibs="$deplib $deplibs"
		      test "$linkmode" = lib && newdependency_libs="$deplib $newdependency_libs"
		    fi
		    continue
		  fi
		fi
		;;
	      *) ;;
	      esac
	    fi
	  fi
	  ;; # -l
	*.ltframework)
	  if test "$linkmode,$pass" = "prog,link"; then
	    compile_deplibs="$deplib $compile_deplibs"
	    finalize_deplibs="$deplib $finalize_deplibs"
	  else
	    deplibs="$deplib $deplibs"
	    if test "$linkmode" = lib ; then
		case "$new_inherited_linker_flags " in
		    *" $deplib "*) ;;
		    * ) new_inherited_linker_flags="$new_inherited_linker_flags $deplib" ;;
		esac
	    fi
	  fi
	  continue
	  ;;
	-L*)
	  case $linkmode in
	  lib)
	    deplibs="$deplib $deplibs"
	    test "$pass" = conv && continue
	    newdependency_libs="$deplib $newdependency_libs"
	    func_stripname '-L' '' "$deplib"
	    newlib_search_path="$newlib_search_path $func_stripname_result"
	    ;;
	  prog)
	    if test "$pass" = conv; then
	      deplibs="$deplib $deplibs"
	      continue
	    fi
	    if test "$pass" = scan; then
	      deplibs="$deplib $deplibs"
	    else
	      compile_deplibs="$deplib $compile_deplibs"
	      finalize_deplibs="$deplib $finalize_deplibs"
	    fi
	    func_stripname '-L' '' "$deplib"
	    newlib_search_path="$newlib_search_path $func_stripname_result"
	    ;;
	  *)
	    func_warning "\`-L' is ignored for archives/objects"
	    ;;
	  esac # linkmode
	  continue
	  ;; # -L
	-R*)
	  if test "$pass" = link; then
	    func_stripname '-R' '' "$deplib"
	    dir=$func_stripname_result
	    # Make sure the xrpath contains only unique directories.
	    case "$xrpath " in
	    *" $dir "*) ;;
	    *) xrpath="$xrpath $dir" ;;
	    esac
	  fi
	  deplibs="$deplib $deplibs"
	  continue
	  ;;
	*.la) lib="$deplib" ;;
	*.$libext)
	  if test "$pass" = conv; then
	    deplibs="$deplib $deplibs"
	    continue
	  fi
	  case $linkmode in
	  lib)
	    # Linking convenience modules into shared libraries is allowed,
	    # but linking other static libraries is non-portable.
	    case " $dlpreconveniencelibs " in
	    *" $deplib "*) ;;
	    *)
	      valid_a_lib=no
	      case $deplibs_check_method in
		match_pattern*)
		  set dummy $deplibs_check_method; shift
		  match_pattern_regex=`expr "$deplibs_check_method" : "$1 \(.*\)"`
		  if eval "\$ECHO \"X$deplib\"" 2>/dev/null | $Xsed -e 10q \
		    | $EGREP "$match_pattern_regex" > /dev/null; then
		    valid_a_lib=yes
		  fi
		;;
		pass_all)
		  valid_a_lib=yes
		;;
	      esac
	      if test "$valid_a_lib" != yes; then
		$ECHO
		$ECHO "*** Warning: Trying to link with static lib archive $deplib."
		$ECHO "*** I have the capability to make that library automatically link in when"
		$ECHO "*** you link to this library.  But I can only do this if you have a"
		$ECHO "*** shared version of the library, which you do not appear to have"
		$ECHO "*** because the file extensions .$libext of this argument makes me believe"
		$ECHO "*** that it is just a static archive that I should not use here."
	      else
		$ECHO
		$ECHO "*** Warning: Linking the shared library $output against the"
		$ECHO "*** static library $deplib is not portable!"
		deplibs="$deplib $deplibs"
	      fi
	      ;;
	    esac
	    continue
	    ;;
	  prog)
	    if test "$pass" != link; then
	      deplibs="$deplib $deplibs"
	    else
	      compile_deplibs="$deplib $compile_deplibs"
	      finalize_deplibs="$deplib $finalize_deplibs"
	    fi
	    continue
	    ;;
	  esac # linkmode
	  ;; # *.$libext
	*.lo | *.$objext)
	  if test "$pass" = conv; then
	    deplibs="$deplib $deplibs"
	  elif test "$linkmode" = prog; then
	    if test "$pass" = dlpreopen || test "$dlopen_support" != yes || test "$build_libtool_libs" = no; then
	      # If there is no dlopen support or we're linking statically,
	      # we need to preload.
	      newdlprefiles="$newdlprefiles $deplib"
	      compile_deplibs="$deplib $compile_deplibs"
	      finalize_deplibs="$deplib $finalize_deplibs"
	    else
	      newdlfiles="$newdlfiles $deplib"
	    fi
	  fi
	  continue
	  ;;
	%DEPLIBS%)
	  alldeplibs=yes
	  continue
	  ;;
	esac # case $deplib

	if test "$found" = yes || test -f "$lib"; then :
	else
	  func_fatal_error "cannot find the library \`$lib' or unhandled argument \`$deplib'"
	fi

	# Check to see that this really is a libtool archive.
	func_lalib_unsafe_p "$lib" \
	  || func_fatal_error "\`$lib' is not a valid libtool archive"

	func_dirname "$lib" "" "."
	ladir="$func_dirname_result"

	dlname=
	dlopen=
	dlpreopen=
	libdir=
	library_names=
	old_library=
	inherited_linker_flags=
	# If the library was installed with an old release of libtool,
	# it will not redefine variables installed, or shouldnotlink
	installed=yes
	shouldnotlink=no
	avoidtemprpath=


	# Read the .la file
	func_source "$lib"

	# Convert "-framework foo" to "foo.ltframework"
	if test -n "$inherited_linker_flags"; then
	  tmp_inherited_linker_flags=`$ECHO "X$inherited_linker_flags" | $Xsed -e 's/-framework \([^ $]*\)/\1.ltframework/g'`
	  for tmp_inherited_linker_flag in $tmp_inherited_linker_flags; do
	    case " $new_inherited_linker_flags " in
	      *" $tmp_inherited_linker_flag "*) ;;
	      *) new_inherited_linker_flags="$new_inherited_linker_flags $tmp_inherited_linker_flag";;
	    esac
	  done
	fi
	dependency_libs=`$ECHO "X $dependency_libs" | $Xsed -e 's% \([^ $]*\).ltframework% -framework \1%g'`
	if test "$linkmode,$pass" = "lib,link" ||
	   test "$linkmode,$pass" = "prog,scan" ||
	   { test "$linkmode" != prog && test "$linkmode" != lib; }; then
	  test -n "$dlopen" && dlfiles="$dlfiles $dlopen"
	  test -n "$dlpreopen" && dlprefiles="$dlprefiles $dlpreopen"
	fi

	if test "$pass" = conv; then
	  # Only check for convenience libraries
	  deplibs="$lib $deplibs"
	  if test -z "$libdir"; then
	    if test -z "$old_library"; then
	      func_fatal_error "cannot find name of link library for \`$lib'"
	    fi
	    # It is a libtool convenience library, so add in its objects.
	    convenience="$convenience $ladir/$objdir/$old_library"
	    old_convenience="$old_convenience $ladir/$objdir/$old_library"
	  elif test "$linkmode" != prog && test "$linkmode" != lib; then
	    func_fatal_error "\`$lib' is not a convenience library"
	  fi
	  tmp_libs=
	  for deplib in $dependency_libs; do
	    deplibs="$deplib $deplibs"
	    if $opt_duplicate_deps ; then
	      case "$tmp_libs " in
	      *" $deplib "*) specialdeplibs="$specialdeplibs $deplib" ;;
	      esac
	    fi
	    tmp_libs="$tmp_libs $deplib"
	  done
	  continue
	fi # $pass = conv


	# Get the name of the library we link against.
	linklib=
	for l in $old_library $library_names; do
	  linklib="$l"
	done
	if test -z "$linklib"; then
	  func_fatal_error "cannot find name of link library for \`$lib'"
	fi

	# This library was specified with -dlopen.
	if test "$pass" = dlopen; then
	  if test -z "$libdir"; then
	    func_fatal_error "cannot -dlopen a convenience library: \`$lib'"
	  fi
	  if test -z "$dlname" ||
	     test "$dlopen_support" != yes ||
	     test "$build_libtool_libs" = no; then
	    # If there is no dlname, no dlopen support or we're linking
	    # statically, we need to preload.  We also need to preload any
	    # dependent libraries so libltdl's deplib preloader doesn't
	    # bomb out in the load deplibs phase.
	    dlprefiles="$dlprefiles $lib $dependency_libs"
	  else
	    newdlfiles="$newdlfiles $lib"
	  fi
	  continue
	fi # $pass = dlopen

	# We need an absolute path.
	case $ladir in
	[\\/]* | [A-Za-z]:[\\/]*) abs_ladir="$ladir" ;;
	*)
	  abs_ladir=`cd "$ladir" && pwd`
	  if test -z "$abs_ladir"; then
	    func_warning "cannot determine absolute directory name of \`$ladir'"
	    func_warning "passing it literally to the linker, although it might fail"
	    abs_ladir="$ladir"
	  fi
	  ;;
	esac
	func_basename "$lib"
	laname="$func_basename_result"

	# Find the relevant object directory and library name.
	if test "X$installed" = Xyes; then
	  if test ! -f "$libdir/$linklib" && test -f "$abs_ladir/$linklib"; then
	    func_warning "library \`$lib' was moved."
	    dir="$ladir"
	    absdir="$abs_ladir"
	    libdir="$abs_ladir"
	  else
	    dir="$libdir"
	    absdir="$libdir"
	  fi
	  test "X$hardcode_automatic" = Xyes && avoidtemprpath=yes
	else
	  if test ! -f "$ladir/$objdir/$linklib" && test -f "$abs_ladir/$linklib"; then
	    dir="$ladir"
	    absdir="$abs_ladir"
	    # Remove this search path later
	    notinst_path="$notinst_path $abs_ladir"
	  else
	    dir="$ladir/$objdir"
	    absdir="$abs_ladir/$objdir"
	    # Remove this search path later
	    notinst_path="$notinst_path $abs_ladir"
	  fi
	fi # $installed = yes
	func_stripname 'lib' '.la' "$laname"
	name=$func_stripname_result

	# This library was specified with -dlpreopen.
	if test "$pass" = dlpreopen; then
	  if test -z "$libdir" && test "$linkmode" = prog; then
	    func_fatal_error "only libraries may -dlpreopen a convenience library: \`$lib'"
	  fi
	  # Prefer using a static library (so that no silly _DYNAMIC symbols
	  # are required to link).
	  if test -n "$old_library"; then
	    newdlprefiles="$newdlprefiles $dir/$old_library"
	    # Keep a list of preopened convenience libraries to check
	    # that they are being used correctly in the link pass.
	    test -z "$libdir" && \
		dlpreconveniencelibs="$dlpreconveniencelibs $dir/$old_library"
	  # Otherwise, use the dlname, so that lt_dlopen finds it.
	  elif test -n "$dlname"; then
	    newdlprefiles="$newdlprefiles $dir/$dlname"
	  else
	    newdlprefiles="$newdlprefiles $dir/$linklib"
	  fi
	fi # $pass = dlpreopen

	if test -z "$libdir"; then
	  # Link the convenience library
	  if test "$linkmode" = lib; then
	    deplibs="$dir/$old_library $deplibs"
	  elif test "$linkmode,$pass" = "prog,link"; then
	    compile_deplibs="$dir/$old_library $compile_deplibs"
	    finalize_deplibs="$dir/$old_library $finalize_deplibs"
	  else
	    deplibs="$lib $deplibs" # used for prog,scan pass
	  fi
	  continue
	fi


	if test "$linkmode" = prog && test "$pass" != link; then
	  newlib_search_path="$newlib_search_path $ladir"
	  deplibs="$lib $deplibs"

	  linkalldeplibs=no
	  if test "$link_all_deplibs" != no || test -z "$library_names" ||
	     test "$build_libtool_libs" = no; then
	    linkalldeplibs=yes
	  fi

	  tmp_libs=
	  for deplib in $dependency_libs; do
	    case $deplib in
	    -L*) func_stripname '-L' '' "$deplib"
	         newlib_search_path="$newlib_search_path $func_stripname_result"
		 ;;
	    esac
	    # Need to link against all dependency_libs?
	    if test "$linkalldeplibs" = yes; then
	      deplibs="$deplib $deplibs"
	    else
	      # Need to hardcode shared library paths
	      # or/and link against static libraries
	      newdependency_libs="$deplib $newdependency_libs"
	    fi
	    if $opt_duplicate_deps ; then
	      case "$tmp_libs " in
	      *" $deplib "*) specialdeplibs="$specialdeplibs $deplib" ;;
	      esac
	    fi
	    tmp_libs="$tmp_libs $deplib"
	  done # for deplib
	  continue
	fi # $linkmode = prog...

	if test "$linkmode,$pass" = "prog,link"; then
	  if test -n "$library_names" &&
	     { { test "$prefer_static_libs" = no ||
	         test "$prefer_static_libs,$installed" = "built,yes"; } ||
	       test -z "$old_library"; }; then
	    # We need to hardcode the library path
	    if test -n "$shlibpath_var" && test -z "$avoidtemprpath" ; then
	      # Make sure the rpath contains only unique directories.
	      case "$temp_rpath:" in
	      *"$absdir:"*) ;;
	      *) temp_rpath="$temp_rpath$absdir:" ;;
	      esac
	    fi

	    # Hardcode the library path.
	    # Skip directories that are in the system default run-time
	    # search path.
	    case " $sys_lib_dlsearch_path " in
	    *" $absdir "*) ;;
	    *)
	      case "$compile_rpath " in
	      *" $absdir "*) ;;
	      *) compile_rpath="$compile_rpath $absdir"
	      esac
	      ;;
	    esac
	    case " $sys_lib_dlsearch_path " in
	    *" $libdir "*) ;;
	    *)
	      case "$finalize_rpath " in
	      *" $libdir "*) ;;
	      *) finalize_rpath="$finalize_rpath $libdir"
	      esac
	      ;;
	    esac
	  fi # $linkmode,$pass = prog,link...

	  if test "$alldeplibs" = yes &&
	     { test "$deplibs_check_method" = pass_all ||
	       { test "$build_libtool_libs" = yes &&
		 test -n "$library_names"; }; }; then
	    # We only need to search for static libraries
	    continue
	  fi
	fi

	link_static=no # Whether the deplib will be linked statically
	use_static_libs=$prefer_static_libs
	if test "$use_static_libs" = built && test "$installed" = yes; then
	  use_static_libs=no
	fi
	if test -n "$library_names" &&
	   { test "$use_static_libs" = no || test -z "$old_library"; }; then
	  case $host in
	  *cygwin* | *mingw*)
	      # No point in relinking DLLs because paths are not encoded
	      notinst_deplibs="$notinst_deplibs $lib"
	      need_relink=no
	    ;;
	  *)
	    if test "$installed" = no; then
	      notinst_deplibs="$notinst_deplibs $lib"
	      need_relink=yes
	    fi
	    ;;
	  esac
	  # This is a shared library

	  # Warn about portability, can't link against -module's on some
	  # systems (darwin).  Don't bleat about dlopened modules though!
	  dlopenmodule=""
	  for dlpremoduletest in $dlprefiles; do
	    if test "X$dlpremoduletest" = "X$lib"; then
	      dlopenmodule="$dlpremoduletest"
	      break
	    fi
	  done
	  if test -z "$dlopenmodule" && test "$shouldnotlink" = yes && test "$pass" = link; then
	    $ECHO
	    if test "$linkmode" = prog; then
	      $ECHO "*** Warning: Linking the executable $output against the loadable module"
	    else
	      $ECHO "*** Warning: Linking the shared library $output against the loadable module"
	    fi
	    $ECHO "*** $linklib is not portable!"
	  fi
	  if test "$linkmode" = lib &&
	     test "$hardcode_into_libs" = yes; then
	    # Hardcode the library path.
	    # Skip directories that are in the system default run-time
	    # search path.
	    case " $sys_lib_dlsearch_path " in
	    *" $absdir "*) ;;
	    *)
	      case "$compile_rpath " in
	      *" $absdir "*) ;;
	      *) compile_rpath="$compile_rpath $absdir"
	      esac
	      ;;
	    esac
	    case " $sys_lib_dlsearch_path " in
	    *" $libdir "*) ;;
	    *)
	      case "$finalize_rpath " in
	      *" $libdir "*) ;;
	      *) finalize_rpath="$finalize_rpath $libdir"
	      esac
	      ;;
	    esac
	  fi

	  if test -n "$old_archive_from_expsyms_cmds"; then
	    # figure out the soname
	    set dummy $library_names
	    shift
	    realname="$1"
	    shift
	    libname=`eval "\\$ECHO \"$libname_spec\""`
	    # use dlname if we got it. it's perfectly good, no?
	    if test -n "$dlname"; then
	      soname="$dlname"
	    elif test -n "$soname_spec"; then
	      # bleh windows
	      case $host in
	      *cygwin* | mingw*)
	        func_arith $current - $age
		major=$func_arith_result
		versuffix="-$major"
		;;
	      esac
	      eval soname=\"$soname_spec\"
	    else
	      soname="$realname"
	    fi

	    # Make a new name for the extract_expsyms_cmds to use
	    soroot="$soname"
	    func_basename "$soroot"
	    soname="$func_basename_result"
	    func_stripname 'lib' '.dll' "$soname"
	    newlib=libimp-$func_stripname_result.a

	    # If the library has no export list, then create one now
	    if test -f "$output_objdir/$soname-def"; then :
	    else
	      func_verbose "extracting exported symbol list from \`$soname'"
	      func_execute_cmds "$extract_expsyms_cmds" 'exit $?'
	    fi

	    # Create $newlib
	    if test -f "$output_objdir/$newlib"; then :; else
	      func_verbose "generating import library for \`$soname'"
	      func_execute_cmds "$old_archive_from_expsyms_cmds" 'exit $?'
	    fi
	    # make sure the library variables are pointing to the new library
	    dir=$output_objdir
	    linklib=$newlib
	  fi # test -n "$old_archive_from_expsyms_cmds"

	  if test "$linkmode" = prog || test "$mode" != relink; then
	    add_shlibpath=
	    add_dir=
	    add=
	    lib_linked=yes
	    case $hardcode_action in
	    immediate | unsupported)
	      if test "$hardcode_direct" = no; then
		add="$dir/$linklib"
		case $host in
		  *-*-sco3.2v5.0.[024]*) add_dir="-L$dir" ;;
		  *-*-sysv4*uw2*) add_dir="-L$dir" ;;
		  *-*-sysv5OpenUNIX* | *-*-sysv5UnixWare7.[01].[10]* | \
		    *-*-unixware7*) add_dir="-L$dir" ;;
		  *-*-darwin* )
		    # if the lib is a (non-dlopened) module then we can not
		    # link against it, someone is ignoring the earlier warnings
		    if /usr/bin/file -L $add 2> /dev/null |
			 $GREP ": [^:]* bundle" >/dev/null ; then
		      if test "X$dlopenmodule" != "X$lib"; then
			$ECHO "*** Warning: lib $linklib is a module, not a shared library"
			if test -z "$old_library" ; then
			  $ECHO
			  $ECHO "*** And there doesn't seem to be a static archive available"
			  $ECHO "*** The link will probably fail, sorry"
			else
			  add="$dir/$old_library"
			fi
		      elif test -n "$old_library"; then
			add="$dir/$old_library"
		      fi
		    fi
		esac
	      elif test "$hardcode_minus_L" = no; then
		case $host in
		*-*-sunos*) add_shlibpath="$dir" ;;
		esac
		add_dir="-L$dir"
		add="-l$name"
	      elif test "$hardcode_shlibpath_var" = no; then
		add_shlibpath="$dir"
		add="-l$name"
	      else
		lib_linked=no
	      fi
	      ;;
	    relink)
	      if test "$hardcode_direct" = yes &&
	         test "$hardcode_direct_absolute" = no; then
		add="$dir/$linklib"
	      elif test "$hardcode_minus_L" = yes; then
		add_dir="-L$dir"
		# Try looking first in the location we're being installed to.
		if test -n "$inst_prefix_dir"; then
		  case $libdir in
		    [\\/]*)
		      add_dir="$add_dir -L$inst_prefix_dir$libdir"
		      ;;
		  esac
		fi
		add="-l$name"
	      elif test "$hardcode_shlibpath_var" = yes; then
		add_shlibpath="$dir"
		add="-l$name"
	      else
		lib_linked=no
	      fi
	      ;;
	    *) lib_linked=no ;;
	    esac

	    if test "$lib_linked" != yes; then
	      func_fatal_configuration "unsupported hardcode properties"
	    fi

	    if test -n "$add_shlibpath"; then
	      case :$compile_shlibpath: in
	      *":$add_shlibpath:"*) ;;
	      *) compile_shlibpath="$compile_shlibpath$add_shlibpath:" ;;
	      esac
	    fi
	    if test "$linkmode" = prog; then
	      test -n "$add_dir" && compile_deplibs="$add_dir $compile_deplibs"
	      test -n "$add" && compile_deplibs="$add $compile_deplibs"
	    else
	      test -n "$add_dir" && deplibs="$add_dir $deplibs"
	      test -n "$add" && deplibs="$add $deplibs"
	      if test "$hardcode_direct" != yes &&
		 test "$hardcode_minus_L" != yes &&
		 test "$hardcode_shlibpath_var" = yes; then
		case :$finalize_shlibpath: in
		*":$libdir:"*) ;;
		*) finalize_shlibpath="$finalize_shlibpath$libdir:" ;;
		esac
	      fi
	    fi
	  fi

	  if test "$linkmode" = prog || test "$mode" = relink; then
	    add_shlibpath=
	    add_dir=
	    add=
	    # Finalize command for both is simple: just hardcode it.
	    if test "$hardcode_direct" = yes &&
	       test "$hardcode_direct_absolute" = no; then
	      add="$libdir/$linklib"
	    elif test "$hardcode_minus_L" = yes; then
	      add_dir="-L$libdir"
	      add="-l$name"
	    elif test "$hardcode_shlibpath_var" = yes; then
	      case :$finalize_shlibpath: in
	      *":$libdir:"*) ;;
	      *) finalize_shlibpath="$finalize_shlibpath$libdir:" ;;
	      esac
	      add="-l$name"
	    elif test "$hardcode_automatic" = yes; then
	      if test -n "$inst_prefix_dir" &&
		 test -f "$inst_prefix_dir$libdir/$linklib" ; then
		add="$inst_prefix_dir$libdir/$linklib"
	      else
		add="$libdir/$linklib"
	      fi
	    else
	      # We cannot seem to hardcode it, guess we'll fake it.
	      add_dir="-L$libdir"
	      # Try looking first in the location we're being installed to.
	      if test -n "$inst_prefix_dir"; then
		case $libdir in
		  [\\/]*)
		    add_dir="$add_dir -L$inst_prefix_dir$libdir"
		    ;;
		esac
	      fi
	      add="-l$name"
	    fi

	    if test "$linkmode" = prog; then
	      test -n "$add_dir" && finalize_deplibs="$add_dir $finalize_deplibs"
	      test -n "$add" && finalize_deplibs="$add $finalize_deplibs"
	    else
	      test -n "$add_dir" && deplibs="$add_dir $deplibs"
	      test -n "$add" && deplibs="$add $deplibs"
	    fi
	  fi
	elif test "$linkmode" = prog; then
	  # Here we assume that one of hardcode_direct or hardcode_minus_L
	  # is not unsupported.  This is valid on all known static and
	  # shared platforms.
	  if test "$hardcode_direct" != unsupported; then
	    test -n "$old_library" && linklib="$old_library"
	    compile_deplibs="$dir/$linklib $compile_deplibs"
	    finalize_deplibs="$dir/$linklib $finalize_deplibs"
	  else
	    compile_deplibs="-l$name -L$dir $compile_deplibs"
	    finalize_deplibs="-l$name -L$dir $finalize_deplibs"
	  fi
	elif test "$build_libtool_libs" = yes; then
	  # Not a shared library
	  if test "$deplibs_check_method" != pass_all; then
	    # We're trying link a shared library against a static one
	    # but the system doesn't support it.

	    # Just print a warning and add the library to dependency_libs so
	    # that the program can be linked against the static library.
	    $ECHO
	    $ECHO "*** Warning: This system can not link to static lib archive $lib."
	    $ECHO "*** I have the capability to make that library automatically link in when"
	    $ECHO "*** you link to this library.  But I can only do this if you have a"
	    $ECHO "*** shared version of the library, which you do not appear to have."
	    if test "$module" = yes; then
	      $ECHO "*** But as you try to build a module library, libtool will still create "
	      $ECHO "*** a static module, that should work as long as the dlopening application"
	      $ECHO "*** is linked with the -dlopen flag to resolve symbols at runtime."
	      if test -z "$global_symbol_pipe"; then
		$ECHO
		$ECHO "*** However, this would only work if libtool was able to extract symbol"
		$ECHO "*** lists from a program, using \`nm' or equivalent, but libtool could"
		$ECHO "*** not find such a program.  So, this module is probably useless."
		$ECHO "*** \`nm' from GNU binutils and a full rebuild may help."
	      fi
	      if test "$build_old_libs" = no; then
		build_libtool_libs=module
		build_old_libs=yes
	      else
		build_libtool_libs=no
	      fi
	    fi
	  else
	    deplibs="$dir/$old_library $deplibs"
	    link_static=yes
	  fi
	fi # link shared/static library?

	if test "$linkmode" = lib; then
	  if test -n "$dependency_libs" &&
	     { test "$hardcode_into_libs" != yes ||
	       test "$build_old_libs" = yes ||
	       test "$link_static" = yes; }; then
	    # Extract -R from dependency_libs
	    temp_deplibs=
	    for libdir in $dependency_libs; do
	      case $libdir in
	      -R*) func_stripname '-R' '' "$libdir"
	           temp_xrpath=$func_stripname_result
		   case " $xrpath " in
		   *" $temp_xrpath "*) ;;
		   *) xrpath="$xrpath $temp_xrpath";;
		   esac;;
	      *) temp_deplibs="$temp_deplibs $libdir";;
	      esac
	    done
	    dependency_libs="$temp_deplibs"
	  fi

	  newlib_search_path="$newlib_search_path $absdir"
	  # Link against this library
	  test "$link_static" = no && newdependency_libs="$abs_ladir/$laname $newdependency_libs"
	  # ... and its dependency_libs
	  tmp_libs=
	  for deplib in $dependency_libs; do
	    newdependency_libs="$deplib $newdependency_libs"
	    if $opt_duplicate_deps ; then
	      case "$tmp_libs " in
	      *" $deplib "*) specialdeplibs="$specialdeplibs $deplib" ;;
	      esac
	    fi
	    tmp_libs="$tmp_libs $deplib"
	  done

	  if test "$link_all_deplibs" != no; then
	    # Add the search paths of all dependency libraries
	    for deplib in $dependency_libs; do
	      case $deplib in
	      -L*) path="$deplib" ;;
	      *.la)
	        func_dirname "$deplib" "" "."
		dir="$func_dirname_result"
		# We need an absolute path.
		case $dir in
		[\\/]* | [A-Za-z]:[\\/]*) absdir="$dir" ;;
		*)
		  absdir=`cd "$dir" && pwd`
		  if test -z "$absdir"; then
		    func_warning "cannot determine absolute directory name of \`$dir'"
		    absdir="$dir"
		  fi
		  ;;
		esac
		if $GREP "^installed=no" $deplib > /dev/null; then
		case $host in
		*-*-darwin*)
		  depdepl=
		  eval deplibrary_names=`${SED} -n -e 's/^library_names=\(.*\)$/\1/p' $deplib`
		  if test -n "$deplibrary_names" ; then
		    for tmp in $deplibrary_names ; do
		      depdepl=$tmp
		    done
		    if test -f "$absdir/$objdir/$depdepl" ; then
		      depdepl="$absdir/$objdir/$depdepl"
		      darwin_install_name=`${OTOOL} -L $depdepl | awk '{if (NR == 2) {print $1;exit}}'`
                      if test -z "$darwin_install_name"; then
                          darwin_install_name=`${OTOOL64} -L $depdepl  | awk '{if (NR == 2) {print $1;exit}}'`
                      fi
		      compiler_flags="$compiler_flags ${wl}-dylib_file ${wl}${darwin_install_name}:${depdepl}"
		      linker_flags="$linker_flags -dylib_file ${darwin_install_name}:${depdepl}"
		      path=
		    fi
		  fi
		  ;;
		*)
		  path="-L$absdir/$objdir"
		  ;;
		esac
		else
		  eval libdir=`${SED} -n -e 's/^libdir=\(.*\)$/\1/p' $deplib`
		  test -z "$libdir" && \
		    func_fatal_error "\`$deplib' is not a valid libtool archive"
		  test "$absdir" != "$libdir" && \
		    func_warning "\`$deplib' seems to be moved"

		  path="-L$absdir"
		fi
		;;
	      esac
	      case " $deplibs " in
	      *" $path "*) ;;
	      *) deplibs="$path $deplibs" ;;
	      esac
	    done
	  fi # link_all_deplibs != no
	fi # linkmode = lib
      done # for deplib in $libs
      if test "$pass" = link; then
	if test "$linkmode" = "prog"; then
	  compile_deplibs="$new_inherited_linker_flags $compile_deplibs"
	  finalize_deplibs="$new_inherited_linker_flags $finalize_deplibs"
	else
	  compiler_flags="$compiler_flags "`$ECHO "X $new_inherited_linker_flags" | $Xsed -e 's% \([^ $]*\).ltframework% -framework \1%g'`
	fi
      fi
      dependency_libs="$newdependency_libs"
      if test "$pass" = dlpreopen; then
	# Link the dlpreopened libraries before other libraries
	for deplib in $save_deplibs; do
	  deplibs="$deplib $deplibs"
	done
      fi
      if test "$pass" != dlopen; then
	if test "$pass" != conv; then
	  # Make sure lib_search_path contains only unique directories.
	  lib_search_path=
	  for dir in $newlib_search_path; do
	    case "$lib_search_path " in
	    *" $dir "*) ;;
	    *) lib_search_path="$lib_search_path $dir" ;;
	    esac
	  done
	  newlib_search_path=
	fi

	if test "$linkmode,$pass" != "prog,link"; then
	  vars="deplibs"
	else
	  vars="compile_deplibs finalize_deplibs"
	fi
	for var in $vars dependency_libs; do
	  # Add libraries to $var in reverse order
	  eval tmp_libs=\"\$$var\"
	  new_libs=
	  for deplib in $tmp_libs; do
	    # FIXME: Pedantically, this is the right thing to do, so
	    #        that some nasty dependency loop isn't accidentally
	    #        broken:
	    #new_libs="$deplib $new_libs"
	    # Pragmatically, this seems to cause very few problems in
	    # practice:
	    case $deplib in
	    -L*) new_libs="$deplib $new_libs" ;;
	    -R*) ;;
	    *)
	      # And here is the reason: when a library appears more
	      # than once as an explicit dependence of a library, or
	      # is implicitly linked in more than once by the
	      # compiler, it is considered special, and multiple
	      # occurrences thereof are not removed.  Compare this
	      # with having the same library being listed as a
	      # dependency of multiple other libraries: in this case,
	      # we know (pedantically, we assume) the library does not
	      # need to be listed more than once, so we keep only the
	      # last copy.  This is not always right, but it is rare
	      # enough that we require users that really mean to play
	      # such unportable linking tricks to link the library
	      # using -Wl,-lname, so that libtool does not consider it
	      # for duplicate removal.
	      case " $specialdeplibs " in
	      *" $deplib "*) new_libs="$deplib $new_libs" ;;
	      *)
		case " $new_libs " in
		*" $deplib "*) ;;
		*) new_libs="$deplib $new_libs" ;;
		esac
		;;
	      esac
	      ;;
	    esac
	  done
	  tmp_libs=
	  for deplib in $new_libs; do
	    case $deplib in
	    -L*)
	      case " $tmp_libs " in
	      *" $deplib "*) ;;
	      *) tmp_libs="$tmp_libs $deplib" ;;
	      esac
	      ;;
	    *) tmp_libs="$tmp_libs $deplib" ;;
	    esac
	  done
	  eval $var=\"$tmp_libs\"
	done # for var
      fi
      # Last step: remove runtime libs from dependency_libs
      # (they stay in deplibs)
      tmp_libs=
      for i in $dependency_libs ; do
	case " $predeps $postdeps $compiler_lib_search_path " in
	*" $i "*)
	  i=""
	  ;;
	esac
	if test -n "$i" ; then
	  tmp_libs="$tmp_libs $i"
	fi
      done
      dependency_libs=$tmp_libs
    done # for pass
    if test "$linkmode" = prog; then
      dlfiles="$newdlfiles"
    fi
    if test "$linkmode" = prog || test "$linkmode" = lib; then
      dlprefiles="$newdlprefiles"
    fi

    case $linkmode in
    oldlib)
      if test -n "$dlfiles$dlprefiles" || test "$dlself" != no; then
	func_warning "\`-dlopen' is ignored for archives"
      fi

      case " $deplibs" in
      *\ -l* | *\ -L*)
	func_warning "\`-l' and \`-L' are ignored for archives" ;;
      esac

      test -n "$rpath" && \
	func_warning "\`-rpath' is ignored for archives"

      test -n "$xrpath" && \
	func_warning "\`-R' is ignored for archives"

      test -n "$vinfo" && \
	func_warning "\`-version-info/-version-number' is ignored for archives"

      test -n "$release" && \
	func_warning "\`-release' is ignored for archives"

      test -n "$export_symbols$export_symbols_regex" && \
	func_warning "\`-export-symbols' is ignored for archives"

      # Now set the variables for building old libraries.
      build_libtool_libs=no
      oldlibs="$output"
      objs="$objs$old_deplibs"
      ;;

    lib)
      # Make sure we only generate libraries of the form `libNAME.la'.
      case $outputname in
      lib*)
	func_stripname 'lib' '.la' "$outputname"
	name=$func_stripname_result
	eval shared_ext=\"$shrext_cmds\"
	eval libname=\"$libname_spec\"
	;;
      *)
	test "$module" = no && \
	  func_fatal_help "libtool library \`$output' must begin with \`lib'"

	if test "$need_lib_prefix" != no; then
	  # Add the "lib" prefix for modules if required
	  func_stripname '' '.la' "$outputname"
	  name=$func_stripname_result
	  eval shared_ext=\"$shrext_cmds\"
	  eval libname=\"$libname_spec\"
	else
	  func_stripname '' '.la' "$outputname"
	  libname=$func_stripname_result
	fi
	;;
      esac

      if test -n "$objs"; then
	if test "$deplibs_check_method" != pass_all; then
	  func_fatal_error "cannot build libtool library \`$output' from non-libtool objects on this host:$objs"
	else
	  $ECHO
	  $ECHO "*** Warning: Linking the shared library $output against the non-libtool"
	  $ECHO "*** objects $objs is not portable!"
	  libobjs="$libobjs $objs"
	fi
      fi

      test "$dlself" != no && \
	func_warning "\`-dlopen self' is ignored for libtool libraries"

      set dummy $rpath
      shift
      test "$#" -gt 1 && \
	func_warning "ignoring multiple \`-rpath's for a libtool library"

      install_libdir="$1"

      oldlibs=
      if test -z "$rpath"; then
	if test "$build_libtool_libs" = yes; then
	  # Building a libtool convenience library.
	  # Some compilers have problems with a `.al' extension so
	  # convenience libraries should have the same extension an
	  # archive normally would.
	  oldlibs="$output_objdir/$libname.$libext $oldlibs"
	  build_libtool_libs=convenience
	  build_old_libs=yes
	fi

	test -n "$vinfo" && \
	  func_warning "\`-version-info/-version-number' is ignored for convenience libraries"

	test -n "$release" && \
	  func_warning "\`-release' is ignored for convenience libraries"
      else

	# Parse the version information argument.
	save_ifs="$IFS"; IFS=':'
	set dummy $vinfo 0 0 0
	shift
	IFS="$save_ifs"

	test -n "$7" && \
	  func_fatal_help "too many parameters to \`-version-info'"

	# convert absolute version numbers to libtool ages
	# this retains compatibility with .la files and attempts
	# to make the code below a bit more comprehensible

	case $vinfo_number in
	yes)
	  number_major="$1"
	  number_minor="$2"
	  number_revision="$3"
	  #
	  # There are really only two kinds -- those that
	  # use the current revision as the major version
	  # and those that subtract age and use age as
	  # a minor version.  But, then there is irix
	  # which has an extra 1 added just for fun
	  #
	  case $version_type in
	  darwin|linux|osf|windows|none)
	    func_arith $number_major + $number_minor
	    current=$func_arith_result
	    age="$number_minor"
	    revision="$number_revision"
	    ;;
	  freebsd-aout|freebsd-elf|sunos)
	    current="$number_major"
	    revision="$number_minor"
	    age="0"
	    ;;
	  irix|nonstopux)
	    func_arith $number_major + $number_minor
	    current=$func_arith_result
	    age="$number_minor"
	    revision="$number_minor"
	    lt_irix_increment=no
	    ;;
	  *)
	    func_fatal_configuration "$modename: unknown library version type \`$version_type'"
	    ;;
	  esac
	  ;;
	no)
	  current="$1"
	  revision="$2"
	  age="$3"
	  ;;
	esac

	# Check that each of the things are valid numbers.
	case $current in
	0|[1-9]|[1-9][0-9]|[1-9][0-9][0-9]|[1-9][0-9][0-9][0-9]|[1-9][0-9][0-9][0-9][0-9]) ;;
	*)
	  func_error "CURRENT \`$current' must be a nonnegative integer"
	  func_fatal_error "\`$vinfo' is not valid version information"
	  ;;
	esac

	case $revision in
	0|[1-9]|[1-9][0-9]|[1-9][0-9][0-9]|[1-9][0-9][0-9][0-9]|[1-9][0-9][0-9][0-9][0-9]) ;;
	*)
	  func_error "REVISION \`$revision' must be a nonnegative integer"
	  func_fatal_error "\`$vinfo' is not valid version information"
	  ;;
	esac

	case $age in
	0|[1-9]|[1-9][0-9]|[1-9][0-9][0-9]|[1-9][0-9][0-9][0-9]|[1-9][0-9][0-9][0-9][0-9]) ;;
	*)
	  func_error "AGE \`$age' must be a nonnegative integer"
	  func_fatal_error "\`$vinfo' is not valid version information"
	  ;;
	esac

	if test "$age" -gt "$current"; then
	  func_error "AGE \`$age' is greater than the current interface number \`$current'"
	  func_fatal_error "\`$vinfo' is not valid version information"
	fi

	# Calculate the version variables.
	major=
	versuffix=
	verstring=
	case $version_type in
	none) ;;

	darwin)
	  # Like Linux, but with the current version available in
	  # verstring for coding it into the library header
	  func_arith $current - $age
	  major=.$func_arith_result
	  versuffix="$major.$age.$revision"
	  # Darwin ld doesn't like 0 for these options...
	  func_arith $current + 1
	  minor_current=$func_arith_result
	  xlcverstring="${wl}-compatibility_version ${wl}$minor_current ${wl}-current_version ${wl}$minor_current.$revision"
	  verstring="-compatibility_version $minor_current -current_version $minor_current.$revision"
	  ;;

	freebsd-aout)
	  major=".$current"
	  versuffix=".$current.$revision";
	  ;;

	freebsd-elf)
	  major=".$current"
	  versuffix=".$current"
	  ;;

	irix | nonstopux)
	  if test "X$lt_irix_increment" = "Xno"; then
	    func_arith $current - $age
	  else
	    func_arith $current - $age + 1
	  fi
	  major=$func_arith_result

	  case $version_type in
	    nonstopux) verstring_prefix=nonstopux ;;
	    *)         verstring_prefix=sgi ;;
	  esac
	  verstring="$verstring_prefix$major.$revision"

	  # Add in all the interfaces that we are compatible with.
	  loop=$revision
	  while test "$loop" -ne 0; do
	    func_arith $revision - $loop
	    iface=$func_arith_result
	    func_arith $loop - 1
	    loop=$func_arith_result
	    verstring="$verstring_prefix$major.$iface:$verstring"
	  done

	  # Before this point, $major must not contain `.'.
	  major=.$major
	  versuffix="$major.$revision"
	  ;;

	linux)
	  func_arith $current - $age
	  major=.$func_arith_result
	  versuffix="$major.$age.$revision"
	  ;;

	osf)
	  func_arith $current - $age
	  major=.$func_arith_result
	  versuffix=".$current.$age.$revision"
	  verstring="$current.$age.$revision"

	  # Add in all the interfaces that we are compatible with.
	  loop=$age
	  while test "$loop" -ne 0; do
	    func_arith $current - $loop
	    iface=$func_arith_result
	    func_arith $loop - 1
	    loop=$func_arith_result
	    verstring="$verstring:${iface}.0"
	  done

	  # Make executables depend on our current version.
	  verstring="$verstring:${current}.0"
	  ;;

	qnx)
	  major=".$current"
	  versuffix=".$current"
	  ;;

	sunos)
	  major=".$current"
	  versuffix=".$current.$revision"
	  ;;

	windows)
	  # Use '-' rather than '.', since we only want one
	  # extension on DOS 8.3 filesystems.
	  func_arith $current - $age
	  major=$func_arith_result
	  versuffix="-$major"
	  ;;

	*)
	  func_fatal_configuration "unknown library version type \`$version_type'"
	  ;;
	esac

	# Clear the version info if we defaulted, and they specified a release.
	if test -z "$vinfo" && test -n "$release"; then
	  major=
	  case $version_type in
	  darwin)
	    # we can't check for "0.0" in archive_cmds due to quoting
	    # problems, so we reset it completely
	    verstring=
	    ;;
	  *)
	    verstring="0.0"
	    ;;
	  esac
	  if test "$need_version" = no; then
	    versuffix=
	  else
	    versuffix=".0.0"
	  fi
	fi

	# Remove version info from name if versioning should be avoided
	if test "$avoid_version" = yes && test "$need_version" = no; then
	  major=
	  versuffix=
	  verstring=""
	fi

	# Check to see if the archive will have undefined symbols.
	if test "$allow_undefined" = yes; then
	  if test "$allow_undefined_flag" = unsupported; then
	    func_warning "undefined symbols not allowed in $host shared libraries"
	    build_libtool_libs=no
	    build_old_libs=yes
	  fi
	else
	  # Don't allow undefined symbols.
	  allow_undefined_flag="$no_undefined_flag"
	fi

      fi

      func_generate_dlsyms "$libname" "$libname" "yes"
      libobjs="$libobjs $symfileobj"
      test "X$libobjs" = "X " && libobjs=

      if test "$mode" != relink; then
	# Remove our outputs, but don't remove object files since they
	# may have been created when compiling PIC objects.
	removelist=
	tempremovelist=`$ECHO "$output_objdir/*"`
	for p in $tempremovelist; do
	  case $p in
	    *.$objext)
	       ;;
	    $output_objdir/$outputname | $output_objdir/$libname.* | $output_objdir/${libname}${release}.*)
	       if test "X$precious_files_regex" != "X"; then
		 if $ECHO "$p" | $EGREP -e "$precious_files_regex" >/dev/null 2>&1
		 then
		   continue
		 fi
	       fi
	       removelist="$removelist $p"
	       ;;
	    *) ;;
	  esac
	done
	test -n "$removelist" && \
	  func_show_eval "${RM}r \$removelist"
      fi

      # Now set the variables for building old libraries.
      if test "$build_old_libs" = yes && test "$build_libtool_libs" != convenience ; then
	oldlibs="$oldlibs $output_objdir/$libname.$libext"

	# Transform .lo files to .o files.
	oldobjs="$objs "`$ECHO "X$libobjs" | $SP2NL | $Xsed -e '/\.'${libext}'$/d' -e "$lo2o" | $NL2SP`
      fi

      # Eliminate all temporary directories.
      #for path in $notinst_path; do
      #	lib_search_path=`$ECHO "X$lib_search_path " | $Xsed -e "s% $path % %g"`
      #	deplibs=`$ECHO "X$deplibs " | $Xsed -e "s% -L$path % %g"`
      #	dependency_libs=`$ECHO "X$dependency_libs " | $Xsed -e "s% -L$path % %g"`
      #done

      if test -n "$xrpath"; then
	# If the user specified any rpath flags, then add them.
	temp_xrpath=
	for libdir in $xrpath; do
	  temp_xrpath="$temp_xrpath -R$libdir"
	  case "$finalize_rpath " in
	  *" $libdir "*) ;;
	  *) finalize_rpath="$finalize_rpath $libdir" ;;
	  esac
	done
	if test "$hardcode_into_libs" != yes || test "$build_old_libs" = yes; then
	  dependency_libs="$temp_xrpath $dependency_libs"
	fi
      fi

      # Make sure dlfiles contains only unique files that won't be dlpreopened
      old_dlfiles="$dlfiles"
      dlfiles=
      for lib in $old_dlfiles; do
	case " $dlprefiles $dlfiles " in
	*" $lib "*) ;;
	*) dlfiles="$dlfiles $lib" ;;
	esac
      done

      # Make sure dlprefiles contains only unique files
      old_dlprefiles="$dlprefiles"
      dlprefiles=
      for lib in $old_dlprefiles; do
	case "$dlprefiles " in
	*" $lib "*) ;;
	*) dlprefiles="$dlprefiles $lib" ;;
	esac
      done

      if test "$build_libtool_libs" = yes; then
	if test -n "$rpath"; then
	  case $host in
	  *-*-cygwin* | *-*-mingw* | *-*-pw32* | *-*-os2* | *-*-beos*)
	    # these systems don't actually have a c library (as such)!
	    ;;
	  *-*-rhapsody* | *-*-darwin1.[012])
	    # Rhapsody C library is in the System framework
	    deplibs="$deplibs System.ltframework"
	    ;;
	  *-*-netbsd*)
	    # Don't link with libc until the a.out ld.so is fixed.
	    ;;
	  *-*-openbsd* | *-*-freebsd* | *-*-dragonfly*)
	    # Do not include libc due to us having libc/libc_r.
	    ;;
	  *-*-sco3.2v5* | *-*-sco5v6*)
	    # Causes problems with __ctype
	    ;;
	  *-*-sysv4.2uw2* | *-*-sysv5* | *-*-unixware* | *-*-OpenUNIX*)
	    # Compiler inserts libc in the correct place for threads to work
	    ;;
	  *)
	    # Add libc to deplibs on all other systems if necessary.
	    if test "$build_libtool_need_lc" = "yes"; then
	      deplibs="$deplibs -lc"
	    fi
	    ;;
	  esac
	fi

	# Transform deplibs into only deplibs that can be linked in shared.
	name_save=$name
	libname_save=$libname
	release_save=$release
	versuffix_save=$versuffix
	major_save=$major
	# I'm not sure if I'm treating the release correctly.  I think
	# release should show up in the -l (ie -lgmp5) so we don't want to
	# add it in twice.  Is that correct?
	release=""
	versuffix=""
	major=""
	newdeplibs=
	droppeddeps=no
	case $deplibs_check_method in
	pass_all)
	  # Don't check for shared/static.  Everything works.
	  # This might be a little naive.  We might want to check
	  # whether the library exists or not.  But this is on
	  # osf3 & osf4 and I'm not really sure... Just
	  # implementing what was already the behavior.
	  newdeplibs=$deplibs
	  ;;
	test_compile)
	  # This code stresses the "libraries are programs" paradigm to its
	  # limits. Maybe even breaks it.  We compile a program, linking it
	  # against the deplibs as a proxy for the library.  Then we can check
	  # whether they linked in statically or dynamically with ldd.
	  $opt_dry_run || $RM conftest.c
	  cat > conftest.c <<EOF
	  int main() { return 0; }
EOF
	  $opt_dry_run || $RM conftest
	  if $LTCC $LTCFLAGS -o conftest conftest.c $deplibs; then
	    ldd_output=`ldd conftest`
	    for i in $deplibs; do
	      case $i in
	      -l*)
		func_stripname -l '' "$i"
		name=$func_stripname_result
		if test "X$allow_libtool_libs_with_static_runtimes" = "Xyes" ; then
		  case " $predeps $postdeps " in
		  *" $i "*)
		    newdeplibs="$newdeplibs $i"
		    i=""
		    ;;
		  esac
		fi
		if test -n "$i" ; then
		  libname=`eval "\\$ECHO \"$libname_spec\""`
		  deplib_matches=`eval "\\$ECHO \"$library_names_spec\""`
		  set dummy $deplib_matches; shift
		  deplib_match=$1
		  if test `expr "$ldd_output" : ".*$deplib_match"` -ne 0 ; then
		    newdeplibs="$newdeplibs $i"
		  else
		    droppeddeps=yes
		    $ECHO
		    $ECHO "*** Warning: dynamic linker does not accept needed library $i."
		    $ECHO "*** I have the capability to make that library automatically link in when"
		    $ECHO "*** you link to this library.  But I can only do this if you have a"
		    $ECHO "*** shared version of the library, which I believe you do not have"
		    $ECHO "*** because a test_compile did reveal that the linker did not use it for"
		    $ECHO "*** its dynamic dependency list that programs get resolved with at runtime."
		  fi
		fi
		;;
	      *)
		newdeplibs="$newdeplibs $i"
		;;
	      esac
	    done
	  else
	    # Error occurred in the first compile.  Let's try to salvage
	    # the situation: Compile a separate program for each library.
	    for i in $deplibs; do
	      case $i in
	      -l*)
		func_stripname -l '' "$i"
		name=$func_stripname_result
		$opt_dry_run || $RM conftest
		if $LTCC $LTCFLAGS -o conftest conftest.c $i; then
		  ldd_output=`ldd conftest`
		  if test "X$allow_libtool_libs_with_static_runtimes" = "Xyes" ; then
		    case " $predeps $postdeps " in
		    *" $i "*)
		      newdeplibs="$newdeplibs $i"
		      i=""
		      ;;
		    esac
		  fi
		  if test -n "$i" ; then
		    libname=`eval "\\$ECHO \"$libname_spec\""`
		    deplib_matches=`eval "\\$ECHO \"$library_names_spec\""`
		    set dummy $deplib_matches; shift
		    deplib_match=$1
		    if test `expr "$ldd_output" : ".*$deplib_match"` -ne 0 ; then
		      newdeplibs="$newdeplibs $i"
		    else
		      droppeddeps=yes
		      $ECHO
		      $ECHO "*** Warning: dynamic linker does not accept needed library $i."
		      $ECHO "*** I have the capability to make that library automatically link in when"
		      $ECHO "*** you link to this library.  But I can only do this if you have a"
		      $ECHO "*** shared version of the library, which you do not appear to have"
		      $ECHO "*** because a test_compile did reveal that the linker did not use this one"
		      $ECHO "*** as a dynamic dependency that programs can get resolved with at runtime."
		    fi
		  fi
		else
		  droppeddeps=yes
		  $ECHO
		  $ECHO "*** Warning!  Library $i is needed by this library but I was not able to"
		  $ECHO "*** make it link in!  You will probably need to install it or some"
		  $ECHO "*** library that it depends on before this library will be fully"
		  $ECHO "*** functional.  Installing it before continuing would be even better."
		fi
		;;
	      *)
		newdeplibs="$newdeplibs $i"
		;;
	      esac
	    done
	  fi
	  ;;
	file_magic*)
	  set dummy $deplibs_check_method; shift
	  file_magic_regex=`expr "$deplibs_check_method" : "$1 \(.*\)"`
	  for a_deplib in $deplibs; do
	    case $a_deplib in
	    -l*)
	      func_stripname -l '' "$a_deplib"
	      name=$func_stripname_result
	      if test "X$allow_libtool_libs_with_static_runtimes" = "Xyes" ; then
		case " $predeps $postdeps " in
		*" $a_deplib "*)
		  newdeplibs="$newdeplibs $a_deplib"
		  a_deplib=""
		  ;;
		esac
	      fi
	      if test -n "$a_deplib" ; then
		libname=`eval "\\$ECHO \"$libname_spec\""`
		for i in $lib_search_path $sys_lib_search_path $shlib_search_path; do
		  potential_libs=`ls $i/$libname[.-]* 2>/dev/null`
		  for potent_lib in $potential_libs; do
		      # Follow soft links.
		      if ls -lLd "$potent_lib" 2>/dev/null |
			 $GREP " -> " >/dev/null; then
			continue
		      fi
		      # The statement above tries to avoid entering an
		      # endless loop below, in case of cyclic links.
		      # We might still enter an endless loop, since a link
		      # loop can be closed while we follow links,
		      # but so what?
		      potlib="$potent_lib"
		      while test -h "$potlib" 2>/dev/null; do
			potliblink=`ls -ld $potlib | ${SED} 's/.* -> //'`
			case $potliblink in
			[\\/]* | [A-Za-z]:[\\/]*) potlib="$potliblink";;
			*) potlib=`$ECHO "X$potlib" | $Xsed -e 's,[^/]*$,,'`"$potliblink";;
			esac
		      done
		      if eval $file_magic_cmd \"\$potlib\" 2>/dev/null |
			 $SED -e 10q |
			 $EGREP "$file_magic_regex" > /dev/null; then
			newdeplibs="$newdeplibs $a_deplib"
			a_deplib=""
			break 2
		      fi
		  done
		done
	      fi
	      if test -n "$a_deplib" ; then
		droppeddeps=yes
		$ECHO
		$ECHO "*** Warning: linker path does not have real file for library $a_deplib."
		$ECHO "*** I have the capability to make that library automatically link in when"
		$ECHO "*** you link to this library.  But I can only do this if you have a"
		$ECHO "*** shared version of the library, which you do not appear to have"
		$ECHO "*** because I did check the linker path looking for a file starting"
		if test -z "$potlib" ; then
		  $ECHO "*** with $libname but no candidates were found. (...for file magic test)"
		else
		  $ECHO "*** with $libname and none of the candidates passed a file format test"
		  $ECHO "*** using a file magic. Last file checked: $potlib"
		fi
	      fi
	      ;;
	    *)
	      # Add a -L argument.
	      newdeplibs="$newdeplibs $a_deplib"
	      ;;
	    esac
	  done # Gone through all deplibs.
	  ;;
	match_pattern*)
	  set dummy $deplibs_check_method; shift
	  match_pattern_regex=`expr "$deplibs_check_method" : "$1 \(.*\)"`
	  for a_deplib in $deplibs; do
	    case $a_deplib in
	    -l*)
	      func_stripname -l '' "$a_deplib"
	      name=$func_stripname_result
	      if test "X$allow_libtool_libs_with_static_runtimes" = "Xyes" ; then
		case " $predeps $postdeps " in
		*" $a_deplib "*)
		  newdeplibs="$newdeplibs $a_deplib"
		  a_deplib=""
		  ;;
		esac
	      fi
	      if test -n "$a_deplib" ; then
		libname=`eval "\\$ECHO \"$libname_spec\""`
		for i in $lib_search_path $sys_lib_search_path $shlib_search_path; do
		  potential_libs=`ls $i/$libname[.-]* 2>/dev/null`
		  for potent_lib in $potential_libs; do
		    potlib="$potent_lib" # see symlink-check above in file_magic test
		    if eval "\$ECHO \"X$potent_lib\"" 2>/dev/null | $Xsed -e 10q | \
		       $EGREP "$match_pattern_regex" > /dev/null; then
		      newdeplibs="$newdeplibs $a_deplib"
		      a_deplib=""
		      break 2
		    fi
		  done
		done
	      fi
	      if test -n "$a_deplib" ; then
		droppeddeps=yes
		$ECHO
		$ECHO "*** Warning: linker path does not have real file for library $a_deplib."
		$ECHO "*** I have the capability to make that library automatically link in when"
		$ECHO "*** you link to this library.  But I can only do this if you have a"
		$ECHO "*** shared version of the library, which you do not appear to have"
		$ECHO "*** because I did check the linker path looking for a file starting"
		if test -z "$potlib" ; then
		  $ECHO "*** with $libname but no candidates were found. (...for regex pattern test)"
		else
		  $ECHO "*** with $libname and none of the candidates passed a file format test"
		  $ECHO "*** using a regex pattern. Last file checked: $potlib"
		fi
	      fi
	      ;;
	    *)
	      # Add a -L argument.
	      newdeplibs="$newdeplibs $a_deplib"
	      ;;
	    esac
	  done # Gone through all deplibs.
	  ;;
	none | unknown | *)
	  newdeplibs=""
	  tmp_deplibs=`$ECHO "X $deplibs" | $Xsed \
	      -e 's/ -lc$//' -e 's/ -[LR][^ ]*//g'`
	  if test "X$allow_libtool_libs_with_static_runtimes" = "Xyes" ; then
	    for i in $predeps $postdeps ; do
	      # can't use Xsed below, because $i might contain '/'
	      tmp_deplibs=`$ECHO "X $tmp_deplibs" | $Xsed -e "s,$i,,"`
	    done
	  fi
	  if $ECHO "X $tmp_deplibs" | $Xsed -e 's/[	 ]//g' |
	     $GREP . >/dev/null; then
	    $ECHO
	    if test "X$deplibs_check_method" = "Xnone"; then
	      $ECHO "*** Warning: inter-library dependencies are not supported in this platform."
	    else
	      $ECHO "*** Warning: inter-library dependencies are not known to be supported."
	    fi
	    $ECHO "*** All declared inter-library dependencies are being dropped."
	    droppeddeps=yes
	  fi
	  ;;
	esac
	versuffix=$versuffix_save
	major=$major_save
	release=$release_save
	libname=$libname_save
	name=$name_save

	case $host in
	*-*-rhapsody* | *-*-darwin1.[012])
	  # On Rhapsody replace the C library with the System framework
	  newdeplibs=`$ECHO "X $newdeplibs" | $Xsed -e 's/ -lc / System.ltframework /'`
	  ;;
	esac

	if test "$droppeddeps" = yes; then
	  if test "$module" = yes; then
	    $ECHO
	    $ECHO "*** Warning: libtool could not satisfy all declared inter-library"
	    $ECHO "*** dependencies of module $libname.  Therefore, libtool will create"
	    $ECHO "*** a static module, that should work as long as the dlopening"
	    $ECHO "*** application is linked with the -dlopen flag."
	    if test -z "$global_symbol_pipe"; then
	      $ECHO
	      $ECHO "*** However, this would only work if libtool was able to extract symbol"
	      $ECHO "*** lists from a program, using \`nm' or equivalent, but libtool could"
	      $ECHO "*** not find such a program.  So, this module is probably useless."
	      $ECHO "*** \`nm' from GNU binutils and a full rebuild may help."
	    fi
	    if test "$build_old_libs" = no; then
	      oldlibs="$output_objdir/$libname.$libext"
	      build_libtool_libs=module
	      build_old_libs=yes
	    else
	      build_libtool_libs=no
	    fi
	  else
	    $ECHO "*** The inter-library dependencies that have been dropped here will be"
	    $ECHO "*** automatically added whenever a program is linked with this library"
	    $ECHO "*** or is declared to -dlopen it."

	    if test "$allow_undefined" = no; then
	      $ECHO
	      $ECHO "*** Since this library must not contain undefined symbols,"
	      $ECHO "*** because either the platform does not support them or"
	      $ECHO "*** it was explicitly requested with -no-undefined,"
	      $ECHO "*** libtool will only create a static version of it."
	      if test "$build_old_libs" = no; then
		oldlibs="$output_objdir/$libname.$libext"
		build_libtool_libs=module
		build_old_libs=yes
	      else
		build_libtool_libs=no
	      fi
	    fi
	  fi
	fi
	# Done checking deplibs!
	deplibs=$newdeplibs
      fi
      # Time to change all our "foo.ltframework" stuff back to "-framework foo"
      case $host in
	*-*-darwin*)
	  newdeplibs=`$ECHO "X $newdeplibs" | $Xsed -e 's% \([^ $]*\).ltframework% -framework \1%g'`
	  new_inherited_linker_flags=`$ECHO "X $new_inherited_linker_flags" | $Xsed -e 's% \([^ $]*\).ltframework% -framework \1%g'`
	  deplibs=`$ECHO "X $deplibs" | $Xsed -e 's% \([^ $]*\).ltframework% -framework \1%g'`
	  ;;
      esac

      # move library search paths that coincide with paths to not yet
      # installed libraries to the beginning of the library search list
      new_libs=
      for path in $notinst_path; do
	case " $new_libs " in
	*" -L$path/$objdir "*) ;;
	*)
	  case " $deplibs " in
	  *" -L$path/$objdir "*)
	    new_libs="$new_libs -L$path/$objdir" ;;
	  esac
	  ;;
	esac
      done
      for deplib in $deplibs; do
	case $deplib in
	-L*)
	  case " $new_libs " in
	  *" $deplib "*) ;;
	  *) new_libs="$new_libs $deplib" ;;
	  esac
	  ;;
	*) new_libs="$new_libs $deplib" ;;
	esac
      done
      deplibs="$new_libs"

      # All the library-specific variables (install_libdir is set above).
      library_names=
      old_library=
      dlname=

      # Test again, we may have decided not to build it any more
      if test "$build_libtool_libs" = yes; then
	if test "$hardcode_into_libs" = yes; then
	  # Hardcode the library paths
	  hardcode_libdirs=
	  dep_rpath=
	  rpath="$finalize_rpath"
	  test "$mode" != relink && rpath="$compile_rpath$rpath"
	  for libdir in $rpath; do
	    if test -n "$hardcode_libdir_flag_spec"; then
	      if test -n "$hardcode_libdir_separator"; then
		if test -z "$hardcode_libdirs"; then
		  hardcode_libdirs="$libdir"
		else
		  # Just accumulate the unique libdirs.
		  case $hardcode_libdir_separator$hardcode_libdirs$hardcode_libdir_separator in
		  *"$hardcode_libdir_separator$libdir$hardcode_libdir_separator"*)
		    ;;
		  *)
		    hardcode_libdirs="$hardcode_libdirs$hardcode_libdir_separator$libdir"
		    ;;
		  esac
		fi
	      else
		eval flag=\"$hardcode_libdir_flag_spec\"
		dep_rpath="$dep_rpath $flag"
	      fi
	    elif test -n "$runpath_var"; then
	      case "$perm_rpath " in
	      *" $libdir "*) ;;
	      *) perm_rpath="$perm_rpath $libdir" ;;
	      esac
	    fi
	  done
	  # Substitute the hardcoded libdirs into the rpath.
	  if test -n "$hardcode_libdir_separator" &&
	     test -n "$hardcode_libdirs"; then
	    libdir="$hardcode_libdirs"
	    if test -n "$hardcode_libdir_flag_spec_ld"; then
	      eval dep_rpath=\"$hardcode_libdir_flag_spec_ld\"
	    else
	      eval dep_rpath=\"$hardcode_libdir_flag_spec\"
	    fi
	  fi
	  if test -n "$runpath_var" && test -n "$perm_rpath"; then
	    # We should set the runpath_var.
	    rpath=
	    for dir in $perm_rpath; do
	      rpath="$rpath$dir:"
	    done
	    eval "$runpath_var='$rpath\$$runpath_var'; export $runpath_var"
	  fi
	  test -n "$dep_rpath" && deplibs="$dep_rpath $deplibs"
	fi

	shlibpath="$finalize_shlibpath"
	test "$mode" != relink && shlibpath="$compile_shlibpath$shlibpath"
	if test -n "$shlibpath"; then
	  eval "$shlibpath_var='$shlibpath\$$shlibpath_var'; export $shlibpath_var"
	fi

	# Get the real and link names of the library.
	eval shared_ext=\"$shrext_cmds\"
	eval library_names=\"$library_names_spec\"
	set dummy $library_names
	shift
	realname="$1"
	shift

	if test -n "$soname_spec"; then
	  eval soname=\"$soname_spec\"
	else
	  soname="$realname"
	fi
	if test -z "$dlname"; then
	  dlname=$soname
	fi

	lib="$output_objdir/$realname"
	linknames=
	for link
	do
	  linknames="$linknames $link"
	done

	# Use standard objects if they are pic
	test -z "$pic_flag" && libobjs=`$ECHO "X$libobjs" | $SP2NL | $Xsed -e "$lo2o" | $NL2SP`
	test "X$libobjs" = "X " && libobjs=

	delfiles=
	if test -n "$export_symbols" && test -n "$include_expsyms"; then
	  $opt_dry_run || cp "$export_symbols" "$output_objdir/$libname.uexp"
	  export_symbols="$output_objdir/$libname.uexp"
	  delfiles="$delfiles $export_symbols"
	fi

	orig_export_symbols=
	case $host_os in
	cygwin* | mingw*)
	  if test -n "$export_symbols" && test -z "$export_symbols_regex"; then
	    # exporting using user supplied symfile
	    if test "x`$SED 1q $export_symbols`" != xEXPORTS; then
	      # and it's NOT already a .def file. Must figure out
	      # which of the given symbols are data symbols and tag
	      # them as such. So, trigger use of export_symbols_cmds.
	      # export_symbols gets reassigned inside the "prepare
	      # the list of exported symbols" if statement, so the
	      # include_expsyms logic still works.
	      orig_export_symbols="$export_symbols"
	      export_symbols=
	      always_export_symbols=yes
	    fi
	  fi
	  ;;
	esac

	# Prepare the list of exported symbols
	if test -z "$export_symbols"; then
	  if test "$always_export_symbols" = yes || test -n "$export_symbols_regex"; then
	    func_verbose "generating symbol list for \`$libname.la'"
	    export_symbols="$output_objdir/$libname.exp"
	    $opt_dry_run || $RM $export_symbols
	    cmds=$export_symbols_cmds
	    save_ifs="$IFS"; IFS='~'
	    for cmd in $cmds; do
	      IFS="$save_ifs"
	      eval cmd=\"$cmd\"
	      func_len " $cmd"
	      len=$func_len_result
	      if test "$len" -lt "$max_cmd_len" || test "$max_cmd_len" -le -1; then
		func_show_eval "$cmd" 'exit $?'
		skipped_export=false
	      else
		# The command line is too long to execute in one step.
		func_verbose "using reloadable object file for export list..."
		skipped_export=:
		# Break out early, otherwise skipped_export may be
		# set to false by a later but shorter cmd.
		break
	      fi
	    done
	    IFS="$save_ifs"
	    if test -n "$export_symbols_regex" && test "X$skipped_export" != "X:"; then
	      func_show_eval '$EGREP -e "$export_symbols_regex" "$export_symbols" > "${export_symbols}T"'
	      func_show_eval '$MV "${export_symbols}T" "$export_symbols"'
	    fi
	  fi
	fi

	if test -n "$export_symbols" && test -n "$include_expsyms"; then
	  tmp_export_symbols="$export_symbols"
	  test -n "$orig_export_symbols" && tmp_export_symbols="$orig_export_symbols"
	  $opt_dry_run || eval '$ECHO "X$include_expsyms" | $Xsed | $SP2NL >> "$tmp_export_symbols"'
	fi

	if test "X$skipped_export" != "X:" && test -n "$orig_export_symbols"; then
	  # The given exports_symbols file has to be filtered, so filter it.
	  func_verbose "filter symbol list for \`$libname.la' to tag DATA exports"
	  # FIXME: $output_objdir/$libname.filter potentially contains lots of
	  # 's' commands which not all seds can handle. GNU sed should be fine
	  # though. Also, the filter scales superlinearly with the number of
	  # global variables. join(1) would be nice here, but unfortunately
	  # isn't a blessed tool.
	  $opt_dry_run || $SED -e '/[ ,]DATA/!d;s,\(.*\)\([ \,].*\),s|^\1$|\1\2|,' < $export_symbols > $output_objdir/$libname.filter
	  delfiles="$delfiles $export_symbols $output_objdir/$libname.filter"
	  export_symbols=$output_objdir/$libname.def
	  $opt_dry_run || $SED -f $output_objdir/$libname.filter < $orig_export_symbols > $export_symbols
	fi

	tmp_deplibs=
	for test_deplib in $deplibs; do
	  case " $convenience " in
	  *" $test_deplib "*) ;;
	  *)
	    tmp_deplibs="$tmp_deplibs $test_deplib"
	    ;;
	  esac
	done
	deplibs="$tmp_deplibs"

	if test -n "$convenience"; then
	  if test -n "$whole_archive_flag_spec" &&
	    test "$compiler_needs_object" = yes &&
	    test -z "$libobjs"; then
	    # extract the archives, so we have objects to list.
	    # TODO: could optimize this to just extract one archive.
	    whole_archive_flag_spec=
	  fi
	  if test -n "$whole_archive_flag_spec"; then
	    save_libobjs=$libobjs
	    eval libobjs=\"\$libobjs $whole_archive_flag_spec\"
	    test "X$libobjs" = "X " && libobjs=
	  else
	    gentop="$output_objdir/${outputname}x"
	    generated="$generated $gentop"

	    func_extract_archives $gentop $convenience
	    libobjs="$libobjs $func_extract_archives_result"
	    test "X$libobjs" = "X " && libobjs=
	  fi
	fi

	if test "$thread_safe" = yes && test -n "$thread_safe_flag_spec"; then
	  eval flag=\"$thread_safe_flag_spec\"
	  linker_flags="$linker_flags $flag"
	fi

	# Make a backup of the uninstalled library when relinking
	if test "$mode" = relink; then
	  $opt_dry_run || eval '(cd $output_objdir && $RM ${realname}U && $MV $realname ${realname}U)' || exit $?
	fi

	# Do each of the archive commands.
	if test "$module" = yes && test -n "$module_cmds" ; then
	  if test -n "$export_symbols" && test -n "$module_expsym_cmds"; then
	    eval test_cmds=\"$module_expsym_cmds\"
	    cmds=$module_expsym_cmds
	  else
	    eval test_cmds=\"$module_cmds\"
	    cmds=$module_cmds
	  fi
	else
	  if test -n "$export_symbols" && test -n "$archive_expsym_cmds"; then
	    eval test_cmds=\"$archive_expsym_cmds\"
	    cmds=$archive_expsym_cmds
	  else
	    eval test_cmds=\"$archive_cmds\"
	    cmds=$archive_cmds
	  fi
	fi

	if test "X$skipped_export" != "X:" &&
	   func_len " $test_cmds" &&
	   len=$func_len_result &&
	   test "$len" -lt "$max_cmd_len" || test "$max_cmd_len" -le -1; then
	  :
	else
	  # The command line is too long to link in one step, link piecewise
	  # or, if using GNU ld and skipped_export is not :, use a linker
	  # script.

	  # Save the value of $output and $libobjs because we want to
	  # use them later.  If we have whole_archive_flag_spec, we
	  # want to use save_libobjs as it was before
	  # whole_archive_flag_spec was expanded, because we can't
	  # assume the linker understands whole_archive_flag_spec.
	  # This may have to be revisited, in case too many
	  # convenience libraries get linked in and end up exceeding
	  # the spec.
	  if test -z "$convenience" || test -z "$whole_archive_flag_spec"; then
	    save_libobjs=$libobjs
	  fi
	  save_output=$output
	  output_la=`$ECHO "X$output" | $Xsed -e "$basename"`

	  # Clear the reloadable object creation command queue and
	  # initialize k to one.
	  test_cmds=
	  concat_cmds=
	  objlist=
	  last_robj=
	  k=1

	  if test -n "$save_libobjs" && test "X$skipped_export" != "X:" && test "$with_gnu_ld" = yes; then
	    output=${output_objdir}/${output_la}.lnkscript
	    func_verbose "creating GNU ld script: $output"
	    $ECHO 'INPUT (' > $output
	    for obj in $save_libobjs
	    do
	      $ECHO "$obj" >> $output
	    done
	    $ECHO ')' >> $output
	    delfiles="$delfiles $output"
	  elif test -n "$save_libobjs" && test "X$skipped_export" != "X:" && test "X$file_list_spec" != X; then
	    output=${output_objdir}/${output_la}.lnk
	    func_verbose "creating linker input file list: $output"
	    : > $output
	    set x $save_libobjs
	    shift
	    firstobj=
	    if test "$compiler_needs_object" = yes; then
	      firstobj="$1 "
	      shift
	    fi
	    for obj
	    do
	      $ECHO "$obj" >> $output
	    done
	    delfiles="$delfiles $output"
	    output=$firstobj\"$file_list_spec$output\"
	  else
	    if test -n "$save_libobjs"; then
	      func_verbose "creating reloadable object files..."
	      output=$output_objdir/$output_la-${k}.$objext
	      eval test_cmds=\"$reload_cmds\"
	      func_len " $test_cmds"
	      len0=$func_len_result
	      len=$len0

	      # Loop over the list of objects to be linked.
	      for obj in $save_libobjs
	      do
		func_len " $obj"
		func_arith $len + $func_len_result
		len=$func_arith_result
		if test "X$objlist" = X ||
		   test "$len" -lt "$max_cmd_len"; then
		  func_append objlist " $obj"
		else
		  # The command $test_cmds is almost too long, add a
		  # command to the queue.
		  if test "$k" -eq 1 ; then
		    # The first file doesn't have a previous command to add.
		    eval concat_cmds=\"$reload_cmds $objlist $last_robj\"
		  else
		    # All subsequent reloadable object files will link in
		    # the last one created.
		    eval concat_cmds=\"\$concat_cmds~$reload_cmds $objlist $last_robj~\$RM $last_robj\"
		  fi
		  last_robj=$output_objdir/$output_la-${k}.$objext
		  func_arith $k + 1
		  k=$func_arith_result
		  output=$output_objdir/$output_la-${k}.$objext
		  objlist=$obj
		  func_len " $last_robj"
		  func_arith $len0 + $func_len_result
		  len=$func_arith_result
		fi
	      done
	      # Handle the remaining objects by creating one last
	      # reloadable object file.  All subsequent reloadable object
	      # files will link in the last one created.
	      test -z "$concat_cmds" || concat_cmds=$concat_cmds~
	      eval concat_cmds=\"\${concat_cmds}$reload_cmds $objlist $last_robj\"
	      if test -n "$last_robj"; then
	        eval concat_cmds=\"\${concat_cmds}~\$RM $last_robj\"
	      fi
	      delfiles="$delfiles $output"

	    else
	      output=
	    fi

	    if ${skipped_export-false}; then
	      func_verbose "generating symbol list for \`$libname.la'"
	      export_symbols="$output_objdir/$libname.exp"
	      $opt_dry_run || $RM $export_symbols
	      libobjs=$output
	      # Append the command to create the export file.
	      test -z "$concat_cmds" || concat_cmds=$concat_cmds~
	      eval concat_cmds=\"\$concat_cmds$export_symbols_cmds\"
	      if test -n "$last_robj"; then
		eval concat_cmds=\"\$concat_cmds~\$RM $last_robj\"
	      fi
	    fi

	    test -n "$save_libobjs" &&
	      func_verbose "creating a temporary reloadable object file: $output"

	    # Loop through the commands generated above and execute them.
	    save_ifs="$IFS"; IFS='~'
	    for cmd in $concat_cmds; do
	      IFS="$save_ifs"
	      $opt_silent || {
		  func_quote_for_expand "$cmd"
		  eval "func_echo $func_quote_for_expand_result"
	      }
	      $opt_dry_run || eval "$cmd" || {
		lt_exit=$?

		# Restore the uninstalled library and exit
		if test "$mode" = relink; then
		  ( cd "$output_objdir" && \
		    $RM "${realname}T" && \
		    $MV "${realname}U" "$realname" )
		fi

		exit $lt_exit
	      }
	    done
	    IFS="$save_ifs"

	    if test -n "$export_symbols_regex" && ${skipped_export-false}; then
	      func_show_eval '$EGREP -e "$export_symbols_regex" "$export_symbols" > "${export_symbols}T"'
	      func_show_eval '$MV "${export_symbols}T" "$export_symbols"'
	    fi
	  fi

          if ${skipped_export-false}; then
	    if test -n "$export_symbols" && test -n "$include_expsyms"; then
	      tmp_export_symbols="$export_symbols"
	      test -n "$orig_export_symbols" && tmp_export_symbols="$orig_export_symbols"
	      $opt_dry_run || eval '$ECHO "X$include_expsyms" | $Xsed | $SP2NL >> "$tmp_export_symbols"'
	    fi

	    if test -n "$orig_export_symbols"; then
	      # The given exports_symbols file has to be filtered, so filter it.
	      func_verbose "filter symbol list for \`$libname.la' to tag DATA exports"
	      # FIXME: $output_objdir/$libname.filter potentially contains lots of
	      # 's' commands which not all seds can handle. GNU sed should be fine
	      # though. Also, the filter scales superlinearly with the number of
	      # global variables. join(1) would be nice here, but unfortunately
	      # isn't a blessed tool.
	      $opt_dry_run || $SED -e '/[ ,]DATA/!d;s,\(.*\)\([ \,].*\),s|^\1$|\1\2|,' < $export_symbols > $output_objdir/$libname.filter
	      delfiles="$delfiles $export_symbols $output_objdir/$libname.filter"
	      export_symbols=$output_objdir/$libname.def
	      $opt_dry_run || $SED -f $output_objdir/$libname.filter < $orig_export_symbols > $export_symbols
	    fi
	  fi

	  libobjs=$output
	  # Restore the value of output.
	  output=$save_output

	  if test -n "$convenience" && test -n "$whole_archive_flag_spec"; then
	    eval libobjs=\"\$libobjs $whole_archive_flag_spec\"
	    test "X$libobjs" = "X " && libobjs=
	  fi
	  # Expand the library linking commands again to reset the
	  # value of $libobjs for piecewise linking.

	  # Do each of the archive commands.
	  if test "$module" = yes && test -n "$module_cmds" ; then
	    if test -n "$export_symbols" && test -n "$module_expsym_cmds"; then
	      cmds=$module_expsym_cmds
	    else
	      cmds=$module_cmds
	    fi
	  else
	    if test -n "$export_symbols" && test -n "$archive_expsym_cmds"; then
	      cmds=$archive_expsym_cmds
	    else
	      cmds=$archive_cmds
	    fi
	  fi
	fi

	if test -n "$delfiles"; then
	  # Append the command to remove temporary files to $cmds.
	  eval cmds=\"\$cmds~\$RM $delfiles\"
	fi

	# Add any objects from preloaded convenience libraries
	if test -n "$dlprefiles"; then
	  gentop="$output_objdir/${outputname}x"
	  generated="$generated $gentop"

	  func_extract_archives $gentop $dlprefiles
	  libobjs="$libobjs $func_extract_archives_result"
	  test "X$libobjs" = "X " && libobjs=
	fi

	save_ifs="$IFS"; IFS='~'
	for cmd in $cmds; do
	  IFS="$save_ifs"
	  eval cmd=\"$cmd\"
	  $opt_silent || {
	    func_quote_for_expand "$cmd"
	    eval "func_echo $func_quote_for_expand_result"
	  }
	  $opt_dry_run || eval "$cmd" || {
	    lt_exit=$?

	    # Restore the uninstalled library and exit
	    if test "$mode" = relink; then
	      ( cd "$output_objdir" && \
	        $RM "${realname}T" && \
		$MV "${realname}U" "$realname" )
	    fi

	    exit $lt_exit
	  }
	done
	IFS="$save_ifs"

	# Restore the uninstalled library and exit
	if test "$mode" = relink; then
	  $opt_dry_run || eval '(cd $output_objdir && $RM ${realname}T && $MV $realname ${realname}T && $MV ${realname}U $realname)' || exit $?

	  if test -n "$convenience"; then
	    if test -z "$whole_archive_flag_spec"; then
	      func_show_eval '${RM}r "$gentop"'
	    fi
	  fi

	  exit $EXIT_SUCCESS
	fi

	# Create links to the real library.
	for linkname in $linknames; do
	  if test "$realname" != "$linkname"; then
	    func_show_eval '(cd "$output_objdir" && $RM "$linkname" && $LN_S "$realname" "$linkname")' 'exit $?'
	  fi
	done

	# If -module or -export-dynamic was specified, set the dlname.
	if test "$module" = yes || test "$export_dynamic" = yes; then
	  # On all known operating systems, these are identical.
	  dlname="$soname"
	fi
      fi
      ;;

    obj)
      if test -n "$dlfiles$dlprefiles" || test "$dlself" != no; then
	func_warning "\`-dlopen' is ignored for objects"
      fi

      case " $deplibs" in
      *\ -l* | *\ -L*)
	func_warning "\`-l' and \`-L' are ignored for objects" ;;
      esac

      test -n "$rpath" && \
	func_warning "\`-rpath' is ignored for objects"

      test -n "$xrpath" && \
	func_warning "\`-R' is ignored for objects"

      test -n "$vinfo" && \
	func_warning "\`-version-info' is ignored for objects"

      test -n "$release" && \
	func_warning "\`-release' is ignored for objects"

      case $output in
      *.lo)
	test -n "$objs$old_deplibs" && \
	  func_fatal_error "cannot build library object \`$output' from non-libtool objects"

	libobj=$output
	func_lo2o "$libobj"
	obj=$func_lo2o_result
	;;
      *)
	libobj=
	obj="$output"
	;;
      esac

      # Delete the old objects.
      $opt_dry_run || $RM $obj $libobj

      # Objects from convenience libraries.  This assumes
      # single-version convenience libraries.  Whenever we create
      # different ones for PIC/non-PIC, this we'll have to duplicate
      # the extraction.
      reload_conv_objs=
      gentop=
      # reload_cmds runs $LD directly, so let us get rid of
      # -Wl from whole_archive_flag_spec and hope we can get by with
      # turning comma into space..
      wl=

      if test -n "$convenience"; then
	if test -n "$whole_archive_flag_spec"; then
	  eval tmp_whole_archive_flags=\"$whole_archive_flag_spec\"
	  reload_conv_objs=$reload_objs\ `$ECHO "X$tmp_whole_archive_flags" | $Xsed -e 's|,| |g'`
	else
	  gentop="$output_objdir/${obj}x"
	  generated="$generated $gentop"

	  func_extract_archives $gentop $convenience
	  reload_conv_objs="$reload_objs $func_extract_archives_result"
	fi
      fi

      # Create the old-style object.
      reload_objs="$objs$old_deplibs "`$ECHO "X$libobjs" | $SP2NL | $Xsed -e '/\.'${libext}$'/d' -e '/\.lib$/d' -e "$lo2o" | $NL2SP`" $reload_conv_objs" ### testsuite: skip nested quoting test

      output="$obj"
      func_execute_cmds "$reload_cmds" 'exit $?'

      # Exit if we aren't doing a library object file.
      if test -z "$libobj"; then
	if test -n "$gentop"; then
	  func_show_eval '${RM}r "$gentop"'
	fi

	exit $EXIT_SUCCESS
      fi

      if test "$build_libtool_libs" != yes; then
	if test -n "$gentop"; then
	  func_show_eval '${RM}r "$gentop"'
	fi

	# Create an invalid libtool object if no PIC, so that we don't
	# accidentally link it into a program.
	# $show "echo timestamp > $libobj"
	# $opt_dry_run || eval "echo timestamp > $libobj" || exit $?
	exit $EXIT_SUCCESS
      fi

      if test -n "$pic_flag" || test "$pic_mode" != default; then
	# Only do commands if we really have different PIC objects.
	reload_objs="$libobjs $reload_conv_objs"
	output="$libobj"
	func_execute_cmds "$reload_cmds" 'exit $?'
      fi

      if test -n "$gentop"; then
	func_show_eval '${RM}r "$gentop"'
      fi

      exit $EXIT_SUCCESS
      ;;

    prog)
      case $host in
	*cygwin*) func_stripname '' '.exe' "$output"
	          output=$func_stripname_result.exe;;
      esac
      test -n "$vinfo" && \
	func_warning "\`-version-info' is ignored for programs"

      test -n "$release" && \
	func_warning "\`-release' is ignored for programs"

      test "$preload" = yes \
        && test "$dlopen_support" = unknown \
	&& test "$dlopen_self" = unknown \
	&& test "$dlopen_self_static" = unknown && \
	  func_warning "\`LT_INIT([dlopen])' not used. Assuming no dlopen support."

      case $host in
      *-*-rhapsody* | *-*-darwin1.[012])
	# On Rhapsody replace the C library is the System framework
	compile_deplibs=`$ECHO "X $compile_deplibs" | $Xsed -e 's/ -lc / System.ltframework /'`
	finalize_deplibs=`$ECHO "X $finalize_deplibs" | $Xsed -e 's/ -lc / System.ltframework /'`
	;;
      esac

      case $host in
      *-*-darwin*)
	# Don't allow lazy linking, it breaks C++ global constructors
	# But is supposedly fixed on 10.4 or later (yay!).
	if test "$tagname" = CXX ; then
	  case ${MACOSX_DEPLOYMENT_TARGET-10.0} in
	    10.[0123])
	      compile_command="$compile_command ${wl}-bind_at_load"
	      finalize_command="$finalize_command ${wl}-bind_at_load"
	    ;;
	  esac
	fi
	# Time to change all our "foo.ltframework" stuff back to "-framework foo"
	compile_deplibs=`$ECHO "X $compile_deplibs" | $Xsed -e 's% \([^ $]*\).ltframework% -framework \1%g'`
	finalize_deplibs=`$ECHO "X $finalize_deplibs" | $Xsed -e 's% \([^ $]*\).ltframework% -framework \1%g'`
	;;
      esac


      # move library search paths that coincide with paths to not yet
      # installed libraries to the beginning of the library search list
      new_libs=
      for path in $notinst_path; do
	case " $new_libs " in
	*" -L$path/$objdir "*) ;;
	*)
	  case " $compile_deplibs " in
	  *" -L$path/$objdir "*)
	    new_libs="$new_libs -L$path/$objdir" ;;
	  esac
	  ;;
	esac
      done
      for deplib in $compile_deplibs; do
	case $deplib in
	-L*)
	  case " $new_libs " in
	  *" $deplib "*) ;;
	  *) new_libs="$new_libs $deplib" ;;
	  esac
	  ;;
	*) new_libs="$new_libs $deplib" ;;
	esac
      done
      compile_deplibs="$new_libs"


      compile_command="$compile_command $compile_deplibs"
      finalize_command="$finalize_command $finalize_deplibs"

      if test -n "$rpath$xrpath"; then
	# If the user specified any rpath flags, then add them.
	for libdir in $rpath $xrpath; do
	  # This is the magic to use -rpath.
	  case "$finalize_rpath " in
	  *" $libdir "*) ;;
	  *) finalize_rpath="$finalize_rpath $libdir" ;;
	  esac
	done
      fi

      # Now hardcode the library paths
      rpath=
      hardcode_libdirs=
      for libdir in $compile_rpath $finalize_rpath; do
	if test -n "$hardcode_libdir_flag_spec"; then
	  if test -n "$hardcode_libdir_separator"; then
	    if test -z "$hardcode_libdirs"; then
	      hardcode_libdirs="$libdir"
	    else
	      # Just accumulate the unique libdirs.
	      case $hardcode_libdir_separator$hardcode_libdirs$hardcode_libdir_separator in
	      *"$hardcode_libdir_separator$libdir$hardcode_libdir_separator"*)
		;;
	      *)
		hardcode_libdirs="$hardcode_libdirs$hardcode_libdir_separator$libdir"
		;;
	      esac
	    fi
	  else
	    eval flag=\"$hardcode_libdir_flag_spec\"
	    rpath="$rpath $flag"
	  fi
	elif test -n "$runpath_var"; then
	  case "$perm_rpath " in
	  *" $libdir "*) ;;
	  *) perm_rpath="$perm_rpath $libdir" ;;
	  esac
	fi
	case $host in
	*-*-cygwin* | *-*-mingw* | *-*-pw32* | *-*-os2*)
	  testbindir=`${ECHO} "$libdir" | ${SED} -e 's*/lib$*/bin*'`
	  case :$dllsearchpath: in
	  *":$libdir:"*) ;;
	  *) dllsearchpath="$dllsearchpath:$libdir";;
	  esac
	  case :$dllsearchpath: in
	  *":$testbindir:"*) ;;
	  *) dllsearchpath="$dllsearchpath:$testbindir";;
	  esac
	  ;;
	esac
      done
      # Substitute the hardcoded libdirs into the rpath.
      if test -n "$hardcode_libdir_separator" &&
	 test -n "$hardcode_libdirs"; then
	libdir="$hardcode_libdirs"
	eval rpath=\" $hardcode_libdir_flag_spec\"
      fi
      compile_rpath="$rpath"

      rpath=
      hardcode_libdirs=
      for libdir in $finalize_rpath; do
	if test -n "$hardcode_libdir_flag_spec"; then
	  if test -n "$hardcode_libdir_separator"; then
	    if test -z "$hardcode_libdirs"; then
	      hardcode_libdirs="$libdir"
	    else
	      # Just accumulate the unique libdirs.
	      case $hardcode_libdir_separator$hardcode_libdirs$hardcode_libdir_separator in
	      *"$hardcode_libdir_separator$libdir$hardcode_libdir_separator"*)
		;;
	      *)
		hardcode_libdirs="$hardcode_libdirs$hardcode_libdir_separator$libdir"
		;;
	      esac
	    fi
	  else
	    eval flag=\"$hardcode_libdir_flag_spec\"
	    rpath="$rpath $flag"
	  fi
	elif test -n "$runpath_var"; then
	  case "$finalize_perm_rpath " in
	  *" $libdir "*) ;;
	  *) finalize_perm_rpath="$finalize_perm_rpath $libdir" ;;
	  esac
	fi
      done
      # Substitute the hardcoded libdirs into the rpath.
      if test -n "$hardcode_libdir_separator" &&
	 test -n "$hardcode_libdirs"; then
	libdir="$hardcode_libdirs"
	eval rpath=\" $hardcode_libdir_flag_spec\"
      fi
      finalize_rpath="$rpath"

      if test -n "$libobjs" && test "$build_old_libs" = yes; then
	# Transform all the library objects into standard objects.
	compile_command=`$ECHO "X$compile_command" | $SP2NL | $Xsed -e "$lo2o" | $NL2SP`
	finalize_command=`$ECHO "X$finalize_command" | $SP2NL | $Xsed -e "$lo2o" | $NL2SP`
      fi

      func_generate_dlsyms "$outputname" "@PROGRAM@" "no"

      # template prelinking step
      if test -n "$prelink_cmds"; then
	func_execute_cmds "$prelink_cmds" 'exit $?'
      fi

      wrappers_required=yes
      case $host in
      *cygwin* | *mingw* )
        if test "$build_libtool_libs" != yes; then
          wrappers_required=no
        fi
        ;;
      *)
        if test "$need_relink" = no || test "$build_libtool_libs" != yes; then
          wrappers_required=no
        fi
        ;;
      esac
      if test "$wrappers_required" = no; then
	# Replace the output file specification.
	compile_command=`$ECHO "X$compile_command" | $Xsed -e 's%@OUTPUT@%'"$output"'%g'`
	link_command="$compile_command$compile_rpath"

	# We have no uninstalled library dependencies, so finalize right now.
	exit_status=0
	func_show_eval "$link_command" 'exit_status=$?'

	# Delete the generated files.
	if test -f "$output_objdir/${outputname}S.${objext}"; then
	  func_show_eval '$RM "$output_objdir/${outputname}S.${objext}"'
	fi

	exit $exit_status
      fi

      if test -n "$compile_shlibpath$finalize_shlibpath"; then
	compile_command="$shlibpath_var=\"$compile_shlibpath$finalize_shlibpath\$$shlibpath_var\" $compile_command"
      fi
      if test -n "$finalize_shlibpath"; then
	finalize_command="$shlibpath_var=\"$finalize_shlibpath\$$shlibpath_var\" $finalize_command"
      fi

      compile_var=
      finalize_var=
      if test -n "$runpath_var"; then
	if test -n "$perm_rpath"; then
	  # We should set the runpath_var.
	  rpath=
	  for dir in $perm_rpath; do
	    rpath="$rpath$dir:"
	  done
	  compile_var="$runpath_var=\"$rpath\$$runpath_var\" "
	fi
	if test -n "$finalize_perm_rpath"; then
	  # We should set the runpath_var.
	  rpath=
	  for dir in $finalize_perm_rpath; do
	    rpath="$rpath$dir:"
	  done
	  finalize_var="$runpath_var=\"$rpath\$$runpath_var\" "
	fi
      fi

      if test "$no_install" = yes; then
	# We don't need to create a wrapper script.
	link_command="$compile_var$compile_command$compile_rpath"
	# Replace the output file specification.
	link_command=`$ECHO "X$link_command" | $Xsed -e 's%@OUTPUT@%'"$output"'%g'`
	# Delete the old output file.
	$opt_dry_run || $RM $output
	# Link the executable and exit
	func_show_eval "$link_command" 'exit $?'
	exit $EXIT_SUCCESS
      fi

      if test "$hardcode_action" = relink; then
	# Fast installation is not supported
	link_command="$compile_var$compile_command$compile_rpath"
	relink_command="$finalize_var$finalize_command$finalize_rpath"

	func_warning "this platform does not like uninstalled shared libraries"
	func_warning "\`$output' will be relinked during installation"
      else
	if test "$fast_install" != no; then
	  link_command="$finalize_var$compile_command$finalize_rpath"
	  if test "$fast_install" = yes; then
	    relink_command=`$ECHO "X$compile_var$compile_command$compile_rpath" | $Xsed -e 's%@OUTPUT@%\$progdir/\$file%g'`
	  else
	    # fast_install is set to needless
	    relink_command=
	  fi
	else
	  link_command="$compile_var$compile_command$compile_rpath"
	  relink_command="$finalize_var$finalize_command$finalize_rpath"
	fi
      fi

      # Replace the output file specification.
      link_command=`$ECHO "X$link_command" | $Xsed -e 's%@OUTPUT@%'"$output_objdir/$outputname"'%g'`

      # Delete the old output files.
      $opt_dry_run || $RM $output $output_objdir/$outputname $output_objdir/lt-$outputname

      func_show_eval "$link_command" 'exit $?'

      # Now create the wrapper script.
      func_verbose "creating $output"

      # Quote the relink command for shipping.
      if test -n "$relink_command"; then
	# Preserve any variables that may affect compiler behavior
	for var in $variables_saved_for_relink; do
	  if eval test -z \"\${$var+set}\"; then
	    relink_command="{ test -z \"\${$var+set}\" || $lt_unset $var || { $var=; export $var; }; }; $relink_command"
	  elif eval var_value=\$$var; test -z "$var_value"; then
	    relink_command="$var=; export $var; $relink_command"
	  else
	    func_quote_for_eval "$var_value"
	    relink_command="$var=$func_quote_for_eval_result; export $var; $relink_command"
	  fi
	done
	relink_command="(cd `pwd`; $relink_command)"
	relink_command=`$ECHO "X$relink_command" | $Xsed -e "$sed_quote_subst"`
      fi

      # Quote $ECHO for shipping.
      if test "X$ECHO" = "X$SHELL $progpath --fallback-echo"; then
	case $progpath in
	[\\/]* | [A-Za-z]:[\\/]*) qecho="$SHELL $progpath --fallback-echo";;
	*) qecho="$SHELL `pwd`/$progpath --fallback-echo";;
	esac
	qecho=`$ECHO "X$qecho" | $Xsed -e "$sed_quote_subst"`
      else
	qecho=`$ECHO "X$ECHO" | $Xsed -e "$sed_quote_subst"`
      fi

      # Only actually do things if not in dry run mode.
      $opt_dry_run || {
	# win32 will think the script is a binary if it has
	# a .exe suffix, so we strip it off here.
	case $output in
	  *.exe) func_stripname '' '.exe' "$output"
	         output=$func_stripname_result ;;
	esac
	# test for cygwin because mv fails w/o .exe extensions
	case $host in
	  *cygwin*)
	    exeext=.exe
	    func_stripname '' '.exe' "$outputname"
	    outputname=$func_stripname_result ;;
	  *) exeext= ;;
	esac
	case $host in
	  *cygwin* | *mingw* )
	    func_dirname_and_basename "$output" "" "."
	    output_name=$func_basename_result
	    output_path=$func_dirname_result
	    cwrappersource="$output_path/$objdir/lt-$output_name.c"
	    cwrapper="$output_path/$output_name.exe"
	    $RM $cwrappersource $cwrapper
	    trap "$RM $cwrappersource $cwrapper; exit $EXIT_FAILURE" 1 2 15

	    func_emit_cwrapperexe_src > $cwrappersource

	    # we should really use a build-platform specific compiler
	    # here, but OTOH, the wrappers (shell script and this C one)
	    # are only useful if you want to execute the "real" binary.
	    # Since the "real" binary is built for $host, then this
	    # wrapper might as well be built for $host, too.
	    $opt_dry_run || {
	      $LTCC $LTCFLAGS -o $cwrapper $cwrappersource
	      $STRIP $cwrapper
	    }

	    # Now, create the wrapper script for func_source use:
	    func_ltwrapper_scriptname $cwrapper
	    $RM $func_ltwrapper_scriptname_result
	    trap "$RM $func_ltwrapper_scriptname_result; exit $EXIT_FAILURE" 1 2 15
	    $opt_dry_run || {
	      # note: this script will not be executed, so do not chmod.
	      if test "x$build" = "x$host" ; then
		$cwrapper --lt-dump-script > $func_ltwrapper_scriptname_result
	      else
		func_emit_wrapper no > $func_ltwrapper_scriptname_result
	      fi
	    }
	  ;;
	  * )
	    $RM $output
	    trap "$RM $output; exit $EXIT_FAILURE" 1 2 15

	    func_emit_wrapper no > $output
	    chmod +x $output
	  ;;
	esac
      }
      exit $EXIT_SUCCESS
      ;;
    esac

    # See if we need to build an old-fashioned archive.
    for oldlib in $oldlibs; do

      if test "$build_libtool_libs" = convenience; then
	oldobjs="$libobjs_save $symfileobj"
	addlibs="$convenience"
	build_libtool_libs=no
      else
	if test "$build_libtool_libs" = module; then
	  oldobjs="$libobjs_save"
	  build_libtool_libs=no
	else
	  oldobjs="$old_deplibs $non_pic_objects"
	  if test "$preload" = yes && test -f "$symfileobj"; then
	    oldobjs="$oldobjs $symfileobj"
	  fi
	fi
	addlibs="$old_convenience"
      fi

      if test -n "$addlibs"; then
	gentop="$output_objdir/${outputname}x"
	generated="$generated $gentop"

	func_extract_archives $gentop $addlibs
	oldobjs="$oldobjs $func_extract_archives_result"
      fi

      # Do each command in the archive commands.
      if test -n "$old_archive_from_new_cmds" && test "$build_libtool_libs" = yes; then
	cmds=$old_archive_from_new_cmds
      else

	# Add any objects from preloaded convenience libraries
	if test -n "$dlprefiles"; then
	  gentop="$output_objdir/${outputname}x"
	  generated="$generated $gentop"

	  func_extract_archives $gentop $dlprefiles
	  oldobjs="$oldobjs $func_extract_archives_result"
	fi

	# POSIX demands no paths to be encoded in archives.  We have
	# to avoid creating archives with duplicate basenames if we
	# might have to extract them afterwards, e.g., when creating a
	# static archive out of a convenience library, or when linking
	# the entirety of a libtool archive into another (currently
	# not supported by libtool).
	if (for obj in $oldobjs
	    do
	      func_basename "$obj"
	      $ECHO "$func_basename_result"
	    done | sort | sort -uc >/dev/null 2>&1); then
	  :
	else
	  $ECHO "copying selected object files to avoid basename conflicts..."
	  gentop="$output_objdir/${outputname}x"
	  generated="$generated $gentop"
	  func_mkdir_p "$gentop"
	  save_oldobjs=$oldobjs
	  oldobjs=
	  counter=1
	  for obj in $save_oldobjs
	  do
	    func_basename "$obj"
	    objbase="$func_basename_result"
	    case " $oldobjs " in
	    " ") oldobjs=$obj ;;
	    *[\ /]"$objbase "*)
	      while :; do
		# Make sure we don't pick an alternate name that also
		# overlaps.
		newobj=lt$counter-$objbase
		func_arith $counter + 1
		counter=$func_arith_result
		case " $oldobjs " in
		*[\ /]"$newobj "*) ;;
		*) if test ! -f "$gentop/$newobj"; then break; fi ;;
		esac
	      done
	      func_show_eval "ln $obj $gentop/$newobj || cp $obj $gentop/$newobj"
	      oldobjs="$oldobjs $gentop/$newobj"
	      ;;
	    *) oldobjs="$oldobjs $obj" ;;
	    esac
	  done
	fi
	eval cmds=\"$old_archive_cmds\"

	func_len " $cmds"
	len=$func_len_result
	if test "$len" -lt "$max_cmd_len" || test "$max_cmd_len" -le -1; then
	  cmds=$old_archive_cmds
	else
	  # the command line is too long to link in one step, link in parts
	  func_verbose "using piecewise archive linking..."
	  save_RANLIB=$RANLIB
	  RANLIB=:
	  objlist=
	  concat_cmds=
	  save_oldobjs=$oldobjs
	  oldobjs=
	  # Is there a better way of finding the last object in the list?
	  for obj in $save_oldobjs
	  do
	    last_oldobj=$obj
	  done
	  eval test_cmds=\"$old_archive_cmds\"
	  func_len " $test_cmds"
	  len0=$func_len_result
	  len=$len0
	  for obj in $save_oldobjs
	  do
	    func_len " $obj"
	    func_arith $len + $func_len_result
	    len=$func_arith_result
	    func_append objlist " $obj"
	    if test "$len" -lt "$max_cmd_len"; then
	      :
	    else
	      # the above command should be used before it gets too long
	      oldobjs=$objlist
	      if test "$obj" = "$last_oldobj" ; then
		RANLIB=$save_RANLIB
	      fi
	      test -z "$concat_cmds" || concat_cmds=$concat_cmds~
	      eval concat_cmds=\"\${concat_cmds}$old_archive_cmds\"
	      objlist=
	      len=$len0
	    fi
	  done
	  RANLIB=$save_RANLIB
	  oldobjs=$objlist
	  if test "X$oldobjs" = "X" ; then
	    eval cmds=\"\$concat_cmds\"
	  else
	    eval cmds=\"\$concat_cmds~\$old_archive_cmds\"
	  fi
	fi
      fi
      func_execute_cmds "$cmds" 'exit $?'
    done

    test -n "$generated" && \
      func_show_eval "${RM}r$generated"

    # Now create the libtool archive.
    case $output in
    *.la)
      old_library=
      test "$build_old_libs" = yes && old_library="$libname.$libext"
      func_verbose "creating $output"

      # Preserve any variables that may affect compiler behavior
      for var in $variables_saved_for_relink; do
	if eval test -z \"\${$var+set}\"; then
	  relink_command="{ test -z \"\${$var+set}\" || $lt_unset $var || { $var=; export $var; }; }; $relink_command"
	elif eval var_value=\$$var; test -z "$var_value"; then
	  relink_command="$var=; export $var; $relink_command"
	else
	  func_quote_for_eval "$var_value"
	  relink_command="$var=$func_quote_for_eval_result; export $var; $relink_command"
	fi
      done
      # Quote the link command for shipping.
      relink_command="(cd `pwd`; $SHELL $progpath $preserve_args --mode=relink $libtool_args @inst_prefix_dir@)"
      relink_command=`$ECHO "X$relink_command" | $Xsed -e "$sed_quote_subst"`
      if test "$hardcode_automatic" = yes ; then
	relink_command=
      fi

      # Only create the output if not a dry run.
      $opt_dry_run || {
	for installed in no yes; do
	  if test "$installed" = yes; then
	    if test -z "$install_libdir"; then
	      break
	    fi
	    output="$output_objdir/$outputname"i
	    # Replace all uninstalled libtool libraries with the installed ones
	    newdependency_libs=
	    for deplib in $dependency_libs; do
	      case $deplib in
	      *.la)
		func_basename "$deplib"
		name="$func_basename_result"
		eval libdir=`${SED} -n -e 's/^libdir=\(.*\)$/\1/p' $deplib`
		test -z "$libdir" && \
		  func_fatal_error "\`$deplib' is not a valid libtool archive"
		newdependency_libs="$newdependency_libs $libdir/$name"
		;;
	      *) newdependency_libs="$newdependency_libs $deplib" ;;
	      esac
	    done
	    dependency_libs="$newdependency_libs"
	    newdlfiles=

	    for lib in $dlfiles; do
	      case $lib in
	      *.la)
	        func_basename "$lib"
		name="$func_basename_result"
		eval libdir=`${SED} -n -e 's/^libdir=\(.*\)$/\1/p' $lib`
		test -z "$libdir" && \
		  func_fatal_error "\`$lib' is not a valid libtool archive"
		newdlfiles="$newdlfiles $libdir/$name"
		;;
	      *) newdlfiles="$newdlfiles $lib" ;;
	      esac
	    done
	    dlfiles="$newdlfiles"
	    newdlprefiles=
	    for lib in $dlprefiles; do
	      case $lib in
	      *.la)
		# Only pass preopened files to the pseudo-archive (for
		# eventual linking with the app. that links it) if we
		# didn't already link the preopened objects directly into
		# the library:
		func_basename "$lib"
		name="$func_basename_result"
		eval libdir=`${SED} -n -e 's/^libdir=\(.*\)$/\1/p' $lib`
		test -z "$libdir" && \
		  func_fatal_error "\`$lib' is not a valid libtool archive"
		newdlprefiles="$newdlprefiles $libdir/$name"
		;;
	      esac
	    done
	    dlprefiles="$newdlprefiles"
	  else
	    newdlfiles=
	    for lib in $dlfiles; do
	      case $lib in
		[\\/]* | [A-Za-z]:[\\/]*) abs="$lib" ;;
		*) abs=`pwd`"/$lib" ;;
	      esac
	      newdlfiles="$newdlfiles $abs"
	    done
	    dlfiles="$newdlfiles"
	    newdlprefiles=
	    for lib in $dlprefiles; do
	      case $lib in
		[\\/]* | [A-Za-z]:[\\/]*) abs="$lib" ;;
		*) abs=`pwd`"/$lib" ;;
	      esac
	      newdlprefiles="$newdlprefiles $abs"
	    done
	    dlprefiles="$newdlprefiles"
	  fi
	  $RM $output
	  # place dlname in correct position for cygwin
	  tdlname=$dlname
	  case $host,$output,$installed,$module,$dlname in
	    *cygwin*,*lai,yes,no,*.dll | *mingw*,*lai,yes,no,*.dll) tdlname=../bin/$dlname ;;
	  esac
	  $ECHO > $output "\
# $outputname - a libtool library file
# Generated by $PROGRAM (GNU $PACKAGE$TIMESTAMP) $VERSION
#
# Please DO NOT delete this file!
# It is necessary for linking the library.

# The name that we can dlopen(3).
dlname='$tdlname'

# Names of this library.
library_names='$library_names'

# The name of the static archive.
old_library='$old_library'

# Linker flags that can not go in dependency_libs.
inherited_linker_flags='$new_inherited_linker_flags'

# Libraries that this one depends upon.
dependency_libs='$dependency_libs'

# Names of additional weak libraries provided by this library
weak_library_names='$weak_libs'

# Version information for $libname.
current=$current
age=$age
revision=$revision

# Is this an already installed library?
installed=$installed

# Should we warn about portability when linking against -modules?
shouldnotlink=$module

# Files to dlopen/dlpreopen
dlopen='$dlfiles'
dlpreopen='$dlprefiles'

# Directory that this library needs to be installed in:
libdir='$install_libdir'"
	  if test "$installed" = no && test "$need_relink" = yes; then
	    $ECHO >> $output "\
relink_command=\"$relink_command\""
	  fi
	done
      }

      # Do a symbolic link so that the libtool archive can be found in
      # LD_LIBRARY_PATH before the program is installed.
      func_show_eval '( cd "$output_objdir" && $RM "$outputname" && $LN_S "../$outputname" "$outputname" )' 'exit $?'
      ;;
    esac
    exit $EXIT_SUCCESS
}

{ test "$mode" = link || test "$mode" = relink; } &&
    func_mode_link ${1+"$@"}


# func_mode_uninstall arg...
func_mode_uninstall ()
{
    $opt_debug
    RM="$nonopt"
    files=
    rmforce=
    exit_status=0

    # This variable tells wrapper scripts just to set variables rather
    # than running their programs.
    libtool_install_magic="$magic"

    for arg
    do
      case $arg in
      -f) RM="$RM $arg"; rmforce=yes ;;
      -*) RM="$RM $arg" ;;
      *) files="$files $arg" ;;
      esac
    done

    test -z "$RM" && \
      func_fatal_help "you must specify an RM program"

    rmdirs=

    origobjdir="$objdir"
    for file in $files; do
      func_dirname "$file" "" "."
      dir="$func_dirname_result"
      if test "X$dir" = X.; then
	objdir="$origobjdir"
      else
	objdir="$dir/$origobjdir"
      fi
      func_basename "$file"
      name="$func_basename_result"
      test "$mode" = uninstall && objdir="$dir"

      # Remember objdir for removal later, being careful to avoid duplicates
      if test "$mode" = clean; then
	case " $rmdirs " in
	  *" $objdir "*) ;;
	  *) rmdirs="$rmdirs $objdir" ;;
	esac
      fi

      # Don't error if the file doesn't exist and rm -f was used.
      if { test -L "$file"; } >/dev/null 2>&1 ||
	 { test -h "$file"; } >/dev/null 2>&1 ||
	 test -f "$file"; then
	:
      elif test -d "$file"; then
	exit_status=1
	continue
      elif test "$rmforce" = yes; then
	continue
      fi

      rmfiles="$file"

      case $name in
      *.la)
	# Possibly a libtool archive, so verify it.
	if func_lalib_p "$file"; then
	  func_source $dir/$name

	  # Delete the libtool libraries and symlinks.
	  for n in $library_names; do
	    rmfiles="$rmfiles $objdir/$n"
	  done
	  test -n "$old_library" && rmfiles="$rmfiles $objdir/$old_library"

	  case "$mode" in
	  clean)
	    case "  $library_names " in
	    # "  " in the beginning catches empty $dlname
	    *" $dlname "*) ;;
	    *) rmfiles="$rmfiles $objdir/$dlname" ;;
	    esac
	    test -n "$libdir" && rmfiles="$rmfiles $objdir/$name $objdir/${name}i"
	    ;;
	  uninstall)
	    if test -n "$library_names"; then
	      # Do each command in the postuninstall commands.
	      func_execute_cmds "$postuninstall_cmds" 'test "$rmforce" = yes || exit_status=1'
	    fi

	    if test -n "$old_library"; then
	      # Do each command in the old_postuninstall commands.
	      func_execute_cmds "$old_postuninstall_cmds" 'test "$rmforce" = yes || exit_status=1'
	    fi
	    # FIXME: should reinstall the best remaining shared library.
	    ;;
	  esac
	fi
	;;

      *.lo)
	# Possibly a libtool object, so verify it.
	if func_lalib_p "$file"; then

	  # Read the .lo file
	  func_source $dir/$name

	  # Add PIC object to the list of files to remove.
	  if test -n "$pic_object" &&
	     test "$pic_object" != none; then
	    rmfiles="$rmfiles $dir/$pic_object"
	  fi

	  # Add non-PIC object to the list of files to remove.
	  if test -n "$non_pic_object" &&
	     test "$non_pic_object" != none; then
	    rmfiles="$rmfiles $dir/$non_pic_object"
	  fi
	fi
	;;

      *)
	if test "$mode" = clean ; then
	  noexename=$name
	  case $file in
	  *.exe)
	    func_stripname '' '.exe' "$file"
	    file=$func_stripname_result
	    func_stripname '' '.exe' "$name"
	    noexename=$func_stripname_result
	    # $file with .exe has already been added to rmfiles,
	    # add $file without .exe
	    rmfiles="$rmfiles $file"
	    ;;
	  esac
	  # Do a test to see if this is a libtool program.
	  if func_ltwrapper_p "$file"; then
	    if func_ltwrapper_executable_p "$file"; then
	      func_ltwrapper_scriptname "$file"
	      relink_command=
	      func_source $func_ltwrapper_scriptname_result
	      rmfiles="$rmfiles $func_ltwrapper_scriptname_result"
	    else
	      relink_command=
	      func_source $dir/$noexename
	    fi

	    # note $name still contains .exe if it was in $file originally
	    # as does the version of $file that was added into $rmfiles
	    rmfiles="$rmfiles $objdir/$name $objdir/${name}S.${objext}"
	    if test "$fast_install" = yes && test -n "$relink_command"; then
	      rmfiles="$rmfiles $objdir/lt-$name"
	    fi
	    if test "X$noexename" != "X$name" ; then
	      rmfiles="$rmfiles $objdir/lt-${noexename}.c"
	    fi
	  fi
	fi
	;;
      esac
      func_show_eval "$RM $rmfiles" 'exit_status=1'
    done
    objdir="$origobjdir"

    # Try to remove the ${objdir}s in the directories where we deleted files
    for dir in $rmdirs; do
      if test -d "$dir"; then
	func_show_eval "rmdir $dir >/dev/null 2>&1"
      fi
    done

    exit $exit_status
}

{ test "$mode" = uninstall || test "$mode" = clean; } &&
    func_mode_uninstall ${1+"$@"}

test -z "$mode" && {
  help="$generic_help"
  func_fatal_help "you must specify a MODE"
}

test -z "$exec_cmd" && \
  func_fatal_help "invalid operation mode \`$mode'"

if test -n "$exec_cmd"; then
  eval exec "$exec_cmd"
  exit $EXIT_FAILURE
fi

exit $exit_status


# The TAGs below are defined such that we never get into a situation
# in which we disable both kinds of libraries.  Given conflicting
# choices, we go for a static library, that is the most portable,
# since we can't tell whether shared libraries were disabled because
# the user asked for that or because the platform doesn't support
# them.  This is particularly important on AIX, because we don't
# support having both static and shared libraries enabled at the same
# time on that platform, so we default to a shared-only configuration.
# If a disable-shared tag is given, we'll fallback to a static-only
# configuration.  But we'll never go from static-only to shared-only.

# ### BEGIN LIBTOOL TAG CONFIG: disable-shared
build_libtool_libs=no
build_old_libs=yes
# ### END LIBTOOL TAG CONFIG: disable-shared

# ### BEGIN LIBTOOL TAG CONFIG: disable-static
build_old_libs=`case $build_libtool_libs in yes) echo no;; *) echo yes;; esac`
# ### END LIBTOOL TAG CONFIG: disable-static

# Local Variables:
# mode:shell-script
# sh-indentation:2
# End:
# vi:sw=2

