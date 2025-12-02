//==================================================================================
//==================================================================================
// File: W25Q128.cpp
// Description: Winbond W25Q128JV QSPI Flash Memory Management
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#include "W25Q128.h"

namespace DadDrivers {

//**********************************************************************************
// Macros
//**********************************************************************************

// Macro to validate memory mapped address and convert to flash address
#define VALID_ADDRESS(AddrFlash, AddrMemoryMap)\
    if((AddrMemoryMap < m_MemoryMappedBaseAddress) || (AddrMemoryMap > (m_MemoryMappedBaseAddress + getSize()))){\
        return HAL_ERROR;\
    }else{\
        AddrFlash = AddrMemoryMap - m_MemoryMappedBaseAddress;\
    }

// Macro to enter command mode (switch from memory mapped to indirect mode if needed)
#define ENTER_CMD\
    bool InitialMapState = m_MappedMode;\
    if(m_MappedMode){\
        if((Result = ModeIndirect()) != HAL_OK){\
            return Result;\
        }\
    }

// Macro to exit command mode (restore memory mapped mode if it was initially active)
#define EXIT_CMD\
    if(InitialMapState == true){\
        Result = ModeMemoryMap();\
    }

// Macro to check HAL function results and return on error
#define CHECK_RESULT(Cmd)\
    if((Result = Cmd) != HAL_OK){\
        return Result;\
    }

// =============================================================================
// Constructor/Destructor
// =============================================================================

// -----------------------------------------------------------------------------
// Constructor - Initialize member variables
cW25Q128::cW25Q128(){
    m_pQSPI = nullptr;                    // QSPI handle pointer
    m_MemoryMappedBaseAddress = 0;        // Memory mapped base address
    m_MappedMode = false;                 // Memory mapped mode flag
    m_DualMode = false;                   // Dual flash mode flag
}

// =============================================================================
// Public Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize the QSPI memory: performs software reset and enables Quad mode
// Parameters:
//   phqspi: Pointer to QSPI handle
//   DualMode: Enable dual flash mode (two flash chips)
//   MemoryMappedBaseAddress: Base address for memory mapping
HAL_StatusTypeDef cW25Q128::Init(QSPI_HandleTypeDef* phqspi, bool DualMode, uint32_t MemoryMappedBaseAddress){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    QSPI_CommandTypeDef cmd;              // QSPI command structure

    // Store configuration parameters
    m_pQSPI = phqspi;                     // Store QSPI handle
    m_DualMode = DualMode;                // Store dual mode setting
    m_MemoryMappedBaseAddress = MemoryMappedBaseAddress; // Store memory mapped base address
    m_MappedMode = false;                 // Start in indirect mode

    // Perform software reset to ensure clean state
    cmd = {0};
    cmd.Instruction = CMD_ENABLE_RESET;   // Enable Reset command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    cmd.Instruction = CMD_RESET_DEVICE;   // Reset Device command
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    HAL_Delay(100);                       // Wait for reset to complete

    // Set Output Driver Strength and protected flag
    W25Q128_StatusReg3 StatusReg3[4] = {}; // Status register 3 buffer

    // Read current status register 3
    cmd = {0};
    cmd.Instruction = CMD_READ_STATUS_REG3; // Read Status Register-3
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    if(m_DualMode){
        cmd.NbData = 2;                   // Read from both chips in dual mode
    }else{
        cmd.NbData = 1;                   // Single chip read
    }
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Receive(m_pQSPI, (uint8_t *)StatusReg3, DEFAULT_TIMEOUT));

    // Set driver strength bits
    StatusReg3[0].DRV0 = 0;               // Configure driver strength
    StatusReg3[0].DRV1 = 0;               // Configure driver strength
    StatusReg3[0].WPS = 0;                // Clear write protect status
    StatusReg3[1] = StatusReg3[0];        // Copy for dual mode

