#include "Arduino.h"
#include "NRF24L01Lib.h"

//--------------------------------------------------------------------------------

NRF24L01Lib::NRF24L01Lib() {}

//--------------------------------------------------------------------------------
void NRF24L01Lib::begin(
		RF24 *radio,
		RF24Network *network,
		uint16_t address,
		PacketAvailableCallback packetAvailableCallback)
{
	_radio = radio;
	_network = network;
	_packetAvailableCallback = packetAvailableCallback;

	_radio->begin();
	_radio->setPALevel(RF24_PA_MAX);
	_radio->setDataRate(RF24_250KBPS);

	_network->begin(address);
	_network->multicastLevel(address);

	_radio->flush_rx();
	_radio->flush_tx();

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
uint8_t NRF24L01Lib::send_with_retries(uint16_t to, uint8_t type, uint8_t *data, uint8_t data_len, uint8_t num_retries)
{
  uint8_t success, retries = 0;
	bool finished;
  do
  {
    success = send_packet(to, type, data, data_len);
    if (success == false)
    {
      vTaskDelay(1);
    }
    finished = success || retries++ == num_retries;
  } while (!finished);

  return retries == num_retries - 1;
}
//---------------------------------------------------------------------------------
bool NRF24L01Lib::send_packet(uint16_t to, uint8_t type, uint8_t *data, uint8_t data_len)
{
	RF24NetworkHeader header(to, type);
	return _network->write(header, data, data_len);
}
//---------------------------------------------------------------------------------
