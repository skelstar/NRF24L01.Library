#define MOCK_GENERIC_CLIENT

#include <RF24.h>
#include <RF24Network.h>

typedef void (*PacketAvailableCallback)(uint16_t from, uint8_t type);
typedef void (*ClientEventCallback)(void);
typedef void (*ClientEventWithBoolCallback)(bool success);

template <typename OUT, typename IN>
class GenericClient
{
  typedef void (*ClientSentPacketCallback)(OUT out);
  typedef void (*ClientReadPacketCallback)(IN in);
  typedef IN (*MockResponseCallback)(OUT out);

public:
  GenericClient(uint8_t to)
  {
    Serial.printf("[MOCK] GenericClient\n");
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
  }

  IN read()
  {
    if (_mock_response_cb == nullptr)
      Serial.printf("ERROR: _mock_response_cb not set!\n");
    return _mock_response_cb(_mock_sent_data);
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
    else
      Serial.printf("ERROR: Generic client unable to take mutex (readAlt)\n");
    return ev;
  }

  bool sendTo(uint8_t type, OUT data)
  {
    _mock_sent_packet_type = type;
    _mock_sent_data = data;
    _since_sent = 0;

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
  void mockResponseTime(unsigned long response_time)
  {
    _receiver_response_time = response_time;
  }

  void mockResponseCallback(MockResponseCallback response_cb)
  {
    _mock_response_cb = response_cb;
  }

  bool update()
  {
    if (_receiver_response_time = 0 || _since_sent > _receiver_response_time)
      _packetAvailableCallback(0, Packet::CONTROL);

    return true;
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
  unsigned long _receiver_response_time = 0;
  elapsedMillis _since_sent;

  uint8_t _mock_sent_packet_type;
  OUT _mock_sent_data;
  MockResponseCallback _mock_response_cb = nullptr;

  bool _connected = true, _oldConnected = false;

  bool _connectedStateChanged()
  {
    bool changed = _oldConnected != _connected;
    _oldConnected = _connected;
    return changed;
  }
};