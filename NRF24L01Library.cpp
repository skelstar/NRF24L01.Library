#include "Arduino.h"
#include "NRF24L01Library.h"

//--------------------------------------------------------------------------------

NRF24L01Lib::NRF24L01Lib() {}

//--------------------------------------------------------------------------------
void NRF24L01Lib::begin(
		RF24 *radio,
		RF24Network *network,
		PacketAvailableCallback packetAvailableCallback)
{
	_radio = radio;
	_network = network;
	_packetAvailableCallback = packetAvailableCallback;

	_radio->begin();
	_radio->setPALevel(RF24_PA_MAX);
	_radio->setDataRate(RF24_1MBPS);

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
bool NRF24L01Lib::sendPacket(uint16_t to, uint8_t type, uint8_t *data, uint8_t data_len) {

	RF24NetworkHeader header( to, type );
	return _network->write(header, data, data_len);
}
//---------------------------------------------------------------------------------
