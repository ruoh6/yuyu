
function(force_redefine_file_macro_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        get_filename_component(filepath ${sourcefile} ABSOLUTE)
        string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
        list(APPEND defs "__FILE__=\"${relpath}\"")
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs}
        )
    endforeach(sourcefile ${source_file})
endfunction(force_redefine_file_macro_for_sources targetname)

function(yuyu_add_executable targetname srcs dependencies libs)
    add_executable(${targetname} ${srcs})
    add_dependencies(${targetname} ${dependencies})
    force_redefine_file_macro_for_sources(${targetname})
    target_link_libraries(${targetname} ${libs})
endfunction(yuyu_add_executable targetname srcs dependencies libs)
