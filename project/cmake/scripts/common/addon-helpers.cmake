# Workaround for the fact that cpack's filenames are not customizable.
# Each add-on is added as a separate component to facilitate zip/tgz packaging.
# The filenames are always of the form basename-component, which is 
# incompatible with the addonid-version scheme we want. This hack renames
# the files from the file names generated by the 'package' target.
# Sadly we cannot extend the 'package' target, as it is a builtin target, see 
# http://public.kitware.com/Bug/view.php?id=8438
# Thus, we have to add an 'addon-package' target.
add_custom_target(addon-package
                  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package)

macro(add_cpack_workaround target version ext)
  if(NOT PACKAGE_DIR)
    set(PACKAGE_DIR "${CMAKE_INSTALL_PREFIX}/zips")
  endif()

  add_custom_command(TARGET addon-package PRE_BUILD
                     COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}
                     COMMAND ${CMAKE_COMMAND} -E copy ${CPACK_PACKAGE_DIRECTORY}/addon-${target}-${version}.${ext} ${PACKAGE_DIR}/${target}-${version}.${ext})
endmacro()

# Grab the version from a given add-on's addon.xml
macro (addon_version dir prefix)
  IF(EXISTS ${PROJECT_SOURCE_DIR}/${dir}/addon.xml.in)
    FILE(READ ${PROJECT_SOURCE_DIR}/${dir}/addon.xml.in ADDONXML)
  ELSE()
    FILE(READ ${dir}/addon.xml ADDONXML)
  ENDIF()

  STRING(REGEX MATCH "<addon[^>]*version.?=.?.[0-9\\.]+" VERSION_STRING ${ADDONXML}) 
  STRING(REGEX REPLACE ".*version=.([0-9\\.]+).*" "\\1" ${prefix}_VERSION ${VERSION_STRING})
  message(STATUS ${prefix}_VERSION=${${prefix}_VERSION})
endmacro()

