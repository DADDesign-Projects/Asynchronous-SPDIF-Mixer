//==================================================================================
//==================================================================================
// File: cMixer.h
// Description: Header for 3-channel asynchronous S/PDIF audio mixer with adaptive
//              drift compensation and sample rate conversion to 48kHz
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once

#include "main.h"
#include <algorithm>

// =============================================================================
// Configuration constants
// =============================================================================

#define CIRCULAR_BUFFER_SIZE 200      // Size of circular buffer in stereo samples
#define RX_BUFFER_SIZE 20             // Input buffer size in stereo samples
#define TX_BUFFER_SIZE 10             // Output buffer size in stereo samples
#define DRIF_CALC_NB_SAMPLES 1000     // Number of samples between drift calculations

// Sample rate detection deltas (for DRIF_CALC_NB_SAMPLES samples)
#define DELTA_DATE_96000 1995         // Expected delta for 96kHz
#define DELTA_DATE_48000 995          // Expected delta for 48kHz
#define DELTA_DATE_44100 915          // Expected delta for 44.1kHz
#define DELTA_DATE_41000 855          // Expected delta for 41kHz
#define DELTA_DATE_32000 665          // Expected delta for 32kHz

// Normalization coefficients for 24-bit to float conversion
constexpr float COEF_NORMALIZE = 1.0f / 8388607.0f;  // 0x7FFFFF (max 24-bit positive)
constexpr float COEF_DENORMALIZE = 8388607.0f;       // Inverse for denormalization

namespace Dad {

// =============================================================================
// Enumerations
// =============================================================================

// -----------------------------------------------------------------------------
// Supported sample rates
// -----------------------------------------------------------------------------
enum class eSampleRate
{
    SR32000,    // 32kHz sample rate
    SR41000,    // 41kHz sample rate
    SR44100,    // 44.1kHz sample rate
    SR48000,    // 48kHz sample rate
    SR96000,    // 96kHz sample rate
    NoSync      // No synchronization detected
};

//**********************************************************************************
// cCircularBuff
// Circular buffer with linear interpolation for audio samples
//**********************************************************************************
class cCircularBuff
{
public:
    // =========================================================================
    // Constructor
    // -------------------------------------------------------------------------
    cCircularBuff() { Clear(); }

    // =========================================================================
    // Public methods
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // Clears buffer and resets state
    // -------------------------------------------------------------------------
    void Clear()
    {
        m_pBuffer = m_Buffer;     // Reset buffer pointer to start
        m_Date = 0.0;             // Reset internal timestamp
    }

    // -------------------------------------------------------------------------
    // Gets current buffer date (timestamp)
    // -------------------------------------------------------------------------
    inline double getDate() const { return m_Date; }

    // -------------------------------------------------------------------------
    // Sets buffer date (timestamp)
    // -------------------------------------------------------------------------
    inline void setDate(double newDate) { m_Date = newDate; }

    // -------------------------------------------------------------------------
    // Pushes signed 24-bit samples (interleaved L/R)
    // -------------------------------------------------------------------------
    void Push(int32_t *pSamples);

    // -------------------------------------------------------------------------
    // Pulls interpolated samples at given date
    // -------------------------------------------------------------------------
    void Pull(float *pSamples, double date);

private:
    // =========================================================================
    // Member variables
    // -------------------------------------------------------------------------
    float m_Buffer[CIRCULAR_BUFFER_SIZE * 2];  // Stereo interleaved buffer
    float* m_pBuffer;                          // Current write pointer
    double m_Date;                             // Internal timestamp
};

//**********************************************************************************
// cMixer
// 3-channel mixer with adaptive drift compensation
//**********************************************************************************
class cMixer
{
public:
    // =========================================================================
    // Constructor
    // -------------------------------------------------------------------------
    cMixer() { Initialise(); }

    // =========================================================================
    // Public methods
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // Initializes mixer state and resets all parameters
    // -------------------------------------------------------------------------
    void Initialise();

    // -------------------------------------------------------------------------
    // Sample rate getters
    // -------------------------------------------------------------------------
    eSampleRate GetSampleRate1() const { return m_SampleRate1; }  // Input 1 sample rate
    eSampleRate GetSampleRate2() const { return m_SampleRate2; }  // Input 2 sample rate
    eSampleRate GetSampleRate3() const { return m_SampleRate3; }  // Input 3 sample rate

