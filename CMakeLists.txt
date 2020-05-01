cmake_minimum_required(VERSION 3.0)
set(CMAKE_TOOLCHAIN_FILE $ENV{HOME}/arm-cross-comp-env/rpi-zero-w-toolchain.cmake)
project(SETR_TP5)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(ALSA REQUIRED)

SET_SOURCE_FILES_PROPERTIES(pipe_com.c socket_com.c bluetooth_com.c audioReceiver.c comEmitter.c comReceiver.c audioEmitter.c delay.c distorsion.c highpass.c lowpass.c audio.c PROPERTIES LANGUAGE CXX )
set(SOURCE_AUDIO_RECEIVER pipe_com.c audio.c audioReceiver.c)
set(SOURCE_COM_EMITTER pipe_com.c bluetooth_com.c  socket_com.c comEmitter.c)
set(SOURCE_COM_RECEIVER pipe_com.c bluetooth_com.c  socket_com.c comReceiver.c)
set(SOURCE_AUDIO_EMITTER pipe_com.c audio.c audioEmitter.c)
set(DISTORSION distorsion.c pipe_com.c)
set(DELAY delay.c pipe_com.c)
set(HIGHPASS highpass.c pipe_com.c)
set(LOWPASS lowpass.c pipe_com.c)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=armv6zk -mfpu=vfp -mfloat-abi=hard -Ofast -funroll-loops -funsafe-math-optimizations -floop-block -flto")

SET(GCC_WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wduplicated-cond -Wlogical-op -Wnull-dereference -Wshadow")
add_definitions("${GCC_WARNING_FLAGS}")

add_executable(audioReceiver ${SOURCE_AUDIO_RECEIVER})
target_link_libraries(audioReceiver rt ${ALSA_LIBRARIES} Threads::Threads)

add_executable(comEmitter ${SOURCE_COM_EMITTER})
target_link_libraries(comEmitter rt ${BlueZ_LIBRARIES} -lbluetooth Threads::Threads)

add_executable(comReceiver ${SOURCE_COM_RECEIVER})
target_link_libraries(comReceiver rt ${BlueZ_LIBRARIES} -lbluetooth Threads::Threads)

add_executable(audioEmitter ${SOURCE_AUDIO_EMITTER})
target_link_libraries(audioEmitter rt  ${ALSA_LIBRARIES} Threads::Threads)

add_executable(distorsion ${DISTORSION})
target_link_libraries(distorsion rt Threads::Threads)
add_executable(delay ${DELAY})
target_link_libraries(delay rt Threads::Threads)
add_executable(highpass ${HIGHPASS})
target_link_libraries(highpass rt Threads::Threads)
add_executable(lowpass ${LOWPASS})
target_link_libraries(lowpass rt Threads::Threads)