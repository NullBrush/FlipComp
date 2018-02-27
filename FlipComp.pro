
TARGET = FlipComp

HEADERS = FlipComp.h

SOURCES = FlipComp.cpp

DEFINES *= COMPILING_FLIPCOMP

include(../plugins_qmake.pri)

PLUGIN_ID = FlipComp
PLUGIN_TEXT = "FlipComp"
PLUGIN_DESCRIPTION = Change order of how things are rendered with one attribute.
PLUGIN_TYPE = Module
PLUGIN_CATEGORY = Misc
PLUGIN_ICON = resources/flipcomp.png



include(../plugins_xml)