    // -------------------------------------------------------------------------
    // Channel gain setters
    // -------------------------------------------------------------------------
    void setGain1(float gain) { m_Gain1 = gain; }           // Set input 1 gain
    void setGain2(float gain) { m_Gain2 = gain; }           // Set input 2 gain
    void setGain3(float gain) { m_Gain3 = gain; }           // Set input 3 gain
    void setGainMaster(float gain) { m_GainMaster = gain; } // Set master gain

    // -------------------------------------------------------------------------
    // Sample input/output methods
    // -------------------------------------------------------------------------
    void pushSamples1(int32_t* pSamples);  // Push samples to input 1
    void pushSamples2(int32_t* pSamples);  // Push samples to input 2
    void pushSamples3(int32_t* pSamples);  // Push samples to input 3
    void pullSamples(int32_t* pSamples);   // Pull mixed samples from all inputs

private:
    // =========================================================================
    // Private methods
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // Detects sample rate from sample count
    // -------------------------------------------------------------------------
    eSampleRate detectSampleRate(uint16_t sampleCount);

    // -------------------------------------------------------------------------
    // Converts sample rate enum to frequency value
    // -------------------------------------------------------------------------
    float getSampleRate(eSampleRate sr);

    // -------------------------------------------------------------------------
    // Updates buffer synchronization parameters
    // -------------------------------------------------------------------------
    void updateBufferSync(
        uint16_t& ctIN,               // Input sample counter
        eSampleRate& currentRate,     // Current sample rate
        float& nominalFactor,         // Nominal resampling factor
        float& driftFactor,           // Current drift compensation factor
        cCircularBuff& buffer,        // Circular buffer
        double& dateOut               // Output date
    );

    // -------------------------------------------------------------------------
    // Adjusts drift factor based on buffer fill level
    // -------------------------------------------------------------------------
    void adjustDrift(
        float& driftFactor,            // Current drift factor to adjust
        float nominalFactor,           // Nominal resampling factor
        const cCircularBuff& buffer,   // Circular buffer
        double readDate                // Current read position
    );

    // =========================================================================
    // Member variables
    // -------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Circular buffers for each input
    // -----------------------------------------------------------------------------
    cCircularBuff BuffIn1;  // Input buffer for channel 1
    cCircularBuff BuffIn2;  // Input buffer for channel 2
    cCircularBuff BuffIn3;  // Input buffer for channel 3

    // -----------------------------------------------------------------------------
    // Drift compensation factors (adaptive)
    // -----------------------------------------------------------------------------
    float m_Drif_Factor1;  // Drift factor for input 1
    float m_Drif_Factor2;  // Drift factor for input 2
    float m_Drif_Factor3;  // Drift factor for input 3

    // -----------------------------------------------------------------------------
    // Nominal resampling factors (input_rate / 48000)
    // -----------------------------------------------------------------------------
    float m_nominal_factor1;  // Nominal factor for input 1
    float m_nominal_factor2;  // Nominal factor for input 2
    float m_nominal_factor3;  // Nominal factor for input 3

    // -----------------------------------------------------------------------------
    // Adaptation parameters
    // -----------------------------------------------------------------------------
    double m_alpha = 0.0000001;  // Low-pass IIR filter coefficient
    float m_gain = 0.5f;         // Correction gain based on fill level

    // -----------------------------------------------------------------------------
    // Counters
    // -----------------------------------------------------------------------------
    uint16_t m_ctPull;        // Pull sample counter
    uint16_t m_ctIN1;         // Input 1 sample counter
    uint16_t m_ctIN2;         // Input 2 sample counter
    uint16_t m_ctIN3;         // Input 3 sample counter

    // -----------------------------------------------------------------------------
    // Output timestamps
    // -----------------------------------------------------------------------------
    double m_DateOut1;  // Output date for channel 1
    double m_DateOut2;  // Output date for channel 2
    double m_DateOut3;  // Output date for channel 3

    // -----------------------------------------------------------------------------
    // Detected sample rates
    // -----------------------------------------------------------------------------
    eSampleRate m_SampleRate1;  // Sample rate for input 1
    eSampleRate m_SampleRate2;  // Sample rate for input 2
    eSampleRate m_SampleRate3;  // Sample rate for input 3

    // -----------------------------------------------------------------------------
    // Gain controls
    // -----------------------------------------------------------------------------
    float m_Gain1;        // Gain for input channel 1
    float m_Gain2;        // Gain for input channel 2
    float m_Gain3;        // Gain for input channel 3
    float m_GainMaster;   // Master output gain
};

} // namespace Dad

//***End of file**************************************************************
