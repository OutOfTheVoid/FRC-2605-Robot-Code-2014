#include "Mouse.h"

Mouse :: Mouse ( uint8_t ClockChannel, uint8_t DataChannel, uint8_t Module )
{

	this -> ClockChannel = ClockChannel;
	this -> DataChannel = DataChannel;

	IOModule = DigitalModule :: GetInstance ( Module );

	if ( IOModule == NULL )
		throw "Mouse error: Digital module out of range.";

	IOModule -> AllocateDIO ( ClockChannel, false );
	IOModule -> AllocateDIO ( DataChannel, false );

	ClockPinStatus = false;
	DataPinStatus = false;

	IOModule -> SetDIO ( ClockChannel, 1 );
	IOModule -> SetDIO ( DataChannel, 1 );

};

Mouse :: Mouse ( uint8_t ClockChannel, uint8_t DataChannel )
{

	Mouse ( ClockChannel, DataChannel, SensorBase :: GetDefaultDigitalModule () );

};

Mouse :: ~Mouse ()
{

	IOModule -> FreeDIO ( ClockChannel );
	IOModule -> FreeDIO ( DataChannel );

};

bool Mouse :: Init ( MouseResolution Resolution )
{

	uint8_t BAT;

	printf ( "Mouse :: WriteByte ( 0xFF );\n" );
	WriteByte ( 0xFF );
	printf ( "Mouse :: ReadByte ();\n" );
	ReadByte ();
	
	BAT = ReadByte ();
	MouseID = ReadByte ();

	if ( BAT != 0xAA )
		return false;

	SetRemoteMode ();

	SetResolution ( Resolution );

	return true;

};

void Mouse :: SetRemoteMode ()
{

	WriteByte ( 0xF0 );
	ReadByte ();

};

void Mouse :: SetResolution ( MouseResolution Resolution )
{

	WriteByte ( 0xE8 );
	ReadByte ();
	WriteByte ( static_cast <uint8_t> ( Resolution ) );
	ReadByte ();

};

mouse_data_t Mouse :: GetData ()
{

	mouse_data_t Data;

	uint8_t Bytes [ 3 ];

	for ( uint8_t i = 0; i < 3; i ++ )
		Bytes [ i ] = ReadByte ();

	Data.X = Bytes [ 1 ] * ( ( Bytes [ 0 ] & 0x10 ) == 1 ? -1 : 1 );
	Data.Y = Bytes [ 2 ] * ( ( Bytes [ 0 ] & 0x20 ) == 1 ? -1 : 1 );

	Data.MiddleButton = Bytes [ 0 ] & 0x04;
	Data.RightButton = Bytes [ 0 ] & 0x02;
	Data.LeftButton = Bytes [ 0 ] & 0x01;

	return Data;

};

void Mouse :: WriteByte ( uint8_t Data )
{

	uint8_t Parity = 0;

	printf ( "Mouse :: SetClockToOutput ();\n" );

	SetClockToOutput ();

	printf ( "Mouse :: SetDataToOutput ();\n" );

	SetDataToOutput ();

	printf ( "IOModule -> SetDIO ( ClockChannel, 0 );\n" );

	IOModule -> SetDIO ( ClockChannel, 0 );

	Wait ( 0.0002 );

	IOModule -> SetDIO ( DataChannel, 0 );
	IOModule -> SetDIO ( ClockChannel, 1 );

	SetClockToInput ();

	while ( IOModule -> GetDIO ( ClockChannel ) == 1 )
		Wait ( MOUSE_CONDITIONAL_WAIT );

	for ( uint8_t i = 0; i < 8; i ++ )
	{

		IOModule -> SetDIO ( DataChannel, 1 );

		Wait ( MOUSE_CONDITIONAL_WAIT );

		IOModule -> SetDIO ( DataChannel, Data & ( 0x80 >> i ) );
		Parity += ( Data & ( 0x80 >> i ) ) != 0 ? 1 : 0;

		while ( IOModule -> GetDIO ( ClockChannel ) == 0 )
			Wait ( MOUSE_CONDITIONAL_WAIT );

		while ( IOModule -> GetDIO ( ClockChannel ) == 1 )
			Wait ( MOUSE_CONDITIONAL_WAIT );

	}

	IOModule -> SetDIO ( DataChannel, ! Parity );

	while ( IOModule -> GetDIO ( ClockChannel ) == 0 )
		Wait ( MOUSE_CONDITIONAL_WAIT );

	while ( IOModule -> GetDIO ( ClockChannel ) == 1 )
		Wait ( MOUSE_CONDITIONAL_WAIT );

	IOModule -> SetDIO ( DataChannel, 1 );

	SetDataToInput ();

	while ( IOModule -> GetDIO ( ClockChannel ) == 1 || IOModule -> GetDIO ( DataChannel ) == 1 )
		Wait ( MOUSE_CONDITIONAL_WAIT );

	while ( IOModule -> GetDIO ( ClockChannel ) == 0 || IOModule -> GetDIO ( DataChannel ) == 0 )
		Wait ( MOUSE_CONDITIONAL_WAIT );

};

uint8_t Mouse :: ReadByte ()
{

	uint8_t Data = 0;

	SetClockToInput ();
	SetDataToInput ();

	while ( IOModule -> GetDIO ( ClockChannel ) == 1 )
		Wait ( MOUSE_CONDITIONAL_WAIT );

	while ( IOModule -> GetDIO ( ClockChannel ) == 0 )
		Wait ( MOUSE_CONDITIONAL_WAIT );

	for ( uint8_t i = 0; i < 10; i ++ )
	{

		while ( IOModule -> GetDIO ( ClockChannel ) == 1 )
			Wait ( MOUSE_CONDITIONAL_WAIT );

		if ( i < 8 )
			Data |= ( IOModule -> GetDIO ( DataChannel ) << i );

		while ( IOModule -> GetDIO ( ClockChannel ) == 0 )
			Wait ( MOUSE_CONDITIONAL_WAIT );

	}

	return Data;

};

void Mouse :: SetClockToInput ()
{

	if ( ClockPinStatus )
		return;

	IOModule -> FreeDIO ( ClockChannel );
	IOModule -> AllocateDIO ( ClockChannel, true );

	ClockPinStatus = true;

};

void Mouse :: SetClockToOutput ()
{

	if ( ! ClockPinStatus )
		return;

	printf ( "IOModule -> FreeDIO ( ClockChannel );\n" );
	
	IOModule -> FreeDIO ( ClockChannel );

	printf("IOModule -> AllocateDIO ( ClockChannel, false );\n");

	IOModule -> AllocateDIO ( ClockChannel, false );

	ClockPinStatus = false;

};

void Mouse :: SetDataToInput ()
{

	if ( DataPinStatus )
		return;

	printf ( "IOModule -> FreeDIO ( DataChannel );\n" );

	IOModule -> SetDO_PWMOutputChannel ( ~0ul, SensorBase :: kDigitalChannels );
	IOModule -> FreeDIO ( DataChannel );

	printf ( "IOModule -> AllocateDIO ( DataChannel, true );\n" );

	IOModule -> AllocateDIO ( DataChannel, true );

	DataPinStatus = true;

};

void Mouse :: SetDataToOutput ()
{

	if ( ! DataPinStatus )
		return;

	IOModule -> FreeDIO ( DataChannel );
	IOModule -> AllocateDIO ( DataChannel, false );

	DataPinStatus = false;

};
