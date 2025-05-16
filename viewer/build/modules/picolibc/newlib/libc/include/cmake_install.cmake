# Install script for directory: /home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/james/toolchains/xtensa-esp32-elf/bin/xtensa-esp32-elf-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/james/csse4011/repo/mycode/apps/prac3/viewer_node/build/modules/picolibc/newlib/libc/include/sys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/james/csse4011/repo/mycode/apps/prac3/viewer_node/build/modules/picolibc/newlib/libc/include/machine/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/james/csse4011/repo/mycode/apps/prac3/viewer_node/build/modules/picolibc/newlib/libc/include/ssp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/james/csse4011/repo/mycode/apps/prac3/viewer_node/build/modules/picolibc/newlib/libc/include/rpc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/james/csse4011/repo/mycode/apps/prac3/viewer_node/build/modules/picolibc/newlib/libc/include/arpa/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/alloca.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/argz.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/ar.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/assert.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/byteswap.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/cpio.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/ctype.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/devctl.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/dirent.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/elf.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/endian.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/envlock.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/envz.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/errno.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/fastmath.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/fcntl.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/fenv.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/fnmatch.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/getopt.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/glob.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/grp.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/iconv.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/ieeefp.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/inttypes.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/langinfo.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/libgen.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/limits.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/locale.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/malloc.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/math.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/memory.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/newlib.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/paths.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/picotls.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/pwd.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/regdef.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/regex.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/sched.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/search.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/setjmp.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/signal.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/spawn.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/stdint.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/stdnoreturn.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/stdlib.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/string.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/strings.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/_syslist.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/tar.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/termios.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/threads.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/time.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/unctrl.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/unistd.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/utime.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/utmp.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/wchar.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/wctype.h"
    "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/wordexp.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/james/csse4011/repo/modules/lib/picolibc/newlib/libc/include/complex.h")
endif()

