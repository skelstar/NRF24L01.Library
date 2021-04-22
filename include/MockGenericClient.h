typedef void (*PacketAvailableCallback)(uint16_t from, uint8_t type);
typedef void (*ClientEventCallback)(void);
typedef void (*ClientEventWithBoolCallback)(bool success);

#include <RF24.h>
#include <RF24Network.h>
#include <NRF24L01Lib.h>

#ifndef RADIO_OBJECTS
#define RADIO_OBJECTS
NRF24L01Lib nrf24;

RF24 radio(NRF_CE, NRF_CS);
RF24Network network(radio);
#endif

template <typename OUT, typename IN>
class GenericClient
{
  typedef void (*ClientSentPacketCallback)(OUT out);
  typedef void (*ClientReadPacketCallback)(IN in);
  typedef IN (*MockResponseCallback)(OUT out);
  typedef bool (*MockClientIsAvailableCallback)(OUT out);

public:
  bool printWarnings = true;

public:
  GenericClient(uint8_t to)
  {
    _to = to;
    _mocked_from = (_to == COMMS_CONTROLLER)
                       ? COMMS_BOARD
                       : COMMS_CONTROLLER;
  }

  void begin(
      RF24Network *network,
      PacketAvailableCallback packetAvailableCallback,
      xSemaphoreHandle mutex = nullptr)
  {
    _network = network;
    _packetAvailableCallback = packetAvailableCallback;
    _mutex = mutex;
    _sent_id = 0;
  }

  IN read()
  {
    if (_mock_response_cb == nullptr)
    {
      if (printWarnings == true)
        Serial.printf("ERROR: _mock_response_cb not set1!\n");
      IN dummy;
      return dummy;
    }
    IN response = _mock_response_cb(_mock_sent_data);
    return response;
  }

  template <typename INALT>
  INALT readAlt()
  {
    RF24NetworkHeader header;
    INALT ev;
    uint8_t len = sizeof(INALT);
    uint8_t buff[len];

    bool taken = _mutex == nullptr || xSemaphoreTake(_mutex, (TickType_t)10);
    if (taken)
    {
      _network->read(header, buff, len);
      if (_mutex != nullptr)
        xSemaphoreGive(_mutex);
      memcpy(&ev, &buff, len);
    }
    return ev;
  }

  // template <typename OUT>
  bool sendTo(uint8_t type, OUT data)
  {
    _mock_sent_packet_type = type;
    _mock_sent_data = data;
    _since_sent = 0;
    _sent_id++;

    return true;
  }

  template <typename OUTALT>
  bool sendAltTo(uint8_t type, OUTALT data)
  {
    uint8_t len = sizeof(OUTALT);
    uint8_t bs[len];
    memcpy(bs, &data, len);
    // takes 3ms if OK, 30ms if not OK
    RF24NetworkHeader header(_to, type);

    bool taken = _mutex == nullptr || xSemaphoreTake(_mutex, (TickType_t)10);

    if (taken)
    {
      _connected = _network->write(header, bs, len);
      if (_mutex != nullptr)
        xSemaphoreGive(_mutex);
    }
    else
      return false;

    if (_connectedStateChanged() && _connectionStateChangeCallback != nullptr)
      _connectionStateChangeCallback();

    if (_sentEventCallback != nullptr)
      _sentEventCallback(_connected);

    return _connected && taken;
  }

  void setSentPacketCallback(ClientSentPacketCallback cb)
  {
    _sentPacketCallback = cb;
  }

  void setReadPacketCallback(ClientReadPacketCallback cb)
  {
    _readPacketCallback = cb;
  }

  void setConnectedStateChangeCallback(ClientEventCallback cb)
  {
    _connectionStateChangeCallback = cb;
  }

  void setSentEventCallback(ClientEventWithBoolCallback cb)
  {
    _sentEventCallback = cb;
  }

  // Mock methods
  void mockResponseDelay(unsigned long response_delay)
  {
    _receiver_response_delay = response_delay;
  }

  void mockResponseCallback(MockResponseCallback response_cb)
  {
    _mock_response_cb = response_cb;
  }

  void mockClientAvailableCallback(MockClientIsAvailableCallback available_cb)
  {
    _mock_client_is_available_cb = available_cb;
  }

  // - check for a new (or unresponded to) packet
  // - if delay==0 then send response
  // - otherwise if there is a delay and delay is over, send the response
  bool update()
  {
    if (_mock_client_is_available_cb != nullptr &&
        !_mock_client_is_available_cb(_mock_sent_data))
    {
      // mocked target device is not available
      DEBUG("WARNING: _mock_client_is_available_cb has not been set!");
      return false;
    }

    // mock the delay before board would respond
    if (_receiver_response_delay = 0 || (_since_sent > _receiver_response_delay))
    {
      if (_last_replied_to_id != _sent_id)
      {
        _packetAvailableCallback(/*from*/ COMMS_BOARD, /*type*/ Packet::CONTROL);
      }
      _last_replied_to_id = _sent_id;
    }

    return true;
  }

  bool ready()
  {
    return //_mock_client_is_available_cb != nullptr &&
        _mock_response_cb != nullptr &&
        true;
  }

  bool connected()
  {
    return _connected;
  }

private:
  RF24Network *_network;
  uint8_t _to, _mocked_from;
  SemaphoreHandle_t _mutex;
  PacketAvailableCallback _packetAvailableCallback = nullptr;
  ClientEventCallback _connectionStateChangeCallback = nullptr;
  ClientEventWithBoolCallback _sentEventCallback = nullptr;
  ClientSentPacketCallback _sentPacketCallback = nullptr;
  ClientReadPacketCallback _readPacketCallback = nullptr;
  unsigned long _receiver_response_delay = 0,
                _last_replied_to_id = 100,
                _sent_id = 0;
  elapsedMillis _since_sent;

  uint8_t _mock_sent_packet_type;
  OUT _mock_sent_data;
  MockResponseCallback _mock_response_cb = nullptr;
  MockClientIsAvailableCallback _mock_client_is_available_cb = nullptr;

  bool _connected = true, _oldConnected = false;

  bool _connectedStateChanged()
  {
    bool changed = _oldConnected != _connected;
    _oldConnected = _connected;
    return changed;
  }
};