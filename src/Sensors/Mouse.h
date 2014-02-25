#ifndef SHS_2605_MOUSE_H
#define SHS_2605_MOUSE_H

#include "WPILib.h"

#include <stdint.h>

#define MOUSE_CONDITIONAL_WAIT 0.00001

typedef struct mouse_data_struct
{

	int32_t X;
	int32_t Y;
	int32_t Z;

	bool MiddleButton;
	bool RightButton;
	bool LeftButton;

} mouse_data_t;

class Mouse
{
public:

	enum MouseResolution
	{

		Resolution_8 = 0x03,
		Resolution_4 = 0x02,
		Resolution_2 = 0x01,
		Resolution_1 = 0x00

	};

	Mouse ( uint8_t ClockChannel, uint8_t DataChannel, uint8_t Module );
	Mouse ( uint8_t ClockChannel, uint8_t DataChannel );
	~Mouse ();

	bool Init ( MouseResolution Resolution = Resolution_1 );

	void SetResolution ( MouseResolution Resolution = Resolution_1 );

	mouse_data_t GetData ();

private:

	void SetClockToInput ();
	void SetClockToOutput ();

	void SetDataToInput ();
	void SetDataToOutput ();

	void WriteByte ( uint8_t Data );
	uint8_t ReadByte ();

	void SetRemoteMode ();

	bool DataPinStatus;
	bool ClockPinStatus;

	DigitalModule * IOModule;

	uint8_t ClockChannel;
	uint8_t DataChannel;

	uint8_t MouseID;

};

#endif
