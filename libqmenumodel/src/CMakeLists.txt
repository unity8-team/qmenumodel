project(src)

set(QMENUMODEL_SRC
    converter.cpp
    dbus-enums.h
    qmenumodel.cpp
    qdbusobject.cpp
    qdbusmenumodel.cpp
    qdbusactiongroup.cpp
    qstateaction.cpp
)

add_library(qmenumodel SHARED
    ${QMENUMODEL_SRC}
)

set_target_properties(qmenumodel PROPERTIES
    COMPILE_FLAGS -fPIC
    SOVERSION 0
    VERSION 0.1.1
)

include_directories(
    ${GLIB_INCLUDE_DIRS}
)

qt5_use_modules(qmenumodel Core Widgets)

install(TARGETS qmenumodel LIBRARY DESTINATION lib${LIB_SUFFIX})

set(QMENUMODEL_HEADERS
    dbus-enums.h
    qdbusactiongroup.h
    qdbusmenumodel.h
    qdbusobject.h
    qmenumodel.h
    qstateaction.h
)

install(FILES ${QMENUMODEL_HEADERS}
    DESTINATION include/qmenumodel
)

