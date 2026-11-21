cmake_minimum_required(VERSION 3.21)

project(w32oop VERSION 5.6.5.2 LANGUAGES CXX)

if (NOT WIN32)
    message(FATAL_ERROR "w32oop supports Windows only.")
endif ()
if (NOT MSVC)
    message(WARNING "w32oop is developed against MSVC; other compilers are unsupported.")
endif ()
if (BUILD_SHARED_LIBS)
    message(WARNING "w32oop only supports static libraries. BUILD_SHARED_LIBS is ignored.")
endif ()

option(W32OOP_ENABLE_PCH "Use w32use.hpp as a precompiled header" ON)
option(W32OOP_USE_STATIC_CRT "Use the static CRT (/MT) instead of /MD" ON)
option(W32OOP_ENABLE_LTO "Enable interprocedural optimization (/GL + /LTCG)" OFF)
option(W32OOP_ENABLE_CFG "Enable /guard:cf Control Flow Guard" ON)
option(W32OOP_ENABLE_SPECTRE "Enable /Qspectre Spectre mitigation" ON)
option(W32OOP_ENABLE_JCC_ERRATUM "Enable /QIntel-jcc-erratum mitigation" ON)
option(W32OOP_ENABLE_EHCONT "Compile and link with /guard:ehcont" ON)
option(W32OOP_INSTALL "Generate install rules and package config files" ON)
option(W32OOP_BUILD_TESTS "Build the headless layout tests" OFF)

set(W32OOP_SOURCES
    Concurrency/EventObject.cpp
    Concurrency/Process.cpp
    Concurrency/Thread.cpp
    GUI/FoundationLib.cpp
    GUI/FlexLayout.cpp
    GUI/FlexSolver.cpp
    GUI/Menu.cpp
    GUI/TrayIcon.cpp
    GUI/Window.cpp
    IO/File.cpp
    IO/PipeClient.cpp
    IO/PipeServer.cpp
    Network/HttpRequest.cpp
    Network/HttpResponse.cpp
    System/Hooks.cpp
    System/Registry.cpp
    System/Service.cpp
)

if (MSVC)
    # Must be set before any target is created so that consumers use the same CRT.
    if (W32OOP_USE_STATIC_CRT)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else ()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif ()

    if (W32OOP_ENABLE_CFG)
        add_compile_options(/guard:cf)
        add_link_options(/guard:cf)
    endif ()

    if (W32OOP_ENABLE_SPECTRE)
        add_compile_options(/Qspectre)
    endif ()

    if (W32OOP_ENABLE_JCC_ERRATUM)
        add_compile_options(/QIntel-jcc-erratum)
    endif ()

    # The library objects must carry EH continuation metadata so that they can
    # be linked into applications that enable /guard:ehcont. LNK1386 is raised
    # when a linked module was compiled without this option.
    if (W32OOP_ENABLE_EHCONT)
        add_compile_options(/guard:ehcont)
        add_link_options(/guard:ehcont)
    endif ()
endif ()

if (W32OOP_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _w32oop_ipo_supported OUTPUT _w32oop_ipo_output)
    if (_w32oop_ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else ()
        message(WARNING "LTO is not supported by this toolchain: ${_w32oop_ipo_output}")
    endif ()
endif ()

add_library(w32oop STATIC ${W32OOP_SOURCES})
add_library(w32oop::w32oop ALIAS w32oop)

target_compile_features(w32oop PUBLIC cxx_std_20)
target_compile_definitions(w32oop PUBLIC UNICODE _UNICODE)
target_include_directories(w32oop PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/w32oop>
)

if (MSVC)
    target_compile_options(w32oop PRIVATE /W4 /permissive- /EHsc /utf-8 /Zc:__cplusplus /MP)
endif ()

target_link_libraries(w32oop PUBLIC
    kernel32
    user32
    gdi32
    comdlg32
    comctl32
    advapi32
    shell32
    ole32
    oleaut32
    uuid
    winhttp
    rpcrt4
)

if (W32OOP_ENABLE_PCH)
    target_precompile_headers(w32oop PRIVATE w32use.hpp)
endif ()

if (W32OOP_BUILD_TESTS)
    enable_testing()
    # 布局求解器不依赖 Windows，可以脱离窗口直接测试。
    add_executable(w32oop_layout_tests
        .test/layout/flex_solver_tests.cpp
        GUI/FlexSolver.cpp
    )
    target_compile_features(w32oop_layout_tests PRIVATE cxx_std_20)
    target_include_directories(w32oop_layout_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    if (MSVC)
        target_compile_options(w32oop_layout_tests PRIVATE /W4 /permissive- /utf-8)
    endif ()
    add_test(NAME layout.flex_solver COMMAND w32oop_layout_tests)
endif ()

if (W32OOP_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS w32oop EXPORT w32oopTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )

    install(FILES w32use.hpp DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/w32oop")
    foreach (module Core Concurrency GUI IO Network System Utility)
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${module}"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/w32oop"
            FILES_MATCHING PATTERN "*.hpp")
    endforeach ()

    install(EXPORT w32oopTargets
        FILE w32oopTargets.cmake
        NAMESPACE w32oop::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/w32oop"
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/w32oopConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/w32oopConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/w32oop"
    )
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/w32oopConfigVersion.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/w32oopConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/w32oopConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/w32oop"
    )

    install(FILES README.md LICENSE
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/w32oop")
endif ()
