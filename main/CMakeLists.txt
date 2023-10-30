# Embed CA, certificate & key directly into binary
idf_component_register(SRCS "controller.c" "light_controller.c" "light.c" "main.c" "c_coap.c"
                    INCLUDE_DIRS "."
                    )
