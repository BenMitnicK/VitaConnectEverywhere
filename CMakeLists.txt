#
#  VitaConnectEverywhere
#  Copyright (C) 2024, BenMitnicK
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(VitaConnectEverywhere)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-pic -fno-PIC -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

add_subdirectory(src/modules/kernel)
add_subdirectory(src/modules/user)
add_subdirectory(src/modules/patch)
add_subdirectory(src/modules/usbdevice)

include_directories(
  src/modules/kernel
  src/modules/user
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
    src/main.c
    src/thread.c
	src/utils.c
	src/ftpvita.c
	src/pfs.c
	src/package_installer.c
	src/refresh.c
	src/pbp.c
	src/file.c
	src/sfo.c
	src/rif.c
	src/sqlite3.c
	src/sha1.c
	src/sha256.c
)

target_link_libraries(${PROJECT_NAME}
	${CMAKE_CURRENT_BINARY_DIR}/src/modules/user/libVitaConnectUser_stub_weak.a
	taipool
	taihen_stub
	z
    m
	k
	c
	gcc
	SceCommonDialog_stub_weak
	SceIncomingDialog_stub_weak
	SceSysmodule_stub_weak
	SceRegistryMgr_stub_weak
	SceNotificationUtil_stub_weak
	SceLibKernel_stub
	SceNet_stub_weak
	SceNetCtl_stub_weak
	SceKernelThreadMgr_stub_weak
	SceIofilemgr_stub_weak
	SceRtc_stub_weak
	SceAppMgr_stub
	ScePower_stub
	QuickMenuReborn_stub
	SceMtpIfDriver_stub_weak
	SceDriverUser_stub_weak
	SceUdcd_stub_weak
	SceUsbstorVStorDriver_stub_weak
	SceVshBridge_stub_weak
	SceSysmem_stub_weak
	SceShellSvc_stub_weak
    SceKernelModulemgr_stub_weak
	SceWlanBt_stub_weak
	SceWlanBtForDriver_stub_weak
	ScePromoterUtil_stub_weak
	SceSqlite_stub_weak
	SceNpDrm_stub_weak
)

set_target_properties(${PROJECT_NAME}
	PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(${PROJECT_NAME}.suprx ${PROJECT_NAME}
	UNSAFE
	CONFIG ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.yml
)

add_custom_target(release
  COMMAND cp VitaConnectEverywhere.suprx ../release/VitaConnectEverywhere.suprx
  COMMAND cp ../src/modules/kernel/build/kernel.skprx ../release/VitaConnectEverywhere/kernel.skprx
  COMMAND cp ../src/modules/user/build/user.skprx ../release/VitaConnectEverywhere/user.skprx
  COMMAND cp ../src/modules/patch/build/patch.skprx ../release/VitaConnectEverywhere/patch.skprx
  COMMAND cp ../src/modules/usbdevice/build/usbdevice.skprx ../release/VitaConnectEverywhere/usbdevice.skprx
  DEPENDS VitaConnectEverywhere.suprx
  DEPENDS kernel.skprx
  DEPENDS user.skprx
  DEPENDS patch.skprx
  DEPENDS usbdevice.skprx
)

set(PSVITAIP "192.168.0.16" CACHE STRING "PSVita IP (for FTP access)")
set(PARSECOREPATH "/home/BMK/vita-parse-core/main.py" CACHE STRING "vita-parse-core main.py")

add_custom_target(dump
                  COMMAND ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.sh ${PSVITAIP}
                  COMMAND python3 ${PARSECOREPATH} coredump ${CMAKE_PROJECT_NAME})
