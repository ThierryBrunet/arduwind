/* ArduWind = Nanode + wind speed/direction (Davis anemometer/vane) + datastreaming to Pachube
==============================================================================================
V1.0
MercinatLabs / MERCINAT SARL France
By: Thierry Brunet de Courssou
http://www.mercinat.com
Created 28 May 2011
Last update: 25 Nov 2011
Project hosted at: http://code.google.com/p/arduwind/ - Repository type: Subversion
Version Control System: TortoiseSVN 1.7.1, Subversion 1.7.1, for Window7 64-bit - http://tortoisesvn.net/downloads.html

Configuration
-------------
Hardware: Nanode v5 + Davis anemometer/vane (or Inspeed Vortex anemometer)
Arduino IDE RC2 for Windows at http://code.google.com/p/arduino/wiki/Arduino1
-- did not test with Arduino IDE 0022 or 0023

Project summary
---------------
Real-time streaming of wind speed & direction to Pachube using Davis/Vortex pulse anemometers

Comments
--------
This code has been abundantly commented with lots of external links to serve as a tutorial for newbies

View the 4-anemometer tower and produced real-time embedded Pachube graphs at: http://code.google.com/p/arduwind/

For the Etel test site project, there are 4x anemometers and 4x vanes, at 18, 12, 9 and 6 meters

Davis are reed relay anemometers giving a pulse per revolution
Davis vane gives an analog signal via a 360 degree free turning potentiometer
A closure every 1 second equates to 2.25 mph of wind measured 
(or 1.00584 metres per second or 3.62102 km/h).
http://www.davisnet.com/weather/products/weather_product.asp?pnum=07911

if using Inspeed Vortex anemometer, a closure every 1 second equates to 2.5 mph of wind measured
(or 1.1176 m/s or 4.02336 km/h).
http://www.inspeed.com/anemometers/Hand_Held_Wind_Meter.asp
 
Davis standard cable connection:
--------------------------------
  BLACK  --> Wind Speed --> goes to digital input D3 (12k pull up resistor)
  RED    --> OV
  GREEN  --> Direction  --> goes to analog input A0
  YELLOW --> 3.3V
  
also consult http://code.google.com/p/arduwind/downloads/list for details on anemometer wiring.

Anemometer is attached to digital input D3 via a 12K pull-up resistor
- interrupt 1 is for digital pin D3 --> Anemo  http://www.arduino.cc/en/Reference/AttachInterrupt
   -- Note: Interrupt 0 on digital pin D2 is used in Nanode for Ethernet timers. 
   -- Interrup 0 on D2 cannot be used for other purposes (we tried connecting a second anemometer 
   -- to it but we could not get it to work properly)
 
- Vane is connected to analog input A0

// RAM space is very limited so use PROGMEM to conserve memory with strings
// However PROGMEM brings some instabilities, so not using it for now. 
// Will check from time to time if this is solved in future versions of 
// Arduino IDE and EtherCard library.

// As Pachube feeds may hang at times, we reboot regularly. 
// We will monitor stability then remove reboot when OK.

// The last datastream ID6 is a health indicator. This is simply an incremeting number so any
// interruption can be easily addentified as a discontinuity on the ramp graph shown on Pachube

// Mercinat Etel test site Nanodes -- As we use one Nanode per sensor, making use of the MAC allows to assign IP by DHCP and identify each sensor
// -------------------------------
// Nanode: 1	Serial: 266	 Mac: 00:04:A3:2C:2B:D6 --> Aurora
// Nanode: 2	Serial: 267	 Mac: 00:04:A3:2C:30:C2 --> FemtoGrid
// Nanode: 3	Serial: 738	 Mac: 00:04:A3:2C:1D:EA --> Skystream
// Nanode: 4	Serial: 739	 Mac: 00:04:A3:2C:1C:AC --> Grid RMS #1
// Nanode: 5	Serial: 740	 Mac: 00:04:A3:2C:10:8E --> Grid RMS #2
// Nanode: 6	Serial: 835	 Mac: 00:04:A3:2C:28:FA -->  6 m anemometer/vane
// Nanode: 7	Serial: 836	 Mac: 00:04:A3:2C:26:AF --> 18 m anemometer/vane
// Nanode: 8	Serial: 837	 Mac: 00:04:A3:2C:13:F4 --> 12 m anemometer/vane
// Nanode: 9	Serial: 838	 Mac: 00:04:A3:2C:2F:C4 -->  9 m anemometer/vane

// Pachube feeds assignement
// -------------------------
// The is one Pachube feed per sensor. This appears to be the most flexible scheme.
// Derived/computed feeds/datastreams may be uploaded later via the Pachube API
// Will post such applications on project repository 
//  38277  - Anemometer/Vane Etel  6 m -- https://pachube.com/feeds/38277
//  38278  - Anemometer/Vane Etel  9 m -- https://pachube.com/feeds/38278
//  38279  - Anemometer/Vane Etel 12 m -- https://pachube.com/feeds/38279
//  38281  - Anemometer/Vane Etel 18 m -- https://pachube.com/feeds/38281
//  35020  - Skystream    (ADE7755 via CF & REVP - modified Hoch Energy Meter board) -- https://pachube.com/feeds/35020
//  37668  - WT FemtoGrid (ADE7755 via CF & REVP - modified Hoch Energy Meter board) -- Private feed -- https://pachube.com/feeds/37668
//  37667  - WT Aurora on (ADE7755 via CF & REVP - modified Hoch Energy Meter board) -- Private feed -- https://pachube.com/feeds/37667
//  40385  - Grid RMS #1  (ADE7753 via SPI - Olimex Energy Shield) -- https://pachube.com/feeds/40385
//  40386  - Grid RMS #2  (ADE7753 via SPI - Olimex Energy Shield) -- https://pachube.com/feeds/40386
//  40388 -- Turbine A -- Private feed -- https://pachube.com/feeds/40388
//  40389 -- Turbine B -- Private feed -- https://pachube.com/feeds/40389
//  37267 -- newly built Nanode test feed (Mercinat) -- https://pachube.com/feeds/37267
//  40447 -- for anyone to test the ArduWind code)   -- https://pachube.com/feeds/40447
//  40448 -- for anyone to test the ArduGrid code)   -- https://pachube.com/feeds/40448
//  40449 -- for anyone to test the ArduSky code)    -- https://pachube.com/feeds/40449
//  40450 -- for anyone to test the SkyChube code)   -- https://pachube.com/feeds/40450
//  40451 -- for anyone to test the NanodeKit code)  -- https://pachube.com/feeds/40451

========================================================================================================*/
 
