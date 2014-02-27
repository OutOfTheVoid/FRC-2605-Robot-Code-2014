#include "SerialDriver.h"

SerialDriver :: SerialDriver ( uint32_t Baud, uint8_t Data, SerialDriver :: Parity Par, SerialDriver :: StopBits Stop )
{

	PortHandle = 0;
	ResourceHandle = 0;

	ViStatus LocalStatus = VI_SUCCESS;

	LocalStatus = viOpenDefaultRM ( ( ViSession * ) & ResourceHandle );
	wpi_setError ( LocalStatus );

	LocalStatus = viOpen ( ResourceHandle, const_cast <char *> ( "ASRL1::INSTR" ), VI_NULL, VI_NULL, (ViSession*) & PortHandle );
	wpi_setError ( LocalStatus );

	if ( LocalStatus != 0 )
		throw "SerialDriver Error: Failed to open port.";

	SetBaudRate ( Baud );
	SetParity ( Par );
	SetStopBits ( Stop );
	SetTimeout ( 1.0 );

	LocalStatus = viSetAttribute ( PortHandle, VI_ATTR_ASRL_DATA_BITS, Data );
	wpi_setError ( LocalStatus );

	LocalStatus = viSetAttribute ( PortHandle, VI_ATTR_TERMCHAR_EN, VI_FALSE );
	wpi_setError ( LocalStatus );

	LocalStatus = viSetAttribute ( PortHandle, VI_ATTR_ASRL_END_IN, VI_ASRL_END_NONE );
	wpi_setError ( LocalStatus );

	nUsageReporting::report ( nUsageReporting::kResourceType_SerialPort, 0 );

};

SerialDriver :: ~SerialDriver ()
{

	viClose ( PortHandle );
	viClose ( ResourceHandle );

};

void SerialDriver :: SetBaudRate ( uint32_t Baud )
{

	ViStatus LocalStatus = viSetAttribute ( PortHandle, VI_ATTR_ASRL_BAUD, Baud );
	wpi_setError ( LocalStatus );

};

void SerialDriver :: SetFlowControl ( SerialDriver :: FlowControl Control )
{

	ViStatus LocalStatus = viSetAttribute ( PortHandle, VI_ATTR_ASRL_FLOW_CNTRL, Control );
	wpi_setError ( LocalStatus );

};

void SerialDriver :: SetParity ( SerialDriver :: Parity Par )
{

	ViStatus LocalStatus = viSetAttribute ( PortHandle, VI_ATTR_ASRL_PARITY, Par );
	wpi_setError ( LocalStatus );

};

void SerialDriver :: SetStopBits ( SerialDriver :: StopBits Stop )
{

	ViStatus LocalStatus = viSetAttribute ( PortHandle, VI_ATTR_ASRL_STOP_BITS, Stop );
	wpi_setError ( LocalStatus );

}

void SerialDriver :: SetWriteBifferMode ( SerialDriver :: WriteBufferMode Mode )
{

	ViStatus localStatus = viSetAttribute ( PortHandle, VI_ATTR_WR_BUF_OPER_MODE, Mode );
	wpi_setError(localStatus);

};

void SerialDriver :: SetTimeout ( double Time )
{

	ViStatus LocalStatus = viSetAttribute ( PortHandle, VI_ATTR_TMO_VALUE, static_cast <uint32_t> ( Time * 1000 ) );
	wpi_setError ( LocalStatus );

};
	
uint32_t SerialDriver :: Write ( uint8_t * Buffer, uint32_t Count )
{

	uint32_t BytesWritten = 0;

	ViStatus LocalStatus = viWrite ( PortHandle, reinterpret_cast <ViPBuf> ( Buffer ), Count, reinterpret_cast <ViPUInt32> ( & BytesWritten ) );
	wpi_setError ( LocalStatus );

	return BytesWritten;

};

uint32_t SerialDriver :: Read ( uint8_t * Buffer, uint32_t MaxCount )
{

	uint32_t BytesRead = 0;

	ViStatus LocalStatus = viRead ( PortHandle, reinterpret_cast <ViPBuf> ( Buffer ), MaxCount, reinterpret_cast <ViPUInt32> ( & BytesRead ) );
	
	switch ( LocalStatus )
	{
		
		case VI_SUCCESS_TERM_CHAR:
		case VI_SUCCESS_MAX_CNT:
		case VI_ERROR_TMO:
			
			break;
		
		default:

			wpi_setError ( LocalStatus );
			
			break;
		
	}

	return BytesRead;

};

void SerialDriver :: SetReadBufferSize ( uint32_t BufferSize )
{

	ViStatus LocalStatus = viSetBuf ( PortHandle, VI_READ_BUF, BufferSize );
	wpi_setError ( LocalStatus );

};

void SerialDriver :: SetWriteBufferSize ( uint32_t BufferSize )
{

	ViStatus LocalStatus = viSetBuf ( PortHandle, VI_WRITE_BUF, BufferSize );
	wpi_setError ( LocalStatus );

};

void SerialDriver :: Clear ()
{

	ViStatus LocalStatus = viClear ( PortHandle );
	wpi_setError ( LocalStatus );

};

void SerialDriver :: FlushWriteBuffer ()
{

	ViStatus LocalStatus = viFlush ( PortHandle, VI_WRITE_BUF );
	wpi_setError ( LocalStatus );

};

void SerialDriver :: FlushReadBuffer ()
{

	ViStatus LocalStatus = viFlush ( PortHandle, VI_READ_BUF );
	wpi_setError ( LocalStatus );

};

void SerialDriver :: FlushDeviceWrite ()
{

	ViStatus LocalStatus = viFlush ( PortHandle, VI_IO_OUT_BUF );
	wpi_setError ( LocalStatus );	

};

void SerialDriver :: FlushDeviceRead ()
{

	ViStatus LocalStatus = viFlush ( PortHandle, VI_IO_IN_BUF );
	wpi_setError ( LocalStatus );

};
