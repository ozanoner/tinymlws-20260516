idf_component_register(SRCS "main.cpp"
    model/model_settings.cc  model/person_detect_model_data.cc
    INCLUDE_DIRS "." data model ../../common
    EMBED_FILES "data/image0" 
                "data/image1" 
                "data/image2" 
                "data/image3"
                "data/image4" 
                "data/image5" 
                "data/image6" 
                "data/image7"
                "data/image8" 
                "data/image9"
   )

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-maybe-uninitialized
    -Wno-missing-field-initializers
    -Wno-error=sign-compare
    -Wno-error=double-promotion
    -Wno-type-limits)        
