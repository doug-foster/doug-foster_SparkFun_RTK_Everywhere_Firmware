//----------------------------------------
//Constants
//----------------------------------------

//A bitfield is used to flag which icon needs to be illuminated
//systemState will dictate most of the icons needed

//The radio area (top left corner of display) has three spots for icons
//Left/Center/Right
//Left Radio spot
#define ICON_WIFI_SYMBOL_0_LEFT          (1<<0) //  0,  0
#define ICON_WIFI_SYMBOL_1_LEFT          (1<<1) //  0,  0
#define ICON_WIFI_SYMBOL_2_LEFT          (1<<2) //  0,  0
#define ICON_WIFI_SYMBOL_3_LEFT          (1<<3) //  0,  0
#define ICON_BT_SYMBOL_LEFT              (1<<4) //  0,  0
#define ICON_MAC_ADDRESS                 (1<<5) //  0,  3
#define ICON_ESPNOW_SYMBOL_0_LEFT        (1<<6) //  0,  0
#define ICON_ESPNOW_SYMBOL_1_LEFT        (1<<7) //  0,  0
#define ICON_ESPNOW_SYMBOL_2_LEFT        (1<<8) //  0,  0
#define ICON_ESPNOW_SYMBOL_3_LEFT        (1<<9) //  0,  0
#define ICON_DOWN_ARROW_LEFT             (1<<10) //  0,  0
#define ICON_UP_ARROW_LEFT               (1<<11) //  0,  0
#define ICON_BLANK_LEFT                  (1<<12) //  0,  0

//Center Radio spot
#define ICON_MAC_ADDRESS_2DIGIT          (1<<13) //  13,  3
#define ICON_BT_SYMBOL_CENTER            (1<<14) //  10,  0
#define ICON_DOWN_ARROW_CENTER           (1<<15) //  0,  0
#define ICON_UP_ARROW_CENTER             (1<<16) //  0,  0

//Right Radio Spot
#define ICON_WIFI_SYMBOL_0_RIGHT         (1<<17) // center, 0
#define ICON_WIFI_SYMBOL_1_RIGHT         (1<<18) // center, 0
#define ICON_WIFI_SYMBOL_2_RIGHT         (1<<19) // center, 0
#define ICON_WIFI_SYMBOL_3_RIGHT         (1<<20) // center, 0
#define ICON_BASE_TEMPORARY              (1<<21) // center,  0
#define ICON_BASE_FIXED                  (1<<22) // center,  0
#define ICON_ROVER_FUSION                (1<<23) // center,  2
#define ICON_ROVER_FUSION_EMPTY          (1<<24) // center,  2
#define ICON_DYNAMIC_MODEL               (1<<25) // 27,  0
#define ICON_DOWN_ARROW_RIGHT            (1<<26) // center,  0
#define ICON_UP_ARROW_RIGHT              (1<<27) // center,  0
#define ICON_BLANK_RIGHT                 (1<<28) // center,  0

//Left + Center Radio spot
#define ICON_IP_ADDRESS                  (1<<29)

//Right top
#define ICON_BATTERY                     (1<<0) // 45,  0

//Left center
#define ICON_CROSS_HAIR                  (1<<1) //  0, 18
#define ICON_CROSS_HAIR_DUAL             (1<<2) //  0, 18

//Right center
#define ICON_HORIZONTAL_ACCURACY         (1<<3) // 16, 20

//Left bottom
#define ICON_SIV_ANTENNA                 (1<<4) //  2, 35
#define ICON_SIV_ANTENNA_LBAND           (1<<5) //  2, 35

//Right bottom
#define ICON_LOGGING                     (1<<6) // right, bottom

//Left center
#define ICON_CLOCK                       (1<<7)
#define ICON_CLOCK_ACCURACY              (1<<8)

//Right top
#define ICON_ETHERNET                    (1<<9)

//Right bottom
#define ICON_LOGGING_NTP                 (1<<10)

//Left bottom
#define ICON_ANTENNA_SHORT               (1<<11)
#define ICON_ANTENNA_OPEN                (1<<12)

//----------------------------------------
//Locals
//----------------------------------------

static QwiicMicroOLED oled;
static uint32_t blinking_icons;
static uint32_t icons;
static uint32_t iconsRadio;

unsigned long ssidDisplayTimer = 0;
bool ssidDisplayFirstHalf = false;

//Fonts
#include <res/qw_fnt_5x7.h>
#include <res/qw_fnt_8x16.h>
#include <res/qw_fnt_largenum.h>

//Icons
#include "icons.h"

//----------------------------------------
//Routines
//----------------------------------------

uint32_t setRadioIcons()
{
  uint32_t icons = 0;

  icons |= ICON_MAC_ADDRESS;

  icons |= setModeIcon();

  return icons;
}

uint32_t setWiFiIcon()
{
  return ICON_WIFI_SYMBOL_3_RIGHT;
}

void beginDisplay()
{
  blinking_icons = 0;

  //At this point we have not identified the RTK platform
  //If it's surveyor, there won't be a display and we have a 100ms delay
  //If it's other platforms, we will try 3 times
  int maxTries = 3;
  for (int x = 0 ; x < maxTries ; x++)
  {
    if (oled.begin() == true)
    {
      online.display = true;

      Serial.println("Display started");

      //Display the SparkFun LOGO
      oled.erase();
      displayBitmap(0, 0, logoSparkFun_Width, logoSparkFun_Height, logoSparkFun);
      oled.display();

      delay(1000);
      
      return;
    }

    delay(50); //Give display time to startup before attempting again
  }

  Serial.println("Display not detected");
}

//Avoid code repetition
void displayBatteryVsEthernet()
{
    if (fuelGaugeType != FUEL_GAUGE_TYPE_NONE) // Product has a battery
    icons |= ICON_BATTERY; //Top right
  else //if (present.ethernet_ws5500 == true)
  {
    if (online.ethernetStatus == ETH_NOT_STARTED)
      blinking_icons &= ~ICON_ETHERNET; //If Ethernet has not stated because not needed, don't display the icon
    else if (online.ethernetStatus == ETH_CONNECTED)
      blinking_icons |= ICON_ETHERNET; //Don't blink if link is up
    else
      blinking_icons ^= ICON_ETHERNET;
    icons |= (blinking_icons & ICON_ETHERNET); //Top Right
  }
}
void displaySivVsOpenShort(bool doUpdate)
{
  if (!HAS_ANTENNA_SHORT_OPEN)
    icons |= paintSIV();
  else
  {
    if (aStatus == SFE_UBLOX_ANTENNA_STATUS_SHORT)
    {
      if (doUpdate)
        blinking_icons ^= ICON_ANTENNA_SHORT;
      else
        blinking_icons |= ICON_ANTENNA_SHORT;
      icons |= (blinking_icons & ICON_ANTENNA_SHORT);
    }
    else if (aStatus == SFE_UBLOX_ANTENNA_STATUS_OPEN)
    {
      if (doUpdate)
        blinking_icons ^= ICON_ANTENNA_OPEN;
      else
        blinking_icons |= ICON_ANTENNA_OPEN;
      icons |= (blinking_icons & ICON_ANTENNA_OPEN);
    }
    else
    {
      blinking_icons &= ~ICON_ANTENNA_SHORT;
      blinking_icons &= ~ICON_ANTENNA_OPEN;
      icons |= paintSIV();
    }
  }
}