// ==================================
// -- Ethernet/Pachube section
// ==================================

#include <EtherCard.h>  // get latest version from https://github.com/jcw/ethercard
  // EtherShield uses the enc28j60 IC (not the WIZnet W5100 which requires a different library)
  
#include <NanodeUNIO.h>   // get latest version from https://github.com/sde1000/NanodeUNIO 
  // All Nanodes have a Microchip 11AA02E48 serial EEPROM chip
  // soldered to the underneath of the board (it's the three-pin
  // surface-mount device).  This chip contains a unique ethernet address
  // ("MAC address") for the Nanode.
  // To read the MAC address the library NanodeUNIO is needed
  
// If the above library has not yet been updated for Arduino1 (Rev01, not the beta 0022), 
// in the 2 files NanodeUNIO.h and NanoUNIO.cpp
// you will have to make the following modification:
    //#if ARDUINO >= 100
    //  #include <Arduino.h> // Arduino 1.0
    //#else
    //  #include <WProgram.h> // Arduino 0022+
    //#endif

byte macaddr[6];  // Buffer used by NanodeUNIO library
NanodeUNIO unio(NANODE_MAC_DEVICE);
boolean bMac; // Success or Failure upon function return

#define APIKEY  "fqJn9Y0oPQu3rJb46l_Le5GYxJQ1SSLo1ByeEG-eccE"  // MercinatLabs FreeRoom Pachube key for anyone to test this code
                        
#define REQUEST_RATE 10000 // in milliseconds - Pachube update rate
unsigned long lastupdate;  // timer value when last Pachube update was done
uint32_t timer;  // a local timer

byte Ethernet::buffer[550];
Stash stash;     // For filling/controlling EtherCard send buffer using satndard "print" instructions

int MyNanode = 0;
// END -- Ethernet/Pachube section


// ===========================
// -- Anemometer section
// ===========================

unsigned long PulseTimeNow = 0; // Time stamp (in millisecons) for pulse triggering the interrupt

