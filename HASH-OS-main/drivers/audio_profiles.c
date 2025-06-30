#include "audio_profiles.h"

static AudioProfile current_profile = PROFILE_UI_SOUNDS;

void set_active_profile(AudioProfile profile) {
    current_profile = profile;
}

AudioProfile get_active_profile() {
    return current_profile;
}