    // Enable write operations
    cmd = {0};
    cmd.Instruction = CMD_WRITE_ENABLE;   // Write Enable command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    // Write modified status register 3
    cmd.Instruction = CMD_WRITE_STATUS_REG3; // Write Status Register-3
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;                       // Write one byte
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Transmit(m_pQSPI, (uint8_t *)StatusReg3, DEFAULT_TIMEOUT));

    CHECK_RESULT(WaitWhileBusy());        // Wait for write operation to complete

    // Enable quad mode
    W25Q128_StatusReg2 StatusReg2[4] = {}; // Status register 2 buffer

    // Read current status register 2
    cmd = {0};
    cmd.Instruction = CMD_READ_STATUS_REG2; // Read Status Register-2
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    if(m_DualMode){
        cmd.NbData = 2;                   // Read from both chips in dual mode
    }else{
        cmd.NbData = 1;                   // Single chip read
    }
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Receive(m_pQSPI, (uint8_t *)StatusReg2, DEFAULT_TIMEOUT));

    // Set QE (Quad Enable) bit
    StatusReg2[0].QE = 1;                 // Enable quad mode
    StatusReg2[1] = StatusReg2[0];        // Copy for dual mode

    // Enable write operations
    cmd = {0};
    cmd.Instruction = CMD_WRITE_ENABLE;   // Write Enable command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    // Write modified status register 2
    cmd = {0};
    cmd.Instruction = CMD_WRITE_STATUS_REG2; // Write Status Register-2
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;                       // Write one byte
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Transmit(m_pQSPI, (uint8_t *)StatusReg2, DEFAULT_TIMEOUT));

    CHECK_RESULT(WaitWhileBusy());        // Wait for write operation to complete

    CHECK_RESULT(ModeMemoryMap());        // Switch to memory mapped mode
    return Result;
}

// -----------------------------------------------------------------------------
// Configure the memory in memory-mapped mode for direct CPU access
HAL_StatusTypeDef cW25Q128::ModeMemoryMap(){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result

    if(m_MappedMode == true) return Result; // Already in memory mapped mode

    // Send NOP command to prepare for mode change
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_NOP;            // No operation command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    // Configure memory mapped settings
    QSPI_MemoryMappedTypeDef mem_mapped_cfg = {0};
    mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    // Setup fast read quad I/O command (4-4-4 mode)
    cmd = {0};
    cmd.Instruction = CMD_READ_FAST_QUAD_IO; // Fast Read Quad I/O command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    cmd.AlternateBytes = 0xF0;            // Mode bits for continuous read
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 4;                  // Required dummy cycles for fast read
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    // Enable memory mapped mode
    CHECK_RESULT(HAL_QSPI_MemoryMapped(m_pQSPI, &cmd, &mem_mapped_cfg));
    m_MappedMode = true;                  // Update mode flag

    // Invalidate CPU caches for memory mapped region
    if ((SCB->CCR & (1 << 17)) != 0) {
        SCB_InvalidateICache_by_Addr((uint32_t*)m_MemoryMappedBaseAddress, getSize());
    }
    if ((SCB->CCR & (1 << 16)) != 0) {
        SCB_InvalidateDCache_by_Addr((uint32_t*)m_MemoryMappedBaseAddress, getSize());
    }

    return Result;
}

// -----------------------------------------------------------------------------
// Configure the memory back to indirect mode for command operations
HAL_StatusTypeDef cW25Q128::ModeIndirect(){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result

    if(m_MappedMode == false) return Result; // Already in indirect mode

    CHECK_RESULT(HAL_QSPI_Abort(m_pQSPI)); // Abort memory mapped mode
    m_MappedMode = false;                 // Update mode flag

    // Send NOP command after mode change
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_NOP;            // No operation command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    return Result;
}

// -----------------------------------------------------------------------------
// Read data from flash memory using quad I/O mode (4-4-4)
// Parameters:
//   pData: Pointer to buffer to store read data
//   MappedAddress: Memory mapped address to read from
//   NbData: Number of bytes to read
HAL_StatusTypeDef cW25Q128::Read(uint8_t* pData, uint32_t MappedAddress, uint32_t NbData){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    ENTER_CMD;                            // Enter command mode if needed

    uint32_t Address;                     // Flash memory address
    VALID_ADDRESS(Address, MappedAddress); // Validate and convert address

    // Setup fast read quad I/O command
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_READ_FAST_QUAD_IO; // Fast Read Quad I/O command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = Address;                // Target flash address
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    cmd.AlternateBytes = 0xF0;            // Mode bits
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 4;                  // Dummy cycles for timing
    cmd.NbData = NbData;                  // Number of bytes to read
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    // Send command and receive data
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Receive(m_pQSPI, pData, DEFAULT_TIMEOUT));

    EXIT_CMD;                             // Restore previous mode
    return Result;
}

