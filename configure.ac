dnl
dnl Configure script for DeadEnds software
dnl Portions derived from GDB, the GNU debugger, and are
dnl Copyright 1995-2016 Free Software Foundation, Inc.
dnl
dnl Process this file with autoconf to produce a configure script.

dnl **************************************************************
dnl Autoconf Section
dnl **************************************************************

AC_INIT([DeadEnds],[0.6])
AC_PREREQ([2.71])
AC_REVISION([for DeadEnds, built with autoconf] AC_ACVERSION)
AC_DISABLE_OPTION_CHECKING
AC_CONFIG_AUX_DIR(build/autotools)
AC_CANONICAL_HOST
AC_CONFIG_HEADERS([config.h])

dnl **************************************************************
dnl Automake Section
dnl **************************************************************

dnl Use 'foreign' to avoid requiring GNU files in top-level.
AM_INIT_AUTOMAKE([foreign])
#AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

# Dependency checking.
dnl ZW_CREATE_DEPDIR
dnl ZW_PROG_COMPILER_DEPENDENCIES([CC])

dnl **************************************************************
dnl Internationalization Section
dnl **************************************************************

dnl gettext
AM_GNU_GETTEXT([external],[need-ngettext])
AM_GNU_GETTEXT_VERSION([0.11.5])

dnl iconv
AM_ICONV

#dnl langinfo
#AM_LANGINFO_CODESET

dnl **************************************************************
dnl Compile-Chain Section
dnl **************************************************************

dnl Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_INSTALL

#dnl MTE: 01/11/04: Force the use of GNU Bison only.
#dnl MTE: AC_PROG_YACC will find any YACC, but only
#dnl MTE: Bison works for us.
AC_PROG_YACC
#if test "$YACC" != "bison -y";
#then
#  echo "LifeLines requires GNU Bison to compile src/interp/yacc.y."
#  exit
#fi

dnl **************************************************************
dnl OS/Compiler-Specific Section
dnl **************************************************************

dnl Add warning flags when using The GNU C Compiler
if test "${ac_cv_c_compiler_gnu}" = "yes"; then
  CFLAGS="${CFLAGS} -W -Wall -Wextra"
  CFLAGS="${CFLAGS} -Wcast-align -Wmissing-declarations -Wmissing-prototypes"
# CFLAGS="${CFLAGS} -Wreturn-type -Wstrict-prototypes -pedantic"
  CFLAGS="${CFLAGS} -Wreturn-type -Wstrict-prototypes"

  dnl Add debugging as well
  CFLAGS="${CFLAGS} -g"
fi

# OS-Specific Fixups
# Cygwin - fixes linker problems
# Darwin - will pick up Fink-installed headers and libraries
case $host in
  *pc-cygwin*)
    CFLAGS="${CFLAGS} -DBROKEN_LINKER"
    ;;  
  *apple-darwin*)
    CPPFLAGS="${CPPFLAGS} -I/sw/include";
    LDFLAGS="${LDFLAGS} -L/sw/lib"
    ;;
esac

dnl Specify bit width if requested
AC_ARG_WITH(32bit,
    [  --with-32bit             Build in 32-bit mode ],
    [build_32bit=$withval],
    [build_32bit=no])

AC_ARG_WITH(64bit,
    [  --with-64bit             Build in 64-bit mode ],
    [build_64bit=$withval],
    [build_64bit=no])

if test "$build_32bit" = "yes"
then
  if test "${ac_cv_c_compiler_gnu}" = "yes"; then
    CFLAGS="${CFLAGS} -m32"
    LDFLAGS="${LDFLAGS} -m32"
  else
    echo "The use of --with-32bit is only supported when using gcc!"
    exit
  fi
fi

if test "$build_64bit" = "yes"
then
  if test "${ac_cv_c_compiler_gnu}" = "yes"; then
    CFLAGS="${CFLAGS} -m64"
    LDFLAGS="${LDFLAGS} -m64"
  else
    echo "The use of --with-64bit is only supported when using gcc!"
    exit
  fi
fi

dnl Specify code coverage if requested
AC_ARG_WITH(coverage,
    [  --with-coverage             Build with code coverage ],
    [build_coverage=$withval],
    [build_coverage=no])

if test "$build_coverage" = "yes"
then
  if test "${ac_cv_c_compiler_gnu}" = "yes"; then
    CFLAGS="${CFLAGS} --coverage"
  else
    echo "The use of --with-coverage is only supported when using gcc!"
    exit
  fi
