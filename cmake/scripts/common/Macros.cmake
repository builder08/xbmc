# This script holds the main functions used to construct the build system

# Include system specific macros but only if this file is included from
# kodi main project. It's not needed for kodi-addons project
# If CORE_SOURCE_DIR is set, it was called from kodi-addons project
# TODO: drop check if we ever integrate kodi-addons into kodi project
if(NOT CORE_SOURCE_DIR)
  include(${CMAKE_SOURCE_DIR}/cmake/scripts/${CORE_SYSTEM_NAME}/Macros.cmake)
endif()

# IDEs: Group source files in target in folders (file system hierarchy)
# Source: http://blog.audio-tk.com/2015/09/01/sorting-source-files-and-projects-in-folders-with-cmake-and-visual-studioxcode/
# Arguments:
#   target The target that shall be grouped by folders.
# Optional Arguments:
#   RELATIVE allows to specify a different reference folder.
function(source_group_by_folder target)
  if(NOT TARGET ${target})
    message(FATAL_ERROR "There is no target named '${target}'")
  endif()

  set(SOURCE_GROUP_DELIMITER "/")

  cmake_parse_arguments(arg "" "RELATIVE" "" ${ARGN})
  if(arg_RELATIVE)
    set(relative_dir ${arg_RELATIVE})
  else()
    set(relative_dir ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  get_property(files TARGET ${target} PROPERTY SOURCES)
  if(files)
    list(SORT files)

    if(CMAKE_GENERATOR STREQUAL Xcode)
      set_target_properties(${target} PROPERTIES SOURCES "${files}")
    endif()
  endif()
  foreach(file ${files})
    if(NOT IS_ABSOLUTE ${file})
      set(file ${CMAKE_CURRENT_SOURCE_DIR}/${file})
    endif()
    file(RELATIVE_PATH relative_file ${relative_dir} ${file})
    get_filename_component(dir "${relative_file}" DIRECTORY)
    if(NOT dir STREQUAL "${last_dir}")
      if(files)
        source_group("${last_dir}" FILES ${files})
      endif()
      set(files "")
    endif()
    set(files ${files} ${file})
    set(last_dir "${dir}")
  endforeach(file)
  if(files)
    source_group("${last_dir}" FILES ${files})
  endif()
endfunction()

# Add sources to main application
# Arguments:
#   name name of the library to add
# Implicit arguments:
#   ENABLE_STATIC_LIBS Build static libraries per directory
#   SOURCES the sources of the library
#   HEADERS the headers of the library (only for IDE support)
#   OTHERS  other library related files (only for IDE support)
# On return:
#   Library will be built, optionally added to ${core_DEPENDS}
#   Sets CORE_LIBRARY for calls for setting target specific options
function(core_add_library name)
  if(ENABLE_STATIC_LIBS)
    add_library(${name} STATIC ${SOURCES} ${HEADERS} ${OTHERS})
    set_target_properties(${name} PROPERTIES PREFIX "")
    set(core_DEPENDS ${name} ${core_DEPENDS} CACHE STRING "" FORCE)
    add_dependencies(${name} libcpluff ffmpeg dvdnav crossguid)
    set(CORE_LIBRARY ${name} PARENT_SCOPE)

    # Add precompiled headers to Kodi main libraries
    if(CORE_SYSTEM_NAME STREQUAL windows)
      add_precompiled_header(${name} pch.h ${CMAKE_SOURCE_DIR}/xbmc/platform/win32/pch.cpp PCH_TARGET kodi)
      set_language_cxx(${name})
      target_link_libraries(${name} PUBLIC effects11)
    endif()
  else()
    foreach(src IN LISTS SOURCES HEADERS OTHERS)
      get_filename_component(src_path "${src}" ABSOLUTE)
      list(APPEND FILES ${src_path})
    endforeach()
    target_sources(lib${APP_NAME_LC} PRIVATE ${FILES})
    set(CORE_LIBRARY lib${APP_NAME_LC} PARENT_SCOPE)
  endif()
endfunction()

# Add a test library, and add sources to list for gtest integration macros
function(core_add_test_library name)
  if(ENABLE_STATIC_LIBS)
    add_library(${name} STATIC ${SOURCES} ${SUPPORTED_SOURCES} ${HEADERS} ${OTHERS})
    set_target_properties(${name} PROPERTIES PREFIX ""
                                             EXCLUDE_FROM_ALL 1
                                             FOLDER "Build Utilities/tests")
    add_dependencies(${name} libcpluff ffmpeg dvdnav crossguid)
    set(test_archives ${test_archives} ${name} CACHE STRING "" FORCE)
  endif()
  foreach(src IN LISTS SOURCES SUPPORTED_SOURCES HEADERS OTHERS)
    get_filename_component(src_path "${src}" ABSOLUTE)
    set(test_sources "${src_path}" ${test_sources} CACHE STRING "" FORCE)
  endforeach()
endfunction()

# Add an addon callback library
# Arguments:
#   name name of the library to add
# Implicit arguments:
#   SOURCES the sources of the library
#   HEADERS the headers of the library (only for IDE support)
#   OTHERS  other library related files (only for IDE support)
# On return:
#   Library target is defined and added to LIBRARY_FILES
function(core_add_addon_library name)
  get_filename_component(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  list(APPEND SOURCES lib${name}.cpp)
  core_add_shared_library(${name} OUTPUT_DIRECTORY addons/${DIRECTORY})
  set_target_properties(${name} PROPERTIES FOLDER addons)
  target_include_directories(${name} PRIVATE
                             ${CMAKE_CURRENT_SOURCE_DIR}
                             ${CMAKE_SOURCE_DIR}/xbmc/addons/kodi-addon-dev-kit/include/kodi
                             ${CMAKE_SOURCE_DIR}/xbmc)
endfunction()

# Add an dl-loaded shared library
# Arguments:
#   name name of the library to add
# Optional arguments:
#   WRAPPED wrap this library on POSIX platforms to add VFS support for
#           libraries that would otherwise not support it.
#   OUTPUT_DIRECTORY where to create the library in the build dir
#           (default: system)
# Implicit arguments:
#   SOURCES the sources of the library
#   HEADERS the headers of the library (only for IDE support)
#   OTHERS  other library related files (only for IDE support)
# On return:
#   Library target is defined and added to LIBRARY_FILES
function(core_add_shared_library name)
  cmake_parse_arguments(arg "WRAPPED" "OUTPUT_DIRECTORY" "" ${ARGN})
  if(arg_OUTPUT_DIRECTORY)
    set(OUTPUT_DIRECTORY ${arg_OUTPUT_DIRECTORY})
  else()
    if(NOT CORE_SYSTEM_NAME STREQUAL windows)
      set(OUTPUT_DIRECTORY system)
    endif()
  endif()
  if(CORE_SYSTEM_NAME STREQUAL windows)
    set(OUTPUT_NAME lib${name})
  else()
    set(OUTPUT_NAME lib${name}-${ARCH})
  endif()

  if(NOT arg_WRAPPED OR CORE_SYSTEM_NAME STREQUAL windows)
    add_library(${name} SHARED ${SOURCES} ${HEADERS} ${OTHERS})
    set_target_properties(${name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_DIRECTORY}
                                             RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_DIRECTORY}
                                             OUTPUT_NAME ${OUTPUT_NAME} PREFIX "")
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
      set_target_properties(${name} PROPERTIES  LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUT_DIRECTORY}
                                                RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUT_DIRECTORY})
    endforeach()

    set(LIBRARY_FILES ${LIBRARY_FILES} ${CMAKE_BINARY_DIR}/${OUTPUT_DIRECTORY}/${OUTPUT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE STRING "" FORCE)
    add_dependencies(${APP_NAME_LC}-libraries ${name})
  else()
    add_library(${name} STATIC ${SOURCES} ${HEADERS} ${OTHERS})
    set_target_properties(${name} PROPERTIES POSITION_INDEPENDENT_CODE 1)
    core_link_library(${name} ${OUTPUT_DIRECTORY}/lib${name})
  endif()
endfunction()

# Sets the compile language for all C source files in a target to CXX.
# Needs to be called from the CMakeLists.txt that defines the target.
# Arguments:
#   target   target
function(set_language_cxx target)
  get_property(sources TARGET ${target} PROPERTY SOURCES)
  foreach(file IN LISTS sources)
    if(file MATCHES "\.c$")
      set_source_files_properties(${file} PROPERTIES LANGUAGE CXX)
    endif()
  endforeach()
endfunction()

# Add a data file to installation list with a mirror in build tree
# Mirroring files in the buildtree allows to execute the app from there.
# Arguments:
#   file        full path to file to mirror
# Optional Arguments:
#   NO_INSTALL: exclude file from installation target (only mirror)
#   DIRECTORY:  directory where the file should be mirrored to
#               (default: preserve tree structure relative to CMAKE_SOURCE_DIR)
#   KEEP_DIR_STRUCTURE: preserve tree structure even when DIRECTORY is set
# On return:
#   Files is mirrored to the build tree and added to ${install_data}
#   (if NO_INSTALL is not given).
function(copy_file_to_buildtree file)
  cmake_parse_arguments(arg "NO_INSTALL" "DIRECTORY;KEEP_DIR_STRUCTURE" "" ${ARGN})
  if(arg_DIRECTORY)
    set(outdir ${arg_DIRECTORY})
    if(arg_KEEP_DIR_STRUCTURE)
      get_filename_component(srcdir ${arg_KEEP_DIR_STRUCTURE} DIRECTORY)
      string(REPLACE "${CMAKE_SOURCE_DIR}/${srcdir}/" "" outfile ${file})
      if(NOT IS_DIRECTORY ${file})
        set(outdir ${outdir}/${outfile})
      endif()
    else()
      get_filename_component(outfile ${file} NAME)
      set(outfile ${outdir}/${outfile})
    endif()
  else()
    string(REPLACE "${CMAKE_SOURCE_DIR}/" "" outfile ${file})
    get_filename_component(outdir ${outfile} DIRECTORY)
  endif()

  if(NOT TARGET export-files)
    file(REMOVE ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ExportFiles.cmake)
    add_custom_target(export-files ALL COMMENT "Copying files into build tree"
                      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ExportFiles.cmake)
    set_target_properties(export-files PROPERTIES FOLDER "Build Utilities")
    file(APPEND ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ExportFiles.cmake "# Export files to build tree\n")
  endif()

  # Exclude autotools build artefacts and other blacklisted files in source tree.
  if(file MATCHES "(Makefile|\.in|\.xbt|\.so|\.dylib|\.gitignore)$")
    if(VERBOSE)
      message(STATUS "copy_file_to_buildtree - ignoring file: ${file}")
    endif()
    return()
  endif()

  if(NOT file STREQUAL ${CMAKE_BINARY_DIR}/${outfile})
    if(VERBOSE)
      message(STATUS "copy_file_to_buildtree - copying file: ${file} -> ${CMAKE_BINARY_DIR}/${outfile}")
    endif()
    file(APPEND ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ExportFiles.cmake
         "file(COPY \"${file}\" DESTINATION \"${CMAKE_BINARY_DIR}/${outdir}\")\n")
  endif()

  if(NOT arg_NO_INSTALL)
    list(APPEND install_data ${outfile})
    set(install_data ${install_data} PARENT_SCOPE)
  endif()
endfunction()

# Add data files to installation list with a mirror in build tree.
# reads list of files to install from a given list of text files.
# Arguments:
#   pattern globbing pattern for text files to read
# Optional Arguments:
#   NO_INSTALL: exclude files from installation target
# Implicit arguments:
#   CMAKE_SOURCE_DIR - root of source tree
# On return:
#   Files are mirrored to the build tree and added to ${install_data}
#   (if NO_INSTALL is not given).
function(copy_files_from_filelist_to_buildtree pattern)
  # copies files listed in text files to the buildtree
  # Input: [glob pattern: filepattern]
  cmake_parse_arguments(arg "NO_INSTALL" "" "" ${ARGN})
  list(APPEND pattern ${ARGN})
  list(SORT pattern)
  if(VERBOSE)
    message(STATUS "copy_files_from_filelist_to_buildtree - got pattern: ${pattern}")
  endif()
  foreach(pat ${pattern})
    file(GLOB filenames ${pat})
    foreach(filename ${filenames})
      string(STRIP ${filename} filename)
      core_file_read_filtered(fstrings ${filename})
      foreach(dir ${fstrings})
        string(CONFIGURE ${dir} dir)
        string(REPLACE " " ";" dir ${dir})
        list(GET dir 0 src)
        list(LENGTH dir len)
        if(len EQUAL 1)
          set(dest)
        elseif(len EQUAL 3)
          list(GET dir 1 opt)
          if(opt STREQUAL "KEEP_DIR_STRUCTURE")
            set(DIR_OPTION ${opt} ${src})
            if(VERBOSE)
              message(STATUS "copy_files_from_filelist_to_buildtree - DIR_OPTION: ${DIR_OPTION}")
            endif()
          endif()
          list(GET dir -1 dest)
        else()
          list(GET dir -1 dest)
        endif()

        # If the full path to an existing file is specified then add that single file.
        # Don't recursively add all files with the given name.
        if(EXISTS ${CMAKE_SOURCE_DIR}/${src} AND (NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${src} OR DIR_OPTION))
          set(files ${src})
        else()
          file(GLOB_RECURSE files RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/${src})
        endif()

        foreach(file ${files})
          if(arg_NO_INSTALL)
            copy_file_to_buildtree(${CMAKE_SOURCE_DIR}/${file} DIRECTORY ${dest} NO_INSTALL ${DIR_OPTION})
          else()
            copy_file_to_buildtree(${CMAKE_SOURCE_DIR}/${file} DIRECTORY ${dest} ${DIR_OPTION})
          endif()
        endforeach()
      endforeach()
    endforeach()
  endforeach()
  set(install_data ${install_data} PARENT_SCOPE)
endfunction()

# helper macro to set modified variables in parent scope
macro(export_dep)
  set(SYSTEM_INCLUDES ${SYSTEM_INCLUDES} PARENT_SCOPE)
  set(DEPLIBS ${DEPLIBS} PARENT_SCOPE)
  set(DEP_DEFINES ${DEP_DEFINES} PARENT_SCOPE)
  set(${depup}_FOUND ${${depup}_FOUND} PARENT_SCOPE)
  mark_as_advanced(${depup}_LIBRARIES)
endmacro()

# add a required dependency of main application
# Arguments:
#   dep_list name of find rule for dependency, used uppercased for variable prefix
#            also accepts a list of multiple dependencies
# On return:
#   dependency added to ${SYSTEM_INCLUDES}, ${DEPLIBS} and ${DEP_DEFINES}
function(core_require_dep)
  foreach(dep ${ARGN})
    find_package(${dep} REQUIRED)
    string(TOUPPER ${dep} depup)
    list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
    list(APPEND DEPLIBS ${${depup}_LIBRARIES})
    list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
    export_dep()
  endforeach()
endfunction()

# add a required dyloaded dependency of main application
# Arguments:
#   dep_list name of find rule for dependency, used uppercased for variable prefix
#            also accepts a list of multiple dependencies
# On return:
#   dependency added to ${SYSTEM_INCLUDES}, ${dep}_SONAME is set up
function(core_require_dyload_dep)
  foreach(dep ${ARGN})
    find_package(${dep} REQUIRED)
    string(TOUPPER ${dep} depup)
    list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
    list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
    find_soname(${depup} REQUIRED)
    export_dep()
    set(${depup}_SONAME ${${depup}_SONAME} PARENT_SCOPE)
  endforeach()
endfunction()

# helper macro for optional deps
macro(setup_enable_switch)
  string(TOUPPER ${dep} depup)
  if(${ARGV1})
    set(enable_switch ${ARGV1})
  else()
    set(enable_switch ENABLE_${depup})
  endif()
  # normal options are boolean, so we override set our ENABLE_FOO var to allow "auto" handling
  set(${enable_switch} "AUTO" CACHE STRING "Enable ${depup} support?")
endmacro()

# add an optional dependency of main application
# Arguments:
#   dep_list name of find rule for dependency, used uppercased for variable prefix
#            also accepts a list of multiple dependencies
# On return:
#   dependency optionally added to ${SYSTEM_INCLUDES}, ${DEPLIBS} and ${DEP_DEFINES}
function(core_optional_dep)
  foreach(dep ${ARGN})
    set(_required False)
    setup_enable_switch()
    if(${enable_switch} STREQUAL AUTO)
      find_package(${dep})
    elseif(${${enable_switch}})
      find_package(${dep} REQUIRED)
      set(_required True)
    endif()

    if(${depup}_FOUND)
      list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
      list(APPEND DEPLIBS ${${depup}_LIBRARIES})
      list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
      set(final_message ${final_message} "${depup} enabled: Yes")
      export_dep()
    elseif(_required)
      message(FATAL_ERROR "${depup} enabled but not found")
    else()
      set(final_message ${final_message} "${depup} enabled: No")
    endif()
  endforeach()
  set(final_message ${final_message} PARENT_SCOPE)
endfunction()

# add an optional dyloaded dependency of main application
# Arguments:
#   dep_list name of find rule for dependency, used uppercased for variable prefix
#            also accepts a list of multiple dependencies
# On return:
#   dependency optionally added to ${SYSTEM_INCLUDES}, ${DEP_DEFINES}, ${dep}_SONAME is set up
function(core_optional_dyload_dep)
  foreach(dep ${ARGN})
    set(_required False)
    setup_enable_switch()
    if(${enable_switch} STREQUAL AUTO)
      find_package(${dep})
    elseif(${${enable_switch}})
      find_package(${dep} REQUIRED)
      set(_required True)
    endif()

    if(${depup}_FOUND)
      list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
      find_soname(${depup} REQUIRED)
      list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
      set(final_message ${final_message} "${depup} enabled: Yes" PARENT_SCOPE)
      export_dep()
      set(${depup}_SONAME ${${depup}_SONAME} PARENT_SCOPE)
    elseif(_required)
      message(FATAL_ERROR "${depup} enabled but not found")
    else()
      set(final_message ${final_message} "${depup} enabled: No" PARENT_SCOPE)
    endif()
  endforeach()
endfunction()

function(core_file_read_filtered result filepattern)
  # Reads STRINGS from text files
  #  with comments filtered out
  # Result: [list: result]
  # Input:  [glob pattern: filepattern]
  file(GLOB filenames ${filepattern})
  list(SORT filenames)
  foreach(filename ${filenames})
    if(VERBOSE)
      message(STATUS "core_file_read_filtered - filename: ${filename}")
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${filename})
    file(STRINGS ${filename} fstrings REGEX "^[^#//]")
    foreach(fstring ${fstrings})
      string(REGEX REPLACE "^(.*)#(.*)" "\\1" fstring ${fstring})
      string(REGEX REPLACE "[ \n\r\t]//.*" "" fstring ${fstring})
      string(STRIP ${fstring} fstring)
      list(APPEND filename_strings ${fstring})
    endforeach()
  endforeach()
  set(${result} ${filename_strings} PARENT_SCOPE)
