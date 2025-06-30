#ifndef AUDIO_ENHANCER_H
#define AUDIO_ENHANCER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Audio format definitions
typedef enum {
    AUDIO_FORMAT_PCM_8BIT,
    AUDIO_FORMAT_PCM_16BIT,
    AUDIO_FORMAT_PCM_24BIT,
    AUDIO_FORMAT_PCM_32BIT,
    AUDIO_FORMAT_FLOAT32
} audio_format_t;

// Audio enhancement types
typedef enum {
    ENHANCEMENT_NONE       = 0x00,
    ENHANCEMENT_BOOST      = 0x01,
    ENHANCEMENT_NOISE_REDUCTION = 0x02,
    ENHANCEMENT_EQUALIZER  = 0x04,
    ENHANCEMENT_REVERB     = 0x08,
    ENHANCEMENT_COMPRESSOR = 0x10,
    ENHANCEMENT_ALL        = 0xFF
} audio_enhancement_t;

// Audio configuration structure
typedef struct {
    uint32_t sample_rate;        // Sample rate in Hz (e.g., 44100, 48000)
    uint16_t channels;           // Number of channels (1=mono, 2=stereo)
    uint16_t bit_depth;          // Bits per sample (8, 16, 24, 32)
    audio_format_t format;       // Audio format
    uint8_t enhancement_flags;   // Bitmask of enabled enhancements
    
    // Enhancement parameters
    float boost_gain;            // Audio boost gain (0.0 to 2.0)
    float noise_threshold;       // Noise reduction threshold (0.0 to 1.0)
    float eq_bass;              // Bass adjustment (-12.0 to +12.0 dB)
    float eq_mid;               // Mid adjustment (-12.0 to +12.0 dB)
    float eq_treble;            // Treble adjustment (-12.0 to +12.0 dB)
} audio_config_t;

// Error codes
typedef enum {
    AUDIO_SUCCESS = 0,
    AUDIO_ERROR_INVALID_PARAM,
    AUDIO_ERROR_INVALID_FORMAT,
    AUDIO_ERROR_BUFFER_TOO_SMALL,
    AUDIO_ERROR_OUT_OF_MEMORY,
    AUDIO_ERROR_NOT_INITIALIZED,
    AUDIO_ERROR_PROCESSING_FAILED
} audio_error_t;

// Function prototypes
audio_error_t init_audio_enhancer(const audio_config_t* config);
audio_error_t cleanup_audio_enhancer(void);
audio_error_t set_audio_config(const audio_config_t* config);
audio_error_t get_audio_config(audio_config_t* config);

// Main processing function
audio_error_t apply_audio_enhancements(
    const void* input_buffer, 
    void* output_buffer, 
    size_t buffer_size,
    size_t* processed_size
);

// Individual enhancement functions
audio_error_t apply_audio_boost(const void* input, void* output, size_t size);
audio_error_t apply_noise_reduction(const void* input, void* output, size_t size);
audio_error_t apply_equalizer(const void* input, void* output, size_t size);
audio_error_t apply_reverb(const void* input, void* output, size_t size);
audio_error_t apply_compressor(const void* input, void* output, size_t size);

// Utility functions
bool is_audio_enhancer_initialized(void);
const char* get_audio_error_string(audio_error_t error);
size_t get_required_buffer_size(size_t input_size);

// Configuration presets
void get_default_config(audio_config_t* config);
void get_music_preset(audio_config_t* config);
void get_voice_preset(audio_config_t* config);
void get_gaming_preset(audio_config_t* config);

#endif // AUDIO_ENHANCER_H