//Given the system state, display the appropriate information
void updateDisplay(bool doUpdate)
{
  //Update the display if connected
  if (online.display == true)
  {
    if (millis() - lastDisplayUpdate > 500) //Update display at 2Hz
    {
      lastDisplayUpdate = millis();

      oled.reset(false); //Incase of previous corruption, force re-alignment of CGRAM. Do not init buffers as it takes time and causes screen to blink.

      oled.erase();

      icons = 0;
      iconsRadio = 0;
      switch (systemState)
      {

        /*
                       111111111122222222223333333333444444444455555555556666
             0123456789012345678901234567890123456789012345678901234567890123
            .----------------------------------------------------------------
           0|   *******         **             **         *****************
           1|  *       *        **             **         *               *
           2| *  *****  *       **          ******        * ***  ***  *** *
           3|*  *     *  *      **         *      *       * ***  ***  *** ***
           4|  *  ***  *        **       * * **** * *     * ***  ***  ***   *
           5|    *   *       ** ** **    * * **** * *     * ***  ***  ***   *
           6|      *          ******     * *      * *     * ***  ***  ***   *
           7|     ***          ****      * *      * *     * ***  ***  ***   *
           8|      *            **       * *      * *     * ***  ***  *** ***
           9|                            * *      * *     * ***  ***  *** *
          10|                              *      *       *               *
          11|                               ******        *****************
          12|
          13|
          14|
          15|
          16|
          17|
          18|       *
          19|       *
          20|    *******
          21|   *   *   *               ***               ***      ***
          22|  *    *    *             *   *             *   *    *   *
          23|  *    *    *             *   *             *   *    *   *
          24|  *    *    *     **       * *               * *      * *
          25|******* *******   **        *                 *        *
          26|  *    *    *              * *               * *      * *
          27|  *    *    *             *   *             *   *    *   *
          28|  *    *    *             *   *             *   *    *   *
          29|   *   *   *      **      *   *     **      *   *    *   *
          30|    *******       **       ***      **       ***      ***
          31|       *
          32|       *
          33|
          34|
          35|
          36|   **                                                  *******
          37|   * *                    ***      ***                 *     **
          38|   *  *   *              *   *    *   *                *      **
          39|   *   * *               *   *    *   *                *       *
          40|    *   *        **       * *      * *                 * ***** *
          41|    *    *       **        *        *                  *       *
          42|     *    *               * *      * *                 * ***** *
          43|     **    *             *   *    *   *                *       *
          44|     ****   *            *   *    *   *                * ***** *
          45|     **  ****    **      *   *    *   *                *       *
          46|     **          **       ***      ***                 *       *
          47|   ******                                              *********
        */

        case (STATE_ROVER_NOT_STARTED):
          icons =   ICON_CROSS_HAIR     //Center left
                    | ICON_HORIZONTAL_ACCURACY //Center right
                    | ICON_LOGGING;       //Bottom right
          displaySivVsOpenShort(doUpdate); //Bottom left
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          break;
        case (STATE_ROVER_NO_FIX):
          icons =   ICON_CROSS_HAIR     //Center left
                    | ICON_HORIZONTAL_ACCURACY //Center right
                    | ICON_LOGGING;       //Bottom right
          displaySivVsOpenShort(doUpdate); //Bottom left
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          break;
        case (STATE_ROVER_FIX):
          icons =   ICON_CROSS_HAIR     //Center left
                    | ICON_HORIZONTAL_ACCURACY //Center right
                    | ICON_LOGGING;       //Bottom right
          displaySivVsOpenShort(doUpdate); //Bottom left
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          break;
        case (STATE_ROVER_RTK_FLOAT):
          blinking_icons ^= ICON_CROSS_HAIR_DUAL;
          icons =   (blinking_icons & ICON_CROSS_HAIR_DUAL)  //Center left
                    | ICON_HORIZONTAL_ACCURACY //Center right
                    | ICON_LOGGING;       //Bottom right
          displaySivVsOpenShort(doUpdate); //Bottom left
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          break;
        case (STATE_ROVER_RTK_FIX):
          icons =   ICON_CROSS_HAIR_DUAL//Center left
                    | ICON_HORIZONTAL_ACCURACY //Center right
                    | ICON_LOGGING;       //Bottom right
          displaySivVsOpenShort(doUpdate); //Bottom left
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          break;

        case (STATE_BASE_NOT_STARTED):
          //Do nothing. Static display shown during state change.
          break;

        //Start of base / survey in / NTRIP mode
        //Screen is displayed while we are waiting for horz accuracy to drop to appropriate level
        //Blink crosshair icon until we have we have horz accuracy < user defined level
        case (STATE_BASE_TEMP_SETTLE):
          blinking_icons ^= ICON_CROSS_HAIR;
          icons =   (blinking_icons & ICON_CROSS_HAIR)  //Center left
                    | ICON_HORIZONTAL_ACCURACY //Center right
                    | ICON_LOGGING;       //Bottom right
          displaySivVsOpenShort(doUpdate); //Bottom left
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          break;
        case (STATE_BASE_TEMP_SURVEY_STARTED):
          icons =   ICON_LOGGING;       //Bottom right
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          paintBaseTempSurveyStarted(doUpdate);
          break;
        case (STATE_BASE_TEMP_TRANSMITTING):
          icons =   ICON_LOGGING;       //Bottom right
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          paintRTCM(doUpdate);
          break;
        case (STATE_BASE_FIXED_NOT_STARTED):
          icons = 0;       //Top right
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          break;
        case (STATE_BASE_FIXED_TRANSMITTING):
          icons =   ICON_LOGGING;       //Bottom right
          displayBatteryVsEthernet(); //Top right
          iconsRadio = setRadioIcons(); //Top left
          paintRTCM(doUpdate);
          break;

        case (STATE_NTPSERVER_NOT_STARTED):
        case (STATE_NTPSERVER_NO_SYNC):
          blinking_icons ^= ICON_CLOCK;
          icons =   (blinking_icons & ICON_CLOCK) //Center left
                    | ICON_CLOCK_ACCURACY; //Center right
          displaySivVsOpenShort(doUpdate); //Bottom left
          if (online.ethernetStatus == ETH_CONNECTED)
            blinking_icons |= ICON_ETHERNET; //Don't blink if link is up
          else
            blinking_icons ^= ICON_ETHERNET;
          icons |= (blinking_icons & ICON_ETHERNET); //Top Right
          iconsRadio = ICON_IP_ADDRESS;   //Top left
          break;

        case (STATE_NTPSERVER_SYNC):
          icons =   ICON_CLOCK            //Center left
                    | ICON_CLOCK_ACCURACY //Center right
                    | ICON_LOGGING_NTP;   //Bottom right
          displaySivVsOpenShort(doUpdate); //Bottom left
          if (online.ethernetStatus == ETH_CONNECTED)
            blinking_icons |= ICON_ETHERNET; //Don't blink if link is up
          else
            blinking_icons ^= ICON_ETHERNET;
          icons |= (blinking_icons & ICON_ETHERNET); //Top Right
          iconsRadio = ICON_IP_ADDRESS;   //Top left
          break;

        case (STATE_CONFIG_VIA_ETH_NOT_STARTED):
          break;
        case (STATE_CONFIG_VIA_ETH_STARTED):
          break;
        case (STATE_CONFIG_VIA_ETH):
          displayConfigViaEthernet(doUpdate);
          break;
        case (STATE_CONFIG_VIA_ETH_RESTART_BASE):
          break;

        case (STATE_BUBBLE_LEVEL):
          //paintBubbleLevel();
          break;
        case (STATE_PROFILE):
          paintProfile(displayProfile);
          break;
        case (STATE_MARK_EVENT):
          //Do nothing. Static display shown during state change.
          break;
        case (STATE_DISPLAY_SETUP):
          paintDisplaySetup();
          break;
        case (STATE_WIFI_CONFIG_NOT_STARTED):
          displayWiFiConfigNotStarted(); //Display 'WiFi Config'
          break;
        case (STATE_WIFI_CONFIG):
          iconsRadio = setWiFiIcon(); //Blink WiFi in center
          displayWiFiConfig(); //Display SSID and IP
          break;
        case (STATE_TEST):
          //paintSystemTest();
          break;
        case (STATE_TESTING):
          //paintSystemTest();
          break;

        case (STATE_KEYS_STARTED):
          //paintRTCWait();
          break;
        case (STATE_KEYS_NEEDED):
          //Do nothing. Quick, fall through state.
          break;
        case (STATE_KEYS_WIFI_STARTED):
          iconsRadio = setWiFiIcon(); //Blink WiFi in center
          //paintGettingKeys();
          break;
        case (STATE_KEYS_WIFI_CONNECTED):
          iconsRadio = setWiFiIcon(); //Blink WiFi in center
          //paintGettingKeys();
          break;
        case (STATE_KEYS_WIFI_TIMEOUT):
          //Do nothing. Quick, fall through state.
          break;
        case (STATE_KEYS_EXPIRED):
          //Do nothing. Quick, fall through state.
          break;
        case (STATE_KEYS_DAYS_REMAINING):
          //Do nothing. Quick, fall through state.
          break;
        case (STATE_KEYS_LBAND_CONFIGURE):
          //paintLBandConfigure();
          break;
        case (STATE_KEYS_LBAND_ENCRYPTED):
          //Do nothing. Quick, fall through state.
          break;
        case (STATE_KEYS_PROVISION_STARTED):
          iconsRadio = setWiFiIcon(); //Blink WiFi in center
          //paintGettingKeys();
          break;
        case (STATE_KEYS_PROVISION_CONNECTED):
          iconsRadio = setWiFiIcon(); //Blink WiFi in center
          //paintGettingKeys();
          break;
        case (STATE_KEYS_PROVISION_WIFI_TIMEOUT):
          //Do nothing. Quick, fall through state.
          break;

        case (STATE_ESPNOW_PAIRING_NOT_STARTED):
          paintEspNowPairing();
          break;
        case (STATE_ESPNOW_PAIRING):
          paintEspNowPairing();
          break;

        case (STATE_SHUTDOWN):
          displayShutdown();
          break;
        default:
          Serial.printf("Unknown display: %d\r\n", systemState);
          displayError("Display");
          break;
      }

      //Top left corner - Radio icon indicators take three spots (left/center/right)
      //Allowed icon combinations:
      //Bluetooth + Rover/Base
      //WiFi + Bluetooth + Rover/Base
      //ESP-Now + Bluetooth + Rover/Base
      //ESP-Now + Bluetooth + WiFi
      //See setRadioIcons() for the icon selection logic

      //Left spot
      if (iconsRadio & ICON_MAC_ADDRESS)
      {
        char macAddress[5];
        const uint8_t * rtkMacAddress = getMacAddress();

        //Print four characters of MAC
        snprintf(macAddress, sizeof(macAddress), "%02X%02X", rtkMacAddress[4], rtkMacAddress[5]);
        oled.setFont(QW_FONT_5X7); //Set font to smallest
        oled.setCursor(0, 3);
        oled.print(macAddress);
      }
      else if (iconsRadio & ICON_BT_SYMBOL_LEFT)
        displayBitmap(1, 0, BT_Symbol_Width, BT_Symbol_Height, BT_Symbol);
      else if (iconsRadio & ICON_WIFI_SYMBOL_0_LEFT)
        displayBitmap(0, 0, WiFi_Symbol_Width, WiFi_Symbol_Height, WiFi_Symbol_0);
      else if (iconsRadio & ICON_WIFI_SYMBOL_1_LEFT)
        displayBitmap(0, 0, WiFi_Symbol_Width, WiFi_Symbol_Height, WiFi_Symbol_1);
      else if (iconsRadio & ICON_WIFI_SYMBOL_2_LEFT)
        displayBitmap(0, 0, WiFi_Symbol_Width, WiFi_Symbol_Height, WiFi_Symbol_2);
      else if (iconsRadio & ICON_WIFI_SYMBOL_3_LEFT)
        displayBitmap(0, 0, WiFi_Symbol_Width, WiFi_Symbol_Height, WiFi_Symbol_3);
      else if (iconsRadio & ICON_ESPNOW_SYMBOL_0_LEFT)
        displayBitmap(0, 0, ESPNOW_Symbol_Width, ESPNOW_Symbol_Height, ESPNOW_Symbol_0);
      else if (iconsRadio & ICON_ESPNOW_SYMBOL_1_LEFT)
        displayBitmap(0, 0, ESPNOW_Symbol_Width, ESPNOW_Symbol_Height, ESPNOW_Symbol_1);
      else if (iconsRadio & ICON_ESPNOW_SYMBOL_2_LEFT)
        displayBitmap(0, 0, ESPNOW_Symbol_Width, ESPNOW_Symbol_Height, ESPNOW_Symbol_2);
      else if (iconsRadio & ICON_ESPNOW_SYMBOL_3_LEFT)
        displayBitmap(0, 0, ESPNOW_Symbol_Width, ESPNOW_Symbol_Height, ESPNOW_Symbol_3);
      else if (iconsRadio & ICON_DOWN_ARROW_LEFT)
        displayBitmap(1, 0, DownloadArrow_Width, DownloadArrow_Height, DownloadArrow);
      else if (iconsRadio & ICON_UP_ARROW_LEFT)
        displayBitmap(1, 0, UploadArrow_Width, UploadArrow_Height, UploadArrow);
      else if (iconsRadio & ICON_BLANK_LEFT)
      {
        ;
      }

      //Center radio spots
      if (iconsRadio & ICON_BT_SYMBOL_CENTER)
      {
        //Moved to center to give space for ESP NOW icon on far left
        displayBitmap(16, 0, BT_Symbol_Width, BT_Symbol_Height, BT_Symbol);
      }
      else if (iconsRadio & ICON_MAC_ADDRESS_2DIGIT)
      {
        char macAddress[5];
        const uint8_t * rtkMacAddress = getMacAddress();

        //Print only last two digits of MAC
        snprintf(macAddress, sizeof(macAddress), "%02X", rtkMacAddress[5]);
        oled.setFont(QW_FONT_5X7); //Set font to smallest
        oled.setCursor(14, 3);
        oled.print(macAddress);
      }
      else if (iconsRadio & ICON_DOWN_ARROW_CENTER)
        displayBitmap(16, 0, DownloadArrow_Width, DownloadArrow_Height, DownloadArrow);
      else if (iconsRadio & ICON_UP_ARROW_CENTER)
        displayBitmap(16, 0, UploadArrow_Width, UploadArrow_Height, UploadArrow);

      //Radio third spot
      if (iconsRadio & ICON_WIFI_SYMBOL_0_RIGHT)
        displayBitmap(28, 0, WiFi_Symbol_Width, WiFi_Symbol_Height, WiFi_Symbol_0);
      else if (iconsRadio & ICON_WIFI_SYMBOL_1_RIGHT)
        displayBitmap(28, 0, WiFi_Symbol_Width, WiFi_Symbol_Height, WiFi_Symbol_1);
      else if (iconsRadio & ICON_WIFI_SYMBOL_2_RIGHT)
        displayBitmap(28, 0, WiFi_Symbol_Width, WiFi_Symbol_Height, WiFi_Symbol_2);
      else if (iconsRadio & ICON_WIFI_SYMBOL_3_RIGHT)
        displayBitmap(28, 0, WiFi_Symbol_Width, WiFi_Symbol_Height, WiFi_Symbol_3);
      else if ((iconsRadio & ICON_DYNAMIC_MODEL) && (online.gnss == true))
        paintDynamicModel();
      else if (iconsRadio & ICON_BASE_TEMPORARY)
        displayBitmap(28, 0, BaseTemporary_Width, BaseTemporary_Height, BaseTemporary);
      else if (iconsRadio & ICON_BASE_FIXED)
        displayBitmap(28, 0, BaseFixed_Width, BaseFixed_Height, BaseFixed); //true - blend with other pixels
      else if (iconsRadio & ICON_DOWN_ARROW_RIGHT)
        displayBitmap(31, 0, DownloadArrow_Width, DownloadArrow_Height, DownloadArrow);
      else if (iconsRadio & ICON_UP_ARROW_RIGHT)
        displayBitmap(31, 0, UploadArrow_Width, UploadArrow_Height, UploadArrow);
      else if (iconsRadio & ICON_BLANK_RIGHT)
      {
        ;
      }

      //Left + center spot
      if (iconsRadio & ICON_IP_ADDRESS)
        paintIPAddress(doUpdate);

      //Top right corner
      if (icons & ICON_BATTERY)
        paintBatteryLevel();
      else if (icons & ICON_ETHERNET)
        displayBitmap(45, 0, Ethernet_Icon_Width, Ethernet_Icon_Height, Ethernet_Icon);

      //Center left
      if (icons & ICON_CROSS_HAIR)
        displayBitmap(0, 18, CrossHair_Width, CrossHair_Height, CrossHair);
      else if (icons & ICON_CROSS_HAIR_DUAL)
        displayBitmap(0, 18, CrossHairDual_Width, CrossHairDual_Height, CrossHairDual);
      else if (icons & ICON_CLOCK)
        paintClock(doUpdate);

      //Center right
      if (icons & ICON_HORIZONTAL_ACCURACY)
        paintHorizontalAccuracy();
      else if (icons & ICON_CLOCK_ACCURACY)
        paintClockAccuracy();

      //Bottom left corner
      if (icons & ICON_SIV_ANTENNA)
        displayBitmap(2, 35, SIV_Antenna_Width, SIV_Antenna_Height, SIV_Antenna);
      else if (icons & ICON_SIV_ANTENNA_LBAND)
        displayBitmap(2, 35, SIV_Antenna_LBand_Width, SIV_Antenna_LBand_Height, SIV_Antenna_LBand);
      else if (icons & ICON_ANTENNA_SHORT)
        displayBitmap(2, 35, Antenna_Short_Width, Antenna_Short_Height, Antenna_Short);
      else if (icons & ICON_ANTENNA_OPEN)
        displayBitmap(2, 35, Antenna_Open_Width, Antenna_Open_Height, Antenna_Open);

      //Bottom right corner
      if (icons & ICON_LOGGING)
        paintLogging(doUpdate);
      else if (icons & ICON_LOGGING_NTP)
        paintLoggingNTP(true, doUpdate); //NTP, no pulse

      oled.display(); //Push internal buffer to display
    }
  } //End display online
}

