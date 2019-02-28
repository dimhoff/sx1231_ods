include_directories(${PROJECT_SOURCE_DIR}/libsx1231_ods)
link_directories(${PROJECT_BUILD_DIR}/libsx1231_ods)

add_executable(sx1231_raw sx1231_raw.c dehexify.c)
add_dependencies(sx1231_raw git_version)
target_link_libraries(sx1231_raw sx1231_ods)

add_executable(sx1231_kaku sx1231_kaku.c)
add_dependencies(sx1231_kaku git_version)
target_link_libraries(sx1231_kaku sx1231_ods)

add_executable(sx1231_somfy sx1231_somfy.c sx1231_rts.c)
#add_dependencies(sx1231_somfy git_version)
target_link_libraries(sx1231_somfy sx1231_ods)
