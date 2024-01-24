cmake_minimum_required(VERSION 3.26)
project(SwcManagerClient)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

string(APPEND CMAKE_PREFIX_PATH ";C:/Qt/6.6.0/msvc2019_64")
message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Svg
        REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include_directories(
        SwcDbmsCommon/Generated/cpp
        src/styles
        ThirdParty/nlohmann_json
)

link_directories(
        .

)

set(QRESOURCE
        Resource/resources.qrc
)

file(GLOB_RECURSE SRC_LIST
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.ui"
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.hpp"
)

set(PROJECT_HEADERS
        src/styles/acss_globals.h
        src/styles/QtAdvancedStylesheet.h
)

set(PROJECT_SOURCES
        main.cpp

        src/styles/QtAdvancedStylesheet.cpp

        SwcDbmsCommon/Generated/cpp/Message/Message.grpc.pb.cc
        SwcDbmsCommon/Generated/cpp/Message/Message.pb.cc
        SwcDbmsCommon/Generated/cpp/Message/Request.grpc.pb.cc
        SwcDbmsCommon/Generated/cpp/Message/Request.pb.cc
        SwcDbmsCommon/Generated/cpp/Message/Response.grpc.pb.cc
        SwcDbmsCommon/Generated/cpp/Message/Response.pb.cc
        SwcDbmsCommon/Generated/cpp/Service/Service.grpc.pb.cc
        SwcDbmsCommon/Generated/cpp/Service/Service.pb.cc



        app_win32.rc
)

add_executable(${PROJECT_NAME} ${QRESOURCE} ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Svg
        PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
        PRIVATE spdlog::spdlog
        PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts
)

if (WIN32)
    Set_Target_Properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif ()

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
