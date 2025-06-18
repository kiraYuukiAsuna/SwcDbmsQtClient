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
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

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

# 自动查找所有 proto 文件
file(GLOB_RECURSE PROTO_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/SwcDbmsCommon/proto/*.proto
)
# 设置生成文件的输出目录
set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
# 获取 proto 文件的根目录路径
get_filename_component(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/SwcDbmsCommon/proto" ABSOLUTE)
# 用于收集生成的所有源文件和头文件
set(GENERATED_SRCS "")
set(GENERATED_HDRS "")

# output PROTO_FILES size
list(LENGTH PROTO_FILES PROTO_FILES_COUNT)
message(STATUS "Found ${PROTO_FILES_COUNT} [proto] files in SwcDbmsCommon/proto/")

# 对每个 proto 文件自动生成 C++ 代码
foreach (PROTO_FILE ${PROTO_FILES})
    message(STATUS "Processing proto file: ${PROTO_FILE}")
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    # 获取相对于proto根目录的路径结构
    file(RELATIVE_PATH PROTO_REL_PATH ${PROTO_PATH} ${PROTO_FILE})
    get_filename_component(PROTO_REL_DIR ${PROTO_REL_PATH} DIRECTORY)

    # 创建目标目录，保持原始结构
    set(OUTPUT_SUBDIR "${PROTO_SRC_DIR}/${PROTO_REL_DIR}")
    file(MAKE_DIRECTORY ${OUTPUT_SUBDIR})

    # 设置生成文件路径，包含子目录结构
    set(PROTO_SRC "${PROTO_SRC_DIR}/${PROTO_REL_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${PROTO_SRC_DIR}/${PROTO_REL_DIR}/${PROTO_NAME}.pb.h")

    # 生成 gRPC 服务文件
    set(GRPC_SRC "${PROTO_SRC_DIR}/${PROTO_REL_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDR "${PROTO_SRC_DIR}/${PROTO_REL_DIR}/${PROTO_NAME}.grpc.pb.h")

    # 收集生成的文件
    list(APPEND GENERATED_SRCS ${PROTO_SRC} ${GRPC_SRC})
    list(APPEND GENERATED_HDRS ${PROTO_HDR} ${GRPC_HDR})

    # 执行代码生成命令
    add_custom_command(
            OUTPUT "${PROTO_SRC}" "${PROTO_HDR}" "${GRPC_SRC}" "${GRPC_HDR}"
            COMMAND $<TARGET_FILE:protobuf::protoc>
            ARGS --grpc_out=${PROTO_SRC_DIR}
            --cpp_out=${PROTO_SRC_DIR}
            --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
            -I${PROTO_PATH}
            ${PROTO_FILE}
            DEPENDS ${PROTO_FILE}
            COMMENT "Generating gRPC and Protobuf code from ${PROTO_FILE}"
            VERBATIM
    )
endforeach ()

# 添加自定义生成目标
add_custom_target(proto_gen DEPENDS ${GENERATED_SRCS} ${GENERATED_HDRS})

add_executable(${PROJECT_NAME}
        main.cpp
        app_win32.rc
        ${NODEEDITOR_SOURCE_FILES} ${NODEEDITOR_HEADER_FILES} ${QRESOURCE}
        ${SRC_LIST}
        ${GENERATED_SRCS}  # 添加生成的源文件
)

# 使主目标依赖于生成的代码
add_dependencies(${PROJECT_NAME} proto_gen)

# 包含生成的目录
target_include_directories(${PROJECT_NAME} PRIVATE ${PROTO_SRC_DIR})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt::Core
            Qt::Gui
            Qt::Widgets
            Qt::Svg
            Qt::OpenGL
            Qt::OpenGLWidgets
            opengl32
            spdlog::spdlog
            PUBLIC
            asio-grpc::asio-grpc-standalone-asio asio::asio
    )
else ()
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt::Core
            Qt::Gui
            Qt::Widgets
            Qt::Svg
            Qt::OpenGL
            Qt::OpenGLWidgets
            spdlog::spdlog
            PUBLIC
            asio-grpc::asio-grpc-standalone-asio asio::asio
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
