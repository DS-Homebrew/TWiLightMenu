#pragma once

/// @brief The Pico Loader API version supported by this header file.
#define PICO_LOADER_API_VERSION     1

/// @brief Enum to specify the drive to boot from.
typedef enum
{
    /// @brief Flashcard through DLDI.
    PLOAD_BOOT_DRIVE_DLDI = 0,

    /// @brief DSi SD card.
    PLOAD_BOOT_DRIVE_DSI_SD = 1,

    /// @brief AGB semihosting on the IS-NITRO-EMULATOR.
    PLOAD_BOOT_DRIVE_AGB_SEMIHOSTING = 2,

    /// @brief Flag to indicate that a multiboot rom needs to be loaded that is already in memory.
    PLOAD_BOOT_DRIVE_MULTIBOOT_FLAG = 1u << 15
} PicoLoaderBootDrive;

/// @brief Struct containing the load params.
typedef struct
{
    /// @brief The path of the rom to load.
    char romPath[256];

    /// @brief The path to the save file to use.
    char savePath[256];

    /// @brief The actual length of the argv arguments buffer.
    u32 argumentsLength;

    /// @brief Argv arguments buffer.
    char arguments[256];
} pload_params_t;

/// @brief Struct representing the header of picoLoader7.bin.
typedef struct
{
    /// @brief Pointer to the Pico Loader arm7 entry point (read-only).
    void* const entryPoint;

    /// @brief Sets the DLDI driver to use.
    void* dldiDriver;

    /// @brief Sets the boot drive. See \see PicoLoaderBootDrive.
    u16 bootDrive;

    /// @brief The supported Pico Loader API version (read-only).
    const u16 apiVersion;

    /// @brief The load params, see \see pload_params_t.
    pload_params_t loadParams;
} pload_header7_t;