# Build, link and optionally package an add-on
macro (build_addon target prefix libs)
  ADD_LIBRARY(${target} ${${prefix}_SOURCES})
  TARGET_LINK_LIBRARIES(${target} ${${libs}})
  addon_version(${target} ${prefix})
  SET_TARGET_PROPERTIES(${target} PROPERTIES VERSION ${${prefix}_VERSION}
                                             SOVERSION ${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}
                                             PREFIX "")
  IF(OS STREQUAL "android")
    SET_TARGET_PROPERTIES(${target} PROPERTIES PREFIX "lib")
  ENDIF(OS STREQUAL "android")

  # get the library's location
  SET(LIBRARY_LOCATION $<TARGET_FILE:${target}>)
  # get the library's filename
  if("${CORE_SYSTEM_NAME}" STREQUAL "android")
    # for android we need the filename without any version numbers
    set(LIBRARY_FILENAME $<TARGET_LINKER_FILE_NAME:${target}>)
  else()
    SET(LIBRARY_FILENAME $<TARGET_FILE_NAME:${target}>)
  endif()

  # if there's an addon.xml.in we need to generate the addon.xml
  IF(EXISTS ${PROJECT_SOURCE_DIR}/${target}/addon.xml.in)
    SET(PLATFORM ${CORE_SYSTEM_NAME})

    FILE(READ ${PROJECT_SOURCE_DIR}/${target}/addon.xml.in addon_file)
    STRING(CONFIGURE "${addon_file}" addon_file_conf @ONLY)
    FILE(GENERATE OUTPUT ${PROJECT_SOURCE_DIR}/${target}/addon.xml CONTENT "${addon_file_conf}")
  ENDIF()

  # set zip as default if addon-package is called without PACKAGE_XXX
  SET(CPACK_GENERATOR "ZIP")
  SET(ext "zip")
  IF(PACKAGE_ZIP OR PACKAGE_TGZ)
    IF(PACKAGE_TGZ)
      SET(CPACK_GENERATOR "TGZ")
      SET(ext "tar.gz")
    ENDIF(PACKAGE_TGZ)
    SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_PACKAGE_FILE_NAME addon)
    IF(CMAKE_BUILD_TYPE STREQUAL "Release")
      SET(CPACK_STRIP_FILES TRUE)
    ENDIF(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_IGNORE_GROUPS 1)
    list(APPEND CPACK_COMPONENTS_ALL ${target}-${${prefix}_VERSION})
    # Pack files together to create an archive
    INSTALL(DIRECTORY ${target} DESTINATION ./ COMPONENT ${target}-${${prefix}_VERSION} PATTERN "addon.xml.in" EXCLUDE)
    IF(WIN32)
      if(NOT CPACK_PACKAGE_DIRECTORY)
        # determine the temporary path
        file(TO_CMAKE_PATH "$ENV{TEMP}" WIN32_TEMP_PATH)
        string(LENGTH "${WIN32_TEMP_PATH}" WIN32_TEMP_PATH_LENGTH)
        string(LENGTH "${PROJECT_BINARY_DIR}" PROJECT_BINARY_DIR_LENGTH)

        # check if the temporary path is shorter than the default packaging directory path
        if(WIN32_TEMP_PATH_LENGTH GREATER 0 AND WIN32_TEMP_PATH_LENGTH LESS PROJECT_BINARY_DIR_LENGTH)
          # set the directory used by CPack for packaging to the temp directory
          set(CPACK_PACKAGE_DIRECTORY ${WIN32_TEMP_PATH})
        endif()
      endif()

      # in case of a VC++ project the installation location contains a $(Configuration) VS variable
      # we replace it with ${CMAKE_BUILD_TYPE} (which doesn't cover the case when the build configuration
      # is changed within Visual Studio)
      string(REPLACE "$(Configuration)" "${CMAKE_BUILD_TYPE}" LIBRARY_LOCATION "${LIBRARY_LOCATION}")

      # install the generated DLL file
      INSTALL(PROGRAMS ${LIBRARY_LOCATION} DESTINATION ${target}
              COMPONENT ${target}-${${prefix}_VERSION})

      IF(CMAKE_BUILD_TYPE MATCHES Debug)
        # for debug builds also install the PDB file
        get_filename_component(LIBRARY_DIR ${LIBRARY_LOCATION} DIRECTORY)
        INSTALL(FILES ${LIBRARY_DIR}/${target}.pdb DESTINATION ${target}
                COMPONENT ${target}-${${prefix}_VERSION})
      ENDIF()
    ELSE(WIN32)
      if(NOT CPACK_PACKAGE_DIRECTORY)
        set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR})
      endif()
      INSTALL(TARGETS ${target} DESTINATION ${target}
              COMPONENT ${target}-${${prefix}_VERSION})
    ENDIF(WIN32)
    add_cpack_workaround(${target} ${${prefix}_VERSION} ${ext})
  ELSE(PACKAGE_ZIP OR PACKAGE_TGZ)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR NOT CMAKE_INSTALL_PREFIX)
        message(STATUS "setting install paths to match ${APP_NAME}: CMAKE_INSTALL_PREFIX: ${${APP_NAME_UC}_PREFIX}")
        set(CMAKE_INSTALL_PREFIX "${${APP_NAME_UC}_PREFIX}" CACHE PATH "${APP_NAME} install prefix" FORCE)
        set(CMAKE_INSTALL_LIBDIR "${${APP_NAME_UC}_LIB_DIR}" CACHE PATH "${APP_NAME} install libdir" FORCE)
      elseif(NOT CMAKE_INSTALL_PREFIX STREQUAL "${${APP_NAME_UC}_PREFIX}" AND NOT OVERRIDE_PATHS)
        message(FATAL_ERROR "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} differs from ${APP_NAME} prefix ${${APP_NAME_UC}_PREFIX}. Please pass -DOVERRIDE_PATHS=1 to skip this check")
      else()
        if(NOT CMAKE_INSTALL_LIBDIR)
          set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib/${APP_NAME_LC}")
        endif()
      endif()
    else()
      set(CMAKE_INSTALL_LIBDIR "lib/${APP_NAME_LC}")
    endif()
    INSTALL(TARGETS ${target} DESTINATION ${CMAKE_INSTALL_LIBDIR}/addons/${target})
    INSTALL(DIRECTORY ${target} DESTINATION share/${APP_NAME_LC}/addons PATTERN "addon.xml.in" EXCLUDE)
  ENDIF(PACKAGE_ZIP OR PACKAGE_TGZ)
endmacro()

# finds a path to a given file (recursive)
function (kodi_find_path var_name filename search_path strip_file)
  file(GLOB_RECURSE PATH_TO_FILE ${search_path} ${filename})
  if(strip_file)
    string(REPLACE ${filename} "" PATH_TO_FILE ${PATH_TO_FILE})
  endif(strip_file)
  set (${var_name} ${PATH_TO_FILE} PARENT_SCOPE)
endfunction()

# Cmake build options
include(addoptions)
include(TestCXXAcceptsFlag)
OPTION(PACKAGE_ZIP "Package Zip file?" OFF)
OPTION(PACKAGE_TGZ "Package TGZ file?" OFF)
OPTION(BUILD_SHARED_LIBS "Build shared libs?" ON)

# LTO support?
CHECK_CXX_ACCEPTS_FLAG("-flto" HAVE_LTO)
IF(HAVE_LTO)
  OPTION(USE_LTO "use link time optimization" OFF)
  IF(USE_LTO)
    add_options(ALL_LANGUAGES ALL_BUILDS "-flto")
  ENDIF(USE_LTO)
ENDIF(HAVE_LTO) 

# set this to try linking dependencies as static as possible
IF(ADDONS_PREFER_STATIC_LIBS)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(ADDONS_PREFER_STATIC_LIBS)

