#include "audio_enhancer.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

// Internal state
static audio_config_t g_audio_config = {0};
static bool g_initialized = false;

// Internal helper functions
static audio_error_t validate_config(const audio_config_t* config);
static audio_error_t validate_buffer_params(const void* input, void* output, size_t size);
static float clamp_float(float value, float min, float max);
static int16_t clamp_int16(int32_t value);

// Error messages
static const char* error_messages[] = {
    "Success",
    "Invalid parameter",
    "Invalid audio format",
    "Buffer too small",
    "Out of memory",
    "Audio enhancer not initialized",
    "Audio processing failed"
};

audio_error_t init_audio_enhancer(const audio_config_t* config) {
    if (!config) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    audio_error_t result = validate_config(config);
    if (result != AUDIO_SUCCESS) {
        return result;
    }
    
    // Copy configuration
    memcpy(&g_audio_config, config, sizeof(audio_config_t));
    
    // Initialize any required resources here
    // For example: allocate filter buffers, initialize DSP state, etc.
    
    g_initialized = true;
    return AUDIO_SUCCESS;
}

audio_error_t cleanup_audio_enhancer(void) {
    if (!g_initialized) {
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    // Clean up any allocated resources
    memset(&g_audio_config, 0, sizeof(audio_config_t));
    g_initialized = false;
    
    return AUDIO_SUCCESS;
}

audio_error_t set_audio_config(const audio_config_t* config) {
    if (!g_initialized) {
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!config) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    audio_error_t result = validate_config(config);
    if (result != AUDIO_SUCCESS) {
        return result;
    }
    
    memcpy(&g_audio_config, config, sizeof(audio_config_t));
    return AUDIO_SUCCESS;
}

audio_error_t get_audio_config(audio_config_t* config) {
    if (!g_initialized) {
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!config) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    memcpy(config, &g_audio_config, sizeof(audio_config_t));
    return AUDIO_SUCCESS;
}

audio_error_t apply_audio_enhancements(
    const void* input_buffer, 
    void* output_buffer, 
    size_t buffer_size,
    size_t* processed_size
) {
    if (!g_initialized) {
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    audio_error_t result = validate_buffer_params(input_buffer, output_buffer, buffer_size);
    if (result != AUDIO_SUCCESS) {
        return result;
    }
    
    // Start with input copied to output
    memcpy(output_buffer, input_buffer, buffer_size);
    
    // Apply enhancements in order
    if (g_audio_config.enhancement_flags & ENHANCEMENT_NOISE_REDUCTION) {
        result = apply_noise_reduction(output_buffer, output_buffer, buffer_size);
        if (result != AUDIO_SUCCESS) return result;
    }
    
    if (g_audio_config.enhancement_flags & ENHANCEMENT_EQUALIZER) {
        result = apply_equalizer(output_buffer, output_buffer, buffer_size);
        if (result != AUDIO_SUCCESS) return result;
    }
    
    if (g_audio_config.enhancement_flags & ENHANCEMENT_COMPRESSOR) {
        result = apply_compressor(output_buffer, output_buffer, buffer_size);
        if (result != AUDIO_SUCCESS) return result;
    }
    
    if (g_audio_config.enhancement_flags & ENHANCEMENT_BOOST) {
        result = apply_audio_boost(output_buffer, output_buffer, buffer_size);
        if (result != AUDIO_SUCCESS) return result;
    }
    
    if (g_audio_config.enhancement_flags & ENHANCEMENT_REVERB) {
        result = apply_reverb(output_buffer, output_buffer, buffer_size);
        if (result != AUDIO_SUCCESS) return result;
    }
    
    if (processed_size) {
        *processed_size = buffer_size;
    }
    
    return AUDIO_SUCCESS;
}

audio_error_t apply_audio_boost(const void* input, void* output, size_t size) {
    if (!g_initialized) {
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    audio_error_t result = validate_buffer_params(input, output, size);
    if (result != AUDIO_SUCCESS) {
        return result;
    }
    
    // Apply boost based on format
    switch (g_audio_config.format) {
        case AUDIO_FORMAT_PCM_16BIT: {
            const int16_t* in_samples = (const int16_t*)input;
            int16_t* out_samples = (int16_t*)output;
            size_t sample_count = size / sizeof(int16_t);
            
            for (size_t i = 0; i < sample_count; i++) {
                int32_t boosted = (int32_t)(in_samples[i] * g_audio_config.boost_gain);
                out_samples[i] = clamp_int16(boosted);
            }
            break;
        }
        
        case AUDIO_FORMAT_FLOAT32: {
            const float* in_samples = (const float*)input;
            float* out_samples = (float*)output;
            size_t sample_count = size / sizeof(float);
            
            for (size_t i = 0; i < sample_count; i++) {
                out_samples[i] = clamp_float(
                    in_samples[i] * g_audio_config.boost_gain, 
                    -1.0f, 1.0f
                );
            }
            break;
        }
        
        default:
            return AUDIO_ERROR_INVALID_FORMAT;
    }
    
    return AUDIO_SUCCESS;
}

audio_error_t apply_noise_reduction(const void* input, void* output, size_t size) {
    if (!g_initialized) {
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    audio_error_t result = validate_buffer_params(input, output, size);
    if (result != AUDIO_SUCCESS) {
        return result;
    }
    
    // Simple noise gate implementation
    switch (g_audio_config.format) {
        case AUDIO_FORMAT_PCM_16BIT: {
            const int16_t* in_samples = (const int16_t*)input;
            int16_t* out_samples = (int16_t*)output;
            size_t sample_count = size / sizeof(int16_t);
            
            int16_t threshold = (int16_t)(32767 * g_audio_config.noise_threshold);
            
            for (size_t i = 0; i < sample_count; i++) {
                if (abs(in_samples[i]) < threshold) {
                    out_samples[i] = 0;  // Silence below threshold
                } else {
                    out_samples[i] = in_samples[i];
                }
            }
            break;
        }
        
        case AUDIO_FORMAT_FLOAT32: {
            const float* in_samples = (const float*)input;
            float* out_samples = (float*)output;
            size_t sample_count = size / sizeof(float);
            
            for (size_t i = 0; i < sample_count; i++) {
                if (fabsf(in_samples[i]) < g_audio_config.noise_threshold) {
                    out_samples[i] = 0.0f;
                } else {
                    out_samples[i] = in_samples[i];
                }
            }
            break;
        }
        
        default:
            return AUDIO_ERROR_INVALID_FORMAT;
    }
    
    return AUDIO_SUCCESS;
}

audio_error_t apply_equalizer(const void* input, void* output, size_t size) {
    // Simple 3-band EQ implementation
    // This is a simplified version - real EQ would use proper filters
    if (!g_initialized) {
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    // For now, just copy input to output
    // TODO: Implement proper EQ filters (low-pass, band-pass, high-pass)
    memcpy(output, input, size);
    
    return AUDIO_SUCCESS;
}

audio_error_t apply_reverb(const void* input, void* output, size_t size) {
    // Simple reverb implementation
    if (!g_initialized) {
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    // For now, just copy input to output
    // TODO: Implement delay lines and feedback for reverb effect
    memcpy(output, input, size);
    
    return AUDIO_SUCCESS;
}

audio_error_t apply_compressor(const void* input, void* output, size_t size) {
    // Simple compressor implementation
    if (!g_initialized) {
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    // For now, just copy input to output
    // TODO: Implement dynamic range compression
    memcpy(output, input, size);
    
    return AUDIO_SUCCESS;
}

// Utility functions
bool is_audio_enhancer_initialized(void) {
    return g_initialized;
}

const char* get_audio_error_string(audio_error_t error) {
    if (error >= 0 && error < sizeof(error_messages) / sizeof(error_messages[0])) {
        return error_messages[error];
    }
    return "Unknown error";
}

size_t get_required_buffer_size(size_t input_size) {
    // For most effects, output size equals input size
    // Some effects might require larger buffers
    return input_size;
}

// Configuration presets
void get_default_config(audio_config_t* config) {
    if (!config) return;
    
    config->sample_rate = 44100;
    config->channels = 2;
    config->bit_depth = 16;
    config->format = AUDIO_FORMAT_PCM_16BIT;
    config->enhancement_flags = ENHANCEMENT_NONE;
    config->boost_gain = 1.0f;
    config->noise_threshold = 0.01f;
    config->eq_bass = 0.0f;
    config->eq_mid = 0.0f;
    config->eq_treble = 0.0f;
}

void get_music_preset(audio_config_t* config) {
    get_default_config(config);
    config->enhancement_flags = ENHANCEMENT_EQUALIZER | ENHANCEMENT_REVERB;
    config->eq_bass = 2.0f;
    config->eq_mid = 0.0f;
    config->eq_treble = 1.0f;
}

void get_voice_preset(audio_config_t* config) {
    get_default_config(config);
    config->enhancement_flags = ENHANCEMENT_NOISE_REDUCTION | ENHANCEMENT_COMPRESSOR;
    config->noise_threshold = 0.05f;
    config->eq_mid = 3.0f;
}

void get_gaming_preset(audio_config_t* config) {
    get_default_config(config);
    config->enhancement_flags = ENHANCEMENT_BOOST | ENHANCEMENT_EQUALIZER;
    config->boost_gain = 1.2f;
    config->eq_bass = 3.0f;
    config->eq_treble = 2.0f;
}

// Internal helper functions
static audio_error_t validate_config(const audio_config_t* config) {
    if (!config) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    if (config->sample_rate == 0 || config->channels == 0 || config->bit_depth == 0) {
        return AUDIO_ERROR_INVALID_FORMAT;
    }
    
    if (config->boost_gain < 0.0f || config->boost_gain > 2.0f) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    if (config->noise_threshold < 0.0f || config->noise_threshold > 1.0f) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    return AUDIO_SUCCESS;
}

static audio_error_t validate_buffer_params(const void* input, void* output, size_t size) {
    if (!input || !output) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    if (size == 0) {
        return AUDIO_ERROR_BUFFER_TOO_SMALL;
    }
    
    return AUDIO_SUCCESS;
}

static float clamp_float(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static int16_t clamp_int16(int32_t value) {
    if (value < -32768) return -32768;
    if (value > 32767) return 32767;
    return (int16_t)value;
}