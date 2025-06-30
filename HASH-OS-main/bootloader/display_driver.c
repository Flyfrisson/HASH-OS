// display.h - Complete HASH OS Display Driver Header
#ifndef HASHOS_DISPLAY_H
#define HASHOS_DISPLAY_H

// Function prototypes for display operations
void InitDisplay();
void ClearScreen();
void PutChar(unsigned char aChar);
void PutString(const char* str);
void InitProgressBar(int id, unsigned int limit, const char* title);
void UpdateProgressBar(int id, unsigned int progress);

#endif // HASHOS_DISPLAY_H