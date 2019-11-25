include_directories(${AIOUSB_SOURCE_DIR}/include)
add_library(aiousb STATIC aiousb.c adc-threads.c)