cmake_minimum_required(VERSION 3.16)
project(DataUploadTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt配置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt6或Qt5
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network Widgets REQUIRED)

# 源文件
set(SOURCES
    main.cpp
    MainWindow.cpp
)

set(HEADERS
    MainWindow.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets
)

# Windows特定设置
if(WIN32)
    # 暂时启用控制台以便查看调试信息
    # 如果需要发布版本，可以改为 WIN32_EXECUTABLE TRUE
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# 安装规则（可选）
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

