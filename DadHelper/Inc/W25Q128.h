//==================================================================================
//==================================================================================
// File: W25Q128.h
// Description: Winbond W25Q128JV QSPI Flash Memory Management Header
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once

#include "main.h"
#include "iQSPI_FLashMemory.h"

namespace DadDrivers {

//**********************************************************************************
// Constants and Definitions
//**********************************************************************************

// =============================================================================
// W25Q128JVSQ Command Set
// =============================================================================

enum W25Q128_Commands {
    // -----------------------------------------------------------------------------
    // BASIC SPI COMMANDS
    // -----------------------------------------------------------------------------
    CMD_WRITE_ENABLE                = 0x06,    // Write Enable (WREN)
    CMD_WRITE_ENABLE_VOLATILE       = 0x50,    // Write Enable for Volatile Status Register
    CMD_WRITE_DISABLE               = 0x04,    // Write Disable (WRDI)

    // -----------------------------------------------------------------------------
    // STATUS REGISTER COMMANDS
    // -----------------------------------------------------------------------------
    CMD_READ_STATUS_REG1            = 0x05,    // Read Status Register-1 (RDSR1)
    CMD_READ_STATUS_REG2            = 0x35,    // Read Status Register-2 (RDSR2)
    CMD_READ_STATUS_REG3            = 0x15,    // Read Status Register-3 (RDSR3)
    CMD_WRITE_STATUS_REG1           = 0x01,    // Write Status Register-1 (WRSR1)
    CMD_WRITE_STATUS_REG2           = 0x31,    // Write Status Register-2 (WRSR2)
    CMD_WRITE_STATUS_REG3           = 0x11,    // Write Status Register-3 (WRSR3)

    // -----------------------------------------------------------------------------
    // DEVICE IDENTIFICATION COMMANDS
    // -----------------------------------------------------------------------------
    CMD_READ_ID                     = 0xAB,    // Read ID / Release Power Down (RDID/RDPD)
    CMD_READ_JEDEC_ID               = 0x9F,    // Read JEDEC ID (RDJDID)
    CMD_READ_MANUFACTURER_ID        = 0x90,    // Read Manufacturer & Device ID (RDMDID)
    CMD_READ_MANUFACTURER_ID_DUAL   = 0x92,    // Read Manufacturer & Device ID Dual I/O
    CMD_READ_MANUFACTURER_ID_QUAD   = 0x94,    // Read Manufacturer & Device ID Quad I/O
    CMD_READ_UNIQUE_ID              = 0x4B,    // Read Unique ID (RDUID)
    CMD_READ_SFDP                   = 0x5A,    // Read SFDP Register (RDSFDP)

    // -----------------------------------------------------------------------------
    // READ COMMANDS
    // -----------------------------------------------------------------------------
    CMD_READ_NORMAL                 = 0x03,    // Normal Read (READ)
    CMD_READ_FAST                   = 0x0B,    // Fast Read (FAST_READ)
    CMD_READ_FAST_DUAL_OUT          = 0x3B,    // Fast Read Dual Output (DREAD)
    CMD_READ_FAST_DUAL_IO           = 0xBB,    // Fast Read Dual I/O (2READ)
    CMD_READ_FAST_QUAD_OUT          = 0x6B,    // Fast Read Quad Output (QREAD)
    CMD_READ_FAST_QUAD_IO           = 0xEB,    // Fast Read Quad I/O (4READ)
    CMD_SET_BURST_WITH_WRAP         = 0x77,    // Set Burst with Wrap (SBW)

    // -----------------------------------------------------------------------------
    // PROGRAM COMMANDS
    // -----------------------------------------------------------------------------
    CMD_PAGE_PROGRAM                = 0x02,    // Page Program (PP)
    CMD_QUAD_PAGE_PROGRAM           = 0x32,    // Quad Input Page Program (QPP)

