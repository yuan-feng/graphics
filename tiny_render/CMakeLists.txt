cmake_minimum_required (VERSION 3.1)
project(tiny_renderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(FILES
  model.cpp
  tga_image.cpp
)

add_library(render
  ${FILES}
)

add_executable(main_1_line main_1_line.cpp)
target_link_libraries(main_1_line render)

add_executable(main_2_triangle main_2_triangle.cpp)
target_link_libraries(main_2_triangle render)
