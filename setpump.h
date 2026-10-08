#ifndef SETPUMP_H
#define SETPUMP_H

void drawSetpump() {

    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    u8g2.setFontDirection(0);

    int value = pumpEditing ? pumpEditValue : pumpSet;
    char valueText[3];
    snprintf(valueText, sizeof(valueText), "%02d", value);
    // PumpSet
    u8g2.setFont(u8g2_font_t0_22b_tr);
    u8g2.drawStr(27, 30, "PumpSet");
    // Value
    u8g2.drawStr(53, 54, valueText);
    // Choose value
    if (pumpEditing) {
        u8g2.drawStr(37, 53, ">   <");
    }
    u8g2.sendBuffer();
}

#endif

