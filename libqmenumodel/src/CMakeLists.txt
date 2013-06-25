project(src)

set(QMENUMODEL_SRC
    converter.cpp
    dbus-enums.h
    menunode.cpp
    qmenumodel.cpp
    qdbusobject.cpp
    qdbusmenumodel.cpp
    qdbusactiongroup.cpp
    qstateaction.cpp
    unitymenumodel.cpp
    unitythemediconprovider.cpp
    gtk/gtkactionmuxer.c
    gtk/gtkactionmuxer.h
    gtk/gtkactionobservable.c
    gtk/gtkactionobservable.h
    gtk/gtkactionobserver.c
    gtk/gtkactionobserver.h
    gtk/gtkmenutracker.c
    gtk/gtkmenutracker.h
    gtk/gtkmenutrackeritem.c
    gtk/gtkmenutrackeritem.h
)

set(SHAREDLIBNAME qmenumodel)
add_library(${SHAREDLIBNAME} SHARED
    ${QMENUMODEL_SRC}
)

set_target_properties(${SHAREDLIBNAME} PROPERTIES
    COMPILE_FLAGS -fPIC
    SOVERSION 0
    VERSION 0.1.1
)

include_directories(
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)

target_link_libraries(${SHAREDLIBNAME}
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
)

qt5_use_modules(${SHAREDLIBNAME} Core Qml Quick)

install(TARGETS ${SHAREDLIBNAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(QMENUMODEL_HEADERS
    dbus-enums.h
    qdbusactiongroup.h
    qdbusmenumodel.h
    qdbusobject.h
    qmenumodel.h
    qstateaction.h
    unitymenumodel.h
    unitythemediconprovider.h
)

set(INCLUDEDIR qmenumodel)
install(FILES ${QMENUMODEL_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${INCLUDEDIR}
)

set(PCFILE qmenumodel.pc)
configure_file(${PCFILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${PCFILE} @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PCFILE}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

