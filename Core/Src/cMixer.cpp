//******************************************************************************
// cMixer.cpp
// Implementation of the cCircularBuff and cMixer classes for handling asynchronous
// S/PDIF audio stream synchronization and mixing.
//******************************************************************************

#include "cMixer.h"
#include "Debug.h"

namespace Dad {

//***************************************************************************
// cCircularBuff
// Circular buffer class for storing audio samples with interpolation support.
// This buffer wraps around when full, ensuring that new samples overwrite
// the oldest, and allows for smooth sample transitions through interpolation.

    //---------------------------------------------------------------------
    // Push
    // Converts 24-bit signed samples into normalized floating-point values
    // and stores them in the circular buffer. When the buffer is full, it
    // wraps around, overwriting the oldest samples.
    //
    // Parameters:
    //   pSamples - Pointer to an array containing 24-bit interleaved stereo
    //              samples (Left, Right).
    void cCircularBuff::Push(int32_t *pSamples) {
        // Convert 24-bit signed samples to 32-bit integers with sign extension
        int32_t sampleL = (pSamples[0] << 8) >> 8;  // Left channel
        int32_t sampleR = (pSamples[1] << 8) >> 8;  // Right channel

        // Normalize the 24-bit samples to floating-point values
        *m_pBuffer++ = CoefNormalizeIntToFloat * sampleL;
        *m_pBuffer++ = CoefNormalizeIntToFloat * sampleR;

        // Wrap the buffer pointer when it reaches the end (circular behavior)
        if (m_pBuffer >= &m_Buffer[CIRCULAR_BUFFER_SIZE * 2]) {
            m_pBuffer = m_Buffer;  // Reset pointer to the beginning
        }

        // Increment the timestamp (date) of the buffer for synchronization purposes
        m_Date++;
    }

    //---------------------------------------------------------------------
    // Pull
    // Retrieves interpolated samples from the buffer based on the specified date.
    // If the requested date is out of bounds, silence (zero samples) is returned.
    // Otherwise, it calculates the position in the buffer and interpolates between
    // two adjacent samples for smooth audio transitions.
    //
    // Parameters:
    //   pSamples - Pointer to an array to store the retrieved samples (Left, Right).
    //   Date     - The date (timestamp) to retrieve samples for, with interpolation.
    void cCircularBuff::Pull(float *pSamples, double Date) {
        // If the requested date is out of bounds, return silence
        if ((Date > m_Date) || (Date + CIRCULAR_BUFFER_SIZE < m_Date)) {
            pSamples[0] = 0.0f;  // Left channel
            pSamples[1] = 0.0f;  // Right channel
        } else {
            // Calculate the integer and fractional part of the requested date
            uint32_t IntDate = static_cast<uint32_t>(Date);
            uint32_t IndexDate = static_cast<uint32_t>(m_Date) - IntDate;
            float FracDate = Date - IntDate;

            // Determine the buffer position, ensuring circular wrap-around
            uint32_t bufferIndex = (m_pBuffer - m_Buffer - (IndexDate * 2) + (CIRCULAR_BUFFER_SIZE * 2)) % (CIRCULAR_BUFFER_SIZE * 2);
            float Sample1 = m_Buffer[bufferIndex];       // Left channel sample
            float Sample2 = m_Buffer[bufferIndex + 1];   // Right channel sample

            // Calculate the next buffer index and interpolate between samples
            uint32_t nextIndex = (bufferIndex + 2) % (CIRCULAR_BUFFER_SIZE * 2);
            pSamples[0] = (Sample1 * (1 - FracDate)) + (m_Buffer[nextIndex] * FracDate);  // Interpolated left sample
            pSamples[1] = (Sample2 * (1 - FracDate)) + (m_Buffer[nextIndex + 1] * FracDate);  // Interpolated right sample
        }
    }

//***************************************************************************
// cMixer
// Class responsible for mixing two circular buffers, compensating for drift
// between asynchronous S/PDIF input streams. The mixer calculates drift factors,
// synchronizes the two streams, and produces a single mixed output stream.
//
// It periodically recalculates the drift to ensure the output remains synchronized.

