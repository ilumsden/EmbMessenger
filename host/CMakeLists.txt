cmake_minimum_required(VERSION 3.1)
project(EmbMessengerHost LANGUAGES CXX)

option(EMB_SINGLE_THREADED "Run Emb Messenger in a single thread" OFF)

file(GLOB_RECURSE ${PROJECT_NAME}_HEADERS "include/*.hpp")
file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES "src/*.cpp")

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} EmbMessengerShared)

if(EMB_SINGLE_THREADED OR EmbMessenger_ENABLE_TESTING)
	target_compile_options(${PROJECT_NAME} PUBLIC -DEMB_SINGLE_THREADED)
else()
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)

	target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif()

if(EmbMessenger_ENABLE_TESTING)
	target_compile_options(${PROJECT_NAME} PRIVATE -g -O0 --coverage -DEMB_TESTING)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "--coverage")

	file(GLOB_RECURSE ${PROJECT_NAME}_TEST_SOURCES "test/*.[ch]pp")
	add_executable(${PROJECT_NAME}Test ${${PROJECT_NAME}_TEST_SOURCES})
	set_target_properties(${PROJECT_NAME}Test PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES)
	target_link_libraries(${PROJECT_NAME}Test ${PROJECT_NAME} gtest gmock gtest_main)
    target_compile_options(${PROJECT_NAME}Test PRIVATE -g -O0 --coverage -DEMB_TESTING)
    set_target_properties(${PROJECT_NAME}Test PROPERTIES LINK_FLAGS "--coverage")
endif()
