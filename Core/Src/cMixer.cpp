//==================================================================================
//==================================================================================
// File: cMixer.cpp
// Description: Synchronization and mixing of 3 asynchronous S/PDIF audio streams to 48kHz
//              with adaptive drift compensation based on buffer fill level
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

//**********************************************************************************
// cMixer.cpp
// Synchronization and mixing of 3 asynchronous S/PDIF audio streams to 48kHz
// with adaptive drift compensation based on buffer fill level
//**********************************************************************************
#include "cMixer.h"
#include <algorithm>

namespace Dad {

//**********************************************************************************
// cCircularBuff
//**********************************************************************************

// =============================================================================
// Public methods
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Pushes samples into the circular buffer
// -----------------------------------------------------------------------------
void cCircularBuff::Push(int32_t *pSamples)
{
    // Sign extension 24-bit -> 32-bit then normalization
    int32_t sampleL = (pSamples[0] << 8) >> 8;  // Left channel
    int32_t sampleR = (pSamples[1] << 8) >> 8;  // Right channel

    // Store normalized samples
    *m_pBuffer++ = COEF_NORMALIZE * sampleL;
    *m_pBuffer++ = COEF_NORMALIZE * sampleR;

    // Wrap around if end of buffer reached
    if (m_pBuffer >= &m_Buffer[CIRCULAR_BUFFER_SIZE * 2])
    {
        m_pBuffer = m_Buffer;
    }

    m_Date++;  // Increment internal timestamp
}

// -----------------------------------------------------------------------------
// Pulls samples from the circular buffer with interpolation
// -----------------------------------------------------------------------------
void cCircularBuff::Pull(float *pSamples, double date)
{
    // Return silence if date is out of bounds
    if ((date > m_Date) || (date + CIRCULAR_BUFFER_SIZE < m_Date))
    {
        pSamples[0] = pSamples[1] = 0.0f;
        return;
    }

    // Calculate integer and fractional parts of date for interpolation
    uint32_t intDate = static_cast<uint32_t>(date);          // Integer part
    float fracDate = static_cast<float>(date - intDate);     // Fractional part
    uint32_t indexOffset = static_cast<uint32_t>(m_Date) - intDate;  // Offset from current date

    // Calculate buffer position with wrap-around
    uint32_t bufferIndex = (m_pBuffer - m_Buffer - (indexOffset * 2) +
                            (CIRCULAR_BUFFER_SIZE * 2)) % (CIRCULAR_BUFFER_SIZE * 2);

    // Linear interpolation between current and next sample
    uint32_t nextIndex = (bufferIndex + 2) % (CIRCULAR_BUFFER_SIZE * 2);  // Next sample position
    float oneMinusFrac = 1.0f - fracDate;                                 // Weight for current sample

    // Interpolate left and right channels
    pSamples[0] = m_Buffer[bufferIndex] * oneMinusFrac + m_Buffer[nextIndex] * fracDate;
    pSamples[1] = m_Buffer[bufferIndex + 1] * oneMinusFrac + m_Buffer[nextIndex + 1] * fracDate;
}

//**********************************************************************************
// cMixer
//**********************************************************************************

// =============================================================================
// Public methods
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Initializes mixer state and resets all parameters
// -----------------------------------------------------------------------------
void cMixer::Initialise()
{
    // Clear input buffers
    BuffIn1.Clear();
    BuffIn2.Clear();
    BuffIn3.Clear();

    // Initialize drift and nominal factors
    m_Drif_Factor1 = m_Drif_Factor2 = m_Drif_Factor3 = 0.0f;
    m_nominal_factor1 = m_nominal_factor2 = m_nominal_factor3 = 1.0f;

    // Reset counters and output dates
    m_ctPull = m_ctIN1 = m_ctIN2 = m_ctIN3 = 0;
    m_DateOut1 = m_DateOut2 = m_DateOut3 = 0.0;

    // Reset sample rates and gains
    m_SampleRate1 = m_SampleRate2 = m_SampleRate3 = eSampleRate::NoSync;
    m_Gain1 = m_Gain2 = m_Gain3 = m_GainMaster = 1.0f;
}

// -----------------------------------------------------------------------------
// Converts sample rate enum to floating point value
// -----------------------------------------------------------------------------
float cMixer::getSampleRate(eSampleRate sr)
{
    switch (sr)
    {
        case eSampleRate::SR32000: return 32000.0f;
        case eSampleRate::SR41000: return 41000.0f;
        case eSampleRate::SR44100: return 44100.0f;
        case eSampleRate::SR48000: return 48000.0f;
        case eSampleRate::SR96000: return 96000.0f;
        default: return 48000.0f;
    }
}

// -----------------------------------------------------------------------------
// Detects sample rate based on received sample count
// -----------------------------------------------------------------------------
eSampleRate cMixer::detectSampleRate(uint16_t sampleCount)
{
    // Detection with tolerance ±RX_BUFFER_SIZE
    struct RateCheck
    {
        uint16_t delta;         // Expected delta value
        eSampleRate rate;       // Corresponding sample rate
    };

    // Table of known sample rates and their expected deltas
    const RateCheck rates[] =
    {
        {DELTA_DATE_96000, eSampleRate::SR96000},
        {DELTA_DATE_48000, eSampleRate::SR48000},
        {DELTA_DATE_44100, eSampleRate::SR44100},
        {DELTA_DATE_41000, eSampleRate::SR41000},
        {DELTA_DATE_32000, eSampleRate::SR32000}
    };

    // Check each rate against the received sample count
    for (const auto& r : rates)
    {
        if (sampleCount < (r.delta + RX_BUFFER_SIZE) &&
            sampleCount > (r.delta - RX_BUFFER_SIZE))
        {
            return r.rate;
        }
    }

    return eSampleRate::NoSync;  // No valid rate detected
}

// -----------------------------------------------------------------------------
// Updates buffer synchronization parameters based on detected sample rate
// -----------------------------------------------------------------------------
void cMixer::updateBufferSync(
    uint16_t& ctIN,               // Input sample counter
    eSampleRate& currentRate,     // Current sample rate
    float& nominalFactor,         // Nominal resampling factor
    float& driftFactor,           // Current drift compensation factor
    cCircularBuff& buffer,        // Circular buffer
    double& dateOut               // Output date pointer
)
{
    // Detect sample rate from input counter
    eSampleRate detectedRate = detectSampleRate(ctIN);
    ctIN = 0;  // Reset input counter

    if (detectedRate != eSampleRate::NoSync)
    {
        // If rate changed, update all parameters
        if (detectedRate != currentRate)
        {
            currentRate = detectedRate;
            nominalFactor = getSampleRate(detectedRate) / 48000.0f;  // Calculate resampling ratio
            driftFactor = nominalFactor;                              // Initialize drift factor
            buffer.setDate(0.0);                                      // Reset buffer date
            dateOut = 0.0;                                            // Reset output date
        }
    }
    else
    {
        // No synchronization detected
        driftFactor = 0.0f;
        currentRate = eSampleRate::NoSync;
        buffer.setDate(0.0);
        dateOut = 0.0;
    }
}

// -----------------------------------------------------------------------------
// Adjusts drift compensation factor based on buffer fill level
// -----------------------------------------------------------------------------
void cMixer::adjustDrift(
    float& driftFactor,            // Current drift factor to adjust
    float nominalFactor,           // Nominal resampling factor
    const cCircularBuff& buffer,   // Circular buffer reference
    double readDate                // Current read position
)
{
    if (driftFactor == 0.0f) return;  // No adjustment if no sync

    // Calculate buffer fill level error
    double age = buffer.getDate() - readDate;                // Current buffer age
    double targetAge = static_cast<double>(RX_BUFFER_SIZE);  // Target buffer age
    double error = (age - targetAge) / targetAge;            // Normalized error

    // Proportional adjustment based on error
    double adjustment = 1.0 + error * m_gain;           // Apply gain to error
    double factorTarget = nominalFactor * adjustment;   // Target factor

    // Clamp to ±50% of nominal factor
    factorTarget = std::max(0.5 * nominalFactor,
                            std::min(1.5 * nominalFactor, factorTarget));

    // Low-pass IIR filter to avoid artifacts
    driftFactor = static_cast<float>(m_alpha * factorTarget +
                                     (1.0 - m_alpha) * driftFactor);
}

// -----------------------------------------------------------------------------
// Pushes samples into input buffer 1
// -----------------------------------------------------------------------------
void cMixer::pushSamples1(int32_t* pSamples)
{
    // Process all samples in the buffer
    for (int i = 0; i < RX_BUFFER_SIZE; i += 2)
    {
        BuffIn1.Push(pSamples);  // Push stereo pair
        pSamples += 2;           // Move to next stereo pair
        m_ctIN1++;               // Increment sample counter
    }
}

// -----------------------------------------------------------------------------
// Pushes samples into input buffer 2
// -----------------------------------------------------------------------------
void cMixer::pushSamples2(int32_t* pSamples)
{
    // Process all samples in the buffer
    for (int i = 0; i < RX_BUFFER_SIZE; i += 2)
    {
        BuffIn2.Push(pSamples);  // Push stereo pair
        pSamples += 2;           // Move to next stereo pair
        m_ctIN2++;               // Increment sample counter
    }
}

// -----------------------------------------------------------------------------
// Pushes samples into input buffer 3
// -----------------------------------------------------------------------------
void cMixer::pushSamples3(int32_t* pSamples)
{
    // Process all samples in the buffer
    for (int i = 0; i < RX_BUFFER_SIZE; i += 2)
    {
        BuffIn3.Push(pSamples);  // Push stereo pair
        pSamples += 2;           // Move to next stereo pair
        m_ctIN3++;               // Increment sample counter
    }
}

// -----------------------------------------------------------------------------
// Pulls mixed samples from all synchronized buffers
// -----------------------------------------------------------------------------
void cMixer::pullSamples(int32_t* pSamples)
{
    // Periodically detect and update sample rates
    if (m_ctPull >= DRIF_CALC_NB_SAMPLES)
    {
        m_ctPull = 0;  // Reset pull counter

        // Update synchronization for all three inputs
        updateBufferSync(m_ctIN1, m_SampleRate1, m_nominal_factor1,
                        m_Drif_Factor1, BuffIn1, m_DateOut1);
        updateBufferSync(m_ctIN2, m_SampleRate2, m_nominal_factor2,
                        m_Drif_Factor2, BuffIn2, m_DateOut2);
        updateBufferSync(m_ctIN3, m_SampleRate3, m_nominal_factor3,
                        m_Drif_Factor3, BuffIn3, m_DateOut3);
    }

    // Mix samples for the entire output buffer
    for (int i = 0; i < TX_BUFFER_SIZE; i += 2)
    {
        float sample1[2] = {0.0f, 0.0f};  // Buffer for input 1
        float sample2[2] = {0.0f, 0.0f};  // Buffer for input 2
        float sample3[2] = {0.0f, 0.0f};  // Buffer for input 3

        // Process input 1 if synchronized
        if (m_Drif_Factor1 != 0.0f)
        {
            // Calculate read position with drift compensation
            double readDate1 = (m_DateOut1 * m_Drif_Factor1) - RX_BUFFER_SIZE;
            BuffIn1.Pull(sample1, readDate1);      // Pull samples from buffer
            sample1[0] *= m_Gain1;                 // Apply channel gain
            sample1[1] *= m_Gain1;
            adjustDrift(m_Drif_Factor1, m_nominal_factor1, BuffIn1, readDate1);  // Adjust drift
        }

        // Process input 2 if synchronized
        if (m_Drif_Factor2 != 0.0f)
        {
            // Calculate read position with drift compensation
            double readDate2 = (m_DateOut2 * m_Drif_Factor2) - RX_BUFFER_SIZE;
            BuffIn2.Pull(sample2, readDate2);      // Pull samples from buffer
            sample2[0] *= m_Gain2;                 // Apply channel gain
            sample2[1] *= m_Gain2;
            adjustDrift(m_Drif_Factor2, m_nominal_factor2, BuffIn2, readDate2);  // Adjust drift
        }

        // Process input 3 if synchronized
        if (m_Drif_Factor3 != 0.0f)
        {
            // Calculate read position with drift compensation
            double readDate3 = (m_DateOut3 * m_Drif_Factor3) - RX_BUFFER_SIZE;
            BuffIn3.Pull(sample3, readDate3);      // Pull samples from buffer
            sample3[0] *= m_Gain3;                 // Apply channel gain
            sample3[1] *= m_Gain3;
            adjustDrift(m_Drif_Factor3, m_nominal_factor3, BuffIn3, readDate3);  // Adjust drift
        }

        // Mix all channels and denormalize
        pSamples[0] = static_cast<int32_t>((sample1[0] + sample2[0] + sample3[0]) * m_GainMaster * COEF_DENORMALIZE);
        pSamples[1] = static_cast<int32_t>((sample1[1] + sample2[1] + sample3[1]) * m_GainMaster * COEF_DENORMALIZE);

        pSamples += 2;  // Move to next output stereo pair

        // Increment output dates and pull counter
        m_DateOut1++;
        m_DateOut2++;
        m_DateOut3++;
        m_ctPull++;
    }
}

} // namespace Dad

//***End of file**************************************************************
