#include "DuckDisplay.h"

#ifdef CDPCFG_OLED_CLASS
#include "include/DuckTypes.h"
#include "include/DuckEsp.h"
#include "include/DuckUtils.h"

#endif

#include <vector>

#ifdef CDPCFG_OLED_CLASS

#define u8g_logo_width 128
#define u8g_logo_height 64

//DuckDisplay* DuckDisplay::instance = NULL;

DuckDisplay::DuckDisplay() {}


#ifndef CDPCFG_OLED_NONE
std::string DuckDisplay::duckTypeToString(int duckType) {
  std::string duckTypeStr = "";
  switch (duckType) {
    case DuckType::PAPA:
      duckTypeStr = "Papa";
      break;
    case DuckType::LINK:
      duckTypeStr = "Link";
      break;
    case DuckType::DETECTOR:
      duckTypeStr = "Detector";
      break;
    case DuckType::MAMA:
      duckTypeStr = "Mama"; 
      break; 
    default:
      duckTypeStr = "Duck";
  }
  return duckTypeStr;
}
#endif // CDPCFG_OLED_NONE


void DuckDisplay::showDefaultScreen() {
#ifdef CDPCFG_OLED_64x32
  // small display 64x32
  setCursor(0, 2);
  print("CDP");
  setCursor(0, 4);
  print("DT: " + duckTypeToString(duckType));
  setCursor(0, 5);
  print("ID: " + duckName);
#else
  u8g2.clearBuffer();  
  // default display size 128x64
  drawString(0,10, "Clusterduck ");
  drawString(0,20, "Protocol ");
  setCursor(0, 40);
  print("DT: " + duckTypeToString(duckType));
  drawString(0,50,"v");
  setCursor(5, 50);
  print(duckutils::getCDPVersion().c_str());
  drawString(0,30, "----------------");
  setCursor(0, 60);
  print("ID: " + duckName);
  setCursor(0, 70);
  print(std::string("MC: ") + duckesp::getDuckMacAddress(false));
  u8g2.sendBuffer(); 
#endif // CDPCFG_OLED_64x32
}
#endif // CDPCFG_OLED_NONE
