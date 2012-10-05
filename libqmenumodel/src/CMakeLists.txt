project(src)

set(QMENUMODEL_SRC
    converter.cpp
    qmenumodel.cpp
    qdbusobject.cpp
    qdbusmenumodel.cpp
    qdbusactiongroup.cpp
    qstateaction.cpp
)

set(QMENUMODEL_HEADERS
    converter.h
    dbus-enums.h
    qmenumodel.h
    qdbusobject.h
    qdbusmenumodel.h
    qdbusactiongroup.h
    qstateaction.h
)

qt4_wrap_cpp(QMENUMODEL_MOC
    ${QMENUMODEL_HEADERS}
)

add_library(qmenumodel STATIC
    ${QMENUMODEL_SRC}
    ${QMENUMODEL_MOC}
)

set_target_properties(qmenumodel PROPERTIES COMPILE_FLAGS -fPIC)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)

target_link_libraries(qmenumodel
    ${QT_QTCORE_LIBRARY}
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
)
