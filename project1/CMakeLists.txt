# Project 1
file(GLOB SOURCE_FILES "src/*.cpp")
file(GLOB HEADER_FILES "include/*.h")
file(GLOB TESTS "tests/*.cpp")
source_group("src" FILES ${SOURCE_FILES}) # Group files into logical folder used by a project
source_group("include" FILES ${HEADER_FILES})
source_group("tests" FILES ${TESTS})

add_executable(project1 ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(project1 PRIVATE include)
target_compile_features(project1 PRIVATE cxx_std_20) # gcc version in ics environment is 11.3.0

# Project tests
add_executable(project1Tests ${TESTS})
target_include_directories(project1Tests PRIVATE include)
target_compile_features(project1Tests PRIVATE cxx_std_17) # at least 17 for structural binding
find_package(Catch2 CONFIG REQUIRED) # Dependency
target_link_libraries(project1Tests PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
enable_testing()
add_test(tests projectlTests)
