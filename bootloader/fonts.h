#ifndef HASHOS_FONTS_H
#define HASHOS_FONTS_H

#define FONT_WIDTH  8
#define FONT_HEIGHT 10
#define FONT_FIRST_CHAR 32  // Space character
#define FONT_LAST_CHAR  126 // ~ character
#define FONT_CHAR_COUNT 95  // Total characters in font

extern const unsigned char Font[FONT_CHAR_COUNT][FONT_HEIGHT];

// Helper function to get font data for a character
static inline const unsigned char* GetCharFont(unsigned char ch) {
    if (ch < FONT_FIRST_CHAR || ch > FONT_LAST_CHAR) {
        return Font[0]; // Return space for invalid characters
    }
    return Font[ch - FONT_FIRST_CHAR];
}

#endif // HASHOS_FONTS_H