# Install script for directory: /mnt/dwork/leanring/muduo-1.0.3/examples

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

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/ace/ttcp/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/asio/chat/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/asio/tutorial/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/fastcgi/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/filetransfer/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/hub/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/idleconnection/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/maxconnection/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/memcached/client/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/memcached/server/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/multiplexer/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/netty/discard/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/netty/echo/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/netty/uptime/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/pingpong/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/roundtrip/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/shorturl/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/muduocam/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/chatserver/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/chatclient/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/simple/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/socks4a/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/sudoku/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/twisted/finger/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/wordcount/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/zeromq/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/cdns/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/cdns2/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/curl/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/ace/logging/cmake_install.cmake")
  INCLUDE("/mnt/dwork/leanring/muduo-1.0.3/build/release/examples/protobuf/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

