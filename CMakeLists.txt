cmake_minimum_required(VERSION 3.15)
project(LeagueGameReader LANGUAGES CXX)

option(LeagueGameReader_RemoteSettings	"Build LeagueGameReader with remote settings support"	ON)
option(LeagueGameReader_Python			"Build LeagueGameReader bindings for Python"			ON)
option(LeagueGameReader_NodeJS			"Build LeagueGameReader bindings for NodeJS"			OFF)
option(LeagueGameReader_Example			"Build LeagueGameReader example for C++"				ON)

# Find python
find_package (Python3 COMPONENTS Interpreter Development)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
function(ADD_SRC var folder inc src)
	source_group(${folder} FILES ${inc} ${src})
	set(${var} ${${var}} ${inc} ${src} CACHE INTERNAL "")
endfunction(ADD_SRC)

set(LGR_SRC "" CACHE INTERNAL "LeagueGameReader Source Files")

# Public general stuff
ADD_SRC(LGR_SRC	"Library"		"inc/league_gamereader.hpp"							"")
ADD_SRC(LGR_SRC	"Library"		"inc/league_gamereader/types.hpp"					"")
ADD_SRC(LGR_SRC	"Library"		"inc/league_gamereader/library.hpp"					"src/library.cpp")

# Public types
ADD_SRC(LGR_SRC	"Library"		"inc/league_gamereader/game_object.hpp"				"")
ADD_SRC(LGR_SRC	"Library"		"inc/league_gamereader/game_object_manager.hpp"		"")
ADD_SRC(LGR_SRC	"Library"		"inc/league_gamereader/general_items.hpp"			"")

# Private stuff
ADD_SRC(LGR_SRC	"Reader"		"src/game_reader.hpp"								"src/game_reader.cpp")
ADD_SRC(LGR_SRC	"Process"		"src/process.hpp"									"src/process.cpp")
ADD_SRC(LGR_SRC	"Settings"		"src/settings.hpp"									"src/settings.cpp")
ADD_SRC(LGR_SRC	"\\"			""													"swig/swig.i")

add_library(LeagueGameReader ${LGR_SRC})
target_include_directories(LeagueGameReader PUBLIC	inc)
target_include_directories(LeagueGameReader PRIVATE	src)

if (LeagueGameReader_RemoteSettings)
	target_compile_definitions(LeagueGameReader PUBLIC LGR_REMOTE_SETTINGS)
endif ()

if (WIN32)
	target_compile_definitions(LeagueGameReader PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(LeagueGameReader PUBLIC NOMINMAX)
endif ()

if (LeagueGameReader_NodeJS OR LeagueGameReader_Python)
	add_subdirectory(swig)
endif ()

# Example source code
if (LeagueGameReader_Example)
	set(LGR_EXAMPLE_SRC "" CACHE INTERNAL "LeagueGameReader Source Files")
	ADD_SRC(LGR_EXAMPLE_SRC "\\" "" "src/example.cpp")

	set_target_properties(LeagueGameReader PROPERTIES
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS OFF)
					
	add_executable(LeagueGameReaderExample ${LGR_EXAMPLE_SRC})
	target_include_directories(LeagueGameReaderExample PUBLIC inc)
	target_link_libraries(LeagueGameReaderExample LeagueGameReader)

	if (WIN32)
		target_compile_definitions(LeagueGameReaderExample PUBLIC WIN32_LEAN_AND_MEAN)
		target_compile_definitions(LeagueGameReaderExample PUBLIC NOMINMAX)
	endif ()

	set_target_properties(LeagueGameReaderExample PROPERTIES
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS OFF
						)
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT LeagueGameReaderExample)
endif ()