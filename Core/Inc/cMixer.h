//******************************************************************************
//* cMixer.h
//
// This file defines the cMixer class, designed to synchronize and mix two asynchronous S/PDIF audio streams
// into a single output S/PDIF stream at a fixed sample rate of 48kHz. The cMixer class utilizes circular buffers
// to manage incoming audio samples from two different sources, compensating for potential clock drifts between
// the input streams using interpolation techniques.
//
// - cCircularBuff: A helper class that manages a circular buffer of audio samples, providing functionality to
//   store, retrieve, and interpolate samples over time. It supports operations such as pushing new samples into
//   the buffer and pulling interpolated samples based on a given date, with automatic wrapping.
//
// - cMixer: This class is responsible for handling two input audio streams, synchronizing them to a common
//   output clock by compensating for drift between the streams. It ensures that both streams are mixed smoothly
//   through linear interpolation and drift correction. The class supports input sample rates of 96kHz, 48kHz,
//   44.1kHz, and 32kHz and produces a mixed output at 48kHz.
//******************************************************************************

#ifndef CMIXER_H_
#define CMIXER_H_
#include "main.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define CIRCULAR_BUFFER_SIZE 200
#define RX_BUFFER_SIZE  10
#define TX_BUFFER_SIZE  10
#define DRIF_CALC_NB_SAMPLES 1000

#define DETA_MAX 30
#define DELTA_DATE_96000 1995   // Delta value for synchronizing 96kHz sample rate
#define DELTA_DATE_48000 995    // Delta value for synchronizing 48kHz sample rate
#define DELTA_DATE_44100 915    // Delta value for synchronizing 44.1kHz sample rate
#define DELTA_DATE_32000 665    // Delta value for synchronizing 32kHz sample rate

// Coefficient to normalize a 24-bit integer to a floating-point value (-1.0 to +1.0 range)
constexpr float CoefNormalizeIntToFloat = 1.0f / (float)0xEFFFFF;

namespace Dad {

enum class eSampleRate {
    SR96000,  // 96kHz sample rate
    SR48000,  // 48kHz sample rate
    SR44100,  // 44.1kHz sample rate
    SR32000,  // 32kHz sample rate
    NoSync    // No synchronization detected
};

//***************************************************************************
// cCircularBuff
// Circular buffer class for managing audio samples and providing
// interpolation when pulling data. It supports storing 24-bit samples
// as normalized floating-point values and offers smooth transitions
// between samples by interpolating between buffer positions.
//
// This class is used to handle input audio data streams, ensuring that
// samples are processed in a continuous circular manner and supports
// efficient retrieval with date-based interpolation.
//
class cCircularBuff {
public:
    //---------------------------------------------------------------------
    // Constructor
    // Initializes the buffer by clearing its contents.
    cCircularBuff() {
        Clear();  // Reset buffer pointer and timestamp
    }

    //---------------------------------------------------------------------
    // Destructor
    // Cleans up the circular buffer object.
    ~cCircularBuff() {}

    //---------------------------------------------------------------------
    // Clear
    // Resets the buffer pointer to the start and the timestamp to zero.
    void Clear() {
        m_pBuffer = m_Buffer;  // Set buffer pointer to the start
        m_Date = 0;            // Reset timestamp
    }

    //---------------------------------------------------------------------
    // getDate
    // Returns the current timestamp of the buffer, which tracks the
    // sample date for synchronization purposes.
    inline double getDate() { return m_Date; }

    //---------------------------------------------------------------------
    // setDate
    // Sets a new timestamp for the buffer.
    //
    // Parameters:
    //   newDate - New timestamp to set.
    inline void setDate(double newDate) { m_Date = newDate; }

    //---------------------------------------------------------------------
    // Push
    // Converts 24-bit signed integer samples into normalized floating-point
    // values and stores them in the circular buffer. The buffer wraps around
    // when it reaches the end, ensuring that new samples overwrite the oldest.
    //
    // Parameters:
    //   pSamples - Pointer to an array containing 24-bit audio samples (L, R).
    void Push(int32_t *pSamples);

    //---------------------------------------------------------------------
    // Pull
    // Retrieves interpolated samples from the circular buffer based on the
    // specified date. Interpolation is performed to ensure smooth transitions
    // between buffer positions, compensating for any potential drift or gaps
    // in the sample stream.
    //
    // Parameters:
    //   pSamples - Pointer to store the output samples (L, R).
    //   Date     - The date for which the sample should be retrieved.
    void Pull(float *pSamples, double Date);

private:
    float m_Buffer[CIRCULAR_BUFFER_SIZE * 2];  // Buffer storage for interleaved L/R samples
    float* m_pBuffer;                          // Current write position in the buffer
    double m_Date;                             // Timestamp tracking the position in the buffer
};

//***************************************************************************
// Class cMixer
// The cMixer class is responsible for mixing two asynchronous S/PDIF input
// streams into a single, synchronized output stream at 48kHz. It manages two
// cCircularBuff objects, one for each input stream, and handles the drift
// between them by calculating a drift factor and interpolating samples. The
// result is a smooth, synchronized audio output that blends both streams.
//
// This class compensates for drift by recalculating drift factors after a
// certain number of samples (DRIF_CALC_NB_SAMPLES). It supports multiple
// input sample rates (96kHz, 48kHz, 44.1kHz, 32kHz) and handles transitions
// between them through synchronization mechanisms.
//
class cMixer {
public:
    //---------------------------------------------------------------------
    // Constructor
    // Initializes the mixer by clearing both circular buffers and resetting
    // internal counters and drift factors.
    cMixer() { Initialise(); }

