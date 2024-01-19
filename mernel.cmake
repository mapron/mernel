option( MERNEL_BUILD_SHARED "Build shared libraries for mernel" ON )
mark_as_advanced(MERNEL_BUILD_SHARED)

AddTarget(TYPE [ MERNEL_BUILD_SHARED ? shared : static ] NAME MernelPlatform
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src/MernelPlatform
    EXPORT_PARENT_INCLUDES
    LINK_LIBRARIES rapidjson zlib zstd_static)

AddTarget(TYPE static NAME MernelReflection
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src/MernelReflection
    EXPORT_PARENT_INCLUDES
    EXPORT_LINK
    LINK_LIBRARIES MernelPlatform frozen)

AddTarget(TYPE [ MERNEL_BUILD_SHARED ? shared : static ] NAME MernelExecution
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src/MernelExecution
    EXPORT_PARENT_INCLUDES)
