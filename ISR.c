//ISR Routines go in here
//Created: 4/6/17
//Last update: 4/6/17

#include <msp430.h>
#include <msp430f6779a.h>

//GPS ISR
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR (void)
  {
     static int i = 0;
     //GPS data & gps header
     char gps[100], gpsh[6];
     int comma = 0;
     int cycle, deg, min, sec;
     double GpsLat, GpsLong, GpsElv;

    switch(UCA0IV)
      {
      case 0x00: break;

      case 0x02:
       //Save current char into variable array
       gps[i] = UCA0RXBUF;
       gps[i+1] = '\0';
             
       //Look for 'enter' at the end of the gps line
       if(gps[i] == '\n')
        {
            // reset global counter
            i = 0;
            
            //Store header for checking
            for(i = 0; i = 5; i++)
              {
                gpsh[i] = gps[i];
              }

            //reset global counter
            i = 0;

            //Check header (really all we care about is the GGA data, everything else can be dropped)
            if(!strcmp(gpsh,"$GPGGA"))
              {
                //After confirming the header, the program will cycle through the stored data
                while(gps[i] != '\0')
                  {
                    //comma counting function, used  to parse the data string
                    if(gps[i] == ',')
                     {
                      //ensure counter is not storing the comma
                      i++;
                    switch(comma)
                      {
                        //Header identifier, unneeded data
                        case 0:
                          comma++;
                          break;
                        
                        //UTC time, unneeded
                        case 1:
                          comma++;
                          break;
                        
                        //Latitude Data
                        case 2:
                          cycle = i;
                          for(cycle = i+1; cycle = i+9; cycle++)
                            {
                              if(gps[cycle] == '.' & cycle > 2)
                                {
                                 cycle++;
                                 sec = (gps[cycle]-48)*1000+(gps[cycle+1]-48)*100+(gps[cycle+2]-48)*10+(gps[cycle+3]-48);
                                }
                              else if (cycle > 2)
                                {
                                  min = (gps[cycle]-48)*10+(gps[cycle+1]-48);
                                }
                              else
                                {
                                  deg = (gps[cycle]-48)*10+(gps[cycle+1]-48);
                                }
                            }
                           GpsLat = deg + (min/60) + (sec/3600);
                           comma++;
                           break;
                        
                        //Latitude north or south
                        case 3:
                          cycle = i+1;
                          if(gps[cycle] == 'S')
                            {
                              GpsLat = 0-GpsLat;
                            }
                          else
                            {
                              break;
                            }
                           comma++;
                           break;
                        
                        //Longitude Data
                        case 4:
                         cycle = i;
                          for(cycle = i+1; cycle = i+9; cycle++)
                            {
                              if(gps[cycle] == '.' & cycle > 2)
                                {
                                 cycle++;
                                 sec = (gps[cycle]-48)*1000+(gps[cycle+1]-48)*100+(gps[cycle+2]-48)*10+(gps[cycle+3]-48);
                                }
                              else if (cycle > 2)
                                {
                                  min = (gps[cycle]-48)*10+(gps[cycle+1]-48);
                                }
                              else
                                {
                                  deg = (gps[cycle]-48)*10+(gps[cycle+1]-48);
                                }
                            }
                           GpsLong = deg + (min/60) + (sec/3600);
                           comma++;
                           break;
                        
                        //Longitude East or West
                        case 5:
                          cycle = i+1;
                          if(gps[cycle] == 'W')
                            {
                              GpsLong = 0-GpsLong;
                            }
                          else
                            {
                              break;
                            }
                           comma++;
                           break;

                         //Ignored Data points
                         case 6:
                          comma++;
                          break;
                         case 7:
                          comma++;
                          break;
                         case 8:
                          comma++;
                          break;

                         //Elevation Data
                         case 9:
                          GpsElv = (gps[i]-48)+(gps[i+2]-48)/10;
                          comma++;
                          break;

                      }
                     }
                    i++;  // increment counter
                  }
              }
        
            else
              {
                break;
              }
        }

       else
        {
          i++;
        }
        break;

      case 0x04: break;
      default: break;
      }
  }