void displayShutdown()
{
  displayMessage("Shutting Down...", 0);
}

//Displays a small error message then hard freeze
//Text wraps and is small but legible
void displayError(const char * errorMessage)
{
  if (online.display == true)
  {
    oled.erase(); //Clear the display's internal buffer

    oled.setCursor(0, 0); //x, y
    oled.setFont(QW_FONT_5X7); //Set font to smallest
    oled.print("Error:");

    oled.setCursor(2, 10);
    //oled.setFont(QW_FONT_8X16);
    oled.print(errorMessage);

    oled.display(); //Push internal buffer to display

    while (1) delay(10); //Hard freeze
  }
}

/*
               111111111122222222223333333333444444444455555555556666
     0123456789012345678901234567890123456789012345678901234567890123
    .----------------------------------------------------------------
   0|                                             *****************
   1|                                             *               *
   2|                                             * ***  ***  *** *
   3|                                             * ***  ***  *** ***
   4|                                             * ***  ***  ***   *
   5|                                             * ***  ***  ***   *
   6|                                             * ***  ***  ***   *
   7|                                             * ***  ***  ***   *
   8|                                             * ***  ***  *** ***
   9|                                             * ***  ***  *** *
  10|                                             *               *
  11|                                             *****************
*/

//Print the classic battery icon with levels
void paintBatteryLevel()
{
  if (online.display == true)
  {
    //Current battery charge level
    if (battLevel < 25)
      displayBitmap(45, 0, Battery_0_Width, Battery_0_Height, Battery_0);
    else if (battLevel < 50)
      displayBitmap(45, 0, Battery_1_Width, Battery_1_Height, Battery_1);
    else if (battLevel < 75)
      displayBitmap(45, 0, Battery_2_Width, Battery_2_Height, Battery_2);
    else //batt level > 75
      displayBitmap(45, 0, Battery_3_Width, Battery_3_Height, Battery_3);
  }
}

