
set(module_name "midiplugin")

set(TS_FILES
    ../MIDI_de_DE.ts
    ../MIDI_es_ES.ts
    ../MIDI_fi_FI.ts
    ../MIDI_fr_FR.ts
    ../MIDI_it_IT.ts
    ../MIDI_nl_NL.ts
    ../MIDI_cz_CZ.ts
    ../MIDI_pt_BR.ts
    ../MIDI_ca_ES.ts
    ../MIDI_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_library(${module_name} SHARED
    ${QM_FILES}
)

target_sources(${module_name} PRIVATE
    ../../../../engine/src/qlcfile.cpp ../../../../engine/src/qlcfile.h
    ../../../interfaces/qlcioplugin.cpp ../../../interfaces/qlcioplugin.h
    ../common/configuremidiplugin.cpp ../common/configuremidiplugin.h ../common/configuremidiplugin.ui
    ../common/mididevice.cpp ../common/mididevice.h
    ../common/midienumerator.h
    ../common/midiinputdevice.cpp ../common/midiinputdevice.h
    ../common/midioutputdevice.cpp ../common/midioutputdevice.h
    ../common/${module_name}.cpp ../common/${module_name}.h
    ../common/midiprotocol.cpp ../common/midiprotocol.h
    ../common/miditemplate.cpp ../common/miditemplate.h
    coremidienumerator.cpp
    coremidienumeratorprivate.h
    coremidiinputdevice.cpp coremidiinputdevice.h
    coremidioutputdevice.cpp coremidioutputdevice.h
)
target_include_directories(${module_name} PRIVATE
    ../../../../engine/src
    ../../../interfaces
    ../common ../common
)

target_link_libraries(${module_name} PRIVATE
    "-framework CoreFoundation"
    "-framework CoreMIDI"
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)
