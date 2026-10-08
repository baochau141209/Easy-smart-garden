#ifndef SETHOURS_H
#define SETHOURS_H

void drawSethours() {

    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    u8g2.setFontDirection(0);

    char hourText[3];
    char minText[3];
    const char* ampm = (editWateringHour < 12) ? "AM" : "PM";

    snprintf(hourText, sizeof(hourText), "%02d", editWateringHour);
    snprintf(minText, sizeof(minText), "%02d", editWateringMinute);
    // Scheduled daily watering time
    u8g2.setFont(u8g2_font_t0_22b_tr);
    u8g2.drawStr(18, 37, hourText);
    u8g2.drawStr(58, 37, minText);
    u8g2.drawStr(44, 37, ":");
    u8g2.drawStr(88, 37, ampm);
    // Choose Hours
    if (setHoursEditing && setHoursField == 1) {
        u8g2.setFontDirection(1);
        u8g2.drawStr(23, 8, ">  <");
        u8g2.setFontDirection(0);
    }
    // Choose mins
    else if (setHoursEditing && setHoursField == 2) {
        u8g2.setFontDirection(1);
        u8g2.drawStr(63, 8, ">  <");
        u8g2.setFontDirection(0);
    }
    u8g2.sendBuffer();
}

#endif