endfunction()

function(core_add_subdirs_from_filelist files)
  # Adds subdirectories from a sorted list of files
  # Input: [list: filenames] [bool: sort]
  foreach(arg ${ARGN})
    list(APPEND files ${arg})
  endforeach()
  list(SORT files)
  if(VERBOSE)
    message(STATUS "core_add_subdirs_from_filelist - got pattern: ${files}")
  endif()
  foreach(filename ${files})
    string(STRIP ${filename} filename)
    core_file_read_filtered(fstrings ${filename})
    foreach(subdir ${fstrings})
      string(REPLACE " " ";" subdir ${subdir})
      list(GET subdir  0 subdir_src)
      list(GET subdir -1 subdir_dest)
      if(VERBOSE)
        message(STATUS "  core_add_subdirs_from_filelist - adding subdir: ${CMAKE_SOURCE_DIR}/${subdir_src} -> ${CORE_BUILD_DIR}/${subdir_dest}")
      endif()
      add_subdirectory(${CMAKE_SOURCE_DIR}/${subdir_src} ${CORE_BUILD_DIR}/${subdir_dest})
    endforeach()
  endforeach()
endfunction()

macro(core_add_optional_subdirs_from_filelist pattern)
  # Adds subdirectories from text files
  #  if the option(s) in the 3rd field are enabled
  # Input: [glob pattern: filepattern]
  foreach(arg ${ARGN})
    list(APPEND pattern ${arg})
  endforeach()
  foreach(elem ${pattern})
    string(STRIP ${elem} elem)
    list(APPEND filepattern ${elem})
  endforeach()

  file(GLOB filenames ${filepattern})
  list(SORT filenames)
  if(VERBOSE)
    message(STATUS "core_add_optional_subdirs_from_filelist - got pattern: ${filenames}")
  endif()

  foreach(filename ${filenames})
    if(VERBOSE)
      message(STATUS "core_add_optional_subdirs_from_filelist - reading file: ${filename}")
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${filename})
    file(STRINGS ${filename} fstrings REGEX "^[^#//]")
    foreach(line ${fstrings})
      string(REPLACE " " ";" line "${line}")
      list(GET line 0 subdir_src)
      list(GET line 1 subdir_dest)
      list(GET line 3 opts)
      foreach(opt ${opts})
        if(ENABLE_${opt})
          if(VERBOSE)
            message(STATUS "  core_add_optional_subdirs_from_filelist - adding subdir: ${CMAKE_SOURCE_DIR}/${subdir_src} -> ${CORE_BUILD_DIR}/${subdir_dest}")
          endif()
          add_subdirectory(${CMAKE_SOURCE_DIR}/${subdir_src} ${CORE_BUILD_DIR}/${subdir_dest})
        else()
          if(VERBOSE)
            message(STATUS "  core_add_optional_subdirs_from_filelist: OPTION ${opt} not enabled for ${subdir_src}, skipping subdir")
          endif()
        endif()
      endforeach()
    endforeach()
  endforeach()
