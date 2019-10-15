#include "Arduino.h"
#include "NRF24L01Library.h"

//--------------------------------------------------------------------------------

NRF24L01Lib::NRF24L01Lib() {}

//--------------------------------------------------------------------------------
void NRF24L01Lib::begin(
	RF24 *radio, 
	RF24Network *network, 
	Role role, 
	PacketAvailableCallback packetAvailableCallback) {	

	byte pipes[][6] = { "1Node", "2Node" };              // Radio pipe addresses for the 2 nodes to communicate.

	_role = role;
	_radio = radio;
	_network = network;
	_packetAvailableCallback = packetAvailableCallback;

	_radio->begin();
	_radio->setPALevel(RF24_PA_MAX);
	_radio->setDataRate(RF24_1MBPS);

	switch (_role) {
		case RF24_SERVER:
			_network->begin(/*channel*/ 100, /*node address*/ 00 );
			_network->multicastLevel(0);
			break;
		case RF24_CLIENT:
			_network->begin(/*channel*/ 100, /*node address*/ 01 );
			_network->multicastLevel(1);
			break;
	}

	_radio->printDetails();                   // Dump the configuration of the rf unit for debugging
}
//---------------------------------------------------------------------------------
void NRF24L01Lib::update() {

	_network->update();
	if ( _network->available() ) {
		uint16_t from = readPacket();
		_packetAvailableCallback(from);
	}
}
//---------------------------------------------------------------------------------
bool NRF24L01Lib::sendPacket(uint16_t to) {

	RF24NetworkHeader header( to );
	if (_role == RF24_CLIENT) {
		return _network->write(header, &controllerPacket, sizeof(controllerPacket));
	}
	else if (_role == RF24_SERVER) {
		return _network->write(header, &boardPacket, sizeof(boardPacket));
	}
	else {
		Serial.printf("_role not handled: %d\n", _role);
		return false;
	}
}
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//																		PRIVATE
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
uint16_t NRF24L01Lib::readPacket() {

	RF24NetworkHeader header;                       
	_network->peek(header);

	if ( header.from_node == RF24_CLIENT ) {
		 _network->read(header, &controllerPacket, sizeof(controllerPacket));
	}
	else if ( header.from_node == RF24_SERVER ) {
		_network->read(header, &boardPacket, sizeof(boardPacket));
	}
	else {
		Serial.printf("ERROR CONDITION!!! readPacket (from_node: '%d') (23) \n", header.from_node);
	}
	return header.from_node;
}
//---------------------------------------------------------------------------------
