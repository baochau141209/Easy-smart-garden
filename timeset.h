#ifndef TIMESET_H
#define TIMESET_H

void drawTimeset() {

    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    u8g2.setFontDirection(0);
    // Current real time from NTP
    u8g2.setFont(u8g2_font_helvB08_tr);
    u8g2.drawStr(37, 11, currentTimeText);
    u8g2.drawStr(73, 11, currentAmPmText);
    // SET TIME
    u8g2.setFont(u8g2_font_t0_22b_tr);
    u8g2.drawStr(20, 34, "SET TIME");
    // SET PUMP
    u8g2.drawStr(20, 56, "SET PUMP");
    // Selection frame
    if (timeSetSelecting) {
        if (timeSetChoose == 1) {
            u8g2.drawFrame(7, 18, 115, 19);
        }
        else if (timeSetChoose == 2) {
            u8g2.drawFrame(7, 40, 115, 19);
        }
    }
    u8g2.sendBuffer();
}

#endif