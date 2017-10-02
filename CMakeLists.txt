cmake_minimum_required(VERSION 3.5)

project(SistemaReservaCine)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

set(ADMIN_FILES src/admin/admin.cpp)
set(CLIENT_FILES src/cliente/interfaz.cpp src/cliente/cliente.cpp)
set(MOM_FILES src/mom/mom.cpp)
set(CINE_FILES src/cine/cine.cpp)
set(CINE_LOGIN_FILES src/cine_login/cine_login.cpp)
set(ENVIRONMENT_FILES src/environment/environment.cpp)
set(COMMON_FILES src/common/ipc/msg_queue.cpp src/common/ipc/sh_mem.cpp)


add_executable(admin ${ADMIN_FILES} ${COMMON_FILES})
add_executable(cliente ${CLIENT_FILES} ${COMMON_FILES})
add_executable(mom ${MOM_FILES} ${COMMON_FILES})
add_executable(cine ${CINE_FILES} ${COMMON_FILES})
add_executable(cine_login ${CINE_LOGIN_FILES} ${COMMON_FILES})
add_executable(environment ${ENVIRONMENT_FILES} ${COMMON_FILES})