//Based on system state, turn on the various Rover, Base, Fixed Base icons
uint32_t setModeIcon()
{
  uint32_t icons = 0;

  switch (systemState)
  {
    case (STATE_ROVER_NOT_STARTED):
      break;
    case (STATE_ROVER_NO_FIX):
      icons |= ICON_DYNAMIC_MODEL;
      break;
    case (STATE_ROVER_FIX):
      icons |= ICON_DYNAMIC_MODEL;
      break;
    case (STATE_ROVER_RTK_FLOAT):
      icons |= ICON_DYNAMIC_MODEL;
      break;
    case (STATE_ROVER_RTK_FIX):
      icons |= ICON_DYNAMIC_MODEL;
      break;

    case (STATE_BASE_NOT_STARTED):
      //Do nothing. Static display shown during state change.
      break;
    case (STATE_BASE_TEMP_SETTLE):
      icons |= blinkBaseIcon(ICON_BASE_TEMPORARY);
      break;
    case (STATE_BASE_TEMP_SURVEY_STARTED):
      icons |= blinkBaseIcon(ICON_BASE_TEMPORARY);
      break;
    case (STATE_BASE_TEMP_TRANSMITTING):
      icons |= ICON_BASE_TEMPORARY;
      break;
    case (STATE_BASE_FIXED_NOT_STARTED):
      //Do nothing. Static display shown during state change.
      break;
    case (STATE_BASE_FIXED_TRANSMITTING):
      icons |= ICON_BASE_FIXED;
      break;

    case (STATE_NTPSERVER_NOT_STARTED):
    case (STATE_NTPSERVER_NO_SYNC):
    case (STATE_NTPSERVER_SYNC):
      break;

    default:
      break;
  }
  return (icons);
}

uint32_t blinkBaseIcon(uint32_t iconType)
{
  uint32_t icons = 0;

  //Limit how often we update this spot
  if (millis() - thirdRadioSpotTimer > 1000)
  {
    thirdRadioSpotTimer = millis();
    thirdRadioSpotBlink ^= 1; //Share the spot
  }

  if (thirdRadioSpotBlink == false)
    icons |= iconType;
  else
    icons |= ICON_BLANK_RIGHT;

  return icons;
}

/*
               111111111122222222223333333333444444444455555555556666
     0123456789012345678901234567890123456789012345678901234567890123
    .----------------------------------------------------------------
  17|
  18|
  19|
  20|
  21|                           ***               ***      ***
  22|                          *   *             *   *    *   *
  23|                          *   *             *   *    *   *
  24|                  **       * *               * *      * *
  25|                  **        *                 *        *
  26|                           * *               * *      * *
  27|                          *   *             *   *    *   *
  28|                          *   *             *   *    *   *
  29|                  **      *   *     **      *   *    *   *
  30|                  **       ***      **       ***      ***
  31|
  32|
*/

//Display horizontal accuracy
void paintHorizontalAccuracy()
{
  oled.setFont(QW_FONT_8X16); //Set font to type 1: 8x16
  oled.setCursor(16, 20); //x, y
  oled.print(":");

  if (horizontalAccuracy > 30.0)
  {
    oled.print(">30m");
  }
  else if (horizontalAccuracy > 9.9)
  {
    oled.print(horizontalAccuracy, 1); //Print down to decimeter
  }
  else if (horizontalAccuracy > 1.0)
  {
    oled.print(horizontalAccuracy, 2); //Print down to centimeter
  }
  else
  {
    oled.print("."); //Remove leading zero
    oled.printf("%03d", (int)(horizontalAccuracy * 1000)); //Print down to millimeter
  }
}

//Display clock with moving hands
void paintClock(int doUpdate)
{
  //Animate icon to show system running
  static uint8_t clockIconDisplayed = 3;

  if (doUpdate)
  {
    clockIconDisplayed++; //Goto next icon
    clockIconDisplayed %= 4; //Wrap
  }

  if (clockIconDisplayed == 0)
    displayBitmap(0, 18, Clock_Icon_Width, Clock_Icon_Height, Clock_Icon_1);
  else if (clockIconDisplayed == 1)
    displayBitmap(0, 18, Clock_Icon_Width, Clock_Icon_Height, Clock_Icon_2);
  else if (clockIconDisplayed == 2)
    displayBitmap(0, 18, Clock_Icon_Width, Clock_Icon_Height, Clock_Icon_3);
  else
    displayBitmap(0, 18, Clock_Icon_Width, Clock_Icon_Height, Clock_Icon_4);
}

//Display clock accuracy tAcc
void paintClockAccuracy()
{
  oled.setFont(QW_FONT_8X16); //Set font to type 1: 8x16
  oled.setCursor(16, 20); //x, y
  oled.print(":");

  if (online.gnss == false)
  {
    oled.print(" N/A");
  }
  else if (tAcc < 10) // 9 or less : show as 9ns
  {
    oled.print(tAcc);
    displayBitmap(36, 20, Millis_Icon_Width, Millis_Icon_Height, Nanos_Icon);
  }
  else if (tAcc < 100) // 99 or less : show as 99ns
  {
    oled.print(tAcc);
    displayBitmap(44, 20, Millis_Icon_Width, Millis_Icon_Height, Nanos_Icon);
  }
  else if (tAcc < 10000) // 9999 or less : show as 9.9μs
  {
    oled.print(tAcc / 1000);
    oled.print(".");
    oled.print((tAcc / 100) % 10);
    displayBitmap(52, 20, Millis_Icon_Width, Millis_Icon_Height, Micros_Icon);
  }
  else if (tAcc < 100000) // 99999 or less : show as 99μs
  {
    oled.print(tAcc / 1000);
    displayBitmap(44, 20, Millis_Icon_Width, Millis_Icon_Height, Micros_Icon);
  }
  else if (tAcc < 10000000) // 9999999 or less : show as 9.9ms
  {
    oled.print(tAcc / 1000000);
    oled.print(".");
    oled.print((tAcc / 100000) % 10);
    displayBitmap(52, 20, Millis_Icon_Width, Millis_Icon_Height, Millis_Icon);
  }
  else //if (tAcc >= 100000)
  {
    oled.print(">10");
    displayBitmap(52, 20, Millis_Icon_Width, Millis_Icon_Height, Millis_Icon);
  }
}

/*
               111111111122222222223333333333444444444455555555556666
     0123456789012345678901234567890123456789012345678901234567890123
    .----------------------------------------------------------------
   0|                                  **
   1|                                  **
   2|                               ******
   3|                              *      *
   4|                            * * **** * *
   5|                            * * **** * *
   6|                            * *      * *
   7|                            * *      * *
   8|                            * *      * *
   9|                            * *      * *
  10|                              *      *
  11|                               ******
  12|
*/

//Draw the rover icon depending on screen
void paintDynamicModel()
{
  //Display icon associated with current Dynamic Model
  switch (dynamicModel)
  {
    case (DYN_MODEL_PORTABLE):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_1_Portable);
      break;
    case (DYN_MODEL_STATIONARY):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_2_Stationary);
      break;
    case (DYN_MODEL_PEDESTRIAN):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_3_Pedestrian);
      break;
    case (DYN_MODEL_AUTOMOTIVE):
      //Normal rover for ZED-F9P, fusion rover for ZED-F9R
        displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_4_Automotive);

      }
      else if (zedModuleType == PLATFORM_F9R)
      {
        //Blink fusion rover until we have calibration
        if (fusionMode == 0) //Initializing
        {
          //Blink Fusion Rover icon until sensor calibration is complete
          if (millis() - lastBaseIconUpdate > 500)
          {
            lastBaseIconUpdate = millis();
            if (baseIconDisplayed == false)
            {
              baseIconDisplayed = true;

              //Draw the icon
              displayBitmap(28, 2, Rover_Fusion_Width, Rover_Fusion_Height, Rover_Fusion);
            }
            else
              baseIconDisplayed = false;
          }
        }
        else if (fusionMode == 1) //Calibrated
        {
          //Solid fusion rover
          displayBitmap(28, 2, Rover_Fusion_Width, Rover_Fusion_Height, Rover_Fusion);
        }
        else if (fusionMode == 2 || fusionMode == 3) //Suspended or disabled
        {
          //Empty rover
          displayBitmap(28, 2, Rover_Fusion_Empty_Width, Rover_Fusion_Empty_Height, Rover_Fusion_Empty);
        }
      }
      break;
    case (DYN_MODEL_SEA):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_5_Sea);
      break;
    case (DYN_MODEL_AIRBORNE1g):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_6_Airborne1g);
      break;
    case (DYN_MODEL_AIRBORNE2g):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_7_Airborne2g);
      break;
    case (DYN_MODEL_AIRBORNE4g):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_8_Airborne4g);
      break;
    case (DYN_MODEL_WRIST):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_9_Wrist);
      break;
    case (DYN_MODEL_BIKE):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_10_Bike);
      break;
    case (DYN_MODEL_MOWER):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_11_Mower);
      break;
    case (DYN_MODEL_ESCOOTER):
      displayBitmap(28, 0, DynamicModel_Width, DynamicModel_Height, DynamicModel_12_EScooter);
      break;
  }
}

