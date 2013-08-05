# crlserver CMakeLists.txt
# ========================

project(crlserver)
cmake_minimum_required(VERSION 2.8)

# CMake Configuration
# ------------------
set(CMAKE_MODULE_PATH ${CMAME_MODULE_PATH} ${CMAKE_SOURCE_DIR}/util/cmake/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# CMake global options
# --------------------
option(ENABLE_SQLITE "Enable the sqlite backend" ON)
option(ENABLE_LDAP "Enable the ldap backend" OFF)

# Global packages search
# ----------------------
find_package(CONFUSE REQUIRED)
include_directories(${CONFUSE_INCLUDE_DIR})

# XXX: Find how to define this in a portable way ...
include("/usr/local/share/cmake/Modules/FindCurses.cmake")
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
include_directories(${NCURSES_INCLUDE_DIR})

if(ENABLE_LDAP)
    set(ENABLE_SQLITE OFF)
    find_package(LDAP REQUIRED)
    include_directories(${LDAP_INCLUDE_DIR})
    add_definitions(-DHAVE_LDAP)
else()
    find_package(SQLITE REQUIRED)
    include_directories(${SQLITE_INCLUDE_DIR})
    add_definitions(-DHAVE_SQLITE)
endif()

if (NOT CONFUSE_FOUND OR NOT CURSES_FOUND)
    message(FATAL_ERROR "You have to install libconfuse and libncurses")
endif(NOT CONFUSE_FOUND OR NOT CURSES_FOUND)

if (ENABLE_SQLITE AND NOT SQLITE_FOUND)
    message(FATAL_ERROR "You have to install libsqlite3")
endif(ENABLE_SQLITE AND NOT SQLITE_FOUND)

if (ENABLE_LDAP AND NOT LDAP_FOUND)
    message(FATAL_ERROR "You have to install libldap")
endif(ENABLE_LDAP AND NOT LDAP_FOUND)

# CMake Configuration
# -------------------
set(CMAKE_INSTALL_PREFIX "/usr/local/")
set(${CMAKE_SYSTEM_NAME} True)
set(CMAKE_C_FLAGS "-std=c99")

# Global CPP definitions
# ----------------------
add_definitions(-D${CMAKE_SYSTEM_NAME})

# Portable source files
# ---------------------
set(SOURCES_LIST
    main.c
    conf.c
    init.c
    menus.c
    log.c
    aux.c
    filter.c
    register.c
)

if(ENABLE_SQLITE)
    set(SOURCES_LIST ${SOURCES_LIST} db-sqlite.c)
else()
    set(SOURCES_LIST ${SOURCES_LIST} db-ldap.c)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Executable definitons
# ---------------------
add_executable(crlserver ${SOURCES_LIST})

# Global linked libraries
# -----------------------
target_link_libraries(crlserver ${CONFUSE_LIBRARIES})
target_link_libraries(crlserver ${CURSES_LIBRARIES})
if(ENABLE_SQLITE)
    target_link_libraries(crlserver ${SQLITE_LIBRARIES})
else()
    target_link_libraries(crlserver ${LDAP_LIBRARIES})
endif()

# Optional local modifications, such as compiler settings
# -------------------------------------------------------
include(CMakeLists.txt.local OPTIONAL)

