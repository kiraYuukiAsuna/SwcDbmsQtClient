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

        src/ui/mainwindow.h
        src/ui/mainwindow.ui
        src/ui/mainwindow.cpp
        src/framework/service/RpcCall.cpp
        src/framework/service/RpcCall.h
        src/ui/registerwindow.cpp
        src/ui/registerwindow.h
        src/ui/registerwindow.ui
        src/ui/loginwindow.cpp
        src/ui/loginwindow.h
        src/ui/loginwindow.ui
        src/framework/config/AppConfig.cpp
        src/framework/config/AppConfig.h
        src/ui/leftclientview.cpp
        src/ui/leftclientview.h
        src/ui/leftclientview.ui
        src/ui/rightclientview.cpp
        src/ui/rightclientview.h
        src/ui/rightclientview.ui
        src/framework/service/CachedProtoData.cpp
        src/framework/service/CachedProtoData.h
        src/framework/defination/TypeDef.cpp
        src/framework/defination/TypeDef.h
        src/framework/service/WrappedCall.cpp
        src/framework/service/WrappedCall.h
        src/ui/editorprojectmetainfo.cpp
        src/ui/editorprojectmetainfo.h
        src/ui/editorprojectmetainfo.ui
        src/ui/editorswcmetainfo.cpp
        src/ui/editorswcmetainfo.h
        src/ui/editorswcmetainfo.ui
        src/ui/editordailystatisticsmetainfo.cpp
        src/ui/editordailystatisticsmetainfo.h
        src/ui/editordailystatisticsmetainfo.ui
        src/ui/viewswcnodedata.cpp
        src/ui/viewswcnodedata.h
        src/ui/viewswcnodedata.ui
        src/ui/editorusersettings.cpp
        src/ui/editorusersettings.h
        src/ui/editorusersettings.ui
        src/ui/editoradminusermanager.cpp
        src/ui/editoradminusermanager.h
        src/ui/editoradminusermanager.ui
        src/ui/editorpermissiongroup.cpp
        src/ui/editorpermissiongroup.h
        src/ui/editorpermissiongroup.ui
        src/ui/EditorBase.cpp
        src/ui/EditorBase.h
        src/framework/defination/ImageDefination.h
        src/ui/viewcreateproject.cpp
        src/ui/viewcreateproject.h
        src/ui/viewcreateproject.ui
        src/ui/viewcreateswc.cpp
        src/ui/viewcreateswc.h
        src/ui/viewcreateswc.ui
        src/ui/editorswcnode.cpp
        src/ui/editorswcnode.h
        src/ui/editorswcnode.ui
        src/ui/viewexportswctofile.cpp
        src/ui/viewexportswctofile.h
        src/ui/viewexportswctofile.ui
        src/ui/viewimportswcfromfile.cpp
        src/ui/viewimportswcfromfile.h
        src/ui/viewimportswcfromfile.ui

        app_win32.rc
        src/ui/editorswcsnapshot.cpp
        src/ui/editorswcsnapshot.h
        src/ui/editorswcsnapshot.ui
        src/ui/editorswcincrementrecord.cpp
        src/ui/editorswcincrementrecord.h
        src/ui/editorswcincrementrecord.ui
        src/ui/editorswcversioncontrol.cpp
        src/ui/editorswcversioncontrol.h
        src/ui/editorswcversioncontrol.ui
        src/ui/createswcsnapshotview.cpp
        src/ui/createswcsnapshotview.h
        src/ui/createswcsnapshotview.ui
        src/ui/editorattachmentview.cpp
        src/ui/editorattachmentview.h
        src/ui/editorattachmentview.ui
        src/ui/editoranoattachment.cpp
        src/ui/editoranoattachment.h
        src/ui/editoranoattachment.ui
        src/ui/editorapoattachment.cpp
        src/ui/editorapoattachment.h
        src/ui/editorapoattachment.ui
        src/ui/viewincrementrecord.cpp
        src/ui/viewincrementrecord.h
        src/ui/viewincrementrecord.ui
        src/FileIo/AnoIo.hpp
        src/FileIo/ApoIo.hpp
        src/util/util.hpp
        src/FileIo/FileIoInterface.hpp
        src/ui/viewswcapodata.cpp
        src/ui/viewswcapodata.h
        src/ui/viewswcapodata.ui
)

add_executable(${PROJECT_NAME} ${QRESOURCE} ${PROJECT_HEADERS} ${PROJECT_SOURCES})

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
