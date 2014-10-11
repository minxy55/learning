# Install script for directory: /mnt/dwork/leanring/muduo-1.0.3/muduo/base

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/mnt/dwork/leanring/muduo-1.0.3/build/release-install")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/mnt/dwork/leanring/muduo-1.0.3/build/release/lib/libmuduo_base.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/mnt/dwork/leanring/muduo-1.0.3/build/release/lib/libmuduo_base_cpp11.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/muduo/base" TYPE FILE FILES
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Types.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/CurrentThread.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/BlockingQueue.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/ThreadPool.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Date.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Singleton.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/WeakCallback.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Atomic.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/AsyncLogging.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/LogFile.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Mutex.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/LogStream.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/GzipFile.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Exception.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/copyable.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/ThreadLocalSingleton.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Thread.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/StringPiece.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/ThreadLocal.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/ProcessInfo.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Timestamp.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/BoundedBlockingQueue.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/TimeZone.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Condition.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/CountDownLatch.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/Logging.h"
    "/mnt/dwork/leanring/muduo-1.0.3/muduo/base/FileUtil.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/muduo/base/tests/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

