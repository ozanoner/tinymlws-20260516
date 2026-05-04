idf_component_register(SRCS "main.cpp"
    model/model_settings.cc  model/person_detect_model_data.cc
    INCLUDE_DIRS "." data model ../../common)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-maybe-uninitialized
    -Wno-missing-field-initializers
    -Wno-error=sign-compare
    -Wno-error=double-promotion
    -Wno-type-limits)        
