LIBS += -lz

HEADERS += \
    $$PWD/ExceptionPasteTranslationError.h \
    $$PWD/ExceptionTextForTranslationEmpty.h \
    $$PWD/ExceptionTranslationNotDone.h \
    $$PWD/ExceptionTranslationSame.h \
    $$PWD/ExceptionTranslationSize.h \
    $$PWD/TrTextManager.h \
    $$PWD/TrZipManager.h \
    $$PWD/TranslationLinkManager.h \
    #$$PWD/TrTextManager.h \
    $$PWD/TranslatorManual.h

SOURCES += \
    $$PWD/ExceptionPasteTranslationError.cpp \
    $$PWD/ExceptionTextForTranslationEmpty.cpp \
    $$PWD/ExceptionTranslationNotDone.cpp \
    $$PWD/ExceptionTranslationSame.cpp \
    $$PWD/ExceptionTranslationSize.cpp \
    $$PWD/TrTextManager.cpp \
    $$PWD/TrZipManager.cpp \
    $$PWD/TranslationLinkManager.cpp \
    #$$PWD/TrTextManager.cpp \
    $$PWD/TranslatorManual.cpp