/*
               111111111122222222223333333333444444444455555555556666
     0123456789012345678901234567890123456789012345678901234567890123
    .----------------------------------------------------------------
  35|
  36|   **
  37|   * *                    ***      ***
  38|   *  *   *              *   *    *   *
  39|   *   * *               *   *    *   *
  40|    *   *        **       * *      * *
  41|    *    *       **        *        *
  42|     *    *               * *      * *
  43|     **    *             *   *    *   *
  44|     ****   *            *   *    *   *
  45|     **  ****    **      *   *    *   *
  46|     **          **       ***      ***
  47|   ******
*/

//Select satellite icon and draw sats in view
//Blink icon if no fix
uint32_t paintSIV()
{
  uint32_t blinking;
  uint32_t icons;

  oled.setFont(QW_FONT_8X16); //Set font to type 1: 8x16
  oled.setCursor(16, 36); //x, y
  oled.print(":");

  if (online.gnss)
  {
    if (gnssIsFixed() == false)
      oled.print("0");
    else
      oled.print(gnssGetSatellitesInView());

    //paintResets();

    //Determine which icon to display
    icons = 0;
    if (lbandCorrectionsReceived)
      blinking = ICON_SIV_ANTENNA_LBAND;
    else
      blinking = ICON_SIV_ANTENNA;

    //Determine if there is a fix
    if (gnssIsFixed() == true)
    {
      //Fix, turn on icon
      icons = blinking;
    }
    else
    {
      //Blink satellite dish icon if we don't have a fix
      blinking_icons ^= blinking;
      if (blinking_icons & blinking)
        icons = blinking;
    }
  } //End gnss online
  else
  {
    oled.print("X");

    icons = ICON_SIV_ANTENNA;
  }
  return icons;
}

/*
               111111111122222222223333333333444444444455555555556666
     0123456789012345678901234567890123456789012345678901234567890123
    .----------------------------------------------------------------
  35|
  36|                                                       *******
  37|                                                       *     **
  38|                                                       *      **
  39|                                                       *       *
  40|                                                       * ***** *
  41|                                                       *       *
  42|                                                       * ***** *
  43|                                                       *       *
  44|                                                       * ***** *
  45|                                                       *       *
  46|                                                       *       *
  47|                                                       *********
*/

//Draw log icon
//Turn off icon if log file fails to get bigger
void paintLogging(bool doUpdate)
{
  //Animate icon to show system running
  if (doUpdate)
  {
    loggingIconDisplayed++; //Goto next icon
    loggingIconDisplayed %= 4; //Wrap
  }
  if ((online.logging == true) && (logIncreasing))
  {
    if (loggingType == LOGGING_STANDARD)
    {
      if (loggingIconDisplayed == 0)
        displayBitmap(64 - Logging_0_Width, 48 - Logging_0_Height, Logging_0_Width, Logging_0_Height, Logging_0);
      else if (loggingIconDisplayed == 1)
        displayBitmap(64 - Logging_1_Width, 48 - Logging_1_Height, Logging_1_Width, Logging_1_Height, Logging_1);
      else if (loggingIconDisplayed == 2)
        displayBitmap(64 - Logging_2_Width, 48 - Logging_2_Height, Logging_2_Width, Logging_2_Height, Logging_2);
      else if (loggingIconDisplayed == 3)
        displayBitmap(64 - Logging_3_Width, 48 - Logging_3_Height, Logging_3_Width, Logging_3_Height, Logging_3);
    }
    else if (loggingType == LOGGING_PPP)
    {
      if (loggingIconDisplayed == 0)
        displayBitmap(64 - Logging_0_Width, 48 - Logging_0_Height, Logging_0_Width, Logging_0_Height, Logging_0);
      else if (loggingIconDisplayed == 1)
        displayBitmap(64 - Logging_1_Width, 48 - Logging_1_Height, Logging_1_Width, Logging_1_Height, Logging_PPP_1);
      else if (loggingIconDisplayed == 2)
        displayBitmap(64 - Logging_2_Width, 48 - Logging_2_Height, Logging_2_Width, Logging_2_Height, Logging_PPP_2);
      else if (loggingIconDisplayed == 3)
        displayBitmap(64 - Logging_3_Width, 48 - Logging_3_Height, Logging_3_Width, Logging_3_Height, Logging_PPP_3);
    }
    else if (loggingType == LOGGING_CUSTOM)
    {
      if (loggingIconDisplayed == 0)
        displayBitmap(64 - Logging_0_Width, 48 - Logging_0_Height, Logging_0_Width, Logging_0_Height, Logging_0);
      else if (loggingIconDisplayed == 1)
        displayBitmap(64 - Logging_1_Width, 48 - Logging_1_Height, Logging_1_Width, Logging_1_Height, Logging_Custom_1);
      else if (loggingIconDisplayed == 2)
        displayBitmap(64 - Logging_2_Width, 48 - Logging_2_Height, Logging_2_Width, Logging_2_Height, Logging_Custom_2);
      else if (loggingIconDisplayed == 3)
        displayBitmap(64 - Logging_3_Width, 48 - Logging_3_Height, Logging_3_Width, Logging_3_Height, Logging_Custom_3);
    }
  }
  else
  {
    const int pulseX = 64 - 4;
    const int pulseY = oled.getHeight();
    int height;

    //Paint pulse to show system activity
    height = loggingIconDisplayed << 2;
    if (height)
    {
      oled.line(pulseX, pulseY, pulseX, pulseY - height);
      oled.line(pulseX - 1, pulseY, pulseX - 1, pulseY - height);
    }
  }
}

void paintLoggingNTP(bool noPulse, bool doUpdate)
{
  //Animate icon to show system running
  if (doUpdate)
  {
    loggingIconDisplayed++; //Goto next icon
    loggingIconDisplayed %= 4; //Wrap
  }
  if ((online.logging == true) && (logIncreasing))
  {
    if (loggingIconDisplayed == 0)
      displayBitmap(64 - Logging_0_Width, 48 - Logging_0_Height, Logging_0_Width, Logging_0_Height, Logging_0);
    else if (loggingIconDisplayed == 1)
      displayBitmap(64 - Logging_1_Width, 48 - Logging_1_Height, Logging_1_Width, Logging_1_Height, Logging_NTP_1);
    else if (loggingIconDisplayed == 2)
      displayBitmap(64 - Logging_2_Width, 48 - Logging_2_Height, Logging_2_Width, Logging_2_Height, Logging_NTP_2);
    else if (loggingIconDisplayed == 3)
      displayBitmap(64 - Logging_3_Width, 48 - Logging_3_Height, Logging_3_Width, Logging_3_Height, Logging_NTP_3);
  }
  else if (!noPulse)
  {
    const int pulseX = 64 - 4;
    const int pulseY = oled.getHeight();
    int height;

    //Paint pulse to show system activity
    height = loggingIconDisplayed << 2;
    if (height)
    {
      oled.line(pulseX, pulseY, pulseX, pulseY - height);
      oled.line(pulseX - 1, pulseY, pulseX - 1, pulseY - height);
    }
  }
}

//Survey in is running. Show 3D Mean and elapsed time.
void paintBaseTempSurveyStarted(bool doUpdate)
{
  oled.setFont(QW_FONT_5X7);
  oled.setCursor(0, 23); //x, y
  oled.print("Mean:");

  oled.setCursor(29, 20); //x, y
  oled.setFont(QW_FONT_8X16);
  if (svinMeanAccuracy < 10.0) //Error check
    oled.print(svinMeanAccuracy, 2);
  else
    oled.print(">10");

  if (!HAS_ANTENNA_SHORT_OPEN)
  {
    oled.setCursor(0, 39); //x, y
    oled.setFont(QW_FONT_5X7);
    oled.print("Time:");
  }
  else
  {
    static uint32_t blinkers = 0;
    if (aStatus == SFE_UBLOX_ANTENNA_STATUS_SHORT)
    {
      if (doUpdate)
        blinkers ^= ICON_ANTENNA_SHORT;
      else
        blinkers |= ICON_ANTENNA_SHORT;
      if (blinkers & ICON_ANTENNA_SHORT)
        displayBitmap(2, 35, Antenna_Short_Width, Antenna_Short_Height, Antenna_Short);
    }
    else if (aStatus == SFE_UBLOX_ANTENNA_STATUS_OPEN)
    {
      if (doUpdate)
        blinkers ^= ICON_ANTENNA_OPEN;
      else
        blinkers |= ICON_ANTENNA_OPEN;
      if (blinkers & ICON_ANTENNA_OPEN)
        displayBitmap(2, 35, Antenna_Open_Width, Antenna_Open_Height, Antenna_Open);
    }
    else
    {
      blinkers &= ~ICON_ANTENNA_SHORT;
      blinkers &= ~ICON_ANTENNA_OPEN;
      oled.setCursor(0, 39); //x, y
      oled.setFont(QW_FONT_5X7);
      oled.print("Time:");
    }
  }


  oled.setCursor(30, 36); //x, y
  oled.setFont(QW_FONT_8X16);
  if (svinObservationTime < 1000) //Error check
    oled.print(svinObservationTime);
  else
    oled.print("0");
}

