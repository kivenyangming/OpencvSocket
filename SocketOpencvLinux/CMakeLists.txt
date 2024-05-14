cmake_minimum_required(VERSION 3.10) # 或者是你的CMake版本

# 项目名称
project(CameraStreamServer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找OpenCV库
find_package(OpenCV REQUIRED)

# 如果OpenCV被成功找到，包含其必要的目录和库
if(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
    add_executable(CameraStreamServer main.cpp)
    target_link_libraries(CameraStreamServer ${OpenCV_LIBS})
else()
    message(FATAL_ERROR "OpenCV not found. Please make sure OpenCV is installed and its path is set properly.")
endif()

# 设置运行时库搜索路径（如果OpenCV是动态链接的话）
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
    if(APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework CoreFoundation -framework CoreVideo -framework AVFoundation")
    endif()
endif()