// Davis anemometer
// ================
const float WindTo_mps = 1.00584;
const float WindTo_kph = 3.62102;
const float WindTo_mph = 2.25;
const float WindTo_knt = 1.95515;
float WindSpeed_mps, WindSpeed_kph, WindSpeed_mph, WindSpeed_knt;
unsigned long PulseTimeLast = 0; // Time stamp of the previous pulse
unsigned long PulseTimeInterval = 0;; // Time interval since last pulse

float WindSpeed_mpsx;
unsigned long PulsesCumulatedTime = 0; // Time Interval since last wind speed computation 
unsigned long PulsesNbr = 0;           // Number of pulses since last wind speed computation 
unsigned long LastPulseTimeInterval = 1000000000;
unsigned long MinPulseTimeInterval = 1000000000;
float MaxWind = 0.0; // Max wind speed 
float WindGust = 0.0;

// Davis vane
unsigned int WindDirection = 0; // value from the ADC
float DirectionVolt = 0.0; // voltage read from the vane output
float VaneOffset = 0.0; // Enter here the offset for you vane to get a calibrated wind direction 

// Misc.
// -----
int FirstLoop = 0;

// END -- Anemometer section

int TRUE = 1;
int FALSE = 0;


// **********************
// -- SETUP
// **********************
void setup()
{
  pinMode(6, OUTPUT);
  for (int i=0; i < 10; i++) { digitalWrite(6,!digitalRead(6)); delay (50);} // blink LED 6 a bit to greet us after reboot
  
  Serial.begin(115200);
  Serial.println("\n\nArduWind V1 - MercinatLabs (25 Nov 2011)");
  
  GetMac(); // get MAC adress from the Microchip 11AA02E48 located at the back of the Nanode board
  
  // Identify which sensor is assigned to this board
  // If you have boards with identical MAC last 2 values, you will have to adjust your code accordingly
  switch ( macaddr[5] )
  {
    case 0xFA: MyNanode = 6;  Serial.print("n6 "); Serial.print("f38277 - "); Serial.println("Etel 6 m") ; break; 
    case 0xC4: MyNanode = 9;  Serial.print("n9 "); Serial.print("f38278 - "); Serial.println("Etel 9 m") ; break;
    case 0xF4: MyNanode = 8;  Serial.print("n8 "); Serial.print("f38279 - "); Serial.println("Etel 12 m"); break;
    case 0xAF: MyNanode = 7;  Serial.print("n7 "); Serial.print("f38281 - "); Serial.println("Etel 18 m"); break;
    case 0xD6: MyNanode = 1;  Serial.print("n1 "); Serial.print("f37667 - "); Serial.println("Aurora")   ; break;
    case 0xC2: MyNanode = 2;  Serial.print("n2 "); Serial.print("f37668 - "); Serial.println("FemtoGrid"); break;
    case 0xEA: MyNanode = 3;  Serial.print("n3 "); Serial.print("f35020 - "); Serial.println("Skystream"); break;
    case 0xAC: MyNanode = 4;  Serial.print("n4 "); Serial.print("f40385 - "); Serial.println("Grid RMS #1"); break;
    case 0x8E: MyNanode = 5;  Serial.print("n5 "); Serial.print("f40386 - "); Serial.println("Grid RMS #2"); break;
    default:  
      Serial.println("unknown Nanode");
      Serial.print("nx "); Serial.print("f40447 - "); Serial.println("ArduWind Free Room"); break; 
      return;
  }

  // Ethernet/Internet setup
  while (ether.begin(sizeof Ethernet::buffer, macaddr) == 0) { Serial.println( "Failed to access Ethernet controller"); }
  while (!ether.dhcpSetup()) { Serial.println("DHCP failed"); }
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  
  while (!ether.dnsLookup(PSTR("api.pachube.com"))) { Serial.println("DNS failed"); }
  ether.printIp("SRV: ", ether.hisip);  // IP for Pachupe API found by DNS service
  
  // Anemometer Interrupt setup 
  // Davis anemometer is assigned to interrupt 1 on digital pin D3
  attachInterrupt(1, AnemometerPulse, FALLING);
  PulseTimeLast = micros();
}

