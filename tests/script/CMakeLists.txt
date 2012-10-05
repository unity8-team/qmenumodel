project(dbusmenuscript)

set(DBUSMENUSCRIPT_SRC
    dbusmenuscript.cpp
)

set(DBUSMENUSCRIPT_HEADERS
    dbusmenuscript.h
)

qt5_wrap_cpp(DBUSMENUSCRIPT_MOC
    ${DBUSMENUSCRIPT_HEADERS}
)

add_library(dbusmenuscript STATIC
    ${DBUSMENUSCRIPT_SRC}
    ${DBUSMENUSCRIPT_MOC}
)

set_target_properties(dbusmenuscript PROPERTIES COMPILE_FLAGS -fPIC)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTTEST_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR}
)

qt5_use_modules(dbusmenuscript Core DBus Test)

target_link_libraries(dbusmenuscript
    ${QT_QTCORE_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
)
