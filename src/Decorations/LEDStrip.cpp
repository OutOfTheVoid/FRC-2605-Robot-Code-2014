#include "LEDStrip.h"

LEDStrip :: LEDStrip ( uint32_t ClockPin, uint32_t MOSIPin, uint32_t MISOPin, uint32_t NumLEDS )
{

	this -> ClockPin = new DigitalOutput ( ClockPin );
	this -> MOSIPin = new DigitalOutput ( MOSIPin );
	this -> MISOPin = new DigitalInput ( MISOPin );

	Com = new SPI ( this -> ClockPin, this -> MOSIPin, this -> MISOPin );
	Com -> SetBitsPerWord ( 8 );
	Com -> SetMSBFirst ();

	StripLength = NumLEDS;
	LatchBytes = ( StripLength + 31 ) / 32;
	BufferSize = StripLength * 3 + LatchBytes;

	for ( uint32_t i = ( ( StripLength + 31 ) / 32 ) - 1; i > 0; i ++ )
		Com -> Write ( 0 );

	DataBuffer = new uint8_t [ BufferSize ];

	memset ( DataBuffer, 0x80, BufferSize );
	memset ( & DataBuffer [ StripLength * 3 ], 0x00, LatchBytes );

	NewData = false;

	AccessSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

};

LEDStrip :: ~LEDStrip ()
{

 	// TODO: Destruct everything. EVERYTHING.

};

void LEDStrip :: Clear ( uint32_t Color )
{

	for ( uint32_t i = 0; i < StripLength; i ++ )
		SetPixelColor ( i, Color );

};

void LEDStrip :: SetPixelColor ( uint32_t Pixel, uint32_t Color )
{

	if ( Pixel >= StripLength )
		return;

	semTake ( AccessSemaphore, WAIT_FOREVER );

	uint8_t R = ( Color & 0x7F0000 ) >> 16;
	uint8_t G = ( Color & 0x7F00 ) >> 8;
	uint8_t B = Color & 0x7F;

	R |= 0x80;
	G |= 0x80;
	B |= 0x80;

	DataBuffer [ Pixel * 3 ] = G;
	DataBuffer [ Pixel * 3 + 1 ] = R;
	DataBuffer [ Pixel * 3 + 2 ] = B;

	NewData = true;

	semGive ( AccessSemaphore );

};

void LEDStrip :: PushColors ()
{

	semTake ( AccessSemaphore, WAIT_FOREVER );

	for ( uint32_t i = 0; i < BufferSize; i ++ )
		Com -> Write ( DataBuffer [ i ] );

	NewData = false;

	semGive ( AccessSemaphore );

};

bool LEDStrip :: HasNewData ()
{

	return NewData;

};

uint32_t LEDStrip :: ColorFromRGB ( uint8_t R, uint8_t G, uint8_t B )
{

	return R << 16 | G << 8 | B;

};
