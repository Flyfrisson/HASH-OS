#ifndef HASHOS_DISPLAY_DRIVER_H
#define HASHOS_DISPLAY_DRIVER_H

// These will be implemented by the platform-specific display driver
unsigned int GetScreenWidth();
unsigned int GetScreenHeight();
unsigned char* GetScreenBuffer();
unsigned int GetPixelSize();

#endif
