# SPDX-License-Identifier: MIT

idf_component_register(
    SRCS "src/esp_rand_adapter.c"
         "src/esp_liboqs_init.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_hw_support 
    LDFRAGMENTS linker.lf
    PRIV_REQUIRES esp_system
)

target_link_libraries(${COMPONENT_LIB} PRIVATE "-u esp_liboqs_init_include_impl")

# Only build liboqs if enabled in Kconfig
if(CONFIG_LIBOQS_ENABLED)

    message(STATUS "liboqs: Enabled for ${IDF_TARGET}")

    # Architecture translation for ESP32 variants
    if(CONFIG_IDF_TARGET_ARCH_XTENSA)
        # Xtensa is not natively supported by liboqs
        # We'll treat it as a generic embedded target
        message(STATUS "liboqs: Xtensa architecture - using reference implementations")
        set(OQS_PERMIT_UNSUPPORTED_ARCHITECTURE ON)
        set(CMAKE_SYSTEM_PROCESSOR "generic")
    elseif(CONFIG_IDF_TARGET_ARCH_RISCV)
        message(STATUS "liboqs: RISC-V architecture - optimizations available")
        # All ESP32 RISC-V variants are 32-bit
        set(CMAKE_SYSTEM_PROCESSOR "riscv32")
    endif()

    # Set pointer size (all ESP32 variants are 32-bit)
    set(CMAKE_SIZEOF_VOID_P 4)

    # Core liboqs configuration (from Zephyr lines 40-55)
    set(OQS_DIST_BUILD OFF CACHE BOOL "")
    set(OQS_BUILD_ONLY_LIB ON CACHE BOOL "")
    set(OQS_USE_OPENSSL OFF CACHE BOOL "")
    set(OQS_EMBEDDED_BUILD ON CACHE BOOL "")
    set(CMAKE_CROSSCOMPILING ON)
    set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)
    set(SKIP_INSTALL_ALL TRUE)

    # Disable platform features not available in ESP-IDF (from Zephyr lines 48-55)
    set(CMAKE_HAVE_GETENTROPY OFF)
    set(CMAKE_HAVE_ALIGNED_ALLOC OFF)
    set(CMAKE_HAVE_POSIX_MEMALIGN OFF)
    set(CMAKE_HAVE_MEMALIGN OFF)
    set(CMAKE_HAVE_EXPLICIT_BZERO OFF)
    set(CMAKE_HAVE_MEMSET_S OFF)
    set(CC_SUPPORTS_WA_NOEXECSTACK OFF)
    set(LD_SUPPORTS_WL_Z_NOEXECSTACK OFF)

    # Algorithm selection via Kconfig (Zephyr pattern lines 58-154)
    # Use FORCE so Kconfig always wins over any stale CMake cache from liboqs (liboqs defaults many options ON)
    # KEMs
    if(CONFIG_LIBOQS_ENABLE_KEM_ML_KEM)
        set(OQS_ENABLE_KEM_ML_KEM ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_KEM_ML_KEM OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_KEM_BIKE)
        set(OQS_ENABLE_KEM_BIKE ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_KEM_BIKE OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_KEM_FRODOKEM)
        set(OQS_ENABLE_KEM_FRODOKEM ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_KEM_FRODOKEM OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_KEM_NTRUPRIME)
        set(OQS_ENABLE_KEM_NTRUPRIME ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_KEM_NTRUPRIME OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_KEM_CLASSIC_MCELIECE)
        set(OQS_ENABLE_KEM_CLASSIC_MCELIECE ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_KEM_CLASSIC_MCELIECE OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_KEM_HQC)
        set(OQS_ENABLE_KEM_HQC ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_KEM_HQC OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_KEM_KYBER)
        set(OQS_ENABLE_KEM_KYBER ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_KEM_KYBER OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_KEM_NTRU)
        set(OQS_ENABLE_KEM_NTRU ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_KEM_NTRU OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    # Signatures
    if(CONFIG_LIBOQS_ENABLE_SIG_ML_DSA)
        set(OQS_ENABLE_SIG_ML_DSA ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_SIG_ML_DSA OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_SIG_FALCON)
        set(OQS_ENABLE_SIG_FALCON ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_SIG_FALCON OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_SIG_SPHINCS)
        set(OQS_ENABLE_SIG_SPHINCS ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_SIG_SPHINCS OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_SIG_MAYO)
        set(OQS_ENABLE_SIG_MAYO ON CACHE BOOL "From Kconfig" FORCE)
        # Disable MAYO-5 due to size (from Zephyr lines 119-120)
        set(OQS_ENABLE_SIG_mayo_5 OFF CACHE BOOL "" FORCE)
    else()
        set(OQS_ENABLE_SIG_MAYO OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_SIG_UOV)
        set(OQS_ENABLE_SIG_UOV ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_SIG_UOV OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_SIG_CROSS)
        set(OQS_ENABLE_SIG_CROSS ON CACHE BOOL "From Kconfig" FORCE)
        # Disable CROSS variants with large stack usage (from Zephyr lines 133-139)
        set(OQS_ENABLE_SIG_cross_rsdp_128_small OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_cross_rsdp_192_small OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_cross_rsdp_256_balanced OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_cross_rsdp_256_small OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_cross_rsdpg_192_small OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_cross_rsdpg_256_small OFF CACHE BOOL "" FORCE)
    else()
        set(OQS_ENABLE_SIG_CROSS OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_SIG_SNOVA)
        set(OQS_ENABLE_SIG_SNOVA ON CACHE BOOL "From Kconfig" FORCE)
        # Disable large SNOVA variants (from Zephyr lines 145-151)
        set(OQS_ENABLE_SIG_snova_SNOVA_56_25_2 OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_snova_SNOVA_49_11_3 OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_snova_SNOVA_37_8_4 OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_snova_SNOVA_60_10_4 OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_snova_SNOVA_24_5_5 OFF CACHE BOOL "" FORCE)
        set(OQS_ENABLE_SIG_snova_SNOVA_29_6_5 OFF CACHE BOOL "" FORCE)
    else()
        set(OQS_ENABLE_SIG_SNOVA OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    if(CONFIG_LIBOQS_ENABLE_SIG_SLH_DSA)
        set(OQS_ENABLE_SIG_SLH_DSA ON CACHE BOOL "From Kconfig" FORCE)
    else()
        set(OQS_ENABLE_SIG_SLH_DSA OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    # Minimal common: exclude SHA3/SHA2/fips202 from common layer (ML-KEM/ML-DSA use their own)
    if(CONFIG_LIBOQS_MINIMAL_COMMON)
        set(OQS_EMBEDDED_MINIMAL_COMMON ON CACHE BOOL "From Kconfig" FORCE)
        message(STATUS "liboqs: Minimal common layer (SHA3/fips202 excluded)")
    else()
        set(OQS_EMBEDDED_MINIMAL_COMMON OFF CACHE BOOL "From Kconfig" FORCE)
    endif()

    # Override install() and export() commands to prevent liboqs from trying to install
    # This is necessary because ESP-IDF components don't support install(EXPORT)
    macro(install)
        # No-op: prevent liboqs from trying to install
    endmacro()

    macro(export)
        # No-op: prevent liboqs from trying to export targets
    endmacro()

    # Restore install() and export() commands
    macro(install)
        _install(${ARGV})
    endmacro()

    macro(export)
        _export(${ARGV})
    endmacro()

    # Make ESP32 profiling headers available to all liboqs sub-targets
    # We use include_directories() here so it propagates to all subdirectories
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

    # Add the actual liboqs targets (from Zephyr line 157)
    add_subdirectory(liboqs liboqs_build EXCLUDE_FROM_ALL)

    # Disable position independent code for ALL liboqs targets recursively (Zephyr approach line 188-189)
    # Must be done AFTER add_subdirectory to apply to all targets created by liboqs
    # This function recursively finds all targets in liboqs subdirectories and disables PIC
    function(disable_pic_for_all_targets dir)
        get_directory_property(subdirs DIRECTORY ${dir} SUBDIRECTORIES)
        foreach(subdir ${subdirs})
            disable_pic_for_all_targets(${subdir})
        endforeach()

        get_directory_property(targets DIRECTORY ${dir} BUILDSYSTEM_TARGETS)
        foreach(target ${targets})
            if(TARGET ${target})
                get_target_property(target_type ${target} TYPE)
                # Only set properties on compilable targets (not UTILITY targets)
                if(NOT target_type STREQUAL "UTILITY")
                    set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
                    target_compile_options(${target} PRIVATE -fno-pic)
                endif()
            endif()
        endforeach()
    endfunction()

    disable_pic_for_all_targets(liboqs)

    # Add liboqs include directory to the component
    target_include_directories(${COMPONENT_LIB} PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}/liboqs_build/include"
    )

    # Link liboqs to the component
    target_link_libraries(${COMPONENT_LIB} PUBLIC oqs)

    # Make liboqs build before this component
    add_dependencies(${COMPONENT_LIB} oqs)

    # Apply ESP-IDF compiler flags to liboqs with SAFE loop unrolling
    target_compile_options(oqs PRIVATE
        ${CONFIG_COMPILER_OPTIMIZATION_LEVEL}
    )

    message(STATUS "liboqs: Build configuration complete")

endif()
