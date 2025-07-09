cmake_minimum_required(VERSION 3.28)
project(SwcManagerClient VERSION 1.1.2)

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

# 查找Git包
find_package(Git QUIET)
# 设置默认版本信息（如果无法获取Git信息）
set(GIT_COMMIT_HASH "unknown")
set(GIT_BRANCH "unknown")
set(GIT_COMMIT_DATE "unknown")
# 如果找到Git，尝试获取提交信息
if(GIT_FOUND)
    # 获取最新的commit hash
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # 获取当前分支
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # 获取提交日期
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%cd --date=short
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_DATE
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()
# 配置头文件以传递版本信息
configure_file(
    ${CMAKE_SOURCE_DIR}/version.h.in
    ${CMAKE_BINARY_DIR}/Generated/version.h
    @ONLY
)
# 将生成的头文件添加到包含路径中
include_directories(${CMAKE_BINARY_DIR}/Generated)

include_directories(
        src/styles
        ThirdParty/nlohmann_json
        ThirdParty/nodeeditor/src
        ThirdParty/nodeeditor/include
        ThirdParty/nodeeditor/include/QtNodes
        ThirdParty/nodeeditor/include/QtNodes/internal
        src/framework/util/unsortswc
)

link_directories(
        .

)

# 自动查找 nodeeditor 的源文件和头文件
file(GLOB_RECURSE NODEEDITOR_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/ThirdParty/nodeeditor/src/*.cpp"
)

file(GLOB_RECURSE NODEEDITOR_HEADER_FILES
        "${PROJECT_SOURCE_DIR}/ThirdParty/nodeeditor/src/*.hpp"
        "${PROJECT_SOURCE_DIR}/ThirdParty/nodeeditor/include/*.hpp"
)

set(QRESOURCE
        ThirdParty/nodeeditor/resources/nodeeditorresources.qrc
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
set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/Generated)
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

# 添加自定义目标来更新版本信息
add_custom_target(version_info ALL
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_SOURCE_DIR}/version.h.in
    COMMENT "Updating version information"
)
# 确保在构建可执行文件前更新版本信息
add_dependencies(${PROJECT_NAME} version_info)

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
