#idf_component_register(SRCS "." INCLUDE_DIRS ".")
#idf_component_register(SRCS ${srcs} "." INCLUDE_DIRS ${include_dirs} ".")

set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")


register_component()


littlefs_create_partition_image(data ../data FLASH_IN_PROJECT)
