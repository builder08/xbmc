set(ARCH_DEFINES -D_LINUX -DTARGET_POSIX -DTARGET_LINUX)
set(SYSTEM_DEFINES -D__STDC_CONSTANT_MACROS -D_FILE_DEFINED
                   -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
set(PLATFORM_DIR linux)
set(CMAKE_SYSTEM_NAME Linux)

if(ARCH STREQUAL x86_64)
  set(ARCH x86_64-linux)
  set(NEON False)
elseif(ARCH STREQUAL "x86" OR ARCH MATCHES "i.86")
  set(ARCH x86-linux)
  set(NEON False)
  add_options(CXX ALL_BUILDS "-msse")
elseif(ARCH STREQUAL arm)
  set(NEON True)
elseif(ARCH STREQUAL aarch64)
  set(NEON False)
else()
  message(SEND_ERROR "Unknown ARCH: ${ARCH}")
endif()

# Make sure we strip binaries in Release build
if(CMAKE_BUILD_TYPE STREQUAL Release AND CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

find_package(CXX11 REQUIRED)
include(LDGOLD)

# Code Coverage
if(CMAKE_BUILD_TYPE STREQUAL Coverage)
  set(COVERAGE_TEST_BINARY ${APP_NAME_LC}-test)
  set(COVERAGE_SOURCE_DIR ${CORE_SOURCE_DIR})
  set(COVERAGE_DEPENDS "\${APP_NAME_LC}" "\${APP_NAME_LC}-test")
  set(COVERAGE_EXCLUDES */test/* lib/* */lib/*)
endif()

if(ENABLE_MIR)
  set(ENABLE_VDPAU OFF CACHE BOOL "Disabling VDPAU since no Mir support" FORCE)
endif()