fi

dnl Enable full debugging
AC_ARG_WITH(full-debug,
    [  --with-full-debug           Build with full debug support ],
    [build_full_debug=$withval],
    [build_full_debug=no])

if test "$build_full_debug" = "yes"
then
  if test "${ac_cv_c_compiler_gnu}" = "yes"; then
    CFLAGS="${CFLAGS} -rdynamic"
  else
    echo "The use of --with-full-debug only supported when using gcc!"
    exit
  fi
fi

dnl **************************************************************
dnl Handle testing requirements
dnl **************************************************************

dnl Some tests need the 'expect' program.
echo "Looking for expect"
AC_PATH_PROG(TOOL_EXPECT, expect, FAIL)
if test "$TOOL_EXPECT" = "FAIL"; then
    AC_MSG_RESULT( ************************************************************************ )
    AC_MSG_RESULT( ******** Cannot find expect.  Some tests will be skipped. ************** )
    AC_MSG_RESULT( ************************************************************************ )
fi

AM_CONDITIONAL(HAVE_EXPECT, test "$TOOL_EXPECT" != "FAIL")

dnl **************************************************************
dnl Handle Docs Target
dnl **************************************************************

# Shall we build the docs?
AC_ARG_WITH(docs,
    [  --with-docs             Build the docs from source (requires xmlto and dblatex) ],
    [build_docs=$withval],
    [build_docs=no])

if test "$build_docs" = "yes"
then
    echo "Looking for xmlto"
    AC_PATH_PROG(TOOL_XMLTO, xmlto, FAIL)
    if test "$TOOL_XMLTO" = "FAIL"; then
        AC_MSG_RESULT( ************************************************************************ )
        AC_MSG_RESULT( ******** Cannot find xmlto to build docs.  Will not build docs. ******** )
        AC_MSG_RESULT( ************************************************************************ )
        build_docs=no
    fi
    AC_PATH_PROG(TOOL_DBLATEX, dblatex, FAIL)
    if test "$TOOL_DBLATEX" = "FAIL"; then
        AC_MSG_RESULT( ************************************************************************ )
        AC_MSG_RESULT( ******* Cannot find dblatex to build docs.  Will not build docs. ******* )
        AC_MSG_RESULT( ************************************************************************ )
        build_docs=no
    fi
fi

AM_CONDITIONAL(BUILD_DOCS, test "$build_docs" = "yes")

dnl **************************************************************
dnl Handle Profiling Target
dnl **************************************************************

# Compile with profiling, to find bottlenecks
AC_ARG_WITH(profiling,
  [  --with-profiling        Compile with profiling support],
  [# Does it work for other compilers then GCC? [pere 2000-12-30]
   if test "${ac_cv_c_compiler_gnu}" = "yes"; then
      CFLAGS="${CFLAGS} -pg -a"
      LDFLAGS="${LDFLAGS} -pg -a"

      # Make sure 'config.h' is changed if profiling is turned on, to
      # trigger recompile for every source file.
      AC_DEFINE(PROFILING, 1, [Profiling enabled?])
   else
      echo "Do not know how to perform profiling using this compiler!"
   fi
  ]
)

dnl **************************************************************
dnl Check for Standard Headers/Structures/Libraries
dnl **************************************************************

echo Looking for header files
AC_CHECK_HEADERS( getopt.h dirent.h pwd.h locale.h windows.h )
AC_CHECK_HEADERS( wchar.h wctype.h )
AC_CHECK_HEADERS( math.h )
AC_CHECK_HEADERS( malloc.h malloc/malloc.h )
AC_CHECK_HEADERS( langinfo.h )
AC_CHECK_HEADERS( pwd.h )

echo Looking for library functions
AC_CHECK_FUNCS( _vsnprintf getpwuid setlocale )
AC_CHECK_FUNCS( wcscoll towlower towupper iswspace iswalpha )
AC_CHECK_FUNCS( backtrace backtrace_symbols )
AC_CHECK_FUNCS( malloc_size malloc_usable_size )
#AC_SEARCH_LIBS( sin, m )
#AC_SEARCH_LIBS( cos, m )
#AC_SEARCH_LIBS( tan, m )
#AC_SEARCH_LIBS( asin, m )
#AC_SEARCH_LIBS( acos, m )
#AC_SEARCH_LIBS( atan, m )

