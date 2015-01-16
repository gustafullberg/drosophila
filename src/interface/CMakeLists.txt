if(TARGET_SUFFIX)
set(PROGRAM_NAME ${PROGRAM_NAME}-${TARGET_SUFFIX})
endif()

set(files
    cecp.c
    cecp_features.h
    io.c
    io.h
)

add_executable(
    ${PROGRAM_NAME}
    ${files}
)

target_link_libraries(${PROGRAM_NAME} ${LIB_NAME})
install(TARGETS ${PROGRAM_NAME} DESTINATION .)
