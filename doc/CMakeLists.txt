set(QMENUMODEL_DOC_FILE ${CMAKE_CURRENT_BINARY_DIR}/qmenumodel.qdocconf)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/qmenumodel.qdocconf.in"
               ${QMENUMODEL_DOC_FILE} @ONLY)

add_custom_target(qdoc)
add_custom_command(TARGET qdoc
                   COMMAND ${QDOC_BIN} ${QMENUMODEL_DOC_FILE}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "QDoc running...")
