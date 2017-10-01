cmake_minimum_required(VERSION 3.5)

project(proyecto)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

set(ADMIN_FILES src/admin/main.cpp)
set(CLIENT_FILES src/cliente/cliente.cpp)
set(INTERFAZ_FILES src/cliente/interfaz.cpp)
set(MOM_FILES src/cliente/mom/mom_cliente.cpp)
set(CINE_FILES src/cine/main.cpp)
set(CINE_LOGIN_FILES src/cine_login/main.cpp)
set(ENVIRONMENT_FILES src/environment/main.cpp)
set(COMMON_FILES src/common/canal.cpp src/common/ipc/msg_queue.cpp src/common/ipc/sh_mem.cpp)


add_executable(admin ${ADMIN_FILES} ${COMMON_FILES})
add_executable(cliente ${CLIENT_FILES} ${INTERFAZ_FILES} ${COMMON_FILES})
add_executable(mom_cliente ${MOM_FILES} ${COMMON_FILES})
add_executable(cine ${CINE_FILES} ${COMMON_FILES})
add_executable(cine_login ${CINE_LOGIN_FILES} ${COMMON_FILES})
add_executable(environment ${ENVIRONMENT_FILES} ${COMMON_FILES})