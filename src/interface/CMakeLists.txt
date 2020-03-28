if(TARGET_SUFFIX)
set(PROGRAM_NAME ${PROGRAM_NAME}-${TARGET_SUFFIX})
endif()

set(files
    uci.c
)

add_executable(
    ${PROGRAM_NAME}
    ${files}
)

target_link_libraries(${PROGRAM_NAME} ${LIB_NAME})
install(TARGETS ${PROGRAM_NAME} DESTINATION .)