    // -----------------------------------------------------------------------------
    // ERASE COMMANDS
    // -----------------------------------------------------------------------------
    CMD_SECTOR_ERASE                = 0x20,    // Sector Erase 4KB (SE)
    CMD_BLOCK_ERASE_32K             = 0x52,    // Block Erase 32KB (BE32)
    CMD_BLOCK_ERASE_64K             = 0xD8,    // Block Erase 64KB (BE64)
    CMD_CHIP_ERASE                  = 0xC7,    // Chip Erase (CE) - Primary
    CMD_CHIP_ERASE_ALT              = 0x60,    // Chip Erase (CE) - Alternative

    // -----------------------------------------------------------------------------
    // SUSPEND & RESUME COMMANDS
    // -----------------------------------------------------------------------------
    CMD_ERASE_PROGRAM_SUSPEND       = 0x75,    // Erase/Program Suspend (EPS)
    CMD_ERASE_PROGRAM_RESUME        = 0x7A,    // Erase/Program Resume (EPR)

    // -----------------------------------------------------------------------------
    // POWER MANAGEMENT COMMANDS
    // -----------------------------------------------------------------------------
    CMD_POWER_DOWN                  = 0xB9,    // Deep Power Down (DP)
    CMD_RELEASE_POWER_DOWN          = 0xAB,    // Release Power Down (RPD) - Same as READ_ID

    // -----------------------------------------------------------------------------
    // RESET COMMANDS
    // -----------------------------------------------------------------------------
    CMD_ENABLE_RESET                = 0x66,    // Enable Reset (RSTEN)
    CMD_RESET_DEVICE                = 0x99,    // Reset Device (RST)

    // -----------------------------------------------------------------------------
    // SECURITY REGISTER COMMANDS
    // -----------------------------------------------------------------------------
    CMD_ERASE_SECURITY_REG          = 0x44,    // Erase Security Register (ESR)
    CMD_PROGRAM_SECURITY_REG        = 0x42,    // Program Security Register (PSR)
    CMD_READ_SECURITY_REG           = 0x48,    // Read Security Register (RSR)

    // -----------------------------------------------------------------------------
    // INDIVIDUAL BLOCK/SECTOR LOCK COMMANDS
    // -----------------------------------------------------------------------------
    CMD_INDIVIDUAL_BLOCK_LOCK       = 0x36,    // Individual Block Lock (IBL)
    CMD_INDIVIDUAL_BLOCK_UNLOCK     = 0x39,    // Individual Block Unlock (IBU)
    CMD_READ_BLOCK_LOCK             = 0x3D,    // Read Block Lock (RBL)
    CMD_GLOBAL_BLOCK_LOCK           = 0x7E,    // Global Block Lock (GBL)
    CMD_GLOBAL_BLOCK_UNLOCK         = 0x98,    // Global Block Unlock (GBU)