//Given text, a position, and kerning, print text to display
//This is helpful for squishing or stretching a string to appropriately fill the display
void printTextwithKerning(const char *newText, uint8_t xPos, uint8_t yPos, uint8_t kerning)
{
  for (int x = 0 ; x < strlen(newText) ; x++)
  {
    oled.setCursor(xPos, yPos);
    oled.print(newText[x]);
    xPos += kerning;
  }
}

//Show transmission of RTCM correction data packets to NTRIP caster
void paintRTCM(bool doUpdate)
{
  int yPos = 17;
  if (ntripServerState == NTRIP_SERVER_CASTING)
    printTextCenter("Casting", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
  else
    printTextCenter("Xmitting", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted


  if (!HAS_ANTENNA_SHORT_OPEN)
  {
    oled.setCursor(0, 39); //x, y
    oled.setFont(QW_FONT_5X7);
    oled.print("RTCM:");
  }
  else
  {
    static uint32_t blinkers = 0;
    if (aStatus == SFE_UBLOX_ANTENNA_STATUS_SHORT)
    {
      if (doUpdate)
        blinkers ^= ICON_ANTENNA_SHORT;
      else
        blinkers |= ICON_ANTENNA_SHORT;
      if (blinkers & ICON_ANTENNA_SHORT)
        displayBitmap(2, 35, Antenna_Short_Width, Antenna_Short_Height, Antenna_Short);
    }
    else if (aStatus == SFE_UBLOX_ANTENNA_STATUS_OPEN)
    {
      if (doUpdate)
        blinkers ^= ICON_ANTENNA_OPEN;
      else
        blinkers |= ICON_ANTENNA_OPEN;
      if (blinkers & ICON_ANTENNA_OPEN)
        displayBitmap(2, 35, Antenna_Open_Width, Antenna_Open_Height, Antenna_Open);
    }
    else
    {
      blinkers &= ~ICON_ANTENNA_SHORT;
      blinkers &= ~ICON_ANTENNA_OPEN;
      oled.setCursor(0, 39); //x, y
      oled.setFont(QW_FONT_5X7);
      oled.print("RTCM:");      
    }
  }

  if (rtcmPacketsSent < 100)
    oled.setCursor(30, 36); //x, y - Give space for two digits
  else
    oled.setCursor(28, 36); //x, y - Push towards colon to make room for log icon

  oled.setFont(QW_FONT_8X16); //Set font to type 1: 8x16
  oled.print(rtcmPacketsSent); //rtcmPacketsSent is controlled in processRTCM()

  //paintResets();
}

//Show connecting to NTRIP caster service
void paintConnectingToNtripCaster()
{
  int yPos = 18;
  printTextCenter("Caster", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

  int textX = 3;
  int textY = 33;
  int textKerning = 6;
  oled.setFont(QW_FONT_8X16);

  printTextwithKerning("Connecting", textX, textY, textKerning);
}

//Scroll through IP address. Wipe with spaces both ends.
void paintIPAddress(bool doUpdate)
{
  char ipAddress[32];
  snprintf(ipAddress, sizeof(ipAddress), "       %d.%d.%d.%d       ",
           ETH.localIP[0], ETH.localIP[1], ETH.localIP[2], ETH.localIP[3]);

  static uint8_t ipAddressPosition = 0;

  //Check if IP address is all single digits and can be printed without scrolling
  if (strlen(ipAddress) <= 21)
    ipAddressPosition = 7;

  //Print seven characters of IP address
  char printThis[9];
  snprintf(printThis, sizeof(printThis), "%c%c%c%c%c%c%c",
           ipAddress[ipAddressPosition + 0], ipAddress[ipAddressPosition + 1],
           ipAddress[ipAddressPosition + 2], ipAddress[ipAddressPosition + 3],
           ipAddress[ipAddressPosition + 4], ipAddress[ipAddressPosition + 5],
           ipAddress[ipAddressPosition + 6]);

  oled.setFont(QW_FONT_5X7); //Set font to smallest
  oled.setCursor(0, 3);
  oled.print(printThis);

  if (doUpdate)
  {
    ipAddressPosition++; //Increment the print position
    if (ipAddress[ipAddressPosition + 7] == 0) //Wrap
      ipAddressPosition = 0;
  }
}

void displayBaseStart(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15; //Assume fontsize 1
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Base", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    oled.display();

    oled.display();

    delay(displayTime);
  }
}

void displayBaseSuccess(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15; //Assume fontsize 1
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Base", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("Started", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayBaseFail(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15; //Assume fontsize 1
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Base", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("Failed", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayGNSSFail(uint16_t displayTime)
{
  displayMessage("GNSS Failed", displayTime);
}

void displayNoWiFi(uint16_t displayTime)
{
  displayMessage("No WiFi", displayTime);
}

void displayRoverStart(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Rover", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    //printTextCenter("Started", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayRoverSuccess(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Rover", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("Started", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayRoverFail(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Rover", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("Failed", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayAccelFail(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Accel", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("Failed", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

//When user enters serial config menu the display will freeze so show splash while config happens
void displaySerialConfig()
{
  displayMessage("Serial Config", 0);
}

//Display during blocking stop during to prevent screen freeze
void displayWiFiConnect()
{
  displayMessage("WiFi Connect", 0);
}

//When user enters WiFi Config mode from setup, show splash while config happens
void displayWiFiConfigNotStarted()
{
  displayMessage("WiFi Config", 0);
}

void displayWiFiConfig()
{
  int yPos = WiFi_Symbol_Height + 2;
  int fontHeight = 8;

  const int displayMaxCharacters = 10; //Characters before pixels start getting cut off. 11 characters can cut off a few pixels.

  printTextCenter("SSID:", yPos, QW_FONT_5X7, 1, false); //text, y, font type, kerning, inverted

  yPos = yPos + fontHeight + 1;

  //Toggle display back and forth for long SSIDs and IPs
  //Run the timer no matter what, but load firstHalf/lastHalf with the same thing if strlen < maxWidth
  if (millis() - ssidDisplayTimer > 2000)
  {
    ssidDisplayTimer = millis();

    if (ssidDisplayFirstHalf == false)
      ssidDisplayFirstHalf = true;
    else
      ssidDisplayFirstHalf = false;
  }

  //Convert current SSID to string
  char mySSID[50] = {'\0'};

#ifdef COMPILE_WIFI
  if (settings.wifiConfigOverAP == true)
    snprintf(mySSID, sizeof(mySSID), "%s", "RTK Config");
  else
    snprintf(mySSID, sizeof(mySSID), "%s", WiFi.SSID().c_str());
#else
  snprintf(mySSID, sizeof(mySSID), "%s", "!Compiled");
#endif

  char mySSIDFront[displayMaxCharacters + 1]; //1 for null terminator
  char mySSIDBack[displayMaxCharacters + 1]; //1 for null terminator

  //Trim SSID to a max length
  strncpy(mySSIDFront, mySSID, displayMaxCharacters);

  if (strlen(mySSID) > displayMaxCharacters)
    strncpy(mySSIDBack, mySSID + (strlen(mySSID) - displayMaxCharacters), displayMaxCharacters);
  else
    strncpy(mySSIDBack, mySSID, displayMaxCharacters);

  mySSIDFront[displayMaxCharacters] = '\0';
  mySSIDBack[displayMaxCharacters] = '\0';

  if (ssidDisplayFirstHalf == true)
    printTextCenter(mySSIDFront, yPos, QW_FONT_5X7, 1, false);
  else
    printTextCenter(mySSIDBack, yPos, QW_FONT_5X7, 1, false);

  yPos = yPos + fontHeight + 3;
  printTextCenter("IP:", yPos, QW_FONT_5X7, 1, false);

  yPos = yPos + fontHeight + 1;

#ifdef COMPILE_AP
  IPAddress myIpAddress;
  if (settings.wifiConfigOverAP == true)
    myIpAddress = WiFi.softAPIP();
  else
    myIpAddress = WiFi.localIP();

  //Convert to string
  char myIP[20] = {'\0'};
  snprintf(myIP, sizeof(myIP), "%d.%d.%d.%d", myIpAddress[0], myIpAddress[1], myIpAddress[2], myIpAddress[3]);

  char myIPFront[displayMaxCharacters + 1]; //1 for null terminator
  char myIPBack[displayMaxCharacters + 1]; //1 for null terminator

  strncpy(myIPFront, myIP, displayMaxCharacters);

  if (strlen(myIP) > displayMaxCharacters)
    strncpy(myIPBack, myIP + (strlen(myIP) - displayMaxCharacters), displayMaxCharacters);
  else
    strncpy(myIPBack, myIP, displayMaxCharacters);

  myIPFront[displayMaxCharacters] = '\0';
  myIPBack[displayMaxCharacters] = '\0';

  if (ssidDisplayFirstHalf == true)
    printTextCenter(myIPFront, yPos, QW_FONT_5X7, 1, false);
  else
    printTextCenter(myIPBack, yPos, QW_FONT_5X7, 1, false);

#else
  printTextCenter("!Compiled", yPos, QW_FONT_5X7, 1, false);
#endif
}

//When user does a factory reset, let us know
void displaySystemReset()
{
  displayMessage("Factory Reset", 0);
}

void displaySurveyStart(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Survey", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    //printTextCenter("Started", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displaySurveyStarted(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Survey", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("Started", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

//If the SD card is detected but is not formatted correctly, display warning
void displaySDFail(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Format", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("SD Card", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

//Draw a frame at outside edge
void drawFrame()
{
  //Init and draw box at edge to see screen alignment
  int xMax = 63;
  int yMax = 47;
  oled.line(0, 0, xMax, 0); //Top
  oled.line(0, 0, 0, yMax); //Left
  oled.line(0, yMax, xMax, yMax); //Bottom
  oled.line(xMax, 0, xMax, yMax); //Right
}

void displayForcedFirmwareUpdate()
{
  displayMessage("Forced Update", 0);
}

void displayFirmwareUpdateProgress(int percentComplete)
{
  //Update the display if connected
  if (online.display == true)
  {
    oled.erase(); //Clear the display's internal buffer

    int yPos = 3;
    int fontHeight = 8;

    printTextCenter("Firmware", yPos, QW_FONT_5X7, 1, false); //text, y, font type, kerning, inverted

    yPos = yPos + fontHeight + 1;
    printTextCenter("Update", yPos, QW_FONT_5X7, 1, false); //text, y, font type, kerning, inverted

    yPos = yPos + fontHeight + 3;
    char temp[50];
    snprintf(temp, sizeof(temp), "%d%%", percentComplete);
    printTextCenter(temp, yPos, QW_FONT_8X16, 1, false); //text, y, font type, kerning, inverted

    oled.display(); //Push internal buffer to display
  }
}

void displayEventMarked(uint16_t displayTime)
{
  displayMessage("Event Marked", displayTime);
}

void displayNoLogging(uint16_t displayTime)
{
  displayMessage("No Logging", displayTime);
}

void displayMarked(uint16_t displayTime)
{
  displayMessage("Marked", displayTime);
}

void displayMarkFailure(uint16_t displayTime)
{
  displayMessage("Mark Failure", displayTime);
}

void displayNotMarked(uint16_t displayTime)
{
  displayMessage("Not Marked", displayTime);
}

//Show 'Loading Home2' profile identified
//Profiles may not be sequential (user might have empty profile #2, but filled #3) so we load the profile unit, not the number
void paintProfile(uint8_t profileUnit)
{
  char profileMessage[20]; //'Loading HomeStar' max of ~18 chars

  char profileName[8 + 1];
  //if (getProfileNameFromUnit(profileUnit, profileName, sizeof(profileName)) == true) //Load the profile name, limited to 8 chars
  snprintf(profileName, sizeof(profileName), "Profile1");
  {
    //settings.updateGNSSSettings = true; //When this profile is loaded next, force system to update ZED settings.
    //recordSystemSettings(); //Before switching, we need to record the current settings to LittleFS and SD

    //Lookup profileNumber based on unit
    //uint8_t profileNumber = getProfileNumberFromUnit(profileUnit);
    //recordProfileNumber(profileNumber); //Update internal settings with user's choice, mark unit for config update

    //log_d("Going to profile number %d from unit %d, name '%s'", profileNumber, profileUnit, profileName);

    snprintf(profileMessage, sizeof(profileMessage), "Loading %s", profileName);
    displayMessage(profileMessage, 2000);
    ESP.restart(); //Profiles require full restart to take effect
  }
}

//Display the setup profiles
void paintDisplaySetupProfile(const char * firstState)
{
  int index;
  int itemsDisplayed;
  char profileName[8 + 1];

  //Display the first state if this is the first profile
  itemsDisplayed = 0;
  if (displayProfile == 0)
  {
    printTextCenter(firstState, 12 * itemsDisplayed, QW_FONT_8X16, 1, false);
    itemsDisplayed++;
  }

  //Display Bubble if this is the second profile
  if (displayProfile <= 1)
  {
    printTextCenter("Bubble", 12 * itemsDisplayed, QW_FONT_8X16, 1, false);
    itemsDisplayed++;
  }

  //Display Config if this is the third profile
  if (displayProfile <= 2)
  {
    printTextCenter("Config", 12 * itemsDisplayed, QW_FONT_8X16, 1, false);
    itemsDisplayed++;
  }

  //  displayProfile  itemsDisplayed  index
  //        0               3           0
  //        1               2           0
  //        2               1           0
  //        3               0           0
  //        4               0           1
  //        5               0           2
  //        n >= 3          0           n - 3

  //Display the profile names
  for (index = (displayProfile >= 3) ? displayProfile - 3 : 0; itemsDisplayed < 4; itemsDisplayed++)
  {
    //Lookup next available profile, limit to 8 characters
    //getProfileNameFromUnit(index, profileName, sizeof(profileName));
    snprintf(profileName, sizeof(profileName), "Profile1");
    printTextCenter(profileName, 12 * itemsDisplayed, QW_FONT_8X16, 1, itemsDisplayed == 3);
    index++;
  }
}

//Show different menu 'buttons' to allow user to pause on one to select it
void paintDisplaySetup()
{
  if (zedModuleType == PLATFORM_F9P)
  {
    if (setupState == STATE_MARK_EVENT)
    {
      if (productVariant == REFERENCE_STATION)
      {
        //setupState defaults to STATE_MARK_EVENT, which is not a valid state for the Ref Stn.
        //It will be corrected by ButtonCheckTask. Until then, display but don't highlight an option.
        printTextCenter("Base", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("NTP", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Cfg Eth", 12 * 3, QW_FONT_8X16, 1, false);
      }
      else if (online.accelerometer)
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, true); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Base", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Bubble", 12 * 3, QW_FONT_8X16, 1, false);
      }
      else
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, true); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Base", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, false);
      }
    }
    else if (setupState == STATE_ROVER_NOT_STARTED)
    {
      if (productVariant == REFERENCE_STATION)
      {
        printTextCenter("Base", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, true);
        printTextCenter("NTP", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Cfg Eth", 12 * 3, QW_FONT_8X16, 1, false);
      }
      else if (online.accelerometer)
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, true);
        printTextCenter("Base", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Bubble", 12 * 3, QW_FONT_8X16, 1, false);
      }
      else
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, true);
        printTextCenter("Base", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, false);
      }
    }
    else if (setupState == STATE_BASE_NOT_STARTED)
    {
      if (productVariant == REFERENCE_STATION)
      {
        printTextCenter("Base", 12 * 0, QW_FONT_8X16, 1, true); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("NTP", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Cfg Eth", 12 * 3, QW_FONT_8X16, 1, false);
      }
      else if (online.accelerometer)
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Base", 12 * 2, QW_FONT_8X16, 1, true);
        printTextCenter("Bubble", 12 * 3, QW_FONT_8X16, 1, false);
      }
      else
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Base", 12 * 2, QW_FONT_8X16, 1, true);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, false);
      }
    }
    else if (setupState == STATE_NTPSERVER_NOT_STARTED)
    {
      {
        printTextCenter("Base", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("NTP", 12 * 2, QW_FONT_8X16, 1, true);
        printTextCenter("Cfg Eth", 12 * 3, QW_FONT_8X16, 1, false);
      }
    }
    else if (setupState == STATE_BUBBLE_LEVEL)
    {
      if (online.accelerometer)
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Base", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Bubble", 12 * 3, QW_FONT_8X16, 1, true);
      }
      else
      {
        //We should never get here, but just in case
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Base", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, true);
      }
    }
    else if (setupState == STATE_CONFIG_VIA_ETH_NOT_STARTED)
    {
      printTextCenter("Base", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
      printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
      printTextCenter("NTP", 12 * 2, QW_FONT_8X16, 1, false);
      printTextCenter("Cfg Eth", 12 * 3, QW_FONT_8X16, 1, true);
    }
    else if (setupState == STATE_WIFI_CONFIG_NOT_STARTED)
    {
      if (productVariant == REFERENCE_STATION)
      {
        printTextCenter("Rover", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
        printTextCenter("NTP", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Cfg Eth", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("CfgWiFi", 12 * 3, QW_FONT_8X16, 1, true);
      }
      else if (online.accelerometer)
      {
        printTextCenter("Rover", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Base", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Bubble", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, true);
      }
      else
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Base", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, true);
      }
    }
    else if (setupState == STATE_ESPNOW_PAIRING_NOT_STARTED)
    {
      if (productVariant == REFERENCE_STATION)
      {
        printTextCenter("NTP", 12 * 0, QW_FONT_8X16, 1, false); //string, y, font type, kerning, inverted
        printTextCenter("Cfg Eth", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("CfgWiFi", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("E-Pair", 12 * 3, QW_FONT_8X16, 1, true);
      }
      else if (online.accelerometer)
      {
        printTextCenter("Base", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Bubble", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("E-Pair", 12 * 3, QW_FONT_8X16, 1, true);
      }
      else
      {
        printTextCenter("Rover", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Base", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("E-Pair", 12 * 3, QW_FONT_8X16, 1, true);
      }
    }
    else if (setupState == STATE_PROFILE)
      paintDisplaySetupProfile("Base");
  } //end type F9P
  else if (zedModuleType == PLATFORM_F9R)
  {
    if (setupState == STATE_MARK_EVENT)
    {
      if (online.accelerometer)
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, true); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Bubble", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, false);
      }
      else
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, true); //string, y, font type, kerning, inverted
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("E-Pair", 12 * 3, QW_FONT_8X16, 1, false);
      }
    }
    else if (setupState == STATE_ROVER_NOT_STARTED)
    {
      if (online.accelerometer)
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, true);
        printTextCenter("Bubble", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, false);
      }
      else
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, true);
        printTextCenter("Config", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("E-Pair", 12 * 3, QW_FONT_8X16, 1, false);
      }
    }
    else if (setupState == STATE_BUBBLE_LEVEL)
    {
      if (online.accelerometer)
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Bubble", 12 * 2, QW_FONT_8X16, 1, true);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, false);
      }
      else
      {
        //We should never get here, but just in case
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 2, QW_FONT_8X16, 1, true);
        printTextCenter("E-Pair", 12 * 3, QW_FONT_8X16, 1, false);
      }
    }
    else if (setupState == STATE_WIFI_CONFIG_NOT_STARTED)
    {
      if (online.accelerometer)
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Bubble", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 3, QW_FONT_8X16, 1, true);
      }
      else
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 2, QW_FONT_8X16, 1, true);
        printTextCenter("E-Pair", 12 * 3, QW_FONT_8X16, 1, false);
      }
    }
    else if (setupState == STATE_ESPNOW_PAIRING_NOT_STARTED)
    {
      if (online.accelerometer)
      {
        printTextCenter("Rover", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Bubble", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("E-Pair", 12 * 3, QW_FONT_8X16, 1, true);
      }
      else
      {
        printTextCenter("Mark", 12 * 0, QW_FONT_8X16, 1, false);
        printTextCenter("Rover", 12 * 1, QW_FONT_8X16, 1, false);
        printTextCenter("Config", 12 * 2, QW_FONT_8X16, 1, false);
        printTextCenter("E-Pair", 12 * 3, QW_FONT_8X16, 1, true);
      }
    }
    else if (setupState == STATE_PROFILE)
      paintDisplaySetupProfile("Rover");
  } //end type F9R
}

