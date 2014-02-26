#ifndef SHS_2605_SERIAL_DRIVER_H
#define SHS_2605_SERIAL_DRIVER_H

class SerialDriver
{
public:

	SerialDriver ( const char * File );
	~SerialDriver ();

	void SetBaudRate ( uint32_t BaudRate );
	void Flush ();
	void Write ( uint8_t * Buffer, uint32_t Count );
	void Read ( uint8_t * Buffer, uint32_t MaxCount );

private:

	int fd;



};

#endif