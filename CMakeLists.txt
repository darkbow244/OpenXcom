project ( OpenXcom )

cmake_minimum_required ( VERSION 2.8.5 )
set ( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" )
include(GNUInstallDirs)

option ( DEV_BUILD "Development Build. Disable this for release builds" ON )
option ( BUILD_PACKAGE "Prepares build for creation of a package with CPack" ON )
option ( ENABLE_WARNING "Always show warnings (even for release builds)" OFF )
option ( FATAL_WARNING "Treat warnings as errors" OFF )
option ( ENABLE_CLANG_ANALYSIS "When building with clang, enable the static analyzer" OFF )
set ( MSVC_WARNING_LEVEL 3 CACHE STRING "Visual Studio warning levels" )
option ( FORCE_INSTALL_DATA_TO_BIN "Force installation of data to binary directory" OFF )
set ( DATADIR "" CACHE STRING "Where to place datafiles" )
set ( OPENXCOM_VERSION_STRING "" CACHE STRING "Version string (after x.x)" )


set ( default_deps_dir "${CMAKE_SOURCE_DIR}/deps" )

if ( APPLE )
  set ( MACOS_SDLMAIN_M_PATH "${CMAKE_SOURCE_DIR}" CACHE STRING "Path to SDLMain.m file" )
  option ( CREATE_BUNDLE "Create a Mac OS application bundle" ON )
  if ( NOT EXISTS "${MACOS_SDLMAIN_M_PATH}" )
    message ( FATAL_ERROR "On Mac OS, SDLMain.m is required. Please set the MACOS_SDLMAIN_M_PATH variable" )
  endif ()
endif ()

set ( DEPS_DIR "${default_deps_dir}" CACHE STRING "Dependencies directory" )

if (CMAKE_CL_64)
  set(_WIN_ABI_PREFIX x64)
else()
  set(_WIN_ABI_PREFIX Win32)
endif()
if ((IS_DIRECTORY ${DEPS_DIR}/yaml-cpp) AND (NOT TARGET SDL_gfx))
  message(STATUS "Building yaml-cpp from source")
  add_subdirectory(${DEPS_DIR}/yaml-cpp)
else()
  message(STATUS "Searching for yaml-cpp on your system")
  find_package(Yaml_cpp 0.5.0)
  if ( NOT YAMLCPP_FOUND )
    message ( FATAL_ERROR "Can't find yaml-cpp which is required" )
  else ()
    include_directories ( ${YAMLCPP_INCLUDE_DIR} )
    message ( "found yaml-cpp(${YAMLCPP_LIBRARY_DIRS}:${YAMLCPP_INCLUDE_DIR})" )
  endif ( NOT YAMLCPP_FOUND )
  add_library(yaml-cpp SHARED IMPORTED)
  set_taget_properties(yaml-cpp
          PROPERTIES
          IMPORTED_LOCATION ${YAMLCPP_LIBRARY}
          INTERFACE_INCLUDE_DIRECTORIES ${YAMLCPP_INCLUDE_DIR}
          )
endif()
if ((IS_DIRECTORY ${DEPS_DIR}/SDL_gfx) AND (NOT TARGET SDL_gfx))
  message(STATUS "Building patched SDL_gfx from source")
  add_subdirectory(${DEPS_DIR}/SDL_gfx)
else()
  message(FATAL_ERROR "You need SDL_gfx specifically patched for SDL2 - you have to build from sources")
endif()
if ((IS_DIRECTORY ${DEPS_DIR}/SDL) AND (NOT TARGET SDL2))
  message(STATUS "Building SDL2 from source")
  add_subdirectory(${DEPS_DIR}/SDL)
else()
  message(STATUS "Searching for SDL2 on your system")
  find_library(_SDL2_LIBRARY SDL2)
  find_library(_SDL2MAIN_LIBRARY SDL2main)
  find_path(_SDL2_INCLUDE_DIR SDL2/SDL.h)
  message("Using SDL2 library ${_SDL2_LIBRARY}")
  add_library(SDL2 SHARED IMPORTED)
  set_target_properties(SDL2
          PROPERTIES
          IMPORTED_LOCATION ${_SDL2_LIBRARY}
          INTERFACE_LINK_LIBRARIES ${_SDL2MAIN_LIBRARY}
          INTERFACE_INCLUDE_DIRECTORIES ${_SDL2_INCLUDE_DIR}/SDL2
          )
endif()
# TODO: Suppy CMakeLists.txt for SDL2_image and SDL2_mixer
if ((IS_DIRECTORY ${DEPS_DIR}/SDL2_image) AND (NOT TARGET SDL2_image))
  message(STATUS "Building SDL2_image from source")
  add_subdirectory(${DEPS_DIR}/SDL2_image)
else()
  message(STATUS "Searching for SDL2_image on your system")
  find_library(_SDL2_IMAGE_LIBRARY SDL2_image)
  find_path(_SDL2_IMAGE_INCLUDE_DIR SDL2/SDL_image.h)
  message("Using SDL2_Image library ${_SDL2_IMAGE_LIBRARY}")
  add_library(SDL2_image SHARED IMPORTED)
  set_target_properties(SDL2_image
          PROPERTIES
          IMPORTED_LOCATION ${_SDL2_IMAGE_LIBRARY}
          INTERFACE_LINK_LIBRARIES SDL2
          INTERFACE_INCLUDE_DIRECTORIES ${_SDL2_IMAGE_INCLUDE_DIR}/SDL2
          )
endif()
if ((IS_DIRECTORY ${DEPS_DIR}/SDL2_mixer) AND (NOT TARGET SDL2_mixer))
  message(STATUS "Building SDL2_mixer from source")
  add_subdirectory(${DEPS_DIR}/SDL2_mixer)
else()
  message(STATUS "Searching for SDL2_mixer on your system")
  find_library(_SDL2_MIXER_LIBRARY SDL2_mixer)
  find_path(_SDL2_MIXER_INCLUDE_DIR SDL2/SDL_mixer.h)
  message("Using SDL2_Mixer library ${_SDL2_MIXER_LIBRARY}")
  add_library(SDL2_mixer SHARED IMPORTED)
  set_target_properties(SDL2_mixer
          PROPERTIES
          IMPORTED_LOCATION ${_SDL2_MIXER_LIBRARY}
          INTERFACE_LINK_LIBRARIES SDL2
          INTERFACE_INCLUDE_DIRECTORIES ${_SDL2_MIXER_INCLUDE_DIR}/SDL2
          )
endif()
# Sanity check
if (NOT TARGET yaml-cpp)
  message(FATAL_ERROR "yaml-cpp is not a target!")
endif()
# Find OpenGL
find_package ( OpenGL )
if ( NOT OPENGL_FOUND )
  message ( WARNING "Can't find OpenGL; how does that even happen?\n"
    "Continuing building without OpenGL support."
    )
  add_definitions(-D__NO_OPENGL)
else ()
  message(WARNING "OpenGL temporarily disabled, REMOVE BEFORE SENDING INTO PRODUCTION")
    add_definitions(-D__NO_OPENGL)
    include_directories ( ${OPENGL_INCLUDE_DIR} )
endif ()

# Read version number
set ( file "${CMAKE_SOURCE_DIR}/src/version.h" )
file ( READ ${file} lines )
string ( REGEX MATCH "[.]*OPENXCOM_VERSION_SHORT \"([0-9]).([0-9])" version_line "${lines}" )
set ( CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MATCH_1} )
set ( CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MATCH_2} )
set ( CPACK_PACKAGE_VERSION_PATCH "" )

