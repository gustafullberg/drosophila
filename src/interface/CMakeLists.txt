set(TARGET_NAME pawned)
if(TARGET_SUFFIX)
set(TARGET_NAME ${TARGET_NAME}-${TARGET_SUFFIX})
endif()

set(files
    cecp.c
    cecp_features.h
    io.c
    io.h
)

add_executable(
    ${TARGET_NAME}
    ${files}
)

target_link_libraries(${TARGET_NAME} pawnedengine)
install(TARGETS ${TARGET_NAME} DESTINATION .)
