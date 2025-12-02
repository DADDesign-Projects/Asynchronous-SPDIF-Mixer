//==================================================================================
// File: cFlashManager.h
// Description: Flash Manager for persistent storage of MemStruct in W25Q128
//              Uses a fixed number of contiguous 4KB sectors as a single
//              append-only log kept in a static RAM buffer (loaded at Init).
//              Minimizes erasures by appending entries until full,
//              then erases all sectors and restarts. Each entry includes a checksum
//              for validation. Buffer is updated in RAM on writes/erases and
//              flushed (delta) to flash on writes. Supports up to ~ (NUM_SECTORS*4096 / 10) updates per erase.
//              Entry: 10 bytes (magic 1 + 24-bit seq + 4-byte MemStruct + CRC16 2).
// Copyright (c) 2025 Dad Design.
//==================================================================================
#pragma once

#include "W25Q128.h"  // Assumes the provided cW25Q128 class header
#include <cstdint>
#include <cstring>    // For memset

struct MemStruct {
    uint8_t vol1;
    uint8_t vol2;
    uint8_t vol3;
    uint8_t volMaster;
};

namespace DadDrivers {

class cFlashManager {
public:
    // -----------------------------------------------------------------------------
    // Static Configuration
    // -----------------------------------------------------------------------------
    static constexpr uint32_t NUM_SECTORS = 10;  // Change this to configure the number of contiguous sectors

    // -----------------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------------
    cFlashManager() = default;

    // -----------------------------------------------------------------------------
    // Destructor (no-op)
    // -----------------------------------------------------------------------------
    virtual ~cFlashManager() = default;

    // -----------------------------------------------------------------------------
    // Initializes the manager by loading the entire area into the static RAM buffer.
    // Call this after flash initialization.
    // Parameters:
    //   flash: Reference to the W25Q128 flash instance
    //   baseAddr: Starting address in flash for this manager's sectors (default 0)
    // Returns: HAL_OK on success, error otherwise.
    // -----------------------------------------------------------------------------
    HAL_StatusTypeDef Init(cW25Q128* pflash, uint32_t baseAddr);

    // -----------------------------------------------------------------------------
    // Saves the current MemStruct to flash.
    // Appends a new entry with incremented sequence number and checksum to the buffer,
    // then writes only the new entry to flash.
    // If the area is full, erases all sectors first (updates buffer to 0xFF) and starts from offset 0.
    // Returns: true on success, false on read/write/erase failure.
    // -----------------------------------------------------------------------------
    bool Save(const MemStruct& data);

    // -----------------------------------------------------------------------------
    // Loads the latest valid MemStruct from the RAM buffer.
    // Scans for the entry with the highest sequence number (valid checksum).
    // Returns: The latest MemStruct, or {0,0,0,0} if no valid entries found.
    // -----------------------------------------------------------------------------
    bool Load(MemStruct *pMemStruct);

    // -----------------------------------------------------------------------------
    // Erases all managed sectors (e.g., for reset or initialization) and clears the buffer to 0xFF.
    // Returns: true on success, false on failure.
    // -----------------------------------------------------------------------------
    bool EraseSectors();

private:
    // =============================================================================
    // Private Constants
    // =============================================================================
    static constexpr uint32_t SECTOR_SIZE = 4096;  // 4KB sector
    static constexpr uint32_t TOTAL_SIZE = NUM_SECTORS * SECTOR_SIZE;  // Total size in bytes
    static constexpr uint32_t ENTRY_SIZE = 10;     // magic(1) + seq(3) + data(4) + crc(2)
    static constexpr uint8_t MAGIC_BYTE = 0xA5;    // Magic to validate entries
    static constexpr uint16_t CRC_POLY = 0xA001;   // Reflected CRC-16-IBM polynomial

    // =============================================================================
    // Private Members
    // =============================================================================
    cW25Q128* m_pflash;                   // Reference to flash driver
    uint32_t  m_baseAddr;                 // Base address of the managed sectors
    uint8_t   m_buffer[TOTAL_SIZE];       // Static RAM buffer for the entire area

    // =============================================================================
    // Private Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Writes a single entry to the buffer at the specified offset.
    // Parameters:
    //   offset: Byte offset in the buffer to write the entry
    //   seq: Sequence number for this entry
    //   data: The MemStruct to store
    // -----------------------------------------------------------------------------
    void WriteEntry(size_t offset, uint32_t seq, const MemStruct& data);

    // -----------------------------------------------------------------------------
    // Computes CRC-16 over the given data.
    // Parameters:
    //   data: Pointer to data
    //   len: Length of data
    // Returns: 16-bit CRC
    // -----------------------------------------------------------------------------
    uint16_t ComputeCRC16(const uint8_t* data, size_t len) const;

    // -----------------------------------------------------------------------------
    // Scans the RAM buffer to find the maximum sequence number and its position.
    // Validates magic and checksum for each potential entry.
    // Parameters (output):
    //   max_seq: Highest valid sequence found
    //   latest_pos: Position of the latest entry in the buffer
    //   found: true if at least one valid entry was found
    // -----------------------------------------------------------------------------
    void ScanForLatest(uint32_t& max_seq, size_t& latest_pos, bool& found);
};

} // namespace DadDrivers
