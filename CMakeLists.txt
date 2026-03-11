cmake_minimum_required(VERSION 3.16)

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

# FLTK — prefer fltk-config, fall back to CMake package
find_program(FLTK_CONFIG_EXE fltk-config)
if(FLTK_CONFIG_EXE)
    execute_process(COMMAND ${FLTK_CONFIG_EXE} --cxxflags
        OUTPUT_VARIABLE FLTK_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${FLTK_CONFIG_EXE} --use-images --ldflags
        OUTPUT_VARIABLE FLTK_LDFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Split into lists for CMake consumption
    separate_arguments(FLTK_CXXFLAGS_LIST UNIX_COMMAND "${FLTK_CXXFLAGS}")
    separate_arguments(FLTK_LDFLAGS_LIST  UNIX_COMMAND "${FLTK_LDFLAGS}")
else()
    find_package(FLTK REQUIRED)
endif()

# ALSA
pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)

# libcurl
find_package(CURL REQUIRED)

# nlohmann_json — try CMake package first, then pkg-config, then header-only
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    pkg_check_modules(NLOHMANN_JSON QUIET nlohmann_json)
endif()

# OpenDHT (optional)
if(USE_DHT)
    pkg_check_modules(OPENDHT REQUIRED IMPORTED_TARGET opendht)
endif()

# gettext for i18n
find_package(Intl)
find_program(MSGFMT_EXE msgfmt)

# Main executable
set(MVOICE_SOURCES
    AboutDlg.cpp
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
    TransmitButton.cpp
    UDPSocket.cpp
    UnixDgramSocket.cpp
)

if(USE_44100)
    list(APPEND MVOICE_SOURCES Resampler.cpp)
endif()

add_executable(mvoice ${MVOICE_SOURCES})

target_link_libraries(mvoice PRIVATE
    codec2
    Threads::Threads
    PkgConfig::ALSA
    CURL::libcurl
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

# FLTK flags
if(FLTK_CONFIG_EXE)
    target_compile_options(mvoice PRIVATE ${FLTK_CXXFLAGS_LIST})
    target_link_options(mvoice PRIVATE ${FLTK_LDFLAGS_LIST})
else()
    target_link_libraries(mvoice PRIVATE fltk fltk_images)
endif()

# JSON 
if(nlohmann_json_FOUND)
    target_link_libraries(mvoice PRIVATE nlohmann_json::nlohmann_json)
elseif(NLOHMANN_JSON_FOUND)
    target_include_directories(mvoice PRIVATE ${NLOHMANN_JSON_INCLUDE_DIRS})
endif()
# If neither found, the system header path must already include nlohmann/json.hpp.

# Optional 44100 Hz resampler
if(USE_44100)
    target_compile_definitions(mvoice PRIVATE USE44100)
endif()

# Optional: OpenDHT
if(USE_DHT)
    target_link_libraries(mvoice PRIVATE PkgConfig::OPENDHT)
else()
    target_compile_definitions(mvoice PRIVATE NO_DHT)
endif()

# Internationalization (gettext .mo files)
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
