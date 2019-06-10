PHP_ARG_ENABLE(openrasp, whether to enable openrasp support,
[  --enable-openrasp       Enable openrasp support])

PHP_ARG_WITH(openrasp-v8, for openrasp-v8 support,
[  --with-openrasp-v8=DIR  Set the path to openrasp-v8], no, no)

PHP_ARG_WITH(gettext, for gettext support,
[  --with-gettext=DIR   Set the path to gettext], no, no)

PHP_ARG_ENABLE(openrasp-remote-manager, whether to enable openrasp remote manager support,
[  --enable-openrasp-remote-manager       Enable openrasp remote manager support (Linux Only)], no, no)

PHP_ARG_WITH(openssl, for openssl support,
[  --with-openssl=DIR   Include openssl support], no, no)

PHP_ARG_WITH(curl, for curl support,
[  --with-curl=DIR   Include curl support], no, no)

PHP_ARG_ENABLE(fswatch, Enable fswatch,
[  --enable-fswatch      Enable fswatch], no, no)

PHP_ARG_ENABLE(line-coverage, Enable line coverage,
[  --enable-line-coverage      Enable line coverage], no, no)

PHP_ARG_ENABLE(cli-support, Enable cli support,
[  --enable-cli-support      Enable cli support], no, no)

if test "$PHP_OPENRASP" != "no"; then
  PHP_REQUIRE_CXX()
  if test "$PHP_JSON" = "no" && test "$ext_shared" = "no"; then
    AC_MSG_ERROR([JSON is not enabled! Add --enable-json to your configure line.])
  fi

  AC_MSG_CHECKING([for openrasp-v8 library])
  if test $PHP_OPENRASP_V8 == "no"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please set openrasp-v8 path with "--with-openrasp-v8"])
  else
    AC_MSG_RESULT([yes])
  fi

  PHP_ADD_INCLUDE($PHP_OPENRASP_V8)
  OPENRASP_LIBS="$OPENRASP_LIBS $PHP_OPENRASP_V8/php/build/libopenrasp_v8_php.a"
  case $host_os in
    darwin* )
      PREBUILTS_ROOT="$PHP_OPENRASP_V8/prebuilts/darwin"
      PHP_ADD_INCLUDE($PREBUILTS_ROOT/include)
      OPENRASP_LIBS="$OPENRASP_LIBS -L$PREBUILTS_ROOT/lib64 -lv8_monolith"
      ;;
    * )
      PREBUILTS_ROOT="$PHP_OPENRASP_V8/prebuilts/linux"
      PHP_ADD_INCLUDE($PREBUILTS_ROOT/include)
      PHP_ADD_INCLUDE($PREBUILTS_ROOT/include/c++/v1)
      CXXFLAGS="$CXXFLAGS -nostdinc++"
      OPENRASP_LIBS="$OPENRASP_LIBS -L$PREBUILTS_ROOT/lib64 -Wl,-Bstatic -lv8_monolith -lc++ -lc++abi -Wl,-Bdynamic"
      OPENRASP_LIBS="$OPENRASP_LIBS -nodefaultlibs -lm -lc -lrt -lgcc_s -ldl -lpthread"
      ;;
  esac

  SEARCH_PATH="/usr/local /usr"

  if test "$PHP_GETTEXT" != "no"; then
    SEARCH_FOR="/include/libintl.h"
    if test -r $PHP_GETTEXT/$SEARCH_FOR; then
      GETTEXT_PATH=$PHP_GETTEXT
    else
      AC_MSG_CHECKING([for gettext files in default path])
      for i in $SEARCH_PATH ; do
        if test -r $i/$SEARCH_FOR; then
          GETTEXT_PATH=$i
          AC_MSG_RESULT(found in $i)
        fi
      done
    fi

    if test -z "$GETTEXT_PATH"; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([Please reinstall the gettext distribution])
    fi

    O_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -L$GETTEXT_PATH/$PHP_LIBDIR"
    AC_CHECK_LIB(intl, bindtextdomain, [
    GETTEXT_LIBS=intl
    GETTEXT_CHECK_IN_LIB=intl
    ],
    AC_CHECK_LIB(c, bindtextdomain, [
      GETTEXT_LIBS=
      GETTEXT_CHECK_IN_LIB=c
    ],[
      AC_MSG_ERROR(Unable to find required gettext library)
    ])
    )
    LDFLAGS=$O_LDFLAGS

    AC_DEFINE([HAVE_GETTEXT], [1], [Have gettext support])
    PHP_ADD_INCLUDE($GETTEXT_PATH/include)
    OPENRASP_LIBS="$GETTEXT_LIBS $OPENRASP_LIBS"
  fi

  if test "$PHP_OPENRASP_REMOTE_MANAGER" != "no"; then
    case $host_os in
      darwin* )
        ;;
      * )
        dnl check openssl support
        if test "$ext_shared" != "no"; then
          if test "$PHP_OPENSSL" != "no" && test -n "$PHP_OPENSSL"; then
            OPENSSL_SEARCH_PATH="$PHP_OPENSSL"
          else
            OPENSSL_SEARCH_PATH="$SEARCH_PATH"
          fi

          AC_MSG_CHECKING([for openssl headers location])
          for i in $OPENSSL_SEARCH_PATH ; do
            for j in $i $i/ssl $i/openssl; do    
              test -f $j/include/openssl/evp.h && OPENRASP_OPENSSL_INCDIR=$j/include
            done
          done

          if test -z "$OPENRASP_OPENSSL_INCDIR"; then
            AC_MSG_ERROR([Could not find evp.h in $OPENSSL_SEARCH_PATH])
          fi
          AC_MSG_RESULT([$OPENRASP_OPENSSL_INCDIR])
          PHP_ADD_INCLUDE($OPENRASP_OPENSSL_INCDIR)

          AC_MSG_CHECKING([for ssl library location])
          for i in $OPENSSL_SEARCH_PATH ; do
            for j in $i/$PHP_LIBDIR $i/ssl/$PHP_LIBDIR $i/openssl/$PHP_LIBDIR $i/lib64 $i/ssl/lib64 $i/openssl/lib64 $i/lib/x86_64-linux-gnu; do
              if test -f $j/libssl.$SHLIB_SUFFIX_NAME;then
                OPENRASP_SSL_LIBDIR=$j
                PHP_ADD_LIBRARY_WITH_PATH(ssl, $OPENRASP_SSL_LIBDIR, OPENRASP_SHARED_LIBADD)
              elif test -f $j/libssl.a; then
                OPENRASP_SSL_LIBDIR=$j
                SSL_LIBS="$OPENRASP_SSL_LIBDIR/libssl.a"
                case $host_os in
                  darwin* )
                    OPENRASP_LIBS="-Wl,$SSL_LIBS $OPENRASP_LIBS"
                    ;;
                  * )
                    OPENRASP_LIBS="-Wl,--whole-archive -Wl,$SSL_LIBS -Wl,--no-whole-archive $OPENRASP_LIBS"
                    ;;
                esac
              fi
              if test -n "$OPENRASP_SSL_LIBDIR" ; then
                break 2
              fi
            done
          done
    
          if test -z "$OPENRASP_SSL_LIBDIR" ; then
            AC_MSG_ERROR([Could not find libssl.(a|$SHLIB_SUFFIX_NAME) in $OPENSSL_SEARCH_PATH])
          fi
          AC_MSG_RESULT([$OPENRASP_SSL_LIBDIR])

          AC_MSG_CHECKING([for crypto library location])
          for i in $OPENSSL_SEARCH_PATH ; do
            for j in $i/$PHP_LIBDIR $i/lib64 $i/ssl/$PHP_LIBDIR $i/openssl/$PHP_LIBDIR $i/ssl/lib64 $i/openssl/lib64 $i/lib/x86_64-linux-gnu; do
              if test -f $j/libcrypto.$SHLIB_SUFFIX_NAME;then
                OPENRASP_CRYPTO_LIBDIR=$j
                PHP_ADD_LIBRARY_WITH_PATH(crypto, $OPENRASP_CRYPTO_LIBDIR, OPENRASP_SHARED_LIBADD)
              elif test -f $j/libcrypto.a; then
                OPENRASP_CRYPTO_LIBDIR=$j
                SSL_LIBS="$OPENRASP_CRYPTO_LIBDIR/libcrypto.a"
                case $host_os in
                  darwin* )
                    OPENRASP_LIBS="-Wl,$SSL_LIBS $OPENRASP_LIBS"
                    ;;
                  * )
                    OPENRASP_LIBS="-Wl,--whole-archive -Wl,$SSL_LIBS -Wl,--no-whole-archive $OPENRASP_LIBS"
                    ;;
                esac
              fi
              if test -n "$OPENRASP_CRYPTO_LIBDIR" ; then
                break 2
              fi
            done
          done
    
          if test -z "$OPENRASP_CRYPTO_LIBDIR" ; then
            AC_MSG_ERROR([Could not find libcrypto.(a|$SHLIB_SUFFIX_NAME) in $OPENSSL_SEARCH_PATH])
          fi
          AC_MSG_RESULT([$OPENRASP_CRYPTO_LIBDIR])
        fi
        
        dnl check curl support
        if test "$ext_shared" != "no"; then
          if test "$PHP_CURL" != "no" && test -n "$PHP_CURL"; then
            CURL_SEARCH_PATH="$PHP_CURL"
          else
            CURL_SEARCH_PATH="$SEARCH_PATH"
          fi

          AC_MSG_CHECKING([for curl headers location])
          for i in $CURL_SEARCH_PATH ; do  
            for j in $i/include $i/include/x86_64-linux-gnu; do
              test -f $j/curl/easy.h && OPENRASP_CURL_INCDIR=$j
            done        
          done

          if test -z "$OPENRASP_CURL_INCDIR"; then
            AC_MSG_ERROR([Could not find easy.h in $CURL_SEARCH_PATH])
          fi
          AC_MSG_RESULT([$OPENRASP_CURL_INCDIR])
          PHP_ADD_INCLUDE($OPENRASP_CURL_INCDIR)

          CURL_CONFIG="curl-config"
          AC_MSG_CHECKING(for cURL 7.10.5 or greater)

          if ${OPENRASP_CURL_INCDIR}/bin/curl-config --libs > /dev/null 2>&1; then
            CURL_CONFIG=${OPENRASP_CURL_INCDIR}/bin/curl-config
          else
            if ${OPENRASP_CURL_INCDIR}/curl-config --libs > /dev/null 2>&1; then
              CURL_CONFIG=${OPENRASP_CURL_INCDIR}/curl-config
            fi
          fi

          curl_version_full=`$CURL_CONFIG --version`
          curl_version=`echo ${curl_version_full} | sed -e 's/libcurl //' | $AWK 'BEGIN { FS = "."; } { printf "%d", ($1 * 1000 + $2) * 1000 + $3;}'`
          if test "$curl_version" -ge 7010005; then
            AC_MSG_RESULT($curl_version_full)
          else
            AC_MSG_ERROR(cURL version 7.10.5 or later is required to compile openrasp)
          fi

          AC_MSG_CHECKING([for curl library location])
          
          for i in $CURL_SEARCH_PATH ; do
            for j in $i/$PHP_LIBDIR $i/lib64 $i/lib/x86_64-linux-gnu; do
              if test -f $j/libcurl.$SHLIB_SUFFIX_NAME;then
                OPENRASP_CURL_LIBDIR=$j
                PHP_ADD_LIBRARY_WITH_PATH(curl, $OPENRASP_CURL_LIBDIR, OPENRASP_SHARED_LIBADD)
              elif test -f $j/libcurl.a; then
                OPENRASP_CURL_LIBDIR=$j
                SSL_LIBS="$OPENRASP_CURL_LIBDIR/libcurl.a"
                case $host_os in
                  darwin* )
                    OPENRASP_LIBS="-Wl,$SSL_LIBS $OPENRASP_LIBS"
                    ;;
                  * )
                    OPENRASP_LIBS="-Wl,--whole-archive -Wl,$SSL_LIBS -Wl,--no-whole-archive $OPENRASP_LIBS"
                    ;;
                esac
              fi
              if test -n "$OPENRASP_CURL_LIBDIR" ; then
                break 2
              fi 
            done
          done

          if test -z "$OPENRASP_CURL_LIBDIR" ; then
            AC_MSG_ERROR([Could not find libcurl.(a|$SHLIB_SUFFIX_NAME) in $CURL_SEARCH_PATH])
          fi
          AC_MSG_RESULT([$OPENRASP_CURL_LIBDIR])     
        fi

        OPENRASP_REMOTE_MANAGER_SOURCE="agent/utils/os.cc \
        agent/openrasp_ctrl_block.cc \
        agent/openrasp_agent.cc \
        agent/heartbeat_agent.cc \
        agent/log_agent.cc \
        agent/openrasp_agent_manager.cc \
        agent/log_collect_item.cc \
        agent/plugin_update_pkg.cc \
        agent/backend_request.cc \
        agent/backend_response.cc"
        AC_DEFINE([HAVE_OPENRASP_REMOTE_MANAGER], [1], [Have openrasp remote manager support])
        ;;
    esac
  fi

  if test "$PHP_FSWATCH" != "no"; then
    LIBFSWATCH_SOURCE="openrasp_fswatch.cc \
      libfswatch/c++/path_utils.cpp \
      libfswatch/c++/fen_monitor.cpp \
      libfswatch/c++/fsevents_monitor.cpp \
      libfswatch/c++/monitor.cpp \
      libfswatch/c++/filter.cpp \
      libfswatch/c++/inotify_monitor.cpp \
      libfswatch/c++/windows_monitor.cpp \
      libfswatch/c++/string/string_utils.cpp \
      libfswatch/c++/event.cpp \
      libfswatch/c++/poll_monitor.cpp \
      libfswatch/c++/windows/win_handle.cpp \
      libfswatch/c++/windows/win_error_message.cpp \
      libfswatch/c++/windows/win_strings.cpp \
      libfswatch/c++/windows/win_paths.cpp \
      libfswatch/c++/windows/win_directory_change_event.cpp \
      libfswatch/c++/kqueue_monitor.cpp \
      libfswatch/c++/libfswatch_exception.cpp \
      libfswatch/c/libfswatch_log.cpp \
      libfswatch/c/libfswatch.cpp \
      libfswatch/c/cevent.cpp"
    PHP_ADD_INCLUDE("PHP_EXT_BUILDDIR([openrasp])/libfswatch")
    AC_DEFINE([HAVE_FSWATCH], [1], [Enable fswatch support])
  fi

  if test "$PHP_CLI_SUPPORT" != "no"; then
    AC_DEFINE([HAVE_CLI_SUPPORT], [1], [Enable cli support])
  fi

  YAML_CPP_SOURCE="\
    third_party/yaml-cpp/src/aliasmanager.cpp \
    third_party/yaml-cpp/src/binary.cpp \
    third_party/yaml-cpp/src/conversion.cpp \
    third_party/yaml-cpp/src/directives.cpp \
    third_party/yaml-cpp/src/emitfromevents.cpp \
    third_party/yaml-cpp/src/emitter.cpp \
    third_party/yaml-cpp/src/emitterstate.cpp \
    third_party/yaml-cpp/src/emitterutils.cpp \
    third_party/yaml-cpp/src/exp.cpp \
    third_party/yaml-cpp/src/iterator.cpp \
    third_party/yaml-cpp/src/node.cpp \
    third_party/yaml-cpp/src/nodebuilder.cpp \
    third_party/yaml-cpp/src/nodeownership.cpp \
    third_party/yaml-cpp/src/null.cpp \
    third_party/yaml-cpp/src/ostream.cpp \
    third_party/yaml-cpp/src/parser.cpp \
    third_party/yaml-cpp/src/regex.cpp \
    third_party/yaml-cpp/src/scanner.cpp \
    third_party/yaml-cpp/src/scanscalar.cpp \
    third_party/yaml-cpp/src/scantag.cpp \
    third_party/yaml-cpp/src/scantoken.cpp \
    third_party/yaml-cpp/src/simplekey.cpp \
    third_party/yaml-cpp/src/singledocparser.cpp \
    third_party/yaml-cpp/src/stream.cpp \
    third_party/yaml-cpp/src/tag.cpp \
    third_party/yaml-cpp/src/contrib/graphbuilder.cpp \
    third_party/yaml-cpp/src/contrib/graphbuilderadapter.cpp \
  "
  PHP_ADD_INCLUDE("PHP_EXT_BUILDDIR([openrasp])/third_party/yaml-cpp/include")

  case $host_os in
    darwin* )
      OPENRASP_LIBS="-framework CoreServices $OPENRASP_LIBS"
      ;;
    * )
      ;;
  esac

  if test "$PHP_LINE_COVERAGE" != "no"; then
    CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage"
    CXXFLAGS="$CXXFLAGS -fprofile-arcs -ftest-coverage"
    PHP_ADD_LIBRARY(gcov, , OPENRASP_SHARED_LIBADD)
    AC_DEFINE([HAVE_LINE_COVERAGE], [1], [Enable line coverage support])
  fi
  
  EXTRA_LIBS="$OPENRASP_LIBS $EXTRA_LIBS"
  OPENRASP_SHARED_LIBADD="$OPENRASP_LIBS $OPENRASP_SHARED_LIBADD"
  PHP_SUBST(OPENRASP_SHARED_LIBADD)

  AC_CACHE_CHECK(for C standard version, ac_cv_cstd, [
    ac_cv_cstd="c++11"
    old_CXXFLAGS=$CXXFLAGS
    AC_LANG_PUSH([C++])
    CXXFLAGS="-std="$ac_cv_cstd
    AC_TRY_RUN([int main() { return 0; }],[],[ac_cv_cstd="c++0x"],[])
    AC_LANG_POP([C++])
    CXXFLAGS=$old_CXXFLAGS
  ]);

  AC_CACHE_CHECK(how to allow c++11 narrowing, ac_cv_narrowing, [
    ac_cv_narrowing=""
    old_CXXFLAGS=$CXXFLAGS
    AC_LANG_PUSH([C++])
    CXXFLAGS="-std="$ac_cv_cstd
    AC_TRY_RUN([int main() {
        struct { unsigned int x; } foo = {-1};
        (void) foo;
        return 0;
    }], [ ac_cv_narrowing="" ], [
        CXXFLAGS="-Wno-c++11-narrowing -std="$ac_cv_cstd
        AC_TRY_RUN([int main() {
            struct { unsigned int x; } foo = {-1};
            (void) foo;
            return 0;
        }], [ ac_cv_narrowing="-Wno-c++11-narrowing" ], [
            CXXFLAGS="-Wno-narrowing -std="$ac_cv_cstd
            AC_TRY_RUN([int main() {
                struct { unsigned int x; } foo = {-1};
                (void) foo;
                return 0;
            }], [ ac_cv_narrowing="-Wno-narrowing" ], [
                AC_MSG_RESULT([cannot compile with narrowing])
            ], [])
        ], [])
    ], [])
    AC_LANG_POP([C++])
    CXXFLAGS=$old_CXXFLAGS
  ]);

  EXTRA_CXXFLAGS="$EXTRA_CXXFLAGS $ac_cv_narrowing -std=$ac_cv_cstd -Wno-deprecated-declarations -Wno-write-strings -Wno-deprecated-register -Wno-reserved-user-defined-literal"
  PHP_SUBST(EXTRA_CXXFLAGS)

  # AC_MSG_CHECKING([whether fully support regex])
  # AC_LANG_PUSH([C++])
  # old_CXXFLAGS=$CXXFLAGS
  # CXXFLAGS="-std=$ac_cv_cstd"
  # AC_TRY_LINK([#include <regex>], [std::cregex_token_iterator it;],
  # [
  #   AC_MSG_RESULT(yes)
  #   CXXFLAGS=$old_CXXFLAGS
  #   AC_LANG_POP([C++])
  # ],[
  #   AC_MSG_RESULT(no)
  #   AC_MSG_ERROR([Please install a newer c++ compiler])
  # ])

  AC_MSG_CHECKING(for mmap() using MAP_ANON shared memory support)
  AC_TRY_RUN([
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#ifndef MAP_ANON
# ifdef MAP_ANONYMOUS
#  define MAP_ANON MAP_ANONYMOUS
# endif
#endif
#ifndef MAP_FAILED
# define MAP_FAILED ((void*)-1)
#endif

int main() {
  pid_t pid;
  int status;
  char *shm;

  shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  if (shm == MAP_FAILED) {
    return 1;
  }

  strcpy(shm, "hello");

  pid = fork();
  if (pid < 0) {
    return 5;
  } else if (pid == 0) {
    strcpy(shm, "bye");
    return 6;
  }
  if (wait(&status) != pid) {
    return 7;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 6) {
    return 8;
  }
  if (strcmp(shm, "bye") != 0) {
    return 9;
  }
  return 0;
}
],dnl
    AC_DEFINE(HAVE_SHM_MMAP_ANON, 1, [Define if you have mmap(MAP_ANON) SHM support])
    msg=yes,msg=no,msg=no)
  AC_MSG_RESULT([$msg])

  AC_MSG_CHECKING(for mmap() using /dev/zero shared memory support)
  AC_TRY_RUN([
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#ifndef MAP_FAILED
# define MAP_FAILED ((void*)-1)
#endif

int main() {
  pid_t pid;
  int status;
  int fd;
  char *shm;

  fd = open("/dev/zero", O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    return 1;
  }

  shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm == MAP_FAILED) {
    return 2;
  }

  strcpy(shm, "hello");

  pid = fork();
  if (pid < 0) {
    return 5;
  } else if (pid == 0) {
    strcpy(shm, "bye");
    return 6;
  }
  if (wait(&status) != pid) {
    return 7;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 6) {
    return 8;
  }
  if (strcmp(shm, "bye") != 0) {
    return 9;
  }
  return 0;
}
],dnl
    AC_DEFINE(HAVE_SHM_MMAP_ZERO, 1, [Define if you have mmap("/dev/zero") SHM support])
    msg=yes,msg=no,msg=no)
  AC_MSG_RESULT([$msg])

  AC_MSG_CHECKING(for mmap() using regular file shared memory support)
  AC_TRY_RUN([
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef MAP_FAILED
# define MAP_FAILED ((void*)-1)
#endif

int main() {
  pid_t pid;
  int status;
  int fd;
  char *shm;
  char tmpname[4096];

  sprintf(tmpname,"test.shm.%dXXXXXX", getpid());
  if (mktemp(tmpname) == NULL) {
    return 1;
  }
  fd = open(tmpname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    return 2;
  }
  if (ftruncate(fd, 4096) < 0) {
    close(fd);
    unlink(tmpname);
    return 3;
  }

  shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm == MAP_FAILED) {
    return 4;
  }
  unlink(tmpname);
  close(fd);

  strcpy(shm, "hello");

  pid = fork();
  if (pid < 0) {
    return 5;
  } else if (pid == 0) {
    strcpy(shm, "bye");
    return 6;
  }
  if (wait(&status) != pid) {
    return 7;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 6) {
    return 8;
  }
  if (strcmp(shm, "bye") != 0) {
    return 9;
  }
  return 0;
}
],dnl
    AC_DEFINE(HAVE_SHM_MMAP_FILE, 1, [Define if you have mmap() SHM support])
    msg=yes,msg=no,msg=no)
  AC_MSG_RESULT([$msg])

  flock_type=unknown
  AC_MSG_CHECKING("whether flock struct is linux ordered")
  AC_TRY_RUN([
    #include <fcntl.h>
    struct flock lock = { 1, 2, 3, 4, 5 };
    int main() { 
      if(lock.l_type == 1 && lock.l_whence == 2 && lock.l_start == 3 && lock.l_len == 4) {
      return 0;
      }
      return 1;
    } 
  ], [
    flock_type=linux
      AC_DEFINE([HAVE_FLOCK_LINUX], [], [Struct flock is Linux-type])
      AC_MSG_RESULT("yes")
  ], AC_MSG_RESULT("no"))

  AC_MSG_CHECKING("whether flock struct is BSD ordered")
  AC_TRY_RUN([
    #include <fcntl.h>
    struct flock lock = { 1, 2, 3, 4, 5 };
    int main() { 
      if(lock.l_start == 1 && lock.l_len == 2 && lock.l_type == 4 && lock.l_whence == 5) {
      return 0;
      }
      return 1;
    } 
  ], [
    flock_type=bsd
      AC_DEFINE([HAVE_FLOCK_BSD], [], [Struct flock is BSD-type]) 
      AC_MSG_RESULT("yes")
  ], AC_MSG_RESULT("no") )

  if test "$flock_type" = "unknown"; then
    AC_MSG_ERROR([Don't know how to define struct flock on this system[,] set --enable-openrasp=no])
  fi
  
  AC_MSG_CHECKING([for git commit id])
  if command -v git >/dev/null 2>&1; then
    commit_id="$(git describe --always)"
    if test -n $commit_id; then
      AC_DEFINE_UNQUOTED([OPENRASP_COMMIT_ID], ["${commit_id}"], [Using git commit id as version])
      AC_MSG_RESULT([found])
    fi
  else
    AC_MSG_RESULT([not found])
  fi

  AC_MSG_CHECKING([for build time])
  if command -v date >/dev/null 2>&1; then
    build_time="$(date '+%Y-%m-%d %H:%M:%S')"
    if test -n "${build_time}"; then
      AC_DEFINE_UNQUOTED([OPENRASP_BUILD_TIME], ["${build_time}"], [Using current time as build time])
      AC_MSG_RESULT([found])
    fi
  else
    AC_MSG_RESULT([not found])
  fi

  PHP_NEW_EXTENSION(openrasp,
    openrasp.cc \
    openrasp_action.cc \
    openrasp_check_type.cc \
    openrasp_content_type.cc \
    openrasp_utils.cc \
    openrasp_hook.cc \
    hook/openrasp_directory.cc \
    hook/openrasp_fileupload.cc \
    hook/openrasp_include.cc \ 
    hook/openrasp_command.cc \
    hook/openrasp_array.cc \
    hook/openrasp_sql.cc \
    hook/openrasp_mysql.cc \
    hook/openrasp_mysqli.cc \
    hook/openrasp_pgsql.cc \
    hook/openrasp_pgsql_utils.cc \
    hook/openrasp_sqlite3.cc \
    hook/openrasp_pdo.cc \
    hook/openrasp_file.cc \
    hook/openrasp_ssrf.cc \
    openrasp_output_detect.cc \
    hook/sql_connection_enrty.cc \
    hook/openrasp_echo.cc \
    hook/openrasp_putenv.cc \
    openrasp_conf_holder.cc \
    openrasp_config_block.cc \
    openrasp_inject.cc \
    openrasp_log.cc \
    openrasp_error.cc \
    openrasp_v8.cc \
    openrasp_v8_request_context.cc \
    openrasp_v8_utils.cc \
    openrasp_security_policy.cc \
    openrasp_ini.cc \
    utils/ReadWriteLock.cc \
    utils/DoubleArrayTrie.cc \
    utils/string.cc \
    utils/digest.cc \
    utils/regex.cc \
    utils/debug_trace.cc \
    utils/file.cc \  
    utils/time.cc \
    utils/net.cc \
    utils/JsonReader.cc \
    utils/YamlReader.cc \
    utils/utf.cc \
    agent/base_manager.cc \
    agent/shared_log_manager.cc \
    agent/shared_config_manager.cc \
    agent/mm/shm_manager.cc \
    $LIBFSWATCH_SOURCE \
    $YAML_CPP_SOURCE \
    $OPENRASP_REMOTE_MANAGER_SOURCE \
    , $ext_shared)
  ifdef([PHP_ADD_EXTENSION_DEP],
  [
    PHP_ADD_EXTENSION_DEP(openrasp, standard)
    PHP_ADD_EXTENSION_DEP(openrasp, json)
  ])
fi
