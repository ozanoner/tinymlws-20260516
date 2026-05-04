set(requires hardware_driver player esp_timer)

idf_component_register(SRCS main.cpp
                        INCLUDE_DIRS . data ../../common
                        REQUIRES ${requires})