// -----------------------------------------------------------------------------
// Write data to flash memory with page boundary handling
// Flash memory must be written in pages (256 bytes max per page)
// Parameters:
//   pData: Pointer to data to write
//   MappedAddress: Memory mapped address to write to
//   NbData: Number of bytes to write
HAL_StatusTypeDef cW25Q128::Write(uint8_t* pData, uint32_t MappedAddress, uint32_t NbData){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    ENTER_CMD;                            // Enter command mode if needed

    uint32_t Address;                     // Flash memory address
    VALID_ADDRESS(Address, MappedAddress); // Validate and convert address
    if ((Address + NbData) > getSize()) return HAL_ERROR; // Check bounds

    // Setup quad page program command
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_QUAD_PAGE_PROGRAM; // Quad Page Program command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_4_LINES;

    // Write data in page-aligned chunks
    while (NbData > 0)
    {
        // Calculate remaining bytes in current page
        uint32_t page_size = W25Q128_PAGE_SIZE - (Address % W25Q128_PAGE_SIZE);
        uint32_t write_size = (NbData < page_size) ? NbData : page_size;

        CHECK_RESULT(WriteEnable());      // Enable write operations

        cmd.Address = Address;            // Set target address
        cmd.NbData = write_size;          // Set data size for this page

        // Send command and transmit data
        CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
        CHECK_RESULT(HAL_QSPI_Transmit(m_pQSPI, pData, DEFAULT_TIMEOUT));

        CHECK_RESULT(WaitWhileBusy());    // Wait for write completion

        // Update pointers and counters for next page
        Address += write_size;            // Move to next address
        pData += write_size;              // Move data pointer
        NbData -= write_size;             // Decrement remaining bytes
    }

    EXIT_CMD;                             // Restore previous mode
    return Result;
}

// -----------------------------------------------------------------------------
// Erases a 4KB or 8KB (DualMode) at the given address.
// Parameters:
//   MappedAddress: Memory mapped address of sector to erase
HAL_StatusTypeDef cW25Q128::EraseBlock4K(uint32_t MappedAddress){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    ENTER_CMD;                            // Enter command mode if needed

    uint32_t Address;                     // Flash memory address
    VALID_ADDRESS(Address, MappedAddress); // Validate and convert address

    CHECK_RESULT(WriteEnable());          // Enable write operations

    // Setup 4KB sector erase command
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_SECTOR_ERASE;   // 4KB sector erase command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = Address;                // Target address to erase

    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT)); // Execute erase
    CHECK_RESULT(WaitWhileBusy(ERASE_4K_TIMEOUT)); // Wait for erase completion

    EXIT_CMD;                             // Restore previous mode
    return Result;
}

// -----------------------------------------------------------------------------
// Erases a 32 KB or 64KB (DualMode) at the given address.
// Parameters:
//   MappedAddress: Memory mapped address of block to erase
HAL_StatusTypeDef cW25Q128::EraseBlock32K(uint32_t MappedAddress){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    ENTER_CMD;                            // Enter command mode if needed

    uint32_t Address;                     // Flash memory address
    VALID_ADDRESS(Address, MappedAddress); // Validate and convert address

    CHECK_RESULT(WriteEnable());          // Enable write operations

    // Setup 32KB block erase command
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_BLOCK_ERASE_32K; // 32KB block erase command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = Address;                // Target address to erase
    cmd.DataMode = QSPI_DATA_NONE;

    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT)); // Execute erase
    CHECK_RESULT(WaitWhileBusy(ERASE_32K_TIMEOUT)); // Wait for erase completion

    EXIT_CMD;                             // Restore previous mode
    return Result;
}

// -----------------------------------------------------------------------------
// Erase a 64KB or 128KB(DualMode) of the QSPI memory
// Parameters:
//   MappedAddress: Memory mapped address of block to erase
HAL_StatusTypeDef cW25Q128::EraseBlock64K(uint32_t MappedAddress){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    ENTER_CMD;                            // Enter command mode if needed

    uint32_t Address;                     // Flash memory address
    VALID_ADDRESS(Address, MappedAddress); // Validate and convert address

    CHECK_RESULT(WriteEnable());          // Enable write operations

    // Setup 64KB block erase command
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_BLOCK_ERASE_64K; // 64KB block erase command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = Address;                // Target address to erase
    cmd.DataMode = QSPI_DATA_NONE;

    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT)); // Execute erase
    CHECK_RESULT(WaitWhileBusy(ERASE_64K_TIMEOUT)); // Wait for erase completion

    EXIT_CMD;                             // Restore previous mode
    return Result;
}

