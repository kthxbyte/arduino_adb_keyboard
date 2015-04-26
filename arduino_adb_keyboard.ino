/*
*  arduino_adb_keyboard: Have fun with your old Mac keyboard
*  Copyright (C) 2015 Salvador Muñoz
*  <https://github.com/kthxbyte>
*
*  This program is free software: you can redistribute it and/or
*  modify it  under the terms of the  GNU General Public License
*  as published  by the Free Software Foundation, either version
*  3 of the License, or (at your option) any later version.
*
*  This  program  is distributed  in the  hope  that  it will be
*  useful, but  WITHOUT ANY WARRANTY;  without even  the implied
*  warranty  of  MERCHANTABILITY  or  FITNESS  FOR  A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should  have received  a copy  of the  GNU General Public
*  License along with this program. If not, see
*  <http://www.gnu.org/licenses/>.
*/

unsigned char ADB_DATA = 13; // Arduino Nano v3.0, Pin D13

void setup() {
  Serial.begin( 9600 );
}

void ADBattention(){
  digitalWrite( ADB_DATA, 0 );
  delayMicroseconds( 800 );
  digitalWrite( ADB_DATA, 1 );
}

// Bit-banging to adb keyboard:
// ___------ sending 0 (low 35us / high 65us)
// ______--- sending 1 (low 65us / high 35us)
void ADBsendBit( boolean bit ){
  digitalWrite( ADB_DATA, 0 );
  unsigned char time_low = ( bit )? 35: 65;
  
  delayMicroseconds( time_low );
  digitalWrite( ADB_DATA, 1 );
  delayMicroseconds( 100 - time_low );
}

void ADBsendByte( byte bits ){
  for( int i = 0; i < 8; i++ ) {
    ADBsendBit( bits &( 0x80 >> i ));
  }
}

void ADBreset(){
  ADBattention();
  ADBsendByte( 0x00 );
  ADBsendBit( 0 );
}

void ADBhostListen(
             byte command,
             byte hiByte,
             byte loByte )
{
  ADBattention();
  ADBsendByte( command );
  ADBsendBit( 0 );
  delayMicroseconds( 200 );
  ADBsendBit( 1 );
  ADBsendByte( hiByte );
  ADBsendByte( loByte );
  ADBsendBit( 0 );
}

unsigned int ADBwait( boolean bit, int microseconds ){
  for( ; microseconds > 3; microseconds -= 3 ){
    if( digitalRead( ADB_DATA ) == bit ) break;
    delayMicroseconds( 3 );
  }
  return microseconds;
}

int ADBscanKeyboard(){
  int data = 0;
  int time;
  pinMode( ADB_DATA, OUTPUT );
  ADBattention();
}

boolean ADBscanBit()
{
    unsigned int lo = ADBwait( 1, 130 );
    unsigned int hi = ADBwait( 0, lo );
    hi = lo - hi;
    lo = 130 - lo;
    return ( lo < hi );
}

int ADBreadKeyboard()
{
  int debug;

  pinMode( ADB_DATA, OUTPUT );
  ADBattention();
  ADBsendByte( 0x2C ); // Addr 0010, cmd 11, register 00
  ADBsendBit( 0 );
  pinMode( ADB_DATA, INPUT );

  debug = ADBwait( 1, 500 );
  if( debug < 3 ){
    return -30; // ?
  }
  debug = ADBwait( 0, 500 );
  if( debug < 3 ){
    return 0; // No keyboard data 
  }  
  
  int code = 0;
  if( !ADBscanBit() ){
    return -1;
  }
  for( int n = 0; n < 16; n++ ){
    code = ( code << 1 ) | ADBscanBit();
  }
  return code; 
}

void loop() {
  int data = ADBreadKeyboard();
  if( data != 0 ){
    Serial.print( "[ " );
    Serial.print((data & 0x8000) >> 15);
    Serial.print((data & 0x4000) >> 14);
    Serial.print((data & 0x2000) >> 13);
    Serial.print((data & 0x1000) >> 12);
    Serial.print( " " );
    Serial.print((data & 0x0800) >> 11);
    Serial.print((data & 0x0400) >> 10);
    Serial.print((data & 0x0200) >> 9);
    Serial.print((data & 0x0100) >> 8);
    Serial.print( " " );
    Serial.print((data & 0x0080) >> 7);
    Serial.print((data & 0x0040) >> 6);
    Serial.print((data & 0x0020) >> 5);
    Serial.print((data & 0x0010) >> 4);
    Serial.print( " " );
    Serial.print((data & 0x0008) >> 3);
    Serial.print((data & 0x0004) >> 2);
    Serial.print((data & 0x0002) >> 1);
    Serial.print((data & 0x0001));
    Serial.print( " ] [ " );
    Serial.print(( data & 0xFF00 ) >> 8 );
    Serial.print( " " );
    Serial.print( data & 0x00FF );
    Serial.print( " ] [ " );
    Serial.print( data );
    Serial.println( " ]");
  }
  delay( 30 );  
}

