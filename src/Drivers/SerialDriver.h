#ifndef SHS_2605_SERIAL_DRIVER_H
#define SHS_2605_SERIAL_DRIVER_H

#include "ErrorBase.h"
#include "NetworkCommunication/UsageReporting.h"
#include "visa/visa.h"

class SerialDriver : public ErrorBase
{
public:

	typedef enum 
	{

		Parity_None = 0, 
		Parity_Odd = 1, 
		Parity_Even = 2, 
		Parity_Mark = 3, 
		Parity_Space = 4

	} Parity;

	typedef enum 
	{

		StopBits_One = 10, 
		StopBits_OnePointFive = 15,
		StopBits_Two = 20
	
	} StopBits;

	typedef enum 
	{

		FlowControl_None = 0,
		FlowControl_XonXoff = 1, 
		FlowControl_RtsCts = 2, 
		FlowControl_DtrDsr = 4

	} FlowControl;

	typedef enum 
	{

		FlushOnAccess = 1, 
		FlushWhenFull = 2

	} WriteBufferMode; 

	SerialDriver ( uint32_t BaudRate = 9600, uint8_t DataBits = 8, Parity Par = Parity_None, StopBits Stop = StopBits_One );
	~SerialDriver ();

	void SetBaudRate ( uint32_t BaudRate );
	void SetFlowControl ( FlowControl Control );
	void SetParity ( Parity Par );
	void SetWriteBifferMode ( WriteBufferMode Mode );
	void SetStopBits ( StopBits Stop );

	void SetTimeout ( double Time );
	
	void SetReadBufferSize ( uint32_t BufferSize );
	void SetWriteBufferSize ( uint32_t BufferSize );

	uint32_t Write ( uint8_t * Buffer, uint32_t Count );
	uint32_t Read ( uint8_t * Buffer, uint32_t MaxCount );

	void Clear ();

	void FlushWriteBuffer ();
	void FlushReadBuffer ();

	void FlushDeviceWrite ();
	void FlushDeviceRead ();

private:

	uint32_t PortHandle;
	uint32_t ResourceHandle;

};

#endif
