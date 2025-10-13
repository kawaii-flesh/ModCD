# cmake -B build -DPLATFORM_SWITCH=ON
cmake_minimum_required(VERSION 3.10)

set(EXTERN ${CMAKE_CURRENT_SOURCE_DIR}/extern)
set(BOREALIS_LIBRARY ${EXTERN}/borealis/library)
set(JSON_LIBRARY ${EXTERN}/json)
set(FSLIB_LIBRARY ${EXTERN}/fslib/Switch)

include(${BOREALIS_LIBRARY}/cmake/commonOption.cmake)
include(${BOREALIS_LIBRARY}/cmake/toolchain.cmake)

project(ModCD)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "2")
set(VERSION_ALTER "1")
set(VERSION_BUILD "0")
set(PROJECT_STAGE "beta")
set(PROJECT_AUTHOR "kawaii-flesh")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/icon.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(FULL_VERSION v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}-${PROJECT_STAGE})
set(AGENT "${PROJECT_NAME} ${FULL_VERSION}")
add_definitions(-DMODCD_AGENT="${AGENT}" -DMODCD_FULL_VERSION="${FULL_VERSION}")

add_subdirectory(${BOREALIS_LIBRARY})
add_subdirectory(${JSON_LIBRARY})

file(GLOB_RECURSE MAIN_SRC src/*.cpp)
set(SOURCES
    src/app/config.cpp
    src/app/mcds.cpp
    src/app/modCD.cpp
    src/app/repositoryProviderOffline.cpp
    src/app/repositoryProviderOnline.cpp
    src/app/updater.cpp
    src/core/game.cpp
    src/core/mergedInfo.cpp
    src/core/mod.cpp
    src/core/modInfo.cpp
    src/core/repository.cpp
    src/core/stub.cpp
    src/front/activity/bigPictureActivity.cpp
    src/front/activity/gamesListActivity.cpp
    src/front/activity/modActivity.cpp
    src/front/activity/modsListActivity.cpp
    src/front/activity/screenshotsActivity.cpp
    src/front/activity/stubActivity.cpp
    src/front/app.cpp
    src/front/asyncLock.cpp
    src/front/styles/colors.cpp
    src/front/view/gamesListView.cpp
    src/front/view/gameTileView.cpp
    src/front/view/baView.cpp
    src/front/view/lineEdit.cpp
    src/front/view/loadingView.cpp
    src/front/view/modsListView.cpp
    src/front/view/modTileView.cpp
    src/front/view/modView.cpp
    src/front/view/screenshotsView.cpp
    src/front/view/toastView.cpp
    src/utils/http.cpp
    src/utils/localization.cpp
    src/utils/utils.cpp
    src/utils/flagsManager.cpp
)

list(APPEND MAIN_SRC ${BOREALIS_LIBRARY}/lib/platforms/switch/switch_wrapper.c)

program_target(${PROJECT_NAME} "${MAIN_SRC}")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

add_custom_target(FsLib_build
    COMMAND make FsLib
    WORKING_DIRECTORY ${FSLIB_LIBRARY})
add_library(FsLib STATIC IMPORTED)
add_dependencies(FsLib FsLib_build)
set_target_properties(FsLib PROPERTIES IMPORTED_LOCATION ${FSLIB_LIBRARY}/FsLib/lib/libFsLib.a)

add_custom_target(${PROJECT_NAME}.nro DEPENDS ${PROJECT_NAME}
    COMMAND ${NX_NACPTOOL_EXE} --create "${PROJECT_NAME}" "${PROJECT_AUTHOR}" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}" ${PROJECT_NAME}.nacp --titleid=${PROJECT_TITLEID}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES} ${CMAKE_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources/font
    COMMAND ${NX_ELF2NRO_EXE} ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro --icon=${PROJECT_ICON} --nacp=${PROJECT_NAME}.nacp --romfsdir=${CMAKE_BINARY_DIR}/resources
)

target_include_directories(${PROJECT_NAME} PRIVATE ${JSON_LIBRARY}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${FSLIB_LIBRARY}/FsLib/include)

target_compile_options(${PROJECT_NAME} PRIVATE
    -O3
    -march=armv8-a
    -funroll-loops
    -ftree-vectorize
    -ffunction-sections
    -fdata-sections
    -pipe
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    curl
    nx
    z
    minizip
    borealis
    FsLib
    -flto
    -fomit-frame-pointer
)
