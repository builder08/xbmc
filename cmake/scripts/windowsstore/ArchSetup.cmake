# -------- Architecture settings ---------

check_symbol_exists(_X86_ "Windows.h" _X86_)
check_symbol_exists(_AMD64_ "Windows.h" _AMD64_)
check_symbol_exists(_ARM_ "Windows.h" _ARM_)

if(_X86_)
   set(ARCH win32)
   set(SDK_TARGET_ARCH x86)
elseif(_AMD64_)
   set(ARCH x64)
   set(SDK_TARGET_ARCH x64)
elseif(_ARM_)
   set(ARCH arm)
   set(SDK_TARGET_ARCH arm)
else()
   message(FATAL_ERROR "Unsupported architecture")
endif()
 
unset(_X86_)
unset(_AMD64_)
unset(_ARM_)

# -------- Paths (mainly for find_package) ---------

set(PLATFORM_DIR platform/win32)
set(CORE_MAIN_SOURCE ${CMAKE_SOURCE_DIR}/xbmc/platform/win10/main.cpp)

# Precompiled headers fail with per target output directory. (needs CMake 3.1)
set(PRECOMPILEDHEADER_DIR ${PROJECT_BINARY_DIR}/${CORE_BUILD_CONFIG}/objs)

set(CMAKE_SYSTEM_NAME WindowsStore)
set(CORE_SYSTEM_NAME "windowsstore")
set(PACKAGE_GUID "281d668b-5739-4abd-b3c2-ed1cda572ed2")
set(APP_MANIFEST_NAME package.appxmanifest)
set(DEPS_FOLDER_RELATIVE project/BuildDependencies)

set(DEPENDENCIES_DIR ${CMAKE_SOURCE_DIR}/${DEPS_FOLDER_RELATIVE}/win10-${ARCH})
set(MINGW_LIBS_DIR ${CMAKE_SOURCE_DIR}/${DEPS_FOLDER_RELATIVE}/mingwlibs/win10-${ARCH})

# mingw libs
list(APPEND CMAKE_SYSTEM_PREFIX_PATH ${MINGW_LIBS_DIR})
list(APPEND CMAKE_SYSTEM_LIBRARY_PATH ${MINGW_LIBS_DIR}/bin)
# dependencies 
list(APPEND CMAKE_SYSTEM_PREFIX_PATH ${DEPENDENCIES_DIR})
# for python
set(PYTHON_INCLUDE_DIR ${DEPENDENCIES_DIR}/include/python)


# -------- Compiler options ---------

add_options(CXX ALL_BUILDS "/wd\"4996\"")
add_options(CXX ALL_BUILDS "/wd\"4146\"")
add_options(CXX ALL_BUILDS "/wd\"4251\"")
add_options(CXX ALL_BUILDS "/wd\"4668\"")
set(ARCH_DEFINES -D_WINDOWS -DTARGET_WINDOWS -DTARGET_WINDOWS_STORE -DXBMC_EXPORT -DMS_UWP)
if(NOT SDK_TARGET_ARCH STREQUAL arm)
  list(APPEND ARCH_DEFINES -D__SSE__ -D__SSE2__)
endif()
set(SYSTEM_DEFINES -DNOMINMAX -DHAS_DX -D__STDC_CONSTANT_MACROS
                   -DFMT_HEADER_ONLY -DTAGLIB_STATIC -DNPT_CONFIG_ENABLE_LOGGING
                   -DPLT_HTTP_DEFAULT_USER_AGENT="UPnP/1.0 DLNADOC/1.50 Kodi"
                   -DPLT_HTTP_DEFAULT_SERVER="UPnP/1.0 DLNADOC/1.50 Kodi"
                   -DUNICODE -D_UNICODE
                   $<$<CONFIG:Debug>:-DD3D_DEBUG_INFO>)

# The /MP option enables /FS by default.
set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS} /ZW /EHsc /await")
# Google Test needs to use shared version of runtime libraries
set(gtest_force_shared_crt ON CACHE STRING "" FORCE)


# -------- Linker options ---------

# For #pragma comment(lib X)
# TODO: It would certainly be better to handle these libraries via CMake modules.
link_directories(${MINGW_LIBS_DIR}/lib
                 ${DEPENDENCIES_DIR}/lib)

list(APPEND DEPLIBS d3d11.lib WS2_32.lib dxguid.lib dloadhelper.lib)
if(ARCH STREQUAL win32 OR ARCH STREQUAL x64)
  list(APPEND DEPLIBS DInput8.lib DSound.lib winmm.lib Mpr.lib Iphlpapi.lib PowrProf.lib setupapi.lib dwmapi.lib)
endif()
# NODEFAULTLIB option

set(_nodefaultlibs_RELEASE libcmt)
set(_nodefaultlibs_DEBUG libcmt msvcrt)
foreach(_lib ${_nodefaultlibs_RELEASE})
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:\"${_lib}\"")
endforeach()
foreach(_lib ${_nodefaultlibs_DEBUG})
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:\"${_lib}\"")
endforeach()

# Make the Release version create a PDB
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
# Minimize the size or the resulting DLLs
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF")
# remove warning
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4264")


# -------- Visual Studio options ---------

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Generate a batch file that opens Visual Studio with the necessary env variables set.
  file(WRITE ${CMAKE_BINARY_DIR}/kodi-sln.bat
             "@echo off\n"
             "set KODI_HOME=%~dp0\n"
             "set PATH=%~dp0\\system\n"
             "start %~dp0\\${PROJECT_NAME}.sln")
endif()

# -------- Build options ---------

set(ENABLE_OPTICAL OFF CACHE BOOL "" FORCE)
