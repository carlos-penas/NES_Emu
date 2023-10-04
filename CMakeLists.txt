#General project settings
cmake_minimum_required(VERSION 3.16)
project(NES_Emu VERSION 1.0.0 DESCRIPTION "Emulador de la NES en C++" LANGUAGES CXX)

#Add source files to the build
add_executable(NES_Emu 
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
				
#Add both folders to the include directories
target_include_directories(NES_Emu PRIVATE 
				${CMAKE_CURRENT_SOURCE_DIR}/Exceptions
				${CMAKE_CURRENT_SOURCE_DIR}/Mappers
				)

#Find SFML library		
find_package(SFML 2 COMPONENTS graphics window system REQUIRED)

#Add SFML to the build
target_link_libraries(NES_Emu PRIVATE sfml-graphics sfml-window sfml-system)

#Copy DLLs if using Windows
if(WIN32 AND BUILD_SHARED_LIBS)


	add_custom_command(TARGET NES_Emu POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${SFML_ROOT}/bin"
		$<TARGET_FILE_DIR:NES_Emu>
)
endif()