echo Looking for header files and libraries to support heap walking
dnl We need to cheat on OSX/Darwin.  The standard configure
dnl check brings in a C++ header which bombs and causes
dnl configure to display a particularly nasty warning, which
dnl we don't want to expose to end-users.
case $host in
  *apple-darwin*)
    HAVE_ALLOC_H=no
    HAVE_MALLOC_H=no
    HAVE_HEAPWALK=no
    HAVE__HEAPWALK=no
    ;;
  *)
    AC_CHECK_HEADERS( alloc.h malloc.h )
    AC_CHECK_FUNCS( heapwalk _heapwalk )
    ;;
esac

dnl **************************************************************
dnl Check for curses
dnl **************************************************************

dnl Since some distributions package wide and narrow functions
dnl in separate libraries, we need to check for them separately.
dnl We also need to search for a wide character routine, so that
dnl we can warn if only a narrow version of curses is found.
dnl
dnl We search for libraries first, since its easier to tell if
dnl the right libraries are installed than the right headers.

echo Looking for curses libraries
AC_CHECK_LIB(ncursesw, in_wch)
if test "$ac_cv_lib_ncursesw_in_wch" = "yes"; then
  echo Using ncursesw
else
  AC_CHECK_LIB(ncurses, in_wch)
  if test "$ac_cv_lib_ncurses_in_wch" = "yes"; then
    echo Using ncurses
  else
    AC_CHECK_LIB(cursesw, in_wch)
    if test "$ac_cv_lib_cursesw_in_wch" = "yes"; then
      echo Using cursesw
    else
      AC_CHECK_LIB(curses, in_wch)
      if test "$ac_cv_lib_curses_in_wch" = "yes"; then
        echo Using curses
      else
        case $host in
          *mingw32*)
            echo "Using LifeLines version of curses for Windows."
          ;;
          *)
            echo "LifeLines requires a curses implementation with wide character support!"
            exit
          ;;
        esac
      fi
    fi
  fi
fi

dnl Check for headers
echo Looking for curses headers
AC_CHECK_HEADERS( ncursesw/curses.h ncurses/curses.h curses.h )

dnl **************************************************************
dnl Check for Python
dnl **************************************************************
# --------------------- #
# Check for libpython.  #
# --------------------- #

dnl Utility to simplify finding libpython.
dnl $1 = the shell variable to assign the result to
dnl      If libpython is found we store $version here.
dnl $2 = additional flags to add to CPPFLAGS
dnl $3 = additional flags to add to LIBS

AC_DEFUN([AC_TRY_LIBPYTHON],
[
  define([have_libpython_var],$1)
  new_CPPFLAGS=$2
  new_LIBS=$3
  AC_MSG_CHECKING([for python])
  save_CPPFLAGS=$CPPFLAGS
  save_LIBS=$LIBS
  CPPFLAGS="$CPPFLAGS $new_CPPFLAGS"
  LIBS="$new_LIBS $LIBS"
  found_usable_python=no
  #AC_MSG_NOTICE(AC TRY LIBPYTHON entered; new_CPPFLAGS: $new_CPPFLAGS)
  #AC_MSG_NOTICE(AC TRY LIBPYTHON entered; new_LIBS: $new_LIBS)
  #AC_MSG_NOTICE(AC TRY LIBPYTHON entered; CPPFLAGS: $CPPFLAGS)
  #AC_MSG_NOTICE(AC TRY LIBPYTHON entered; LIBS: $LIBS)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include "Python.h"]],
                                 [[
				    #if PY_MAJOR_VERSION != 3
				    # error "We only support Python 3"
				    #endif
				    Py_Initialize ();
				 ]])],
                 [have_libpython_var=yes
                  found_usable_python=yes
                  PYTHON_CPPFLAGS=$new_CPPFLAGS
                  PYTHON_LIBS=$new_LIBS])
  CPPFLAGS=$save_CPPFLAGS
  LIBS=$save_LIBS
  AC_MSG_RESULT([${found_usable_python}])
])

