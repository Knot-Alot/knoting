file(GLOB_RECURSE KNOTING_SHADERS LIST_DIRECTORIES false "${CMAKE_SOURCE_DIR}/res/shaders/**/*.sc")

file(GLOB_RECURSE KNOTING_SOURCES
        src/*.cpp
        inc/**/*.h
        )

set(PUBLIC_LIBS
        cereal
        glm
        spdlog
        imgui
        imguizmo
        EnTT::EnTT
        stduuid
        ${PHYSX_LIBRARIES}
        bgfx
        bx
        bimg
        rapidjson
        yojimbo
        NoesisGUI
        FMod
        glfw
        quickjs
        )

set(PRIVATE_LIBS
        stb
        )


find_package(NoesisGUI REQUIRED)
find_package(FMod REQUIRED)

add_library(knoting ${KNOTING_SOURCES} ${KNOTING_SHADERS})

set_target_properties(knoting PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

set_target_properties(knoting PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist/lib)
set_target_properties(knoting PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist/lib)
set_target_properties(knoting PROPERTIES OUTPUT_NAME knoting)

target_compile_definitions(knoting PUBLIC ENTT_ID_TYPE=uint64_t)
target_compile_definitions(knoting PUBLIC PX_PHYSX_STATIC_LIB=TRUE)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(knoting PUBLIC KNOTING_DEBUG=1)
    target_compile_definitions(knoting PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(knoting PUBLIC ENTT_DISABLE_ASSERT=1)
    target_compile_definitions(knoting PUBLIC NDEBUG=1)
endif ()

if (WIN32)
    target_compile_definitions(knoting PUBLIC BX_PLATFORM_WINDOWS=1)
elseif (UNIX)
    target_compile_definitions(knoting PUBLIC BX_PLATFORM_LINUX=1)
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_definitions(knoting PUBLIC KNOTING_64=1)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_compile_definitions(knoting PUBLIC KNOTING_32=1)
endif ()

foreach (LIB ${PUBLIC_LIBS})
    target_link_libraries(knoting PUBLIC ${LIB})
endforeach ()

foreach (LIB ${PRIVATE_LIBS})
    target_link_libraries(knoting PRIVATE ${LIB})
endforeach ()

target_include_directories(knoting PUBLIC ${CMAKE_SOURCE_DIR}/core/inc)
target_include_directories(knoting PUBLIC ${PHYSX_INCLUDE_DIRS})
target_include_directories(knoting PUBLIC ${CMAKE_SOURCE_DIR}/third/yojimbo)

target_include_directories(knoting PUBLIC ${CMAKE_SOURCE_DIR}/third/imguizmo)

if (GLFW_FOUND)
    target_include_directories(knoting PUBLIC ${GLFW_INCLUDE_DIR})
endif ()

# BGFX

foreach (SHADER_FILE ${KNOTING_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    get_filename_component(PARENT_DIR ${SHADER_FILE} DIRECTORY)
    string(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/res/shaders" "" PARENT_DIR ${PARENT_DIR})

    set(SHADER_TYPE "")
    if ("${SHADER_NAME}" MATCHES "vs_*")
        message(STATUS "Found vertex shader ${SHADER_NAME}")
        set(SHADER_TYPE VERTEX)
    elseif ("${SHADER_NAME}" MATCHES "fs_*")
        message(STATUS "Found fragment shader ${SHADER_NAME}")
        set(SHADER_TYPE FRAGMENT)
    elseif ("${SHADER_NAME}" MATCHES "cs_*")
        message(STATUS "Found compute shader ${SHADER_NAME}")
        set(SHADER_TYPE COMPUTE)
    else ()
        message(STATUS "Found & Skipped unknown shader ${SHADER_NAME}")
        continue()
    endif ()

    add_shader(
            ${SHADER_FILE} ${SHADER_TYPE}
            OUTPUT ${CMAKE_BINARY_DIR}/dist/res/shaders${PARENT_DIR}
            DX11_MODEL 5_0
            GLSL 430
            INCLUDES "${CMAKE_SOURCE_DIR}/res/shaders/"
    )
endforeach ()

# Copy DLLs into dist/bin

if (WIN32)
    set(KNOTING_LIB_DIR "${CMAKE_SOURCE_DIR}/lib/x86_64/windows")
    file(GLOB_RECURSE KNOTING_DLLs
            ${KNOTING_LIB_DIR}/*.dll
            )
else ()
    set(KNOTING_LIB_DIR "${CMAKE_SOURCE_DIR}/lib/x86_64/linux")
    file(GLOB_RECURSE KNOTING_DLLs
            ${KNOTING_LIB_DIR}/*.so
            )
endif ()

foreach (_DLL ${KNOTING_DLLs})
    get_filename_component(FILE_NAME ${_DLL} NAME)
    set(FILE_NAME "${CMAKE_BINARY_DIR}/dist/bin/${FILE_NAME}")
    configure_file(${_DLL} ${FILE_NAME} COPYONLY)
endforeach ()
