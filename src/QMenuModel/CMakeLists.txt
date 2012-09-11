
project(qmenumodelqmlplugin)

set(QMENUMODEL_SRC
    plugin.cpp
)

set(QMENUMODEL_HEADERS
    plugin.h
)

qt4_wrap_cpp(QMENUMODEL_MOC
    ${QMENUMODEL_HEADERS}
)

add_library(qmenumodel MODULE
    ${QMENUMODEL_SRC}
    ${QMENUMODEL_MOC}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${qmenumodelcommon_SOURCE_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTDECLARATIVE_INCLUDE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)

target_link_libraries(qmenumodel
    qmenumodelcommon
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDCLARATIVE_LIBRARY}
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/qmldir"
                                                              "${CMAKE_CURRENT_BINARY_DIR}/qmldir")

set(QMENUMODEL_INSTALL_PREFIX "${QT_IMPORTS_DIR}/Ubuntu/QMenuModel/")
install(TARGETS qmenumodel DESTINATION ${QMENUMODEL_INSTALL_PREFIX})
install(FILES qmldir DESTINATION ${QMENUMODEL_INSTALL_PREFIX})