    //---------------------------------------------------------------------
    // Destructor
    ~cMixer() {}

    //---------------------------------------------------------------------
    // Initialise
    // Resets the internal state of the mixer, including drift factors,
    // sample counters, and synchronization counters. This ensures that
    // the mixer starts in a consistent state.
    void Initialise() {
        BuffIn1.Clear();
        BuffIn2.Clear();
        BuffIn3.Clear();

        m_Drif_Factor1 = 0;
        m_Drif_Factor2 = 0;
        m_Drif_Factor3 = 0;

        m_ctPull = 0;
        m_ctIN1 = 0;
        m_ctIN2 = 0;
        m_ctIN3 = 0;

        m_CtSynchro1 = 0;
        m_CtSynchro2 = 0;
        m_CtSynchro3 = 0;

        m_DateOut1 = 0;
        m_DateOut2 = 0;
        m_DateOut3 = 0;

        m_SampleRate1 = eSampleRate::NoSync;
        m_SampleRate2 = eSampleRate::NoSync;
        m_SampleRate3 = eSampleRate::NoSync;
    }

    //---------------------------------------------------------------------
    // GetSampleRatex
    eSampleRate GetSampleRate1(){
    	return m_SampleRate1;
    }

    eSampleRate GetSampleRate2(){
    	return m_SampleRate2;
    }

    eSampleRate GetSampleRate3(){
    	return m_SampleRate3;
    }
    //---------------------------------------------------------------------
    // pushSamples1
    // Pushes new samples into the first input circular buffer (Buffer 1).
    //
    // Parameters:
    //   pSamples - Pointer to the input sample array (interleaved L/R).
    void pushSamples1(int32_t* pSamples);

    //---------------------------------------------------------------------
    // pushSamples2
    // Pushes new samples into the second input circular buffer (Buffer 2).
    //
    // Parameters:
    //   pSamples - Pointer to the input sample array (interleaved L/R).
    void pushSamples2(int32_t* pSamples);

    //---------------------------------------------------------------------
    // pushSamples3
    // Pushes new samples into the third input circular buffer (Buffer 1).
    //
    // Parameters:
    //   pSamples - Pointer to the input sample array (interleaved L/R).
    void pushSamples3(int32_t* pSamples);

    //---------------------------------------------------------------------
    // pullSamples
    // Retrieves mixed and synchronized samples from both input buffers,
    // compensating for drift and ensuring a smooth output. The samples
    // are interpolated and mixed to produce a final output at 48kHz.
    //
    // Parameters:
    //   pSamples - Pointer to store the output mixed samples (interleaved L/R).
    void pullSamples(int32_t* pSamples);

private:
    cCircularBuff BuffIn1;  // Circular buffer for the first input stream
    cCircularBuff BuffIn2;  // Circular buffer for the second input stream
    cCircularBuff BuffIn3;  // Circular buffer for the second input stream

    float m_Drif_Factor1;   // Drift compensation factor for the first stream
    float m_Drif_Factor2;   // Drift compensation factor for the second stream
    float m_Drif_Factor3;   // Drift compensation factor for the second stream

    uint16_t m_ctPull;      // Counter for tracking pull operations
    uint16_t m_ctIN1;       // Counter for input 1 samples
    uint16_t m_ctIN2;       // Counter for input 2 samples
    uint16_t m_ctIN3;       // Counter for input 2 samples

    uint8_t m_CtSynchro1;   // Synchronization counter for the first stream
    uint8_t m_CtSynchro2;   // Synchronization counter for the second stream
    uint8_t m_CtSynchro3;   // Synchronization counter for the second stream

    double m_DateOut1;      // Output timestamp for the first stream
    double m_DateOut2;      // Output timestamp for the second stream
    double m_DateOut3;      // Output timestamp for the second stream

    eSampleRate m_SampleRate1;  // Detected sample rate for the first stream
    eSampleRate m_SampleRate2;  // Detected sample rate for the second stream
    eSampleRate m_SampleRate3;  // Detected sample rate for the third stream
};

} /* namespace Dad */

#endif /* CMIXER_H_ */
