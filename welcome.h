#ifndef WELCOME_H
#define WELCOME_H

void drawWelcome() {

    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    // Welcome
    u8g2.setFont(u8g2_font_profont29_tr);
    u8g2.drawStr(9, 29, "Welcome ");
    // Control your garden
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(13, 44, "Control your garden");
    u8g2.sendBuffer();
}

#endif