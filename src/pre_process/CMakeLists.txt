aux_source_directory(./ "PRE_PROCESS_SRC")
add_library(pre_process STATIC ${PRE_PROCESS_SRC})

#add_executable(pre_process_main ${PRE_PROCESS_SRC})
#target_link_libraries(pre_process_main basic)
