#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

// Initialize the audio manager system
void init_audio_manager();
void play_audio(void *buffer, int size);

// Background loop to manage audio services
void audio_manager_background_loop();

#endif // AUDIO_MANAGER_H
