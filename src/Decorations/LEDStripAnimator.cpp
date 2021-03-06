#include "LEDStripAnimator.h"

LEDStripAnimator :: LEDAnimationFrame :: LEDAnimationFrame ( uint32_t Length, uint32_t Offset, double Time )
{

	this -> BufferLength = Length;
	this -> PixelCount = Length;

	this -> Offset = Offset;

	this -> Time = Time;

	Colors = new uint32_t [ BufferLength ];

	memset ( Colors, 0, BufferLength * sizeof ( uint32_t ) );

};

LEDStripAnimator :: LEDAnimationFrame :: LEDAnimationFrame ()
{

	this -> BufferLength = 0;
	this -> PixelCount = 0;

	this -> Time = 0;

	Colors = NULL;

};

LEDStripAnimator :: LEDAnimationFrame :: ~LEDAnimationFrame ()
{

	if ( Colors != NULL )
		delete Colors;

	Colors = NULL;

};

LEDStripAnimator :: LEDStripAnimator ( LEDStrip * Strip, uint32_t FrameCount )
{

	this -> Strip = Strip;

	this -> Frames = new LEDAnimationFrame_t [ FrameCount ];

	this -> FrameCount = FrameCount;
	this -> CurrentFrame = 0;

	this -> DispatchNext = false;

	for ( uint32_t i = 0; i < FrameCount; i ++ )
		new ( & ( this -> Frames [ i ] ) ) LEDAnimationFrame_t ( 0, 0, 0.04 );

	this -> Started = false;
	this -> Paused = false;
	this -> Running = false;

	this -> StopColor = 0x000000;

	this -> StopSections = new Vector <uint32_t> ();

	this -> DispatchInstantaneous = true;

};

LEDStripAnimator :: ~LEDStripAnimator ()
{

	// TODO: Destructor.

};

void LEDStripAnimator :: SetFrame ( uint32_t FrameIndex, uint32_t * PixelColors, uint32_t PixelCount, uint32_t Offset, double Time )
{

	if ( FrameIndex >= FrameCount )
		return;

	LEDAnimationFrame_t * Frame = & Frames [ FrameIndex ];

	if ( Frame -> BufferLength <= PixelCount )
	{

		Frame -> ~LEDAnimationFrame_t ();
		new ( Frame ) LEDAnimationFrame_t ( PixelCount, Offset, Time );

	}
	else
	{

		Frame -> PixelCount = PixelCount;
		Frame -> Offset = Offset;
		Frame -> Time = Time;

	}

	memcpy ( Frame -> Colors, PixelColors, PixelCount * sizeof ( uint32_t ) );

	uint32_t MaxPixel = Offset + PixelCount + 31;
	
	MaxPixel /= 32;

	while ( MaxPixel < StopSections -> GetLength () )
		StopSections -> Push ( 0 );

	CurrentFrame = 0;

};

void LEDStripAnimator :: SetClearedLEDS ( uint32_t Offset, uint32_t PixelCount, bool Cleared )
{

	uint32_t MaxPixel = Offset + PixelCount + 31;
	
	MaxPixel /= 32;

	while ( MaxPixel < StopSections -> GetLength () )
		StopSections -> Push ( 0 );

	for ( uint32_t i = Offset; i < Offset + PixelCount; i ++ )
	{

		uint8_t Bit = i & 0x1F;
		( * StopSections ) [ i / 0x20 ] = ( ( * StopSections ) [ i / 0x20 ] & ~ ( i << Bit ) ) | Cleared ? ( 1 << Bit ) : 0;

	}

};

void LEDStripAnimator :: SetInstantaneousDispatch ( bool Instantaneous )
{

	DispatchInstantaneous = Instantaneous;

};

double LEDStripAnimator :: GetRemainingFrameTime ()
{

	if ( FrameCount == 0 )
		return - 1;

	LEDAnimationFrame_t * Frame = & Frames [ CurrentFrame ];

	double TimeSinceLast = Timer :: GetPPCTimestamp () - LastFrameTime;
	return Frame -> Time - TimeSinceLast;

};

void LEDStripAnimator :: WaitTillNextFrame ()
{

	if ( FrameCount == 0 )
		return;

	LEDAnimationFrame_t * Frame = & Frames [ CurrentFrame ];

	double TimeSinceLast = Timer :: GetPPCTimestamp () - LastFrameTime;
	double TimeLeft = Frame -> Time - TimeSinceLast;

	if ( TimeLeft > 0 )
		Wait ( TimeLeft );

};

void LEDStripAnimator :: Start ()
{

	Running = true;

};

void LEDStripAnimator :: Stop ()
{

	Running = false;

};

void LEDStripAnimator :: Update ()
{

	if ( FrameCount == 0 )
		return;
	
	if ( Running )
	{

		if ( ! Started )
		{

			Started = true;

			LEDAnimationFrame_t * Frame = & Frames [ CurrentFrame ];

			for ( uint32_t i = 0; i < Frame -> PixelCount; i ++ )
					Strip -> SetPixelColor ( Frame -> Offset + i, Frame -> Colors [ i ] );

			LastFrameTime = Timer :: GetPPCTimestamp ();

		}

		if ( ! Paused )
		{

			LEDAnimationFrame_t * Frame = & Frames [ CurrentFrame ];

			double TimeSinceLast = Timer :: GetPPCTimestamp () - LastFrameTime;
			double TimeLeft = Frame -> Time - TimeSinceLast;

			if ( TimeLeft <= 0 )
			{

				while ( true )
				{

					CurrentFrame ++;
					CurrentFrame %= FrameCount;

					Frame = & Frames [ CurrentFrame ];

					for ( uint32_t i = 0; i < Frame -> PixelCount; i ++ )
						Strip -> SetPixelColor ( Frame -> Offset + i, Frame -> Colors [ i ] );

					if ( ( Frame -> Time != 0 ) || ( ! DispatchInstantaneous ) )
						break;

				}

				LastFrameTime = Timer :: GetPPCTimestamp ();

			}

		}

	}
	else
	{

		if ( Started )
		{

			Started = false;

			for ( uint32_t i = 0; i < StopSections -> GetLength (); i ++ )
			{

				uint32_t LEDMask = ( * StopSections ) [ i ];

				for ( uint32_t j = 0; j < 0x20; j ++ )
					if ( LEDMask & 1 << j )
					Strip -> SetPixelColor ( i * 32 + j, StopColor );

			}

		}

	}
	
};

