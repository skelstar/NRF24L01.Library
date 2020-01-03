#ifndef NRF24L01Lib_h
#define NRF24L01Lib_h

#include <Arduino.h>
#include <RF24.h>
#include <RF24Network.h>
#include <VescData.h>

#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

//--------------------------------------------------------------------------------

typedef void (*PacketAvailableCallback)(uint16_t from);

class NRF24L01Lib
{
	public:
		enum Role
		{
			RF24_SERVER = 0,
			RF24_CLIENT = 1
		};

		NRF24L01Lib();

		void begin(
				RF24 *radio,
				RF24Network *network,
				Role role,
				PacketAvailableCallback packetAvailableCallback);
		void update();
		bool sendPacket(uint16_t to);

		VescData boardPacket;
		ControllerData controllerPacket;

	private:
		RF24 *_radio;
		RF24Network *_network;
		Role _role;
		PacketAvailableCallback _packetAvailableCallback;

		uint16_t readPacket();
};

#endif
