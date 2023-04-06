# Project 2
file(GLOB SOURCE_FILES "src/*.cpp")
file(GLOB HEADER_FILES "include/*.h")
source_group("src" FILES ${SOURCE_FILES})
source_group("include" FILES ${HEADER_FILES})
add_executable(project2 ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(project2 PRIVATE include)
target_compile_features(project2 PRIVATE cxx_std_20) # gcc version in ics environment is 11.3.0
