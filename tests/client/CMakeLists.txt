macro(declare_test testname)
    set(TEST_MOC_FILE ${CMAKE_CURRENT_BINARY_DIR}/${testname}.moc)
    qt4_generate_moc(${testname}.cpp ${TEST_MOC_FILE})

    add_executable(${testname} ${testname}.cpp ${TEST_MOC_FILE})
    target_link_libraries(${testname}
                          qmenumodelcommon
                          dbusmenuscript
                          ${QT_QTTEST_LIBRARY}
                          ${QT_QTCORE_LIBRARY}
                          ${QT_QTGUI_LIBRARY}
                          ${QT_QTDBUS_LIBRARY}
                          ${GLIB_LDFLAGS}
                          ${GIO_LDFLAGS})

    add_test(${testname}
             ${DBUS_RUNNER}
             --task ${CMAKE_CURRENT_BINARY_DIR}/${testname} --task-name Client
             --task ${CMAKE_CURRENT_SOURCE_DIR}/script_${testname}.py --task-name Server
             --ignore-return)
    set_tests_properties(${testname} PROPERTIES
                         TIMEOUT ${CTEST_TESTING_TIMEOUT}
                         ENVIRONMENT "PYTHONPATH=${TEST_PYTHONPATH}")

endmacro(declare_test testname)

macro(declare_simple_test testname)
    set(TEST_MOC_FILE ${CMAKE_CURRENT_BINARY_DIR}/${testname}.moc)
    qt4_generate_moc(${testname}.cpp ${TEST_MOC_FILE})

    add_executable(${testname} ${testname}.cpp ${TEST_MOC_FILE})
    target_link_libraries(${testname}
                          qmenumodelcommon
                          ${QT_QTTEST_LIBRARY}
                          ${QT_QTCORE_LIBRARY}
                          ${QT_QTGUI_LIBRARY}
                          ${QT_QTDBUS_LIBRARY}
                          ${GLIB_LDFLAGS}
                          ${GIO_LDFLAGS})

    add_test(${testname}
             ${CMAKE_CURRENT_BINARY_DIR}/${testname})

    set_tests_properties(${testname} PROPERTIES
                         TIMEOUT ${CTEST_TESTING_TIMEOUT})
endmacro(declare_simple_test testname)

include_directories(${qmenumodelcommon_SOURCE_DIR}
                    ${dbusmenuscript_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QT_QTTEST_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR}
                    ${QT_QTGUI_INCLUDE_DIR}
                    ${QT_QTDBUS_INCLUDE_DIR}
                    ${GLIB_INCLUDE_DIRS}
                    ${GIO_INCLUDE_DIRS})

add_definitions(-DTEST_SUITE)
set(TEST_PYTHONPATH ${dbusmenuscript_SOURCE_DIR})
if(NOT CTEST_TESTING_TIMEOUT)
    set(CTEST_TESTING_TIMEOUT 60)
endif()

declare_test(servicetest)
declare_test(menuchangestest)
declare_test(modeltest)
declare_test(actiongrouptest)
declare_simple_test(convertertest)
