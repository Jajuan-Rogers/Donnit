cmake_minimum_required(VERSION 3.20)

project(Donnit VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.cpp src/Window.cpp) 

# 6. Point to your header files
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include
  /usr/include 
  /usr/include/notcurses
  /user/include/fmt
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 3. Link the Notcurses libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    notcurses++ 
    notcurses 
    notcurses-core
    fmt
)