// **********************
// -- MAIN LOOP
// **********************
void loop()  // START Pachube section
{  
  int j = 0;
  while ( j < 180 )  // As Pachube feeds may hang at times, reboot regularly. We will monitor stability then remove reboot when OK
  // a value of 180 with an update to Pachube every 10 seconds provoque a reboot every 30 mn. Reboot is very fast.
  {
    ether.packetLoop(ether.packetReceive());  // check response from Pachube
      
  if ( ( millis()-lastupdate ) > REQUEST_RATE )
  {
    lastupdate = millis();
    timer = lastupdate;
    j++;
    
    // DHCP expiration is a bit brutal, because all other ethernet activity and
    // incoming packets will be ignored until a new lease has been acquired
    if ( ether.dhcpExpired() && !ether.dhcpSetup() )
    { 
      Serial.println("DHCP failed");
      delay (200); // delay to let the serial port buffer some time to send the message before rebooting
      software_Reset() ;  // Reboot so can a new lease can be obtained
    }
   
    // Get Anemometer data 
    AnemometerLoop();
    
    if (FirstLoop <= 2) { return; } // Discard first sets of data to make sure you get clean data
    
    byte sd = stash.create();  // Initialise send data buffer
    
    stash.print("0,"); // Datastream 0 - Wind speed in m/s
    stash.println( WindSpeed_mps );
    stash.print("1,"); // Datastream 1 - Wind Gust in m/s
    stash.println( WindGust );
    stash.print("2,"); // Datastream 2 - Wind Direction
    stash.println( WindDirection );
    stash.print("3,"); // Datastream 3 - km/h
    stash.println( WindSpeed_kph );
    stash.print("4,"); // Datastream 4 - miles/h
    stash.println( WindSpeed_mph );
    stash.print("5,"); // Datastream 5 - knots
    stash.println( WindSpeed_knt );
    stash.print("6,"); // Datastream 6 - Nanode Health
    stash.println( j );
    
    stash.save(); // Close streaming send data buffer
 
    // Select the destination feed according to what the Nanode board is assigned to    
    switch ( MyNanode )
    {
      case 6: // Anemometre/Girouette Etel 6 m 
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
                        PSTR("api.pachube.com"), PSTR("38277"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
        break;
    
      case 9:  // Anemometre/Girouette Etel 9 m 
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
                        PSTR("api.pachube.com"), PSTR("38278"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
        break;
  
      case 8:   // Anemometre/Girouette Etel 12 m 
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
                        PSTR("api.pachube.com"), PSTR("38279"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
        break;
     
      case 7:   // Anemometre/Girouette Etel 18 m 
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
                        PSTR("api.pachube.com"), PSTR("38281"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
        break;
       
      case 1: // Aurora
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
                        PSTR("api.pachube.com"), PSTR("37667"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
         break;
        
      case 2: // FemtoGrid
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
                        PSTR("api.pachube.com"), PSTR("37668"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
        break;
        
      case 3: // Skystream
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
                        PSTR("api.pachube.com"), PSTR("35020"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
        break;
        
      case 4: // Grid RMS #1
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
                        PSTR("api.pachube.com"), PSTR("40385"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
        break;
        
      case 5: // Grid RMS #2
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
                        PSTR("api.pachube.com"), PSTR("40386"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
        break;
        
      default: // ArduGrid Free Room on Pachube -- https://pachube.com/feeds/40447
        Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                  "Host: $F" "\r\n"
                  "X-PachubeApiKey: $F" "\r\n"
                  "Content-Length: $D" "\r\n"
                  "\r\n"
                  "$H"),
                  PSTR("api.pachube.com"), PSTR("40447"), PSTR("api.pachube.com"), PSTR(APIKEY), stash.size(), sd);
        break;
    }
    
    // send the packet - this also releases all stash buffers once done
    ether.tcpSend();
    Serial.println("-- sending --"); 

            
    for (int i=0; i < 4; i++) { digitalWrite(6,!digitalRead(6)); delay (50);} // blink LED 6 a bit to show some activity on the board when sending to Pachube
  }
  }
  
  // reboot now to clean all dirty buffers to avoid Pachube feed hanging.
  Serial.println("-- rebooting --"); delay (250); 
  software_Reset() ;
 
// END -- Ethernet/Pachube section 
}


// **********************
// -- FUNCTIONS
// **********************

void AnemometerLoop ()    // START Anemometer section
{
  
  // with micros()
  WindSpeed_mpsx = 1000000*WindTo_mps/PulseTimeInterval; // calculated on last pulse periode (only 1 pulse)
  WindSpeed_mps = (1000000*WindTo_mps/PulsesCumulatedTime)*PulsesNbr; // averaged every 6 seconds (on Pachube update rate)
  WindSpeed_kph = (1000000*WindTo_kph/PulsesCumulatedTime)*PulsesNbr;
  WindSpeed_mph = (1000000*WindTo_mph/PulsesCumulatedTime)*PulsesNbr;
  WindSpeed_knt = (1000000*WindTo_knt/PulsesCumulatedTime)*PulsesNbr;
  MaxWind       = 1000000*WindTo_mps/MinPulseTimeInterval; // Determine wind gust (i.e the smallest pulse interval between Pachube updates)

  Serial.println("");  
  Serial.println("");  
  Serial.print("Wind speed -- ");
//  Serial.println("");  
//  Serial.print("------------ ");
//  Serial.println("");  
  Serial.print(WindSpeed_mpsx,DEC);
  Serial.print(" m/s      ");
  Serial.print(WindSpeed_kph,DEC);
  Serial.print(" km/h      ");
  Serial.print(WindSpeed_mph,DEC);
  Serial.print(" miles/h      ");
  Serial.print(""); 
  Serial.print(WindSpeed_knt,DEC);
  Serial.print(" knots");
  Serial.println("");
    
  Serial.print("    ");  
  Serial.print(WindSpeed_mps,DEC);
  Serial.print(" m/s      ");
  Serial.print(PulsesNbr,DEC);
  Serial.print(" pulses   ");  
  Serial.print(PulsesCumulatedTime,DEC);
  Serial.print(" time in microseconds between pachube updates   "); 
  Serial.println("");
  
  Serial.print("    ");  
  Serial.print(MaxWind,DEC);
  Serial.print(" max wind speed m/s   "); 
  Serial.println("");
 
  PulsesCumulatedTime = 0;
  PulsesNbr = 0;
  MinPulseTimeInterval = 1000000000;
  LastPulseTimeInterval = 1000000000;
  WindGust = MaxWind;
  MaxWind = 0;
  
  DirectionVolt = analogRead (1); // Analog input 1 (A1) is Davis Vane #2
  WindDirection = (DirectionVolt / 1024.0) * 360.0;
  WindDirection = WindDirection + VaneOffset ;  // add direction offset to calibrade vane position
  if (WindDirection > 360 ) { WindDirection = WindDirection - 360; }
  Serial.print("    ");  
  Serial.print("Wind Direction -- ");
  Serial.print(WindDirection);
  Serial.print(" Deg.");
  Serial.println("");
  
  if (FirstLoop <= 2)
  {
    FirstLoop = FirstLoop + 1;
    Serial.println("> First 2 loops, discard dirty data <");
  }
}

void AnemometerPulse() 
{
   noInterrupts();             // disable global interrupts
   PulseTimeNow = micros();   // Micros() is more precise to compute pulse width that millis();
   PulseTimeInterval = PulseTimeNow - PulseTimeLast;
   PulseTimeLast = PulseTimeNow;
   PulsesCumulatedTime = PulsesCumulatedTime + PulseTimeInterval;
   PulsesNbr++;
  
   if ( PulseTimeInterval < LastPulseTimeInterval )   // faster wind speed == shortest pulse interval
   { 
     MinPulseTimeInterval = PulseTimeInterval;
     LastPulseTimeInterval = MinPulseTimeInterval;
   }
  // deglitch algorith needs to added !!!!
      
   interrupts();              // Re-enable Interrupts
}

 void GetMac()
  {
    Serial.print("Reading MAC address... ");
    bMac=unio.read( macaddr, NANODE_MAC_ADDRESS, 6 );
    if ( bMac ) Serial.println("success");
    else Serial.println("failure");
    
    Serial.print("MAC: ");
    for ( int i=0; i < 6; i++ ) 
    {
      if ( macaddr[i] < 16 ) Serial.print("0");
      Serial.print( macaddr[i], HEX);
      if ( i < 5 ) Serial.print(":"); else Serial.print("");
    }
    Serial.println("");
  }

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
  {
     asm volatile ("  jmp 0");  
  } 
  

