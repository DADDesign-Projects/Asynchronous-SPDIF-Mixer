//==================================================================================
// File: cFlashManager.cpp
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

#include "cFlashManager.h"

namespace DadDrivers {

// -----------------------------------------------------------------------------
// Initializes the manager by loading the entire area into the static RAM buffer.
// Call this after flash initialization.
// Parameters:
//   flash: Reference to the W25Q128 flash instance
//   baseAddr: Starting address in flash for this manager's sectors (default 0)
// Returns: HAL_OK on success, error otherwise.
// -----------------------------------------------------------------------------
HAL_StatusTypeDef cFlashManager::Init(cW25Q128* pflash, uint32_t baseAddr) {
	m_pflash 	=  	pflash;
	m_baseAddr 	= 	baseAddr;
    return m_pflash->Read(m_buffer, m_baseAddr, TOTAL_SIZE);
}

// -----------------------------------------------------------------------------
// Saves the current MemStruct to flash.
// Appends a new entry with incremented sequence number and checksum to the buffer,
// then writes only the new entry to flash.
// If the area is full, erases all sectors first (updates buffer to 0xFF) and starts from offset 0.
// Returns: true on success, false on read/write/erase failure.
// -----------------------------------------------------------------------------
bool cFlashManager::Save(const MemStruct& data) {
    uint32_t max_seq;
    size_t latest_pos;
    bool found;
    ScanForLatest(max_seq, latest_pos, found);

    uint32_t new_seq = found ? (max_seq + 1) : 0;
    size_t next_pos = found ? (latest_pos + ENTRY_SIZE) : 0;

    bool need_erase = (next_pos + ENTRY_SIZE > static_cast<size_t>(TOTAL_SIZE));
    if (need_erase) {
        if (!EraseSectors()) {
            return false;
        }
        next_pos = 0;
        new_seq = 0;
        found = false;
    }

    WriteEntry(next_pos, new_seq, data);

    HAL_StatusTypeDef st = m_pflash->Write(m_buffer + next_pos, m_baseAddr + next_pos, ENTRY_SIZE);
    return (st == HAL_OK);
}

// -----------------------------------------------------------------------------
// Loads the latest valid MemStruct from the RAM buffer.
// Scans for the entry with the highest sequence number (valid checksum).
// Returns: The latest MemStruct, or {0,0,0,0} if no valid entries found.
// -----------------------------------------------------------------------------
bool cFlashManager::Load(MemStruct *pMemStruct) {
    uint32_t max_seq;
    size_t latest_pos;
    bool found;
    ScanForLatest(max_seq, latest_pos, found);

    if (found) {
    	pMemStruct->vol1 = m_buffer[latest_pos + 4];
    	pMemStruct->vol2 = m_buffer[latest_pos + 5];
    	pMemStruct->vol3 = m_buffer[latest_pos + 6];
    	pMemStruct->volMaster = m_buffer[latest_pos + 7];
    }
    return found;
}

// -----------------------------------------------------------------------------
// Erases all managed sectors (e.g., for reset or initialization) and clears the buffer to 0xFF.
// Returns: true on success, false on failure.
// -----------------------------------------------------------------------------
bool cFlashManager::EraseSectors() {
    for (uint32_t i = 0; i < NUM_SECTORS; ++i) {
        uint32_t sectorAddr = m_baseAddr + (i * SECTOR_SIZE);
        if (m_pflash->EraseBlock4K(sectorAddr) != HAL_OK) {
            return false;
        }
    }
    std::memset(m_buffer, 0xFF, TOTAL_SIZE);
    return true;
}

// -----------------------------------------------------------------------------
// Writes a single entry to the buffer at the specified offset.
// Parameters:
//   offset: Byte offset in the buffer to write the entry
//   seq: Sequence number for this entry
//   data: The MemStruct to store
// -----------------------------------------------------------------------------
void cFlashManager::WriteEntry(size_t offset, uint32_t seq, const MemStruct& data) {
    uint8_t* buf = m_buffer + offset;
    buf[0] = MAGIC_BYTE;
    buf[1] = static_cast<uint8_t>(seq & 0xFF);
    buf[2] = static_cast<uint8_t>((seq >> 8) & 0xFF);
    buf[3] = static_cast<uint8_t>((seq >> 16) & 0xFF);
    buf[4] = data.vol1;
    buf[5] = data.vol2;
    buf[6] = data.vol3;
    buf[7] = data.volMaster;

    // Compute CRC over first 8 bytes
    uint16_t crc = ComputeCRC16(buf, 8);
    buf[8] = static_cast<uint8_t>(crc & 0xFF);
    buf[9] = static_cast<uint8_t>((crc >> 8) & 0xFF);
}

// -----------------------------------------------------------------------------
// Computes CRC-16 over the given data.
// Parameters:
//   data: Pointer to data
//   len: Length of data
// Returns: 16-bit CRC
// -----------------------------------------------------------------------------
uint16_t cFlashManager::ComputeCRC16(const uint8_t* data, size_t len) const {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= static_cast<uint16_t>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ CRC_POLY;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// -----------------------------------------------------------------------------
// Scans the RAM buffer to find the maximum sequence number and its position.
// Validates magic and checksum for each potential entry.
// Parameters (output):
//   max_seq: Highest valid sequence found
//   latest_pos: Position of the latest entry in the buffer
//   found: true if at least one valid entry was found
// -----------------------------------------------------------------------------
void cFlashManager::ScanForLatest(uint32_t& max_seq, size_t& latest_pos, bool& found) {
    max_seq = 0;
    latest_pos = 0;
    found = false;

    for (size_t pos = 0; pos < static_cast<size_t>(TOTAL_SIZE); pos += ENTRY_SIZE) {
        if (pos + ENTRY_SIZE > static_cast<size_t>(TOTAL_SIZE)) {
            break;
        }
        const uint8_t* p = m_buffer + pos;
        if (p[0] == MAGIC_BYTE) {
            // Compute CRC over first 8 bytes
            uint16_t computed_crc = ComputeCRC16(p, 8);
            uint16_t stored_crc = static_cast<uint16_t>(p[8]) | (static_cast<uint16_t>(p[9]) << 8);
            if (computed_crc == stored_crc) {
                uint32_t this_seq = static_cast<uint32_t>(p[1]) |
                                    (static_cast<uint32_t>(p[2]) << 8) |
                                    (static_cast<uint32_t>(p[3]) << 16);
                if (!found || this_seq > max_seq) {
                    max_seq = this_seq;
                    latest_pos = pos;
                    found = true;
                }
            }
        }
    }
}

} // namespace DadDrivers
