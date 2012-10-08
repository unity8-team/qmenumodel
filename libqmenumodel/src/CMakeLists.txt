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

