#ifndef SHS_2605_LED_STRIP_ANIMATOR_H
#define SHS_2605_LED_STRIP_ANIMATOR_H

#include "LEDStrip.h"
#include "src/Util/Vector.h"

class LEDStripAnimator
{
public:

	typedef struct LEDAnimationFrame
	{

		LEDAnimationFrame ( uint32_t Length, uint32_t Offset, double Time );
		LEDAnimationFrame ();

		~LEDAnimationFrame ();
		
		uint32_t * Colors;

		uint32_t BufferLength;
		uint32_t PixelCount;
		uint32_t Offset;

		double Time;

	} LEDAnimationFrame_t;

	LEDStripAnimator ( LEDStrip * Strip, uint32_t FrameCount );
	~LEDStripAnimator ();

	void SetClearedLEDS ( uint32_t Offset, uint32_t PixelCount, bool Cleared );

	void SetFrame ( uint32_t Frame, uint32_t * PixelColors, uint32_t PixelCount, uint32_t Offset, double Time );

	double GetRemainingFrameTime ();
	void WaitTillNextFrame ();

	void Update ();

private:

	LEDStrip * Strip;

	LEDAnimationFrame_t * Frames;

	uint32_t FrameCount;
	uint32_t PixelOffset;
	uint32_t PixelCount;

	double LastFrameTime;

	uint32_t CurrentFrame;

	bool DispatchNext;

	bool Started;
	bool Running;
	bool Paused;

	uint32_t StopColor;
	Vector <uint32_t> * StopSections;

};

#endif
