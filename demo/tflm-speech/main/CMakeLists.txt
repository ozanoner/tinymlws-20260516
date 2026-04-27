idf_component_register(SRCS "main.cpp"
            model/micro_features_generator.cc
            model/model.cc
            model/no_micro_features_data.cc
            model/ringbuf.c
            model/yes_micro_features_data.cc
    INCLUDE_DIRS "." data model ../../common
    EMBED_FILES ../resources/no_1000ms.wav 
        ../resources/yes_1000ms.wav)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-maybe-uninitialized
    -Wno-missing-field-initializers
    -Wno-error=sign-compare
    -Wno-error=double-promotion
    -Wno-type-limits)        
