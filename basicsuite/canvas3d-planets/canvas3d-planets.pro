TARGET = canvas3d-planets

include(../shared/shared.pri)
b2qtdemo_deploy_defaults()

content.files = \
    *.qml \
    *.js \
    images

content.path = $$DESTPATH

OTHER_FILES += $${content.files}

INSTALLS += target content
