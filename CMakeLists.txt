cmake_minimum_required(VERSION 3.22)

project(xentara-simple-sample-driver
	VERSION 1.0.0
	DESCRIPTION "A simple sample driver for Xentara"
	HOMEPAGE_URL "https://www.xentara.io"
	LANGUAGES CXX)

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
	"src/Driver.cpp"
	"src/Driver.hpp"
	"src/Output.cpp"
	"src/Output.hpp"
	"src/Plugin.cpp"
	"src/Plugin.hpp"
	"src/Input.cpp"
	"src/Input.hpp"
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