dnl There are several different values for --with-python:
dnl
dnl no -   Don't include python support.
dnl yes -  Include python support, error if it's missing.
dnl        If we find python in $PATH, use it to fetch configure options,
dnl        otherwise assume the compiler can find it with no help from us.
dnl auto - Same as "yes", but if python is missing from the system,
dnl        fall back to "no".
dnl /path/to/python/exec-prefix -
dnl        Use the python located in this directory.
dnl        If /path/to/python/exec-prefix/bin/python exists, use it to find
dnl        the compilation parameters.  Otherwise use
dnl        -I/path/to/python/exec-prefix/include,
dnl        -L/path/to/python/exec-prefix/lib.
dnl        NOTE: This case is historical.  It is what was done for 7.0/7.1
dnl        but is deprecated.
dnl /path/to/python/executable -
dnl        Run python-config.py with this version of python to fetch the
dnl        compilation parameters.
dnl        NOTE: This needn't be the real python executable.
dnl        In a cross-compilation scenario (build != host), this could be
dnl        a shell script that provides what python-config.py provides for
dnl        --ldflags, --includes, --exec-prefix.
dnl python-executable -
dnl        Find python-executable in $PATH, and then handle the same as
dnl        /path/to/python/executable.
dnl
dnl If a python program is specified, it is used to run python-config.py and
dnl is passed --ldflags, --includes, --exec-prefix.

AC_ARG_WITH(python,
  AS_HELP_STRING([--with-python@<:@=PYTHON@:>@], [include python support (auto/yes/no/<python-program>)]),
  [], [with_python=auto])
AC_MSG_CHECKING([whether to use python])
AC_MSG_RESULT([$with_python])

if test "${with_python}" = no; then
  AC_MSG_WARN([python support disabled; some features may be unavailable.])
  have_libpython=no