endmacro()

# Generates an RFC2822 timestamp
#
# The following variable is set:
#   RFC2822_TIMESTAMP
function(rfc2822stamp)
  execute_process(COMMAND date -R
                  OUTPUT_VARIABLE RESULT)
  set(RFC2822_TIMESTAMP ${RESULT} PARENT_SCOPE)
endfunction()

# Generates an user stamp from git config info
#
# The following variable is set:
#   PACKAGE_MAINTAINER - user stamp in the form of "username <username@example.com>"
#                        if no git tree is found, value is set to "nobody <nobody@example.com>"
function(userstamp)
  find_package(Git)
  if(GIT_FOUND AND EXISTS ${CMAKE_SOURCE_DIR}/.git)
    execute_process(COMMAND ${GIT_EXECUTABLE} config user.name
                    OUTPUT_VARIABLE username
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} config user.email
                    OUTPUT_VARIABLE useremail
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(PACKAGE_MAINTAINER "${username} <${useremail}>" PARENT_SCOPE)
  else()
    set(PACKAGE_MAINTAINER "nobody <nobody@example.com>" PARENT_SCOPE)
  endif()
endfunction()

# Parses git info and sets variables used to identify the build
# Arguments:
#   stamp variable name to return
# Optional Arguments:
#   FULL: generate git HEAD commit in the form of 'YYYYMMDD-hash'
#         if git tree is dirty, value is set in the form of 'YYYYMMDD-hash-dirty'
#         if no git tree is found, value is set in the form of 'YYYYMMDD-nogitfound'
#         if FULL is not given, stamp is generated following the same process as above
#         but without 'YYYYMMDD'
# On return:
#   Variable is set with generated stamp to PARENT_SCOPE
function(core_find_git_rev stamp)
  # allow manual setting GIT_VERSION
  if(GIT_VERSION)
    set(${stamp} ${GIT_VERSION} PARENT_SCOPE)
  else()
    find_package(Git)
    if(GIT_FOUND AND EXISTS ${CMAKE_SOURCE_DIR}/.git)
      execute_process(COMMAND ${GIT_EXECUTABLE} update-index --ignore-submodules --refresh -q)
      execute_process(COMMAND ${GIT_EXECUTABLE} diff-files --ignore-submodules --quiet --
                      RESULT_VARIABLE status_code
                      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        if(NOT status_code)
          execute_process(COMMAND ${GIT_EXECUTABLE} diff-index --ignore-submodules --quiet HEAD --
                        RESULT_VARIABLE status_code
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        endif()
        if(status_code)
          execute_process(COMMAND ${GIT_EXECUTABLE} log -n 1 --pretty=format:"%h-dirty" HEAD
                          OUTPUT_VARIABLE HASH
                          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
          string(SUBSTRING ${HASH} 1 13 HASH)
        else()
          execute_process(COMMAND ${GIT_EXECUTABLE} log -n 1 --pretty=format:"%h" HEAD
                          OUTPUT_VARIABLE HASH
                          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
          string(SUBSTRING ${HASH} 1 7 HASH)
        endif()
      execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:"%cd" --date=short HEAD
                      OUTPUT_VARIABLE DATE
                      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
      string(SUBSTRING ${DATE} 1 10 DATE)
      string(REPLACE "-" "" DATE ${DATE})
    else()
      string(TIMESTAMP DATE "%Y%m%d" UTC)
      set(HASH "nogitfound")
    endif()
    cmake_parse_arguments(arg "FULL" "" "" ${ARGN})
    if(arg_FULL)
      set(${stamp} ${DATE}-${HASH} PARENT_SCOPE)
    else()
      set(${stamp} ${HASH} PARENT_SCOPE)
    endif()
  endif()
endfunction()

# Parses version.txt and versions.h and sets variables
# used to construct dirs structure, file naming, API version, etc.
#
# The following variables are set from version.txt:
#   APP_NAME - app name
#   APP_NAME_LC - lowercased app name
#   APP_NAME_UC - uppercased app name
#   APP_PACKAGE - Android full package name
#   COMPANY_NAME - company name
#   APP_VERSION_MAJOR - the app version major
#   APP_VERSION_MINOR - the app version minor
#   APP_VERSION_TAG - the app version tag
#   APP_VERSION_TAG_LC - lowercased app version tag
#   APP_VERSION - the app version (${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}-${APP_VERSION_TAG})
#   APP_ADDON_API - the addon API version in the form of 16.9.702
#   FILE_VERSION - file version in the form of 16,9,702,0 - Windows only
#
# Set various variables defined in "versions.h"
macro(core_find_versions)
  # kodi-addons project also calls this macro and uses CORE_SOURCE_DIR
  # to point to core base dir
  # Set CORE_SOURCE_DIR here, otherwise kodi main project fails
  # TODO: drop this code block and refactor the rest to use CMAKE_SOURCE_DIR
  # if we ever integrate kodi-addons into kodi project
  if(NOT CORE_SOURCE_DIR)
    set(CORE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
  endif()

  include(CMakeParseArguments)
  core_file_read_filtered(version_list ${CORE_SOURCE_DIR}/version.txt)
  string(REPLACE " " ";" version_list "${version_list}")
  cmake_parse_arguments(APP "" "APP_NAME;COMPANY_NAME;WEBSITE;VERSION_MAJOR;VERSION_MINOR;VERSION_TAG;VERSION_CODE;ADDON_API;APP_PACKAGE" "" ${version_list})

  set(APP_NAME ${APP_APP_NAME}) # inconsistency but APP_APP_NAME looks weird
  string(TOLOWER ${APP_APP_NAME} APP_NAME_LC)
  string(TOUPPER ${APP_APP_NAME} APP_NAME_UC)
  set(COMPANY_NAME ${APP_COMPANY_NAME})
  set(APP_VERSION ${APP_VERSION_MAJOR}.${APP_VERSION_MINOR})
  set(APP_PACKAGE ${APP_APP_PACKAGE})
  if(APP_VERSION_TAG)
    set(APP_VERSION ${APP_VERSION}-${APP_VERSION_TAG})
    string(TOLOWER ${APP_VERSION_TAG} APP_VERSION_TAG_LC)
  endif()
  string(REPLACE "." "," FILE_VERSION ${APP_ADDON_API}.0)

  # Set defines used in addon.xml.in and read from versions.h to set add-on
  # version parts automatically
  # This part is nearly identical to "AddonHelpers.cmake", except location of versions.h
  file(STRINGS ${CORE_SOURCE_DIR}/xbmc/addons/kodi-addon-dev-kit/include/kodi/versions.h BIN_ADDON_PARTS)
  foreach(loop_var ${BIN_ADDON_PARTS})
    string(FIND "${loop_var}" "#define ADDON_" matchres)
    if("${matchres}" EQUAL 0)
      string(REGEX MATCHALL "[A-Z0-9._]+|[A-Z0-9._]+$" loop_var "${loop_var}")
      list(GET loop_var 0 include_name)
      list(GET loop_var 1 include_version)
      string(REGEX REPLACE ".*\"(.*)\"" "\\1" ${include_name} ${include_version})
    endif()
  endforeach(loop_var)

  # unset variables not used anywhere else
  unset(version_list)
  unset(APP_APP_NAME)
  unset(BIN_ADDON_PARTS)

  # bail if we can't parse version.txt
  if(NOT DEFINED APP_VERSION_MAJOR OR NOT DEFINED APP_VERSION_MINOR)
    message(FATAL_ERROR "Could not determine app version! Make sure that ${CORE_SOURCE_DIR}/version.txt exists")
  endif()
endmacro()

# add-on xml's
# find all folders containing addon.xml.in and used to define
# ADDON_XML_OUTPUTS, ADDON_XML_DEPENDS and ADDON_INSTALL_DATA
macro(find_addon_xml_in_files)
  file(GLOB ADDON_XML_IN_FILE ${CMAKE_SOURCE_DIR}/addons/*/addon.xml.in)
  foreach(loop_var ${ADDON_XML_IN_FILE})
    list(GET loop_var 0 xml_name)

    string(REPLACE "/addon.xml.in" "" xml_name ${xml_name})
    string(REPLACE "${CORE_SOURCE_DIR}/" "" xml_name ${xml_name})

    list(APPEND ADDON_XML_DEPENDS "${CORE_SOURCE_DIR}/${xml_name}/addon.xml.in")
    list(APPEND ADDON_XML_OUTPUTS "${CMAKE_BINARY_DIR}/${xml_name}/addon.xml")

    # Read content of add-on folder to have on install
    file(GLOB ADDON_FILES "${CORE_SOURCE_DIR}/${xml_name}/*")
    foreach(loop_var ${ADDON_FILES})
      if(loop_var MATCHES "addon.xml.in")
        string(REPLACE "addon.xml.in" "addon.xml" loop_var ${loop_var})
      endif()

      list(GET loop_var 0 file_name)
      string(REPLACE "${CORE_SOURCE_DIR}/" "" file_name ${file_name})
      list(APPEND ADDON_INSTALL_DATA "${file_name}")

      unset(file_name)
    endforeach()
    unset(xml_name)
  endforeach()

  # Append also versions.h to depends
  list(APPEND ADDON_XML_DEPENDS "${CORE_SOURCE_DIR}/xbmc/addons/kodi-addon-dev-kit/include/kodi/versions.h")
endmacro()
