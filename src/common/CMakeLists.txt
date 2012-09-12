project(qmenumodelcommon)

set(QMENUMODELCOMMON_SRC
    converter.cpp
    qmenumodel.cpp
    qdbusobject.cpp
    qdbusmenumodel.cpp
    qdbusactiongroup.cpp
)

set(QMENUMODELCOMMON_HEADERS
    converter.h
    qmenumodel.h
    qdbusobject.h
    qdbusmenumodel.h
    qdbusactiongroup.h
)

qt4_wrap_cpp(QMENUMODELCOMMON_MOC
    ${QMENUMODELCOMMON_HEADERS}
)

add_library(qmenumodelcommon STATIC
    ${QMENUMODELCOMMON_SRC}
    ${QMENUMODELCOMMON_MOC}
)

set_target_properties(qmenumodelcommon PROPERTIES COMPILE_FLAGS -fPIC)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)

target_link_libraries(qmenumodelcommon
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
)