    // -----------------------------------------------------------------------------
    // NO OPERATION
    // -----------------------------------------------------------------------------
    CMD_NOP                         = 0x00     // No Operation (NOP)
};

//**********************************************************************************
// Register Structures
//**********************************************************************************

// =============================================================================
// Status Register 1 Structure
// =============================================================================
union W25Q128_StatusReg1 {
    struct {
        uint8_t BUSY  : 1;  // Bit 0: Busy (1=Busy, 0=Ready)
        uint8_t WEL   : 1;  // Bit 1: Write Enable Latch (1=Enabled)
        uint8_t BP0   : 1;  // Bit 2: Block Protection Bit 0
        uint8_t BP1   : 1;  // Bit 3: Block Protection Bit 1
        uint8_t BP2   : 1;  // Bit 4: Block Protection Bit 2
        uint8_t TB    : 1;  // Bit 5: Top/Bottom Block Protect
        uint8_t SEC   : 1;  // Bit 6: Sector/Block Protect
        uint8_t SRP0  : 1;  // Bit 7: Status Register Protect 0
    };
    uint8_t octet;          // Full byte representation
};

// =============================================================================
// Status Register 2 Structure
// =============================================================================
union W25Q128_StatusReg2 {
    struct {
        uint8_t SRP1  : 1;  // Bit 0: Status Register Protect 1
        uint8_t QE    : 1;  // Bit 1: Quad Enable (1=Quad mode enabled)
        uint8_t Reserved : 1;  // Bit 2: Reserved for Future Use
        uint8_t LB1   : 1;  // Bit 3: Security Register Lock Bit 1
        uint8_t LB2   : 1;  // Bit 4: Security Register Lock Bit 2
        uint8_t LB3   : 1;  // Bit 5: Security Register Lock Bit 3
        uint8_t CMP   : 1;  // Bit 6: Complement Protect
        uint8_t SUS   : 1;  // Bit 7: Suspend Status
    };
    uint8_t octet;          // Full byte representation
};

// =============================================================================
// Status Register 3 Structure
// =============================================================================
union W25Q128_StatusReg3 {
    struct {
        uint8_t RFU1  : 1;  // Bit 0: Reserved for Future Use
        uint8_t RFU2  : 1;  // Bit 1: Reserved for Future Use
        uint8_t WPS   : 1;  // Bit 2: Write Protect Selection
        uint8_t RFU3  : 1;  // Bit 3: Reserved for Future Use
        uint8_t RFU4  : 1;  // Bit 4: Reserved for Future Use
        uint8_t DRV0  : 1;  // Bit 5: Output Drive Strength 0
        uint8_t DRV1  : 1;  // Bit 6: Output Drive Strength 1
        uint8_t HOLD_RST : 1; // Bit 7: /HOLD or /RESET Function
    };
    uint8_t octet;          // Full byte representation
};

//**********************************************************************************
// Device Configuration Constants
//**********************************************************************************

#define W25Q128_MANUFACTURER_ID     0xEF        // Winbond Manufacturer ID
#define W25Q128_DEVICE_ID_JV_IQ     0x4018      // Device ID for W25Q128JV-IQ/JQ variant
#define W25Q128_DEVICE_ID_JV_IM     0x7018      // Device ID for W25Q128JV-IM/JM variant
#define W25Q128_DEVICE_ID_LEGACY    0x17        // Legacy Device ID (ABh, 90h, 92h, 94h)

#define W25Q128_PAGE_SIZE           256         // Page size in bytes
#define W25Q128_SECTOR_SIZE         4096        // Sector size in bytes (4KB)
#define W25Q128_BLOCK_32K_SIZE      32768       // 32KB block size in bytes
#define W25Q128_BLOCK_64K_SIZE      65536       // 64KB block size in bytes
#define W25Q128_TOTAL_SIZE          16777216    // Total size in bytes (16MB)
#define W25Q128_TOTAL_PAGES         65536       // Total number of pages
#define W25Q128_TOTAL_SECTORS       4096        // Total number of 4KB sectors
#define W25Q128_TOTAL_BLOCKS_64K    256         // Total number of 64KB blocks

static constexpr uint32_t DEFAULT_TIMEOUT = 5000;       // Default operation timeout in ms
static constexpr uint32_t ERASE_4K_TIMEOUT = 10000;     // 4KB erase timeout in ms
static constexpr uint32_t ERASE_32K_TIMEOUT = 15000;    // 32KB erase timeout in ms
static constexpr uint32_t ERASE_64K_TIMEOUT = 20000;    // 64KB erase timeout in ms
static constexpr uint32_t CHIP_ERASE_TIMEOUT = 120000;  // Chip erase timeout in ms

//**********************************************************************************
// cW25Q128 Class Declaration
//**********************************************************************************

// =============================================================================
// cW25Q128 Class
//
// Implements the iQSPI_FlashMemory interface for the Winbond W25Q128JV chip.
//
// This class supports full 4-line read/write operations and includes:
// - Initialization with software reset
// - Quad mode configuration
// - Indirect and memory-mapped mode support
// - Sector/block/chip erase functionality
// =============================================================================
class cW25Q128 : public iQSPI_FlashMemory {
public:
    // -----------------------------------------------------------------------------
    // Constructor/Destructor
    // -----------------------------------------------------------------------------
    cW25Q128();
    virtual ~cW25Q128() = default;

