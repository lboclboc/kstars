
SET(CMAKE_SYSTEM_NAME ${HOST_PREFIX})
SET(CMAKE_CROSSCOMPILING TRUE)

SET(ANDROID_NDK_ROOT $ENV{ANDROID_NDK})
SET(ANDROID_PLATFORM android-$ENV{ANDROID_API_LEVEL})
SET(ANDROID_ABI armeabi-v7a)
SET(ANDROID_STL gnustl_static)
SET(ANDROID_TOOLCHAIN_NAME armv7-linux-androideabi)

SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

SET(ANDROID ON)
SET(CMAKE_C_COMPILER_TARGET "${ANDROID_TOOLCHAIN_NAME}")
SET(CMAKE_C_COMPILER ${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin/clang)
SET(CMAKE_CXX_COMPILER_TARGET "${ANDROID_TOOLCHAIN_NAME}")
SET(CMAKE_CXX_COMPILER ${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++)
SET(CMAKE_STRIP ${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-strip)
SET(CMAKE_AR ${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar)

SET(CMAKE_C_FLAGS_RELEASE -Os -gline-tables-only -fomit-frame-pointer -fno-strict-aliasing -DNDEBUG -DRELEASE -D_RELEASE)
SET(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
SET(CMAKE_C_FLAGS_DEBUG -O0 -gdwarf-2 -DDEBUG -D_DEBUG)
SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

# Link size optimization: https://github.com/android-ndk/ndk/issues/133#issuecomment-324165329
SET(CMAKE_C_FLAGS "-target ${ANDROID_TOOLCHAIN_NAME} -gcc-toolchain ${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64 -fno-integrated-as")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE -fsigned-char -ffunction-sections -fdata-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -fno-strict-aliasing")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -mfpu=neon -marm -march=armv7-a -mfloat-abi=softfp -mthumb")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID -D__ANDROID__ -D__ANDROID_API__=${ANDROID_NATIVE_API_LEVEL} -D_FORTIFY_SOURCE=2 -fno-short-enums -Wno-c++11-narrowing")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat -Werror=format-security -Wno-unknown-warning-option -Wno-extern-c-compat -Wno-deprecated-register -Wno-gcc-compat")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${ANDROID_NDK_ROOT}/platforms/${ANDROID_PLATFORM}/arch-arm")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isystem ${ANDROID_NDK_ROOT}/platforms/${ANDROID_PLATFORM}/arch-arm/usr/include")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isystem ${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/include")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isystem ${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isystem ${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/include/backward")
SET(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

INCLUDE_DIRECTORIES(${ANDROID_NDK_ROOT}/platforms/${ANDROID_PLATFORM}/arch-arm/usr/include)
INCLUDE_DIRECTORIES(${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/include)
INCLUDE_DIRECTORIES(${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include)
INCLUDE_DIRECTORIES(${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/include/backward)
INCLUDE_DIRECTORIES(${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/lib64/clang/5.0/include)

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fexceptions -frtti -Wno-unknown-warning-option -Wno-unused-local-typedef -Wa,--noexecstack")

SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -gcc-toolchain ${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64 -no-canonical-prefixes")
SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} --sysroot=${ANDROID_NDK_ROOT}/platforms/${ANDROID_PLATFORM}/arch-arm")
SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -fPIE -L${ANDROID_NDK_ROOT}/platforms/${ANDROID_PLATFORM}/arch-arm/usr/lib")
SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-rpath=${ANDROID_NDK_ROOT}/platforms/${ANDROID_PLATFORM}/arch-arm/usr/lib")
SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -lgcc -Wl,--exclude-libs,libgcc.a")
SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} ${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/libsupc++.a")
SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -L${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a -lgnustl_shared")
SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--no-undefined -Wl,-z,relro -Wl,-z,now -march=armv7-a -Wl,--fix-cortex-a8 -Wl,--gc-sections")
SET(CMAKE_CXX_LINK_FLAGS ${CMAKE_C_LINK_FLAGS})
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
