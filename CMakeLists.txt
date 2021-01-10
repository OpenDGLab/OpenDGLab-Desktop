cmake_minimum_required(VERSION 3.5)

project(OpenDGLab-Desktop LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OPENDGLABCORE_HEADER ${CMAKE_SOURCE_DIR}/opendglab-core/header)
if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()
if (WIN32)
    set(OPENDGLABCORE_LIB ${CMAKE_SOURCE_DIR}/opendglab-core/bin/libopendglab.lib)
    set(OPENDGLABCORE_BIN ${CMAKE_SOURCE_DIR}/opendglab-core/bin/libopendglab.dll)
    SET(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
    SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
    SET(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")
    SET(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")
    if(CMAKE_GENERATOR STREQUAL "Ninja")
        SET(CMAKE_NINJA_FORCE_RESPONSE_FILE 1 CACHE INTERNAL "")
    endif()
elseif(LINUX)
    set(OPENDGLABCORE_LIB ${CMAKE_SOURCE_DIR}/opendglab-core/bin/libopendglab.a)
    find_package(Threads REQUIRED)
elseif(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE res/OpenDGLab-Desktop.icns)
    set(APP_ICON_MACOSX ${CMAKE_SOURCE_DIR}/res/OpenDGLab-Desktop.icns)
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(CMAKE_C_FLAGS "-x objective-c")
    set(OPENDGLABCORE_LIB ${CMAKE_SOURCE_DIR}/opendglab-core/bin/libopendglab.a)
else()
    message(FATAL_ERROR "This platform may not support.")
endif()
if (LINUX)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Bluetooth Network REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Bluetooth Network REQUIRED)
else()
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Bluetooth Network Websockets REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Bluetooth Network Websockets REQUIRED)
endif ()
include_directories(${OPENDGLABCORE_HEADER})
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
find_package(Protobuf REQUIRED)
set_property(SOURCE proto/app.pb.h PROPERTY SKIP_AUTOGEN ON)
set_property(SOURCE proto/app.pb.cc PROPERTY SKIP_AUTOGEN ON)
set_target_properties(protobuf::libprotobuf PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "PROTOBUF_USE_DLLS"
)

if(Qt5_FOUND AND WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
    get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

    execute_process(
        COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt5_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")

    if(EXISTS ${imported_location})
        add_executable(Qt5::windeployqt IMPORTED)

        set_target_properties(Qt5::windeployqt PROPERTIES
            IMPORTED_LOCATION ${imported_location}
        )
    endif()
endif()
file(GLOB_RECURSE OpenDGLabDesktop_SRC
    "src/*.h"
    "src/*.cpp"
    "src/*.ui"
    "proto/*.proto"
)
add_library(libopendglab STATIC IMPORTED)
set_property(TARGET libopendglab PROPERTY IMPORTED_LOCATION ${OPENDGLABCORE_LIB})
if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.rc")
    add_executable(OpenDGLab-Desktop WIN32 ${OpenDGLabDesktop_SRC} ${appProto_PROTO_SRCS} res/image.qrc ${APP_ICON_RESOURCE_WINDOWS})
    target_link_libraries(OpenDGLab-Desktop PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Bluetooth Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::WebSockets libopendglab protobuf::libprotobuf)
elseif(LINUX)
    add_executable(OpenDGLab-Desktop ${OpenDGLabDesktop_SRC} ${appProto_PROTO_SRCS} res/image.qrc)
    target_link_libraries(OpenDGLab-Desktop PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Bluetooth Qt${QT_VERSION_MAJOR}::Network libopendglab protobuf::libprotobuf Threads::Threads ${CMAKE_DL_LIBS})
elseif(APPLE)
    add_executable(OpenDGLab-Desktop MACOSX_BUNDLE ${OpenDGLabDesktop_SRC} ${appProto_PROTO_SRCS} res/image.qrc ${APP_ICON_MACOSX})
    target_link_libraries(OpenDGLab-Desktop PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Bluetooth Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::WebSockets libopendglab protobuf::libprotobuf ${CMAKE_DL_LIBS} "-framework AppKit" "-framework CoreData" "-framework Foundation")
endif()
target_include_directories(OpenDGLab-Desktop PUBLIC "src")

protobuf_generate(TARGET OpenDGLab-Desktop)

if(TARGET Qt5::windeployqt)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(WIN_ARCH "x86")
    else()
        set(WIN_ARCH "x64")
    endif()
    add_custom_command(TARGET OpenDGLab-Desktop
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/OpenDGLab-Desktop-${WIN_ARCH}"
        COMMAND set PATH=%PATH%$<SEMICOLON>${qt5_install_prefix}/bin
        COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/OpenDGLab-Desktop-${WIN_ARCH}" "$<TARGET_FILE_DIR:OpenDGLab-Desktop>/$<TARGET_FILE_NAME:OpenDGLab-Desktop>"
    )
    install(
        TARGETS OpenDGLab-Desktop
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/target/bin
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/target/bin
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/target/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_SOURCE_DIR}/target/bin
    )

    # copy deployment directory during installation
    install(
        DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/OpenDGLab-Desktop-${WIN_ARCH}/"
        DESTINATION ${OpenDGLab-Desktop_BINARY_DIR}
    )

    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)

    include(InstallRequiredSystemLibraries)

    install(
        PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
        DESTINATION ${OpenDGLab-Desktop_BINARY_DIR}
    )
    add_custom_command(TARGET OpenDGLab-Desktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${OPENDGLABCORE_BIN}
            "$<TARGET_FILE_DIR:OpenDGLab-Desktop>/OpenDGLab-Desktop-${WIN_ARCH}/")
    add_custom_command(TARGET OpenDGLab-Desktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:OpenDGLab-Desktop>
            "$<TARGET_FILE_DIR:OpenDGLab-Desktop>/OpenDGLab-Desktop-${WIN_ARCH}/")
    if(CMAKE_BUILD_TYPE MATCHES DEBUG OR CMAKE_BUILD_TYPE MATCHES Debug)
        add_custom_command(TARGET OpenDGLab-Desktop POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/libprotobufd.dll
                "$<TARGET_FILE_DIR:OpenDGLab-Desktop>/OpenDGLab-Desktop-${WIN_ARCH}/")
    else()
        add_custom_command(TARGET OpenDGLab-Desktop POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/libprotobuf.dll
                "$<TARGET_FILE_DIR:OpenDGLab-Desktop>/OpenDGLab-Desktop-${WIN_ARCH}/")
    endif()
    add_custom_command(TARGET OpenDGLab-Desktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar "cfv" "OpenDGLab-Desktop-${WIN_ARCH}.zip" --format=zip
        "$<TARGET_FILE_DIR:OpenDGLab-Desktop>/OpenDGLab-Desktop-${WIN_ARCH}")
endif()
