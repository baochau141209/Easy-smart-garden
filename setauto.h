#ifndef SETAUTO_H
#define SETAUTO_H

void drawSetauto() {

    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    // Reset direction
    u8g2.setFontDirection(0);
    // Title
    u8g2.setFont(u8g2_font_t0_12b_tr);
    u8g2.drawStr(8, 16, "Set auto water pump");
    // Value
    char humdText[4];
    char pumpText[4];
    sprintf(humdText, "%02d", humdSet);
    sprintf(pumpText, "%02d", pumpSet);
    u8g2.setFont(u8g2_font_t0_22b_tr);
    // Humd set value
    u8g2.drawStr(14, 52, humdText);
    // Pump set value
    u8g2.drawStr(76, 52, pumpText);
    // Percent
    u8g2.setFont(u8g2_font_t0_17b_tr);
    u8g2.drawStr(42, 52, "%");
    u8g2.drawStr(104, 52, "%");
    // Label
    u8g2.setFont(u8g2_font_t0_11b_tr);
    u8g2.drawStr(11, 35, "Humd set");
    u8g2.drawStr(71, 35, "Pump set");

    if (editingSetAuto) {

        if (setAutoChoose == 1) {

            u8g2.drawStr(6, 50, ">       <");
        }

        else if (setAutoChoose == 2) {

            u8g2.drawStr(69, 50, ">       <");
        }
    }
    u8g2.sendBuffer();
}

#endif