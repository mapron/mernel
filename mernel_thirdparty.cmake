
# Zlib

if (MSVC)
    AddTarget(TYPE static NAME zlib 
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty/zlib
        EXPORT_INCLUDES
        SKIP_STATIC_CHECK
        COMPILE_OPTIONS
            /wd4996 
            /wd4131 # 'xxxx': uses old-style declarator
        INTERFACE_COMPILE_DEFINITIONS USE_ZLIB
    )
else()
    AddTarget(TYPE interface NAME zlib 
        SOURCE_DIR .
        LINK_LIBRARIES -lz
        INTERFACE_COMPILE_DEFINITIONS USE_ZLIB
    )
endif()

# Json
AddTarget(TYPE interface NAME rapidjson 
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty)

# frozen 
AddTarget(TYPE interface NAME frozen 
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty/frozen/include)


# Google Test
set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
set(GOOGLETEST_VERSION 1.10.0)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rdparty/googletest/googletest)

if(WIN32)
# 7-zip
AddTarget(TYPE static NAME 7zip_static
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty/7zip
    EXPORT_INCLUDES
    SKIP_STATIC_CHECK
    STATIC_RUNTIME
)
endif()

# Zstd
AddTarget(TYPE static NAME zstd_static 
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty/zstd/lib
    EXPORT_INCLUDES
    SKIP_STATIC_CHECK
    EXCLUDE_SOURCES legacy dictBuilder deprecated
    INCLUDES ${CMAKE_CURRENT_LIST_DIR}/3rdparty/zstd/lib/common/
    COMPILE_DEFINITIONS ZSTD_DISABLE_ASM
    INTERFACE_COMPILE_DEFINITIONS USE_ZSTD
    )