else
  case "${with_python}" in
  [[\\/]]* | ?:[[\\/]]*)
    if test -d "${with_python}"; then
      # Assume the python binary is ${with_python}/bin/python.
      python_prog="${with_python}/bin/python3"
      python_prefix=
      # If python does not exit ${with_python}/bin, then try in
      # ${with_python}.  On Windows/MinGW, this is where the Python
      # executable is.
      if test ! -x "${python_prog}"; then
        python_prog="${with_python}/python3"
        python_prefix=
      fi
      if test ! -x "${python_prog}"; then
        # Fall back to gdb 7.0/7.1 behaviour.
        python_prog=missing
        python_prefix=${with_python}
      fi
    elif test -x "${with_python}"; then
      # While we can't run python compiled for $host (unless host == build),
      # the user could write a script that provides the needed information,
      # so we support that.
      python_prog=${with_python}
      python_prefix=
    else
      AC_MSG_ERROR(invalid value for --with-python)
    fi
    ;;
  */*)
    # Disallow --with-python=foo/bar.
    AC_MSG_ERROR(invalid value for --with-python)
    ;;
  *)
    # The user has either specified auto, yes, or the name of the python
    # program assumed to be in $PATH.
    python_prefix=
    case "${with_python}" in
    yes | auto)
      if test "${build}" = "${host}"; then
        AC_PATH_PROG(python_prog_path, python3, missing)
        if test "${python_prog_path}" = missing; then
          python_prog=missing
        else
          python_prog=${python_prog_path}
        fi
      else
        # Not much we can do except assume the cross-compiler will find the
        # right files.
        python_prog=missing
      fi
      ;;
    *)
      # While we can't run python compiled for $host (unless host == build),
      # the user could write a script that provides the needed information,
      # so we support that.
      python_prog="${with_python}"
      AC_PATH_PROG(python_prog_path, ${python_prog}, missing)
      if test "${python_prog_path}" = missing; then
	AC_MSG_ERROR(unable to find python program ${python_prog})
      fi
      ;;
    esac
  esac

  if test "${python_prog}" != missing; then
    # We have a python program to use, but it may be too old.
    # Don't flag an error for --with-python=auto (the default).
    have_python_config=yes
    python_includes=`${python_prog} ${srcdir}/build/python-config.py --includes`
    if test $? != 0; then
      have_python_config=failed
      if test "${with_python}" != auto; then
	AC_MSG_ERROR(failure running python-config --includes)
      fi
    fi
    python_libs=`${python_prog} ${srcdir}/build/python-config.py --ldflags --embed`
    if test $? != 0; then
      have_python_config=failed
      if test "${with_python}" != auto; then
	AC_MSG_ERROR(failure running python-config --ldflags)
      fi
    fi
    python_prefix=`${python_prog} ${srcdir}/build/python-config.py --exec-prefix`
    if test $? != 0; then
      have_python_config=failed
      if test "${with_python}" != auto; then
	AC_MSG_ERROR(failure running python-config --exec-prefix)
      fi
    fi
  else
    # We do not have a python executable we can use to determine where
    # to find the Python headers and libs.  We cannot guess the include
    # path from the python_prefix either, because that include path
    # depends on the Python version.  So, there is nothing much we can
    # do except assume that the compiler will be able to find those files.
    python_includes=
    python_libs=
    have_python_config=no
  fi

  # If we have python-config, only try the configuration it provides.
  # Otherwise fallback on the old way of trying different versions of
  # python in turn.

  have_libpython=no
  if test "${have_python_config}" = yes; then
    AC_TRY_LIBPYTHON(have_libpython,
                     ${python_includes}, ${python_libs})
  fi

  #AC_MSG_NOTICE(final PYTHON_CPPFLAGS: $PYTHON_CPPFLAGS)
  #AC_MSG_NOTICE(final PYTHON_LIBS: $PYTHON_LIBS)

  if test "${have_libpython}" = no; then
    case "${with_python}" in
    yes)
      AC_MSG_ERROR([python is missing or unusable])
      ;;
    auto)
      AC_MSG_WARN([python is missing or unusable; some features may be unavailable.])
      ;;
    *)
      AC_MSG_ERROR([no usable python found at ${with_python}])
      ;;
    esac
  else
    if test -n "${python_prefix}"; then
      AC_DEFINE_UNQUOTED(WITH_PYTHON_PATH, "${python_prefix}",
                         [Define if --with-python provides a path, either directly or via python-config.py --exec-prefix.])
      dnl GDB_AC_DEFINE_RELOCATABLE(PYTHON_PATH, python, ${python_prefix})
    fi
  fi
fi

dnl Use --with-python-libdir to control where DeadEnds looks for the
dnl Python libraries.
dnl
dnl If this is not given then the default will be based on the value
dnl passed to --with-python, which is in the python_prefix variable.
dnl If the --with-python option wasn't given then the default value in
dnl python_prefix is based on running the 'build/python-config.py
dnl --exec-prefix' script.
AC_ARG_WITH(python-libdir,
  AS_HELP_STRING([--with-python-libdir@<:@=DIR@:>@], [search for python's libraries in DIR]),
  [],[
    # If no python libdir is specified then select one based on
    # python's prefix path.
    if test -n "${python_prefix}"; then
      with_python_libdir=${python_prefix}/lib
    fi
  ])

if test "${have_libpython}" != no; then
  AC_DEFINE(HAVE_PYTHON, 1, [Define if Python interpreter is being linked in.])
  CONFIG_OBS="$CONFIG_OBS \$(SUBDIR_PYTHON_OBS)"
  CONFIG_DEPS="$CONFIG_DEPS \$(SUBDIR_PYTHON_DEPS)"
  CONFIG_SRCS="$CONFIG_SRCS \$(SUBDIR_PYTHON_SRCS)"
  CONFIG_INSTALL="$CONFIG_INSTALL install-python"
  ENABLE_CFLAGS="$ENABLE_CFLAGS \$(SUBDIR_PYTHON_CFLAGS)"

  if test -n "${with_python_libdir}"; then
    AC_DEFINE_UNQUOTED(WITH_PYTHON_LIBDIR, "${with_python_libdir}",
		       [Directory containing Python's standard libraries from --with-python-libdir.])
    dnl GDB_AC_DEFINE_RELOCATABLE(PYTHON_LIBDIR, [python lib], ${with_python_libdir})
  fi

  # Flags needed to compile Python code (taken from python-config --cflags).
  # We cannot call python-config directly because it will output whatever was
  # used when compiling the Python interpreter itself, including flags which
  # would make the python-related objects be compiled differently from the
  # rest of GDB (e.g., -O2 and -fPIC).
  if test "${GCC}" = yes; then
    tentative_python_cflags="-fno-strict-aliasing -fwrapv"
    # Python headers recommend -DNDEBUG, but it's unclear if that just
    # refers to building Python itself.  In release mode, though, it
    # doesn't hurt for the Python code in gdb to follow.
    $development || tentative_python_cflags="$tentative_python_cflags -DNDEBUG"
  fi

  if test "x${tentative_python_cflags}" != x; then
    AC_MSG_CHECKING(compiler flags for python code)
    for flag in ${tentative_python_cflags}; do
      # Check that the compiler accepts it
      saved_CFLAGS="$CFLAGS"
      CFLAGS="$CFLAGS $flag"
      AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([], [])],
	[PYTHON_CFLAGS="${PYTHON_CFLAGS} $flag"],
	[]
      )
      CFLAGS="$saved_CFLAGS"
    done
    AC_MSG_RESULT(${PYTHON_CFLAGS})
  fi

  # On x64 Windows, Python's include headers, and pyconfig.h in
  # particular, rely on MS_WIN64 macro to detect that it's a 64bit
  # version of Windows.  Unfortunately, MS_WIN64 is only defined if
  # _MSC_VER, a Microsoft-specific macro, is defined.  So, when
  # building on x64 Windows with GCC, we define MS_WIN64 ourselves.
  # The issue was reported to the Python community, but still isn't
  # solved as of 2012-10-02 (http://bugs.python.org/issue4709).

  case "$gdb_host" in
    mingw64)
           if test "${GCC}" = yes; then
             CPPFLAGS="$CPPFLAGS -DMS_WIN64"
           fi
           ;;
  esac
else
  # Even if Python support is not compiled in, we need to have this file
  # included so that the "python" command, et.al., still exists.
  CONFIG_OBS="$CONFIG_OBS python.o"
  CONFIG_SRCS="$CONFIG_SRCS python/python.c"
fi

# Work around Python http://bugs.python.org/issue10112.  See also
# http://bugs.python.org/issue11410, otherwise -Wl,--dynamic-list has
# no effect.  Note that the only test after this that uses Python is
# the -rdynamic/-Wl,--dynamic-list test, and we do want that one to be
# run without -export-dynamic too.
PYTHON_LIBS=`echo $PYTHON_LIBS | sed -e 's/-Xlinker -export-dynamic//'`

if test "${have_libpython}" != "no"; then
  LIBPYLLINES="../DeadEndsLib/Python/libpyllines.a"
  PYTHON_DIR="python"
else
  LIBPYLLINES=""
  PYTHON_DIR=""
fi

AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_CPPFLAGS)
AC_SUBST(PYTHON_LIBS)
AC_SUBST(LIBPYLLINES)
AC_SUBST(PYTHON_DIR)
AM_CONDITIONAL(HAVE_PYTHON, test "${have_libpython}" != no)

dnl **************************************************************
dnl Check for Replacement Functions for Standard Libraries/Curses
dnl **************************************************************

echo Checking for replacement functions
dnl Standard libraries
AC_REPLACE_FUNCS( sleep scandir alphasort getopt snprintf vsnprintf )
dnl Standard libraries - internationalization / wchar
AC_REPLACE_FUNCS( nl_langinfo wcslen )
dnl Standard libraries - curses
AC_REPLACE_FUNCS( has_key )

dnl **************************************************************
dnl Platform-Specific Configuration
dnl **************************************************************

case $host_os in
  *windows*)
    AC_DEFINE(DEADENDS_CONFIG_FILE,"deadends.cfg",[lifelines configuration file])
    ;;
  *)
    AC_DEFINE(DEADENDS_CONFIG_FILE,".deadendsrc",[lifelines configuration file])
    ;;
esac

dnl **************************************************************
dnl Generate Makefiles
dnl **************************************************************

AC_CONFIG_FILES([Makefile \
	DeadEndsLib/Makefile \
        DeadEndsLib/Database/Makefile \
        DeadEndsLib/DataTypes/Makefile \
        DeadEndsLib/Gedcom/Makefile \
        DeadEndsLib/Interp/Makefile \
        DeadEndsLib/LLSupport/Makefile \
	DeadEndsLib/Operations/Makefile \
        DeadEndsLib/Parser/Makefile \
        DeadEndsLib/Python/Makefile \
        DeadEndsLib/UI/Makefile \
        DeadEndsLib/Utils/Makefile \
        DeadEndsLib/Validate/Makefile \
        Docs/Manual/Makefile \
	GenerateSExpressions/Makefile \
        Lifelines/Makefile \
	MenuLibrary/Makefile \
	MultiBases/Makefile \
	Partition/Makefile \
	PatchSex/Makefile \
        Programs/Makefile \
	RandomizeKeys/Makefile \
        Reports/Makefile \
        Reports/desc-tex2/Makefile \
        Reports/novel/Makefile \
        Reports/pedtex/Makefile \
        Reports/ps-fan/Makefile \
        Reports/st/Makefile \
	RunScript/Makefile \
	TestDates/Makefile \
	TestProgram/Makefile \
	tests/Makefile \
	UseMenus/Makefile])

AC_OUTPUT
