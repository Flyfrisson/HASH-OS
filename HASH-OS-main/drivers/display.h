#ifndef HASHOS_DISPLAY_H
#define HASHOS_DISPLAY_H

// Display system initialization and basic operations
void InitDisplay();
void ClearScreen();
void PutChar(unsigned char aChar);
void PutString(const char* str);

// Progress bar functionality
void InitProgressBar(int id, unsigned int limit, const char* title);
void UpdateProgressBar(int id, unsigned int progress);

// Advanced display functions
void SetCursorPosition(int x, int y);
void SetTextColor(unsigned char foreground, unsigned char background);
void DrawPixelFont(int x, int y, unsigned char ch, unsigned char color);

#endif // HASHOS_DISPLAY_H