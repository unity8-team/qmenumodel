project(qmlplugin)

set(QMLPLUGIN_SRC
    plugin.cpp
)

add_library(qmenumodel-qml MODULE
    ${QMLPLUGIN_SRC}
)

include_directories(
    ${src_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
)

target_link_libraries(qmenumodel-qml
    qmenumodel
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
)

qt5_use_modules(qmenumodel-qml Qml Widgets)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/qmldir"
                                                              "${CMAKE_CURRENT_BINARY_DIR}/qmldir")

if (NOT LOCAL_QML_IMPORT_DIR)
    execute_process(
        COMMAND qmake -query QT_INSTALL_QML
        OUTPUT_VARIABLE QT_IMPORTS_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else ()
    set(QT_IMPORTS_DIR "${LOCAL_QML_IMPORT_DIR}")
endif()

set(QMLPLUGIN_INSTALL_PREFIX "${QT_IMPORTS_DIR}/QMenuModel")
install(TARGETS qmenumodel-qml DESTINATION ${QMLPLUGIN_INSTALL_PREFIX})
install(FILES qmldir DESTINATION ${QMLPLUGIN_INSTALL_PREFIX})
