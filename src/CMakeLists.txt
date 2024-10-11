include_directories(${crack-tracer_SOURCE_DIR}/inc)
# This is marked as SYSTEM to suppress compiler warnings.
include_directories(SYSTEM ${crack-tracer_SOURCE_DIR}/third_party/stb_image_write)

add_executable(
	${PROJECT_NAME}

	entry.cpp
	camera.cpp
)
