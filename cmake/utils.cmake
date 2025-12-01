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

function(ragelmaker src_rl outputlist outputdir)
    #Create a custom build step that will call ragel on the provided src_rl file.
    #The output .cpp file will be appended to the variable name passed in outputlist.

    get_filename_component(src_file ${src_rl} NAME_WE)

    set(rl_out ${outputdir}/${src_file}.rl.cc)

    #adding to the list inside a function takes special care, we cannot use list(APPEND...)
    #because the results are local scope only
    set(${outputlist} ${${outputlist}} ${rl_out} PARENT_SCOPE)

    #Warning: The " -S -M -l -C -T0  --error-format=msvc" are added to match existing window invocation
    #we might want something different for mac and linux
    add_custom_command(
        OUTPUT ${rl_out}
        COMMAND cd ${outputdir}
        COMMAND ragel ${CMAKE_CURRENT_SOURCE_DIR}/${src_rl} -o ${rl_out} -l -C -G2  --error-format=msvc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src_rl}
        )
    set_source_files_properties(${rl_out} PROPERTIES GENERATED TRUE)
endfunction(ragelmaker)

function(yuyu_add_executable targetname srcs dependencies libs)
    add_executable(${targetname} ${srcs})
    add_dependencies(${targetname} ${dependencies})
    force_redefine_file_macro_for_sources(${targetname})
    target_link_libraries(${targetname} ${libs})
endfunction(yuyu_add_executable targetname srcs dependencies libs)
