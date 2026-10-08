#ifndef CHECK_H
#define CHECK_H

void drawCheck() {
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);

  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(34, 13, "Check Now");
  u8g2.drawStr(13, 29, "Soil humidity:");

 
  char rawText[6];
snprintf(rawText, sizeof(rawText), "%d", soilValue);
u8g2.drawStr(18, 42, "Raw:");
u8g2.drawStr(48, 42, rawText);

  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(18, 42, "Cond:");

  const char* condition;
  if (soilValue < 35) {
    condition = "Dry";
  } else if (soilValue < 70) {
    condition = "Normal";
  } else {
    condition = "Warning";
  }

  u8g2.setFont(u8g2_font_t0_11b_tr);
  u8g2.drawStr(41, 56, condition);
  u8g2.sendBuffer();
}

#endif