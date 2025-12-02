//==================================================================================
//==================================================================================
// File: iQSPI_FlashMemory.h
// Description: Quad SPI Flash Memory Interface (Abstract Interface)
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"  // Include HAL for QSPI_HandleTypeDef, HAL_StatusTypeDef

//**********************************************************************************
// FlashID Structure
// Description: Contains flash memory identification information
//**********************************************************************************
struct FlashID {
    uint8_t ManufactuerID;  // Manufacturer identification code
    uint8_t MemoryType;     // Memory type code
    uint8_t Capacity;       // Memory capacity code
};

namespace DadDrivers {

//**********************************************************************************
// iQSPI_FlashMemory - Abstract interface for QSPI flash memory devices
//**********************************************************************************
class iQSPI_FlashMemory {
public:
    // =============================================================================
    // Constructor/Destructor
    // =============================================================================

    iQSPI_FlashMemory() = default;                           // Default constructor
    virtual ~iQSPI_FlashMemory() = default;                  // Virtual destructor

    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Configuration Methods
    // -----------------------------------------------------------------------------

    //**********************************************************************************
    // Method: Init
    // Description: Initializes the QSPI interface
    // Parameters:
    //   - phqspi: QSPI handle pointer
    //   - DualMode: Dual mode operation flag
    //   - MemoryAddress: Memory mapped address
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef Init(QSPI_HandleTypeDef* phqspi, bool DualMode = false, uint32_t MemoryAddress = 0x90000000) = 0;

    //**********************************************************************************
    // Method: ModeMemoryMap
    // Description: Configures the QSPI memory in memory-mapped mode
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef ModeMemoryMap() = 0;

    //**********************************************************************************
    // Method: ModeIndirect
    // Description: Configures the QSPI memory in indirect access mode
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef ModeIndirect() = 0;

    // -----------------------------------------------------------------------------
    // Data Transfer Methods
    // -----------------------------------------------------------------------------

    //**********************************************************************************
    // Method: Read
    // Description: Performs a read operation from the QSPI memory
    // Parameters:
    //   - pData: Pointer to data buffer
    //   - Address: Memory address to read from
    //   - NbData: Number of bytes to read
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef Read(uint8_t* pData, uint32_t Address, uint32_t NbData) = 0;

    //**********************************************************************************
    // Method: Write
    // Description: Performs a write operation to the QSPI memory
    // Parameters:
    //   - pData: Pointer to data buffer
    //   - Address: Memory address to write to
    //   - NbData: Number of bytes to write
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef Write(uint8_t* pData, uint32_t Address, uint32_t NbData) = 0;

    // -----------------------------------------------------------------------------
    // Erase Methods
    // -----------------------------------------------------------------------------

    //**********************************************************************************
    // Method: EraseBlock4K
    // Description: Erases a 4 KB sector of the QSPI memory
    // Parameters:
    //   - Address: Address within the 4K sector to erase
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef EraseBlock4K(uint32_t Address) = 0;

    //**********************************************************************************
    // Method: EraseBlock32K
    // Description: Erases a 32 KB block of the QSPI memory
    // Parameters:
    //   - Address: Address within the 32K block to erase
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef EraseBlock32K(uint32_t Address) = 0;

    //**********************************************************************************
    // Method: EraseBlock64K
    // Description: Erases a 64 KB block of the QSPI memory
    // Parameters:
    //   - Address: Address within the 64K block to erase
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef EraseBlock64K(uint32_t Address) = 0;

    //**********************************************************************************
    // Method: EraseChip
    // Description: Erases the entire QSPI memory chip
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef EraseChip() = 0;

    // -----------------------------------------------------------------------------
    // Information Methods
    // -----------------------------------------------------------------------------

    //**********************************************************************************
    // Method: getSize
    // Description: Returns the total size of the memory in bytes
    // Returns: Memory size in bytes
    //**********************************************************************************
    virtual uint32_t getSize() const = 0;

    //**********************************************************************************
    // Method: getFlashID
    // Description: Returns the ID of flash memory
    // Parameters:
    //   - pID: Pointer to FlashID structure to populate
    // Returns: HAL status code
    //**********************************************************************************
    virtual HAL_StatusTypeDef getFlashID(FlashID* pID) = 0;

    // =============================================================================
    // Deleted Methods (Non-copyable)
    // =============================================================================

    iQSPI_FlashMemory(const iQSPI_FlashMemory&) = delete;                    // Copy constructor
    iQSPI_FlashMemory& operator=(const iQSPI_FlashMemory&) = delete;        // Copy assignment operator

};

} // namespace DadDrivers

//***End of file**************************************************************