//Given text, and location, print text center of the screen
void printTextCenter(const char *text, uint8_t yPos, QwiicFont & fontType, uint8_t kerning, bool highlight) //text, y, font type, kearning, inverted
{
  oled.setFont(fontType);
  oled.setDrawMode(grROPXOR);

  uint8_t fontWidth = fontType.width;
  if (fontWidth == 8) fontWidth = 7; //8x16, but widest character is only 7 pixels.

  uint8_t xStart = (oled.getWidth() / 2) - ((strlen(text) * (fontWidth + kerning)) / 2) + 1;

  uint8_t xPos = xStart;
  for (int x = 0 ; x < strlen(text) ; x++)
  {
    oled.setCursor(xPos, yPos);
    oled.print(text[x]);
    xPos += fontWidth + kerning;
  }

  if (highlight) //Draw a box, inverted over text
  {
    uint8_t textPixelWidth = strlen(text) * (fontWidth + kerning);

    //Error check
    int xBoxStart = xStart - 5;
    if (xBoxStart < 0) xBoxStart = 0;
    int xBoxEnd = textPixelWidth + 9;
    if (xBoxEnd > oled.getWidth() - 1) xBoxEnd = oled.getWidth() - 1;

    oled.rectangleFill(xBoxStart, yPos, xBoxEnd, 12, 1); //x, y, width, height, color
  }
}

