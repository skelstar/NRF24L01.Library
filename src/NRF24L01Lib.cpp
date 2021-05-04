#include "Arduino.h"
#include "NRF24L01Lib.h"

#ifndef PRINT_NRF24L01_DETAILS
#define PRINT_NRF24L01_DETAILS 0
#endif

//--------------------------------------------------------------------------------

NRF24L01Lib::NRF24L01Lib() {}

//--------------------------------------------------------------------------------
void NRF24L01Lib::begin(
		RF24 *radio,
		RF24Network *network,
		uint16_t address,
		PacketAvailableCallback packetAvailableCallback,
		bool multicastEnable,
		// TODO update library
		bool printDetails)
{
	_radio = radio;
	_network = network;
	_packetAvailableCallback = packetAvailableCallback;
	_multicastEnabled = multicastEnable;

	_radio->begin();
	_radio->setPALevel(RF24_PA_MAX);
	_radio->setDataRate(RF24_250KBPS);

	_network->begin(address);

	if (_multicastEnabled)
	{
		if (address == 00)
		{
			_network->multicastLevel(0);
		}
		else if (address == 01 || address == 02)
		{
			_network->multicastLevel(1);
		}
	}
	_radio->flush_rx();
	_radio->flush_tx();

	if (printDetails)
		_radio->printDetails(); // Dump the configuration of the rf unit for debugging
}
//---------------------------------------------------------------------------------
void NRF24L01Lib::update()
{
	_network->update();
	if (_network->available())
	{
		RF24NetworkHeader header;
		_network->peek(header);
		if (_packetAvailableCallback != nullptr)
			_packetAvailableCallback(header.from_node, header.type);
	}
}
//---------------------------------------------------------------------------------
void NRF24L01Lib::read_into(uint8_t *data, uint8_t data_len)
{
	RF24NetworkHeader header;
	_network->read(header, data, data_len);
}
//---------------------------------------------------------------------------------
bool NRF24L01Lib::send(uint16_t to, uint8_t type, uint8_t *data, uint8_t data_len)
{
	RF24NetworkHeader header(to, type);
	return _network->write(header, data, data_len);
}
//---------------------------------------------------------------------------------
bool NRF24L01Lib::broadcast(uint16_t to, uint8_t type, uint8_t *data, uint8_t data_len, uint8_t multicastLevel)
{
	if (_multicastEnabled)
	{
		RF24NetworkHeader header(to, type);
		return _network->multicast(header, data, data_len, multicastLevel);
	}
	Serial.printf("WARNING: you can't call broadcast if multicast is not enabled!\n");
	return false;
}
//---------------------------------------------------------------------------------