// -----------------------------------------------------------------------------
// Erase the entire chip (all memory)
HAL_StatusTypeDef cW25Q128::EraseChip(){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    ENTER_CMD;                            // Enter command mode if needed

    CHECK_RESULT(WriteEnable());          // Enable write operations

    // Setup chip erase command
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_CHIP_ERASE;     // Chip erase command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;

    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT)); // Execute erase
    CHECK_RESULT(WaitWhileBusy(CHIP_ERASE_TIMEOUT)); // Wait for erase completion

    EXIT_CMD;                             // Restore previous mode
    return Result;
}

// -----------------------------------------------------------------------------
// Read the manufacturer and device ID from flash memory
// Parameters:
//   pID: Pointer to store the device ID
HAL_StatusTypeDef cW25Q128::getFlashID(FlashID* pID){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    ENTER_CMD;                            // Enter command mode if needed

    // Setup read manufacturer/device ID command
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_READ_JEDEC_ID;  // Read JEDEC ID command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;

    if(m_DualMode){
        cmd.NbData = 6;                   // Read from both chips
    }else{
        cmd.NbData = 3;                   // Single chip
    }

    uint8_t Data[6] = {};                 // Buffer for ID data

    // Execute command and receive ID data
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Receive(m_pQSPI, Data, DEFAULT_TIMEOUT));

    // Extract device ID information
    if(m_DualMode){
        // Check if both chips have same ID
        if((Data[4] != Data[5]) || (Data[2] != Data[3]) || (Data[0] != Data[1])){
            return HAL_ERROR;              // IDs do not match, error
        }
        pID->ManufactuerID = Data[0];     // Store manufacturer ID
        pID->ManufactuerID = Data[2];     // Store device ID part 1
        pID->ManufactuerID = Data[4];     // Store device ID part 2
    }else{
        pID->ManufactuerID = Data[0];     // Store manufacturer ID
        pID->ManufactuerID = Data[1];     // Store device ID part 1
        pID->ManufactuerID = Data[2];     // Store device ID part 2
    }

    EXIT_CMD;                             // Restore previous mode
    return Result;
}

// -----------------------------------------------------------------------------
// Get the total size of the flash memory in bytes
uint32_t cW25Q128::getSize() const
{
    if(m_DualMode){
        return W25Q128_TOTAL_SIZE * 2;    // Total size in bytes (2*16MB for 2*W25Q128)
    }else{
        return W25Q128_TOTAL_SIZE;        // Single chip size (16MB)
    }
}

// =============================================================================
// Protected Methods
// =============================================================================

// =============================================================================
// Private Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Enable write operations by sending Write Enable command
HAL_StatusTypeDef cW25Q128::WriteEnable(){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result

    // Enable write operation
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_WRITE_ENABLE;   // Write Enable command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    return Result;
}

// -----------------------------------------------------------------------------
// Wait until the memory is no longer busy (WIP = 0 in status register)
// Parameters:
//   Timeout: Maximum time to wait in milliseconds
HAL_StatusTypeDef cW25Q128::WaitWhileBusy(uint32_t Timeout){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    QSPI_CommandTypeDef cmd = {0};
    W25Q128_StatusReg1 Data[4] = {};      // Status register data buffer

    // Setup read status register command
    cmd.Instruction = CMD_READ_STATUS_REG1; // Read Status Register-1 command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;

    if(m_DualMode){
        cmd.NbData = 2;                   // Read from both flash chips
    }else{
        cmd.NbData = 1;                   // Single flash chip
    }

    // Poll status register until not busy
    uint32_t tickstart = HAL_GetTick();   // Get start time
    uint8_t Busy = 0;                     // Busy status flag
    do {
        CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
        CHECK_RESULT(HAL_QSPI_Receive(m_pQSPI, (uint8_t *)Data, DEFAULT_TIMEOUT));

        if ((HAL_GetTick() - tickstart) > Timeout) return HAL_TIMEOUT; // Check timeout

        // Check busy status based on mode
        if(m_DualMode){
            Busy = (Data[0].BUSY | Data[1].BUSY); // Either chip busy
        }else{
            Busy = Data[0].BUSY;          // Single chip busy status
        }

    } while (Busy == 1);                  // Continue while busy

    return Result;
}

} // namespace DadDrivers

//***End of file**************************************************************
