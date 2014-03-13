#ifndef SHS_2605_LED_STRIP_H
#define SHS_2605_LED_STRIP_H

#include "WPILib.h"

class LEDStrip
{
public:

	LEDStrip ( uint32_t ClockPin, uint32_t MOSIPin, uint32_t NumLEDS );
	~LEDStrip ();

	void Clear ( uint32_t Color = 0x000000 );
	void SetPixelColor ( uint32_t Pixel, uint32_t Color );

	bool HasNewData ();
	void PushColors ();

	static uint32_t ColorFromRGB ( uint8_t R, uint8_t G, uint8_t B );

private:

	bool NewData;

	SPI * Com;

	DigitalOutput * ClockPin;
	DigitalOutput * MOSIPin;

	uint32_t StripLength;
	uint32_t LatchBytes;
	uint32_t BufferSize;

	uint8_t * DataBuffer;

	SEM_ID AccessSemaphore;

};

#endif
