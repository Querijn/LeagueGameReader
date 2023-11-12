cmake_policy(SET CMP0078 NEW) # Swig generates standard target names

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

if(LeagueGameReader_Python)
    add_subdirectory(python)
endif()