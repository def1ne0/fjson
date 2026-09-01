project(fjson_tests LANGUAGES CXX)

add_executable(${PROJECT_NAME} test_main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE fjson::fjson)
add_test(NAME fjson.basic_tests COMMAND fjson_tests)