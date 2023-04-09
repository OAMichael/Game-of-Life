cmake_minimum_required(VERSION 3.10)

project(OpenGL VERSION 1.0)
cmake_policy(SET CMP0076 NEW)

set(CMAKE_CXX_COMPILER mpicxx)
set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR ./src)
set(EXTERNAL_DIR ./external)
set(COMPILE_OPT -std=c++17 -O2)
set(LINK_OPT -lglfw -lGL -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)

add_executable(main.exe ${SRC_DIR}/main.cpp ${EXTERNAL_DIR}/glad.c)

target_include_directories(main.exe PUBLIC ${SRC_DIR})
target_include_directories(main.exe PUBLIC ${EXTERNAL_DIR})

add_library(MY_LIB 
	${SRC_DIR}/IApplication.cpp
	${SRC_DIR}/Shader.cpp
	${SRC_DIR}/LifeGame.cpp
	)

target_link_libraries(main.exe PUBLIC MY_LIB)

find_library(IRRKLANG_LIB NAMES IrrKlang PATHS ${EXTERNAL_DIR}/irrKlang/lib/)
target_link_libraries(main.exe PUBLIC ${IRRKLANG_LIB})

add_compile_options(main.exe ${COMPILE_OPT})
target_link_libraries(main.exe PUBLIC ${LINK_OPT})

file(COPY ${EXTERNAL_DIR}/irrKlang/lib/ikpMP3.so
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})