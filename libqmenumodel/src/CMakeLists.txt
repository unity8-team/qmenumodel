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

add_library(qmenumodel STATIC
    ${QMENUMODEL_SRC}
)

set_target_properties(qmenumodel PROPERTIES COMPILE_FLAGS -fPIC)

include_directories(
    ${GLIB_INCLUDE_DIRS}
)

qt5_use_modules(qmenumodel Core Widgets)