    //---------------------------------------------------------------------
    // pushSamples1
    // Pushes audio samples into the first input buffer (Buffer 1).
    //
    // Parameters:
    //   pSamples - Pointer to the input sample array (interleaved Left/Right).
    void cMixer::pushSamples1(int32_t* pSamples) {
        for (int Index = 0; Index < RX_BUFFER_SIZE; Index += 2) {
            BuffIn1.Push(pSamples);  // Push samples into Buffer 1
            pSamples += 2;           // Move to the next stereo sample (L/R)
            m_ctIN1++;               // Increment input sample counter for Buffer 1
        }
    }

    //---------------------------------------------------------------------
    // pushSamples2
    // Pushes audio samples into the second input buffer (Buffer 2).
    //
    // Parameters:
    //   pSamples - Pointer to the input sample array (interleaved Left/Right).
    void cMixer::pushSamples2(int32_t* pSamples) {
        for (int Index = 0; Index < RX_BUFFER_SIZE; Index += 2) {
            BuffIn2.Push(pSamples);  // Push samples into Buffer 2
            pSamples += 2;           // Move to the next stereo sample (L/R)
            m_ctIN2++;               // Increment input sample counter for Buffer 2
        }
    }

    //---------------------------------------------------------------------
    // pullSamples
    // Retrieves mixed samples from both input buffers. This function handles
    // the synchronization of the two streams by calculating drift factors
    // periodically and compensating for timing differences between them.
    // It interpolates the samples from each buffer and mixes them together
    // to produce a final output at a fixed rate (48kHz).
    //
    // Parameters:
    //   pSamples - Pointer to the output array where the mixed samples (L/R) will be stored.
    void cMixer::pullSamples(int32_t* pSamples) {
        // Periodically recalculate drift compensation every DRIF_CALC_NB_SAMPLES samples
        if (m_ctPull >= DRIF_CALC_NB_SAMPLES) {
            eSampleRate SampleRate;
            m_ctPull = 0;

            //---------------------------------------------
            // Determine the sample rate and synchronize Buffer 1
            if ((m_ctIN1 < (DELTA_DATE_48000 + RX_BUFFER_SIZE)) && (m_ctIN1 > (DELTA_DATE_48000 - RX_BUFFER_SIZE))) {
                SampleRate = eSampleRate::SR48000;
            } else if ((m_ctIN1 < (DELTA_DATE_44100 + RX_BUFFER_SIZE)) && (m_ctIN1 > (DELTA_DATE_44100 - RX_BUFFER_SIZE))) {
                SampleRate = eSampleRate::SR44100;
            } else if ((m_ctIN1 < (DELTA_DATE_96000 + RX_BUFFER_SIZE)) && (m_ctIN1 > (DELTA_DATE_96000 - RX_BUFFER_SIZE))) {
                SampleRate = eSampleRate::SR96000;
            } else if ((m_ctIN1 < (DELTA_DATE_32000 + RX_BUFFER_SIZE)) && (m_ctIN1 > (DELTA_DATE_32000 - RX_BUFFER_SIZE))) {
                SampleRate = eSampleRate::SR32000;
            } else {
                SampleRate = eSampleRate::NoSync;
                m_Drif_Factor1 = 0;
                m_SampleRate1 = eSampleRate::NoSync;
                BuffIn1.setDate(0);   // Reset buffer 1 timestamp
                m_DateOut1 = 0;       // Reset output timestamp
            }

            m_ctIN1 = 0;

            // Synchronization logic for Buffer 1 based on sample rate
            if (SampleRate != eSampleRate::NoSync) {
                if (SampleRate == m_SampleRate1) {
                    if (m_CtSynchro1 >= 2) {
                        // If synchronized, calculate drift factor
                        if (m_Drif_Factor1 == 0) {
                            m_Drif_Factor1 = BuffIn1.getDate() / (m_DateOut1 + TX_BUFFER_SIZE);  // Initial drift calculation
                        } else {
                            float DeltaFactor = m_Drif_Factor1 - BuffIn1.getDate() / (m_DateOut1 + TX_BUFFER_SIZE);
                            m_Drif_Factor1 -= DeltaFactor / 8;  // Smooth drift correction
                        }
                    } else {
                        m_CtSynchro1++;  // Increment synchronization counter
                    }
                } else {
                    m_SampleRate1 = SampleRate;
                    m_CtSynchro1 = 0;  // Reset synchronization counter
                }
            }

            //---------------------------------------------
            // Determine the sample rate and synchronize Buffer 2 (same logic as Buffer 1)
            if ((m_ctIN2 < (DELTA_DATE_48000 + RX_BUFFER_SIZE)) && (m_ctIN2 > (DELTA_DATE_48000 - RX_BUFFER_SIZE))) {
                SampleRate = eSampleRate::SR48000;
            } else if ((m_ctIN2 < (DELTA_DATE_44100 + RX_BUFFER_SIZE)) && (m_ctIN2 > (DELTA_DATE_44100 - RX_BUFFER_SIZE))) {
                SampleRate = eSampleRate::SR44100;
            } else if ((m_ctIN2 < (DELTA_DATE_96000 + RX_BUFFER_SIZE)) && (m_ctIN2 > (DELTA_DATE_96000 - RX_BUFFER_SIZE))) {
                SampleRate = eSampleRate::SR96000;
            } else if ((m_ctIN2 < (DELTA_DATE_32000 + RX_BUFFER_SIZE)) && (m_ctIN2 > (DELTA_DATE_32000 - RX_BUFFER_SIZE))) {
                SampleRate = eSampleRate::SR32000;
            } else {
                SampleRate = eSampleRate::NoSync;
                m_Drif_Factor2 = 0;
                m_SampleRate2 = eSampleRate::NoSync;
                BuffIn2.setDate(0);   // Reset buffer 2 timestamp
                m_DateOut2 = 0;       // Reset output timestamp
            }

            m_ctIN2 = 0;

            // Synchronization logic for Buffer 2 (similar to Buffer 1)
            if (SampleRate != eSampleRate::NoSync) {
                if (SampleRate == m_SampleRate2) {
                    if (m_CtSynchro2 >= 1) {
                        // If synchronized, calculate drift factor
                        if (m_Drif_Factor2 == 0) {
                            m_Drif_Factor2 = BuffIn2.getDate() / (m_DateOut2 + TX_BUFFER_SIZE);
                        } else {
                            float DeltaFactor = m_Drif_Factor2 - BuffIn2.getDate() / (m_DateOut2 + TX_BUFFER_SIZE);
                            m_Drif_Factor2 -= DeltaFactor / 8;  // Smooth drift correction
                        }
                    } else {
                        m_CtSynchro2++;  // Increment synchronization counter
                    }
                } else {
                    m_SampleRate2 = SampleRate;
                    m_CtSynchro2 = 0;  // Reset synchronization counter
                }
            }
        }

        // Mix and interpolate samples from both buffers
        for (int Index = 0; Index < TX_BUFFER_SIZE; Index += 2) {
            float Sample1[2];  // Temporary buffer for Buffer 1 samples (L/R)
            float Sample2[2];  // Temporary buffer for Buffer 2 samples (L/R)

            // Pull interpolated samples from Buffer 1
            if (m_Drif_Factor1 != 0) {
                BuffIn1.Pull(Sample1, (m_DateOut1 * m_Drif_Factor1) - RX_BUFFER_SIZE);
            } else {
                Sample1[0] = 0;  // Silence if no valid samples
                Sample1[1] = 0;
            }

            // Pull interpolated samples from Buffer 2
            if (m_Drif_Factor2 != 0) {
                BuffIn2.Pull(Sample2, (m_DateOut2 * m_Drif_Factor2) - RX_BUFFER_SIZE);
            } else {
                Sample2[0] = 0;  // Silence if no valid samples
                Sample2[1] = 0;
            }

            // Mix the two sets of samples (average the values)
            pSamples[0] = static_cast<int32_t>((Sample1[0] + Sample2[0]) / CoefNormalizeIntToFloat);  // Left channel
            pSamples[1] = static_cast<int32_t>((Sample1[1] + Sample2[1]) / CoefNormalizeIntToFloat);  // Right channel

            // Move to the next output sample pair
            pSamples += 2;

            // Increment the output dates for both buffers
            m_DateOut1++;
            m_DateOut2++;
            m_ctPull++;
        }
    }

} /* namespace Dad */
