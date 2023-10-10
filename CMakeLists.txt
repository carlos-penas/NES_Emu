#General project settings
cmake_minimum_required(VERSION 3.16)
project(NES_Emu VERSION 1.1 DESCRIPTION "Un emulador de la NES en C++ y la libreria SFML." LANGUAGES CXX HOMEPAGE_URL "https://github.com/carlos-penas/NES_Emu")

#Copy information from input file into the configuration header file
configure_file(ProjectConfig.h.in ProjectConfig.h)

#Define compilation options 
option(RENDER_SCREEN "If set to OFF, the emulator won't render anything when executing (useful when debugging)" ON)
option(PRINT_LOG "If set to ON, the CPU will print a log with its internal state after executing every instruction (activate only when RENDER_SCREEN=OFF)" OFF)

#Add source files to the build
add_executable(${CMAKE_PROJECT_NAME} 
				main.cpp
				bus.cpp 
				cartridge.cpp
				cpu.cpp 
				cpuinstruction.cpp 
				interrupt.cpp 
				nes.cpp 
				ppu.cpp
				utils.cpp
				Exceptions/notmappedaddressexception.cpp
				Mappers/mapper.cpp
				Mappers/mapper0.cpp
				Mappers/mapper2.cpp
				)
				
#Pass both options as precompiled definitions to the source code
if(RENDER_SCREEN)
	target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE "RENDER_SCREEN")
endif()

if(PRINT_LOG)
	target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE "PRINT_LOG")
endif()

#Add both folders to the include directories
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
				${CMAKE_CURRENT_SOURCE_DIR}/Exceptions
				${CMAKE_CURRENT_SOURCE_DIR}/Mappers
				${CMAKE_CURRENT_BINARY_DIR}
				)

if(RENDER_SCREEN)
	#Find SFML library
	find_package(SFML 2 COMPONENTS graphics window system REQUIRED)

	#Add SFML to the build
	target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system)

	#Copy DLLs if using Windows
	if(WIN32)
		add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${SFML_ROOT}/bin"
				$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
		)
	endif()
endif()
