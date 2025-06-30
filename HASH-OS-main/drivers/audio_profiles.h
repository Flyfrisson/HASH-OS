#ifndef AUDIO_PROFILES_H
#define AUDIO_PROFILES_H

typedef enum {
    PROFILE_UI_SOUNDS,
    PROFILE_MUSIC,
    PROFILE_CALL,
    PROFILE_NOTIFICATION
} AudioProfile;

void set_active_profile(AudioProfile profile);
AudioProfile get_active_profile();

#endif
