cmake_minimum_required(VERSION 3.28)
project(SwcManagerClient)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Svg
        OpenGL
        OpenGLWidgets
        REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(asio-grpc CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)

include_directories(
        SwcDbmsCommon/Generated/cpp
        src/styles
        ThirdParty/nlohmann_json
        ThirdParty/nodeeditor-3.0.10/src
        ThirdParty/nodeeditor-3.0.10/include
        ThirdParty/nodeeditor-3.0.10/include/QtNodes
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal
        src/framework/util/unsortswc
)

link_directories(
        .

)

set(NODEEDITOR_SOURCE_FILES
        ThirdParty/nodeeditor-3.0.10/src/AbstractGraphModel.cpp
        ThirdParty/nodeeditor-3.0.10/src/AbstractNodeGeometry.cpp
        ThirdParty/nodeeditor-3.0.10/src/BasicGraphicsScene.cpp
        ThirdParty/nodeeditor-3.0.10/src/ConnectionGraphicsObject.cpp
        ThirdParty/nodeeditor-3.0.10/src/ConnectionPainter.cpp
        ThirdParty/nodeeditor-3.0.10/src/ConnectionState.cpp
        ThirdParty/nodeeditor-3.0.10/src/ConnectionStyle.cpp
        ThirdParty/nodeeditor-3.0.10/src/DataFlowGraphModel.cpp
        ThirdParty/nodeeditor-3.0.10/src/DataFlowGraphicsScene.cpp
        ThirdParty/nodeeditor-3.0.10/src/DefaultHorizontalNodeGeometry.cpp
        ThirdParty/nodeeditor-3.0.10/src/DefaultVerticalNodeGeometry.cpp
        ThirdParty/nodeeditor-3.0.10/src/Definitions.cpp
        ThirdParty/nodeeditor-3.0.10/src/GraphicsView.cpp
        ThirdParty/nodeeditor-3.0.10/src/GraphicsViewStyle.cpp
        ThirdParty/nodeeditor-3.0.10/src/NodeDelegateModelRegistry.cpp
        ThirdParty/nodeeditor-3.0.10/src/NodeConnectionInteraction.cpp
        ThirdParty/nodeeditor-3.0.10/src/NodeDelegateModel.cpp
        ThirdParty/nodeeditor-3.0.10/src/NodeGraphicsObject.cpp
        ThirdParty/nodeeditor-3.0.10/src/DefaultNodePainter.cpp
        ThirdParty/nodeeditor-3.0.10/src/NodeState.cpp
        ThirdParty/nodeeditor-3.0.10/src/NodeStyle.cpp
        ThirdParty/nodeeditor-3.0.10/src/StyleCollection.cpp
        ThirdParty/nodeeditor-3.0.10/src/UndoCommands.cpp
        ThirdParty/nodeeditor-3.0.10/src/locateNode.cpp
)

set(NODEEDITOR_HEADER_FILES
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/AbstractGraphModel.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/AbstractNodeGeometry.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/AbstractNodePainter.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/BasicGraphicsScene.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/Compiler.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/ConnectionGraphicsObject.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/ConnectionIdHash.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/ConnectionIdUtils.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/ConnectionState.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/ConnectionStyle.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/DataFlowGraphicsScene.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/DataFlowGraphModel.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/DefaultNodePainter.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/Definitions.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/Export.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/GraphicsView.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/GraphicsViewStyle.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/locateNode.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/NodeData.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/NodeDelegateModel.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/NodeDelegateModelRegistry.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/NodeGraphicsObject.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/NodeState.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/NodeStyle.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/OperatingSystem.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/QStringStdHash.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/QUuidStdHash.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/Serializable.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/Style.hpp
        ThirdParty/nodeeditor-3.0.10/include/QtNodes/internal/StyleCollection.hpp
        ThirdParty/nodeeditor-3.0.10/src/ConnectionPainter.hpp
        ThirdParty/nodeeditor-3.0.10/src/DefaultHorizontalNodeGeometry.hpp
        ThirdParty/nodeeditor-3.0.10/src/DefaultVerticalNodeGeometry.hpp
        ThirdParty/nodeeditor-3.0.10/src/NodeConnectionInteraction.hpp
        ThirdParty/nodeeditor-3.0.10/src/UndoCommands.hpp
)

set(QRESOURCE
        ThirdParty/nodeeditor-3.0.10/resources/nodeeditorresources.qrc
        Resource/swcdbmsclientresources.qrc
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

add_executable(${PROJECT_NAME} ${NODEEDITOR_SOURCE_FILES} ${NODEEDITOR_HEADER_FILES} ${QRESOURCE} ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${SRC_LIST})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt::Core
            Qt::Gui
            Qt::Widgets
            Qt::Svg
            Qt::OpenGL
            Qt::OpenGLWidgets
            opengl32
            PRIVATE spdlog::spdlog
            PRIVATE asio-grpc::asio-grpc-standalone-asio asio::asio
    )
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt::Core
            Qt::Gui
            Qt::Widgets
            Qt::Svg
            Qt::OpenGL
            Qt::OpenGLWidgets
            PRIVATE spdlog::spdlog
            PRIVATE asio-grpc::asio-grpc-standalone-asio asio::asio
    )
endif ()

target_compile_definitions(${PROJECT_NAME}
        PRIVATE
        NODE_EDITOR_STATIC
        NODE_EDITOR_EXPORTS
        NOMINMAX
)

if (WIN32)
    Set_Target_Properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
            LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
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