//Given a message (one or two words) display centered
void displayMessage(const char* message, uint16_t displayTime)
{
  if (online.display == true)
  {
    char temp[21];
    uint8_t fontHeight = 15; //Assume fontsize 1

    //Count words based on spaces
    uint8_t wordCount = 0;
    strncpy(temp, message, sizeof(temp) - 1); //strtok modifies the message so make copy
    char * token = strtok(temp, " ");
    while (token != nullptr)
    {
      wordCount++;
      token = strtok(nullptr, " ");
    }

    uint8_t yPos = (oled.getHeight() / 2) - (fontHeight / 2);
    if (wordCount == 2) yPos -= (fontHeight / 2);

    oled.erase();

    //drawFrame();

    strncpy(temp, message, sizeof(temp) - 1);
    token = strtok(temp, " ");
    while (token != nullptr)
    {
      printTextCenter(token, yPos, QW_FONT_8X16, 1, false); //text, y, font type, kerning, inverted
      token = strtok(nullptr, " ");
      yPos += fontHeight;
    }

    oled.display();

    delay(displayTime);
  }
}

//Wrapper to avoid needing to pass width/height data twice
void displayBitmap(uint8_t x, uint8_t y, uint8_t imageWidth, uint8_t imageHeight, const uint8_t *imageData)
{
  oled.bitmap(x, y, x + imageWidth, y + imageHeight, (uint8_t *)imageData, imageWidth, imageHeight);
}

//Show screen while ESP-Now is pairing
void paintEspNowPairing()
{
  displayMessage("ESP-Now Pairing", 0);
}
void paintEspNowPaired()
{
  displayMessage("ESP-Now Paired", 2000);
}

void displayNtpStart(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("NTP", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayNtpStarted(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 15;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("NTP", yPos, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("Started", yPos + fontHeight, QW_FONT_8X16, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayNtpNotReady(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 8;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("Ethernet", yPos, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("Not Ready", yPos + fontHeight, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayNTPFail(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 8;
    uint8_t yPos = oled.getHeight() / 2 - fontHeight;

    printTextCenter("NTP", yPos, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted
    printTextCenter("Failed", yPos + fontHeight, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayConfigViaEthNotStarted(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 8;
    uint8_t yPos = fontHeight;

    printTextCenter("Configure", yPos, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted
    yPos += fontHeight;
    printTextCenter("Via", yPos, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted
    yPos += fontHeight;
    printTextCenter("Ethernet", yPos, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted
    yPos += fontHeight;
    printTextCenter("Restart", yPos, QW_FONT_5X7, 1, true);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayConfigViaEthStarted(uint16_t displayTime)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t fontHeight = 8;
    uint8_t yPos = fontHeight;

    printTextCenter("Configure", yPos, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted
    yPos += fontHeight;
    printTextCenter("Via", yPos, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted
    yPos += fontHeight;
    printTextCenter("Ethernet", yPos, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted
    yPos += fontHeight;
    printTextCenter("Started", yPos, QW_FONT_5X7, 1, false);  //text, y, font type, kerning, inverted

    oled.display();

    delay(displayTime);
  }
}

void displayConfigViaEthernet(bool doUpdate)
{
  if (online.display == true)
  {
    oled.erase();

    uint8_t xPos = (oled.getWidth() / 2) - (Ethernet_Icon_Width / 2);
    uint8_t yPos = Ethernet_Icon_Height / 2;

    static bool blink = 0;
    blink ^= 1;

    if (ETH.linkUp || blink)
      displayBitmap(xPos, yPos, Ethernet_Icon_Width, Ethernet_Icon_Height, Ethernet_Icon);

    yPos += Ethernet_Icon_Height * 1.5;

    printTextCenter("IP:", yPos, QW_FONT_5X7, 1, false); //text, y, font type, kerning, inverted
    yPos += 8;

    char ipAddress[40];
    IPAddress localIP =  ETH.localIP;
    snprintf(ipAddress, sizeof(ipAddress), "          %d.%d.%d.%d          ",
             localIP[0], localIP[1], localIP[2], localIP[3]);

    static uint8_t ipAddressPosition = 0;

    //Print ten characters of IP address
    char printThis[12];

    //Check if the IP address is <= 10 chars and will fit without scrolling
    if (strlen(ipAddress) <= 28)
      ipAddressPosition = 9;
    else if (strlen(ipAddress) <= 30)
      ipAddressPosition = 10;
    
    snprintf(printThis, sizeof(printThis), "%c%c%c%c%c%c%c%c%c%c",
             ipAddress[ipAddressPosition + 0], ipAddress[ipAddressPosition + 1],
             ipAddress[ipAddressPosition + 2], ipAddress[ipAddressPosition + 3],
             ipAddress[ipAddressPosition + 4], ipAddress[ipAddressPosition + 5],
             ipAddress[ipAddressPosition + 6], ipAddress[ipAddressPosition + 7],
             ipAddress[ipAddressPosition + 8], ipAddress[ipAddressPosition + 9]);

    oled.setCursor(0, yPos);
    oled.print(printThis);

    if (doUpdate)
    {
      ipAddressPosition++; //Increment the print position
      if (ipAddress[ipAddressPosition + 10] == 0) //Wrap
        ipAddressPosition = 0;
    }

    oled.display();
  }
}

const uint8_t * getMacAddress()
{
  static const uint8_t zero[6] = {0, 0, 0, 0, 0xAB, 0xCD};
  return zero;
}
