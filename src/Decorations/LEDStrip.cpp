#include "LEDStrip.h"

LEDStrip :: LEDStrip ( uint32_t ClockPin, uint32_t MOSIPin, uint32_t NumLEDS )
{

	printf ( "LEDStrip: Creating Digitial IO\n" );

	this -> ClockPin = new DigitalOutput ( ClockPin );
	this -> MOSIPin = new DigitalOutput ( MOSIPin );

	printf ( "LEDStrip: Setting up SPI\n" );

	Com = new SPI ( this -> ClockPin, this -> MOSIPin );
	Com -> SetBitsPerWord ( 8 );
	Com -> SetMSBFirst ();
	Com -> SetClockActiveLow ();
	Com -> ApplyConfig ();

	printf ( "LEDStrip: Calculating buffer stuff\n" );

	BufferSize = 4 + ( 2 * NumLEDS ) + ( NumLEDS / 8 ) + 1;
	DataBuffer = new uint8_t [ BufferSize ];

	memset ( DataBuffer, 0x00, BufferSize );

	Clear ( 0x000000 );
	PushColors ();

	NewData = false;

	AccessSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );
	semGive ( AccessSemaphore );

	printf ( "LEDStrip: Done!\n" );

};

LEDStrip :: ~LEDStrip ()
{

	semDelete ( AccessSemaphore );

 	delete DataBuffer;

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

	//semTake ( AccessSemaphore, WAIT_FOREVER );

	uint16_t D = ( ( Color & 0xF80000 ) >> 9 ) | ( ( Color & 0xF800 ) >> 6 ) | ( ( Color & 0xF8 ) >> 3 ) | 0x8000;
	DataBuffer [ Pixel * 2 + 4 ] = D >> 8;
	DataBuffer [ Pixel * 2 + 5 ] = D;

	NewData = true;

	//semGive ( AccessSemaphore );

};

void LEDStrip :: PushColors ()
{

	//semTake ( AccessSemaphore, WAIT_FOREVER );

	for ( uint32_t i = 0; i < BufferSize; i ++ )
		Com -> Write ( DataBuffer [ i ] );

	NewData = false;

	//semGive ( AccessSemaphore );

};

bool LEDStrip :: HasNewData ()
{

	return NewData;

};

uint32_t LEDStrip :: ColorFromRGB ( uint8_t R, uint8_t G, uint8_t B )
{

	return R << 16 | G << 8 | B;

};
