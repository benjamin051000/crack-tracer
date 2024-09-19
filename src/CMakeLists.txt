include_directories(${crack-tracer_SOURCE_DIR}/inc)

add_executable(
	${PROJECT_NAME}

	camera.cpp      
	comptime.cpp 
	materials.cpp 
	rand.cpp 
	sphere.cpp
	main.cpp 
	math.cpp 
	render.cpp 
	utils.cpp
)
