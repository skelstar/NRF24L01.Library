#ifndef NRF24L01Lib_h
#define NRF24L01Lib_h

#include <Arduino.h>
#include <RF24.h>
#include <RF24Network.h>

#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

//--------------------------------------------------------------------------------

typedef void (*PacketAvailableCallback)(uint16_t from, uint8_t type);

class NRF24L01Lib
{
public:
	NRF24L01Lib();

	void begin(
			RF24 *radio,
			RF24Network *network,
			uint16_t address,
			PacketAvailableCallback packetAvailableCallback = nullptr,
			bool multicastEnable = false);
	void update();
	void read_into(uint8_t *data, uint8_t data_len);
	bool send(uint16_t to, uint8_t type, uint8_t *data, uint8_t data_len);
	bool broadcast(uint16_t to, uint8_t type, uint8_t *data, uint8_t data_len, uint8_t multicastLevel);

private:
	RF24 *_radio;
	RF24Network *_network;
	bool _multicastEnabled = false;

	PacketAvailableCallback _packetAvailableCallback = nullptr;
};

#endif
