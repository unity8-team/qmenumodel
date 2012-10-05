project(qmlplugin)

set(QMLPLUGIN_SRC
    plugin.cpp
)

set(QMLPLUGIN_HEADERS
    plugin.h
)

qt4_wrap_cpp(QMLPLUGIN_MOC
    ${QMLPLUGIN_HEADERS}
)

add_library(qmenumodel-qml MODULE
    ${QMLPLUGIN_SRC}
    ${QMLPLUGIN_MOC}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${src_SOURCE_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTDECLARATIVE_INCLUDE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)

target_link_libraries(qmenumodel-qml
    qmenumodel
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDCLARATIVE_LIBRARY}
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/qmldir"
                                                              "${CMAKE_CURRENT_BINARY_DIR}/qmldir")

set(QMLPLUGIN_INSTALL_PREFIX "${QT_IMPORTS_DIR}/QMenuModel")
install(TARGETS qmenumodel-qml DESTINATION ${QMLPLUGIN_INSTALL_PREFIX})
install(FILES qmldir DESTINATION ${QMLPLUGIN_INSTALL_PREFIX})

