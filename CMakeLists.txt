cmake_minimum_required(VERSION 3.26)
project(PatchTool C)

set(CMAKE_C_STANDARD 11)

add_executable(${PROJECT_NAME} main.c
        md5.c
        base64.c
        makeBaseDB.c
        date.c
        dbOpt.c
        makePatchDB.c
        stringProcess.c
        applyPatch.c
        config.h
        dbInfo.c
        check.c)
link_directories(${PROJECT_NAME} "D:/msys64/clang64/lib")
target_link_libraries(${PROJECT_NAME} libws2_32.a libssl.a libcrypto.a libcjson.a)
