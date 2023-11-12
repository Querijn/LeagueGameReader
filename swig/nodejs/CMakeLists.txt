set(CMAKE_SWIG_FLAGS "-node") # This sucks, but I can't put it in the property below.
set_property(SOURCE ../swig.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE ../swig.i PROPERTY SWIG_FLAGS "-includeall")
set_property(SOURCE ../swig.i PROPERTY USE_TARGET_INCLUDE_DIRECTORIES ON)
include_directories(../..)

# Add swig module
swig_add_library(LeagueGameReaderNodeJS TYPE SHARED LANGUAGE javascript SOURCES ../swig.i)
swig_link_libraries(LeagueGameReaderNodeJS ${PROJECT_NAME} ${NodeJS_LIBRARIES})
target_include_directories(LeagueGameReaderNodeJS PRIVATE ${NodeJS_INCLUDE_DIRS})
target_link_libraries(LeagueGameReaderNodeJS node)

set_target_properties(LeagueGameReaderNodeJS PROPERTIES
					CXX_STANDARD 20
					CXX_EXTENSIONS OFF)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})