set(files
    cecp.c
    cecp_features.h
)

add_executable(
    pawned
    ${files}
)

target_link_libraries(pawned pawnedengine)
