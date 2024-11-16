if(NOT DEFINED LANGUE_CODES)
    message(FATAL_ERROR "The variable LANGUE_CODES is not defined. Please define it before running the script.")
endif()

# Join the language codes with semicolons
list(JOIN LANGUE_CODES ";" LANGUE_CODES_MERGED)
# Properly escape quotes for CMake definitions
string(REPLACE ";" "\", \"" LANGUE_CODES_CXX_LIST "${LANGUE_CODES_MERGED}")
set(LANGUE_CODES_CXX_JOINED "\"${LANGUE_CODES_CXX_LIST}\"")


set(LANGUAGES_FILES
        ${CMAKE_CURRENT_LIST_DIR}/DialogAddLanguage.h
        ${CMAKE_CURRENT_LIST_DIR}/DialogAddLanguage.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DialogAddLanguage.ui
        ${CMAKE_CURRENT_LIST_DIR}/DialogSelectLangage.h
        ${CMAKE_CURRENT_LIST_DIR}/DialogSelectLangage.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DialogSelectLangage.ui
        ${CMAKE_CURRENT_LIST_DIR}/LangManager.h
        ${CMAKE_CURRENT_LIST_DIR}/LangManager.cpp
)

