find_package (Python3 COMPONENTS Interpreter Development)
if (NOT Python3_FOUND)
	message(FATAL_ERROR "Python3 not found, but LeagueGameReader_Python is enabled")
endif ()

target_include_directories(LeagueGameReader PRIVATE ${Python3_INCLUDE_DIRS})
target_compile_definitions(LeagueGameReader PUBLIC  LGR_PYTHON)

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

# Files to install with Python
set(PYTHON_INSTALL_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/LeagueGameReaderPy.py
        ${CMAKE_CURRENT_BINARY_DIR}/_LeagueGameReaderPy.so)

# Configure setup.py and copy to output directory
set(LGR_SETUP_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in)
set(LGR_SETUP_PY_OUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
configure_file(${LGR_SETUP_PY_IN} ${LGR_SETUP_PY_OUT})

# Declare install target for python
install(FILES ${PYTHON_INSTALL_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/python)