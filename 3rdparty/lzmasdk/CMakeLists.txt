FILE(GLOB LZMA_SDK_SRC C/*.c)
FILE(GLOB LZMA_SDK_H C/*.h)

add_library(lzmasdk STATIC ${LZMA_SDK_SRC} ${LZMA_SDK_H})

