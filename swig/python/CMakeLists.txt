find_package (Python3 COMPONENTS Interpreter Development)
if (NOT Python3_FOUND)
	message(FATAL_ERROR "Python3 not found, but LeagueGameReader_Python is enabled")
endif ()

set(CMAKE_SWIG_FLAGS "")
set_property(SOURCE ../swig.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE ../swig.i PROPERTY SWIG_FLAGS "-includeall")
set_property(SOURCE ../swig.i PROPERTY USE_TARGET_INCLUDE_DIRECTORIES ON)
include_directories(../..)

# Add swig module
swig_add_library(LeagueGameReaderPy TYPE MODULE LANGUAGE python SOURCES ../swig.i)
swig_link_libraries(LeagueGameReaderPy ${PROJECT_NAME} ${Python3_LIBRARIES})
target_include_directories(LeagueGameReaderPy PRIVATE ${Python3_INCLUDE_DIRS})
target_compile_definitions(LeagueGameReaderPy PUBLIC SWIG_PYTHON_INTERPRETER_NO_DEBUG)
