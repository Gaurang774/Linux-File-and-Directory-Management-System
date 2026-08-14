# ── Test executables ──────────────────────────────────────────────────────────

add_executable(test_path_utils test_path_utils.cpp)
target_link_libraries(test_path_utils PRIVATE filemgr_lib)
add_test(NAME PathUtilsTests COMMAND test_path_utils)

add_executable(test_string_utils test_string_utils.cpp)
target_link_libraries(test_string_utils PRIVATE filemgr_lib)
add_test(NAME StringUtilsTests COMMAND test_string_utils)

add_executable(test_file_service test_file_service.cpp)
target_link_libraries(test_file_service PRIVATE filemgr_lib)
add_test(NAME FileServiceTests COMMAND test_file_service)

add_executable(test_search_service test_search_service.cpp)
target_link_libraries(test_search_service PRIVATE filemgr_lib)
add_test(NAME SearchServiceTests COMMAND test_search_service)
