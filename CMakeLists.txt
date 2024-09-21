cmake_minimum_required(VERSION 3.22)

project(crack-tracer)

set(CMAKE_CXX_FLAGS
	"--std=c++23 -Ofast -Wall -Wextra -Wunused -Wshadow=compatible-local -Wpedantic -Wconversion -g -march=native -flto -fno-signed-zeros" 
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(src)

# if(NOT CMAKE_BUILD_TYPE)
#   set(CMAKE_BUILD_TYPE Release)
# endif()

# TODO can this use earlier than 20?
# target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:-g;-Wall;-Wextra>;-Wno-missing-field-initializers;-march=native")

# target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-Ofast;-g;-fno-signed-zeros;-flto;-Wall;-Wextra>;-Wno-missing-field-initializers;-march=native")

# Third-party libs
find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
