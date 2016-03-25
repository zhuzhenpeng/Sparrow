aux_source_directory(./ "VM_SRC")

add_executable(main ${VM_SRC})
target_link_libraries(main basic pre_process)
