project(
	A
	LANGUAGES CXX
	)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(--static -O3 -Wall)
add_executable(a a.cpp qio.cpp)
