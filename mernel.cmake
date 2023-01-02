AddTarget(TYPE shared NAME MernelPlatform
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src/MernelPlatform
    EXPORT_PARENT_INCLUDES
    LINK_LIBRARIES rapidjson zlib)

AddTarget(TYPE static NAME MernelReflection
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src/MernelReflection
    EXPORT_PARENT_INCLUDES
    EXPORT_LINK
    LINK_LIBRARIES MernelPlatform frozen)
