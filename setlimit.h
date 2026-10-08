#ifndef SETLIMIT_H
#define SETLIMIT_H

void drawSetlimit() {
    // Clear OLED buffer
    u8g2.clearBuffer();
    // Normal settings
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    u8g2.setFontDirection(0);
    // TITLE
    u8g2.setFont(u8g2_font_haxrcorp4089_tr);
    u8g2.drawStr(22, 12, "Set limit soil humidity");
    // SOIL LIMIT VALUE
    char limitText[4];
    sprintf(limitText, "%d", soilLimit);
    u8g2.setFont(u8g2_font_profont29_tr);
    u8g2.setFontDirection(0);
    u8g2.drawStr(48, 43, limitText);
    // PERCENT
    u8g2.setFont(u8g2_font_t0_16_tr);
    u8g2.setFontDirection(0);
    u8g2.drawStr(80, 42, "%");
    // EDIT ARROWS
    // Only show when OK is pressed
    if (editingSetLimit) {
        // Left 
        u8g2.setFont(u8g2_font_profont29_tr);
        u8g2.setFontDirection(0);
        u8g2.drawStr(23, 43, ">");
        // Right
        u8g2.setFontDirection(0);
        u8g2.drawStr(107, 43, "<");
        // Reset direction
        u8g2.setFontDirection(0);
    }
    u8g2.sendBuffer();
}

#endif