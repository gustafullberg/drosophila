set(files
    cecp.c
    cecp_features.h
    io.c
    io.h
)

add_executable(
    pawned
    ${files}
)

target_link_libraries(pawned pawnedengine)
install(TARGETS pawned DESTINATION .)
