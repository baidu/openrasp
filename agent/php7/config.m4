PHP_ARG_ENABLE(openrasp, whether to enable openrasp support,
[  --enable-openrasp       Enable openrasp support])

PHP_ARG_WITH(openrasp-v8, for openrasp-v8 support,
[  --with-openrasp-v8=DIR  Set the path to openrasp-v8], no, no)

PHP_ARG_WITH(gettext, for gettext support,
[  --with-gettext=DIR   Set the path to gettext], no, no)

PHP_ARG_ENABLE(openrasp-remote-manager, whether to enable openrasp remote manager support,
[  --enable-openrasp-remote-manager       Enable openrasp remote manager support (Linux Only)], no, no)

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

  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  CXXFLAGS="$CXXFLAGS $($PKG_CONFIG $PHP_OPENRASP_V8/php/openrasp-v8.pc --cflags)"
  OPENRASP_LIBS="$OPENRASP_LIBS $($PKG_CONFIG $PHP_OPENRASP_V8/php/openrasp-v8.pc --libs)"

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
        OPENRASP_REMOTE_MANAGER_SOURCE="agent/utils/os.cc \
        agent/openrasp_ctrl_block.cc \
        agent/plugin_info_block.cc \
        agent/openrasp_agent.cc \
        agent/heartbeat_agent.cc \
        agent/webdir/webdir.cc \
        agent/webdir/webdir_ctrl_block.cc \
        agent/webdir/webdir_utils.cc \
        agent/webdir/webdir_agent.cc \
        agent/webdir/dependency_item.cc \
        agent/webdir/webdir_detector.cc \
        agent/webdir/dependency_writer.cc \
        agent/log_agent.cc \
        agent/openrasp_agent_manager.cc \
        agent/log_collect_item.cc \
        agent/plugin_update_pkg.cc \
        agent/backend_request.cc \
        agent/backend_response.cc \
        agent/crash_reporter.cc"
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

  EXTRA_CXXFLAGS="$EXTRA_CXXFLAGS $ac_cv_narrowing -std=$ac_cv_cstd -Wno-deprecated-declarations -Wno-write-strings -Wno-deprecated-register"
  PHP_SUBST(EXTRA_CXXFLAGS)

  AC_MSG_CHECKING(for check isfinite declared)
  AC_LANG_PUSH([C++])
  AC_TRY_RUN([
#include <math.h>
#include <cmath>
#include <math.h>

int main() {
  double value = 12.3456;
  if (isfinite(value)) {}
  return 0;
}
],dnl
  AC_DEFINE(HAVE_ISFINITE, 1, [Define if you have isfinite declared])
  msg=yes, msg=no, msg=no)
    AC_LANG_POP([C++])
  AC_MSG_RESULT([$msg])

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
    hook/data/sql_object.cc \
    hook/data/mongo_object.cc \
    hook/data/copy_object.cc \
    hook/data/rename_object.cc \
    hook/data/file_op_object.cc \
    hook/data/fileupload_object.cc \
    hook/data/file_put_webshell_object.cc \
    hook/data/ssrf_object.cc \
    hook/data/ssrf_redirect_object.cc \
    hook/data/echo_object.cc \
    hook/data/sql_error_object.cc \
    hook/data/sql_connection_object.cc \
    hook/data/mongo_connection_object.cc \
    hook/data/sql_username_object.cc \
    hook/data/sql_password_object.cc \
    hook/data/callable_object.cc \
    hook/data/command_object.cc \
    hook/data/include_object.cc \
    hook/data/eval_object.cc \
    hook/data/putenv_object.cc \
    hook/data/no_params_object.cc \
    hook/data/xss_userinput_object.cc \
    hook/checker/policy_detector.cc \
    hook/checker/builtin_detector.cc \
    hook/checker/v8_detector.cc \
    hook/checker/check_result.cc \
    hook/checker/check_utils.cc \
    hook/openrasp_directory.cc \
    hook/openrasp_fileupload.cc \
    hook/openrasp_include.cc \ 
    hook/openrasp_command.cc \
    hook/openrasp_array.cc \
    hook/openrasp_sql.cc \
    hook/openrasp_mysqli.cc \
    hook/openrasp_pgsql.cc \
    hook/openrasp_pgsql_utils.cc \
    hook/openrasp_sqlite3.cc \
    hook/openrasp_pdo.cc \
    hook/openrasp_file.cc \
    hook/openrasp_ssrf.cc \
    hook/openrasp_putenv.cc \
    hook/openrasp_mongo.cc \
    openrasp_output_detect.cc \
    hook/openrasp_echo.cc \
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
    utils/validator.cc \
    utils/signal_interceptor.cc \
    utils/read_write_lock.cc \
    utils/string.cc \
    utils/digest.cc \
    utils/regex.cc \
    utils/debug_trace.cc \
    utils/file.cc \    
    utils/time.cc \
    utils/net.cc \
    utils/url.cc \
    utils/json_reader.cc \
    utils/yaml_reader.cc \
    utils/utf.cc \
    utils/hostname.cc \
    model/url.cc \
    model/request.cc \
    model/parameter.cc \
    model/zend_ref_item.cc \
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

