# aicli — AI command-line tool registry and availability table.
#
# Requirements (call in the including subproject's CMakeLists.txt before this):
#   find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)
#   setup_qcoro(COMPONENTS Core)   # from common.cmake
#
# Then link:
#   target_link_libraries(<target> PRIVATE Qt${QT_VERSION_MAJOR}::Widgets QCoro::Core)

set(AICLI_FILES
    ${CMAKE_CURRENT_LIST_DIR}/AbstractCli.h
    ${CMAKE_CURRENT_LIST_DIR}/AbstractCli.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CliClaude.h
    ${CMAKE_CURRENT_LIST_DIR}/CliClaude.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CliGemini.h
    ${CMAKE_CURRENT_LIST_DIR}/CliGemini.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CliCodex.h
    ${CMAKE_CURRENT_LIST_DIR}/CliCodex.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CliDeepseek.h
    ${CMAKE_CURRENT_LIST_DIR}/CliDeepseek.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CliKimi.h
    ${CMAKE_CURRENT_LIST_DIR}/CliKimi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CliMistral.h
    ${CMAKE_CURRENT_LIST_DIR}/CliMistral.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AvailableCliTable.h
    ${CMAKE_CURRENT_LIST_DIR}/AvailableCliTable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AvailableCliList.h
    ${CMAKE_CURRENT_LIST_DIR}/AvailableCliList.cpp
)