if ( "${OPENXCOM_VERSION_STRING}" STREQUAL "" )
  find_package ( Git )
  if ( GIT_FOUND )
    message("git found: ${GIT_EXECUTABLE}")
    execute_process ( COMMAND ${GIT_EXECUTABLE} describe --dirty
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE git_describe_out
      ERROR_VARIABLE git_describe_error
      RESULT_VARIABLE git_describe_result
      )
    string ( REGEX MATCH "([a-z|0-9|.]*)-([0-9]*)-g([a-z|0-9]*)([-|a-z]*)" git_commit "${git_describe_out}" )
    set ( git_tag ${CMAKE_MATCH_1} )
    set ( git_nb_commit ${CMAKE_MATCH_2} )
    set ( git_commit ${CMAKE_MATCH_3} )
    set ( git_dirty ${CMAKE_MATCH_4} )
    set ( OPENXCOM_VERSION_STRING ".${git_commit}${git_dirty}" )
  endif()
endif()

add_definitions( -DGIT_BUILD=1 )

configure_file("${CMAKE_SOURCE_DIR}/src/git_version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/git_version.h" )
include_directories ( "${CMAKE_CURRENT_BINARY_DIR}" )


if ( DEV_BUILD )
  # Append the commit to version number
  set ( CPACK_PACKAGE_VERSION_PATCH "${git_commit}${git_dirty}" )
  set ( CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" )
else ()
  set ( CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_NSIS_PACKAGE_NAME}" )
  set ( CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}" )
endif ()

if ( BUILD_PACKAGE )
  if ( NOT DEV_BUILD )
    string ( LENGTH "${git_dirty}" is_dirty )
    if ( ${is_dirty} GREATER 0 )
      message ( FATAL_ERROR "Release package must be built from a clean tree" )
    endif ()
    if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
      message ( FATAL_ERROR "Release package can't be built from a debug build" )
    endif ()
  endif ( )

  if ( NOT CPACK_GENERATOR )
    set ( CPACK_GENERATOR "TXZ" )
  endif ()
  if ( NOT CPACK_SOURCE_GENERATOR )
    set ( CPACK_SOURCE_GENERATOR "TXZ" )
  endif ()

  set ( CPACK_PACKAGE_VENDOR "The OpenXcom project" )
  set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open-source clone of UFO: Enemy Unknown" )
  set ( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/cmake/modules/Description.txt" )
  set ( CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md" )
  if ( NOT APPLE )
    set ( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt" )
  endif ()
  set ( CPACK_PACKAGE_CONTACT "The OpenXcom project (http://www.openxcom.org)" )

  include ( LinuxDEB )
  include ( LinuxRPM )
  include ( nsis )
  include ( apple )
  include ( CPack )
  message ( "version:${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
)
endif()

if ( NOT WIN32 )
  install(FILES "${CMAKE_SOURCE_DIR}/res/linux/openxcom.desktop"
    DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/applications")
  install(FILES "${CMAKE_SOURCE_DIR}/res/linux/icons/openxcom_48x48.png"
    DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/48x48/apps" RENAME openxcom.png)
  install(FILES "${CMAKE_SOURCE_DIR}/res/linux/icons/openxcom_128x128.png"
    DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/128x128/apps" RENAME openxcom.png)
  install(FILES "${CMAKE_SOURCE_DIR}/res/linux/icons/openxcom.svg"
    DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/scalable/apps")
endif ()

add_subdirectory ( docs )
add_subdirectory ( src )
