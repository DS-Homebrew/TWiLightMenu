#pragma once

/// @brief The Pico Loader API version supported by this header file.
#define PICO_LOADER_API_VERSION     3

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

/// @brief Struct representing the API version 2 part of the header of picoLoader7.bin.
typedef struct
{
    /// @brief The path of the rom to return to when exiting an application.
    ///        When this path is not set, no bootstub will be patched into homebrew applications.
    char launcherPath[256];
} pload_header7_v2_t;

/// @brief Struct representing a single Action Replay cheat opcode.
typedef struct
{
    /// @brief The first part of the opcode.
    u32 a;

    /// @brief The second part of the opcode.
    u32 b;
} pload_cheat_opcode_t;

/// @brief Struct representing a single Action Replay cheat.
typedef struct
{
    /// @brief Length of \see opcodes in bytes.
    u32 length;

    /// @brief The cheat opcodes.
    pload_cheat_opcode_t opcodes[1];
} pload_cheat_t;

/// @brief Struct representing one or more cheats. Cheats are adjacent starting from the firstCheat field.
typedef struct
{
    /// @brief Length of this stucture (length field + numberOfCheats field + all cheats).
    u32 length;

    /// @brief The number of cheats.
    u32 numberOfCheats;

    /// @brief The first cheat.
    pload_cheat_t firstCheat;
} pload_cheats_t;

/// @brief Struct representing the API version 3 part of the header of picoLoader7.bin.
typedef struct
{
    /// @brief Pointer to the cheats, or \c nullptr when there are no cheats.
    const pload_cheats_t* cheats;
} pload_header7_v3_t;

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

    /// @brief The API version 2 part of the header. Only access this when \see apiVersion >= 2.
    pload_header7_v2_t v2;

    /// @brief The API version 3 part of the header. Only access this when \see apiVersion >= 3.
    pload_header7_v3_t v3;
} pload_header7_t;
