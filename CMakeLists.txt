cmake_minimum_required(VERSION 3.25)

project(xentara-simple-sample-driver
	VERSION 1.0.0
	DESCRIPTION "A simple sample driver for Xentara"
	HOMEPAGE_URL "https://www.xentara.io"
	LANGUAGES CXX)

# Force the use of C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Tell MSVC to set __cplusplus to the correct value
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options("/Zc:__cplusplus")
endif()

find_package(XentaraUtils REQUIRED)
find_package(XentaraPlugin REQUIRED)

add_library(
	${PROJECT_NAME} MODULE

	"src/Attributes.cpp"
	"src/Attributes.hpp"
	"src/CustomError.cpp"
	"src/CustomError.hpp"
	"src/Device.cpp"
	"src/Device.hpp"
	"src/Events.cpp"
	"src/Events.hpp"
	"src/Input.cpp"
	"src/Input.hpp"
	"src/Output.cpp"
	"src/Output.hpp"
	"src/Plugin.cpp"
	"src/Plugin.hpp"
	"src/Skill.cpp"
	"src/Skill.hpp"
	"src/Tasks.cpp"
	"src/Tasks.hpp"
)

target_link_libraries(
	${PROJECT_NAME}

	PRIVATE
		Xentara::xentara-utils
		Xentara::xentara-plugin
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set_target_properties(
		${PROJECT_NAME}
		
		PROPERTIES
			OUTPUT_NAME XentaraSimpleSampleDriver
			DEBUG_POSTFIX d
	)
endif()

install_xentara_plugin(${PROJECT_NAME})