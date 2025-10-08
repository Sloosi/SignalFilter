cmake_minimum_required(VERSION 3.15)
project(SignalFilter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка выходных директорий
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Добавление поддиректорий
add_subdirectory(external/glfw)

# Настройка ImGui
set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    external/imgui
    external/imgui/backends
)
target_link_libraries(imgui PUBLIC glfw)

set(IMPLOT_SOURCES
    external/implot/implot.h
    external/implot/implot.cpp
    external/implot/implot_internal.h
    external/implot/implot_items.cpp
    external/implot/implot_demo.cpp
)

add_library(implot STATIC ${IMPLOT_SOURCES})
target_link_libraries(implot PUBLIC imgui)

set(SRC_FILES
    src/main.cpp
    src/fft.h
    src/UI/SinController.h
    src/UI/SinController.cpp
)

# Создание исполняемого файла
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Настройка целевого исполняемого файла
target_include_directories(${PROJECT_NAME} PRIVATE
    external/implot
    external/imgui
    external/imgui/backends
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    implot
    imgui
    glfw
    opengl32
)

# Для Windows требуется дополнительная линковка
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        gdi32
    )
endif()