    // =============================================================================
    // Public Interface Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Initializes the QSPI interface and the flash memory.
    // Performs a reset and enables Quad mode.
    // Parameters:
    //   phqspi: Pointer to QSPI handle
    //   DualMode: Enable dual flash mode (two flash chips)
    //   MemoryMappedBaseAddress: Base address for memory mapping
    HAL_StatusTypeDef Init(QSPI_HandleTypeDef* phqspi, bool DualMode = false,
                          uint32_t MemoryMappedBaseAddress = 0x90000000) override;

    // -----------------------------------------------------------------------------
    // Configures memory-mapped mode so that the CPU can access the flash
    // memory directly without issuing commands.
    HAL_StatusTypeDef ModeMemoryMap() override;

    // -----------------------------------------------------------------------------
    // Reverts the flash memory to indirect mode for standard read/write commands.
    HAL_StatusTypeDef ModeIndirect() override;

    // -----------------------------------------------------------------------------
    // Reads data from the flash using Quad I/O mode (4 lines address and data).
    // Parameters:
    //   pData: Pointer to data buffer
    //   MappedAddress: Memory mapped address to read from
    //   NbData: Number of bytes to read
    HAL_StatusTypeDef Read(uint8_t* pData, uint32_t MappedAddress, uint32_t NbData) override;

    // -----------------------------------------------------------------------------
    // Writes data to the flash memory using Quad Page Program.
    // Parameters:
    //   pData: Pointer to data to write
    //   MappedAddress: Memory mapped address to write to
    //   NbData: Number of bytes to write
    HAL_StatusTypeDef Write(uint8_t* pData, uint32_t MappedAddress, uint32_t NbData) override;

    // -----------------------------------------------------------------------------
    // Erases a 4KB or 8KB (DualMode) at the given address.
    // Parameters:
    //   MappedAddress: Memory mapped address of sector to erase
    HAL_StatusTypeDef EraseBlock4K(uint32_t MappedAddress) override;

    // -----------------------------------------------------------------------------
    // Erases a 32 KB or 64KB (DualMode) at the given address.
    // Parameters:
    //   MappedAddress: Memory mapped address of block to erase
    HAL_StatusTypeDef EraseBlock32K(uint32_t MappedAddress) override;

    // -----------------------------------------------------------------------------
    // Erases a 64 KB or 128KB(DualMode) at the given address.
    // Parameters:
    //   MappedAddress: Memory mapped address of block to erase
    HAL_StatusTypeDef EraseBlock64K(uint32_t MappedAddress) override;

    // -----------------------------------------------------------------------------
    // Erases the entire flash chip.
    HAL_StatusTypeDef EraseChip() override;

    // -----------------------------------------------------------------------------
    // Returns the total size of the flash memory in bytes.
    uint32_t getSize() const override;

    // -----------------------------------------------------------------------------
    // Returns the ID of flash memory.
    // Parameters:
    //   pID: Pointer to FlashID structure to store the device ID
    HAL_StatusTypeDef getFlashID(FlashID* pID) override;

private:
    // =============================================================================
    // Private Member Variables
    // =============================================================================
    QSPI_HandleTypeDef* m_pQSPI;                  // Pointer to the HAL QSPI handle
    uint32_t m_MemoryMappedBaseAddress;           // Base address for memory-mapped access
    bool m_DualMode;                              // Flag for dual memory mode operation
    bool m_MappedMode;                            // Flag to indicate if the memory is in mapped mode

    // =============================================================================
    // Private Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Enables write operations on the memory.
    HAL_StatusTypeDef WriteEnable();

    // -----------------------------------------------------------------------------
    // Waits until the Write-In-Progress bit is cleared.
    // Parameters:
    //   Timeout: Maximum time to wait in milliseconds
    HAL_StatusTypeDef WaitWhileBusy(uint32_t Timeout = DEFAULT_TIMEOUT);
};

} // namespace DadDrivers

//***End of file**************************************************************
