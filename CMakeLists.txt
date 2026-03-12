# Copyright (c) 2026 Joshua Murphy VO1RFX
# This project is based on mvoice by N7TAE
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

cmake_minimum_required(VERSION 3.21)

project(mvoice
    VERSION 1.0.0
    DESCRIPTION "M17 Digital Voice Gateway"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(USE_44100  "Use 44100 Hz audio sample rate instead of 8000 Hz" OFF)
option(USE_DHT    "Enable OpenDHT (Ham-DHT) support"                  ON)

set(CFGDIR ".config/mvoice" CACHE STRING
    "User configuration directory, relative to $HOME")

set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH
    "Installation prefix (binary goes to <prefix>/bin, locale to <prefix>/share/locale)")

add_library(codec2 STATIC
    third_party/codec2/codec2.cpp
    third_party/codec2/codebooks.cpp
    third_party/codec2/kiss_fft.cpp
    third_party/codec2/lpc.cpp
    third_party/codec2/nlp.cpp
    third_party/codec2/pack.cpp
    third_party/codec2/qbase.cpp
    third_party/codec2/quantise.cpp
)

target_include_directories(codec2 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/codec2>
)

target_compile_options(codec2 PRIVATE -W)

# --------------------------------------------------------------------------- #
# Find required packages
# --------------------------------------------------------------------------- #
include(FindPkgConfig)

find_package(Threads REQUIRED)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ALSA
pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)

# libcurl
find_package(CURL REQUIRED)

# nlohmann_json
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# OpenDHT (optional)
if(USE_DHT)
    FetchContent_Declare(opendht
        GIT_REPOSITORY https://github.com/savoirfairelinux/opendht.git
        GIT_TAG v3.7.3
        GIT_SHALLOW TRUE
        PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch-opendht-gcc15.cmake # A funky patch is required because of some compiler shenanigans
    )
    set(OPENDHT_TOOLS OFF CACHE INTERNAL "")
    set(OPENDHT_PYTHON OFF CACHE INTERNAL "")
    set(OPENDHT_C OFF CACHE INTERNAL "")
    set(OPENDHT_PROXY_SERVER OFF CACHE INTERNAL "")
    set(OPENDHT_PROXY_CLIENT OFF CACHE INTERNAL "")
    set(OPENDHT_PEER_DISCOVERY OFF CACHE INTERNAL "")
    set(OPENDHT_INDEX OFF CACHE INTERNAL "")
    set(OPENDHT_HTTP OFF CACHE INTERNAL "")
    set(OPENDHT_DOCUMENTATION OFF CACHE INTERNAL "")
    set(BUILD_TESTING OFF CACHE INTERNAL "")
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(opendht)
endif()

# Main executable
set(MVOICE_SOURCES
    AppCore.cpp
    AudioManager.cpp
    Base.cpp
    Callsign.cpp
    Configure.cpp
    CRC.cpp
    FrameType.cpp
    M17Gateway.cpp
    M17RouteMap.cpp
    MainWindow.cpp
    Message.cpp
    Packet.cpp
    SettingsDlg.cpp
    SMSDlg.cpp
    AboutDlg.cpp
    TransmitButton.cpp
    UDPSocket.cpp
    UnixDgramSocket.cpp
)

if(USE_44100)
    list(APPEND MVOICE_SOURCES Resampler.cpp)
endif()

add_executable(mvoice ${MVOICE_SOURCES})
set_target_properties(mvoice PROPERTIES AUTOMOC ON)

target_link_libraries(mvoice PRIVATE
    codec2
    Threads::Threads
    PkgConfig::ALSA
    CURL::libcurl
    Qt6::Widgets
)

target_include_directories(mvoice PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compile definitions matching the old Makefile
target_compile_definitions(mvoice PRIVATE
    CFGDIR="${CFGDIR}"
    BASEDIR="${INSTALL_PREFIX}"
)

target_compile_options(mvoice PRIVATE -W)

target_link_libraries(mvoice PRIVATE nlohmann_json::nlohmann_json)

# Optional 44100 Hz resampler
if(USE_44100)
    target_compile_definitions(mvoice PRIVATE USE44100)
endif()

# Optional: OpenDHT
if(USE_DHT)
    target_include_directories(mvoice BEFORE PRIVATE ${opendht_SOURCE_DIR}/include)
    target_link_libraries(mvoice PRIVATE opendht)
else()
    target_compile_definitions(mvoice PRIVATE NO_DHT)
endif()

# Internationalization - Qt .ts/.qm files can be added later.
# For now, keep gettext .mo generation for the Italian translation.
find_program(MSGFMT_EXE msgfmt)
set(MVOICE_LINGUAS it)

if(MSGFMT_EXE)
    foreach(lang ${MVOICE_LINGUAS})
        set(_po  "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}.po")
        set(_mo  "${CMAKE_CURRENT_BINARY_DIR}/po/${lang}.mo")
        add_custom_command(
            OUTPUT  ${_mo}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/po"
            COMMAND ${MSGFMT_EXE} ${_po} --output=${_mo}
            DEPENDS ${_po}
            COMMENT "Compiling ${lang}.po -> ${lang}.mo"
        )
        list(APPEND MO_FILES ${_mo})
    endforeach()
    add_custom_target(translations ALL DEPENDS ${MO_FILES})
else()
    message(WARNING "msgfmt not found — translations will not be built")
endif()

# Install rules
include(GNUInstallDirs)

install(TARGETS mvoice RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(MSGFMT_EXE)
    foreach(lang ${MVOICE_LINGUAS})
        install(
            FILES       "${CMAKE_CURRENT_BINARY_DIR}/po/${lang}.mo"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/locale/${lang}/LC_MESSAGES"
            RENAME      mvoice.mo
        )
    endforeach()
endif()

install(FILES mvoice.desktop DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
