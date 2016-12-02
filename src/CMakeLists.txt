SET(pwd ${CMAKE_CURRENT_LIST_DIR})

SET(src "")
AUX_SOURCE_DIRECTORY(${pwd} src)
add_executable(cubehands ${src})
message(STATUS "cubehands added ${src}")

target_link_libraries(cubehands ${stdgl_libraries})
FIND_PACKAGE(JPEG REQUIRED)
TARGET_LINK_LIBRARIES(cubehands ${JPEG_LIBRARIES})
