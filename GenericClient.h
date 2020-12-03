

typedef void (*PacketAvailableCallback)(uint16_t from, uint8_t type);
typedef void (*ClientEventCallback)(void);
typedef void (*ClientEventWithBoolCallback)(bool success);

bool radioInitialised = false;

class GenericClient
{
public:
  GenericClient(uint8_t to)
  {
    _to = to;
  }

  void begin(
      RF24Network *network,
      PacketAvailableCallback packetAvailableCallback)
  {
    _network = network;
    _packetAvailableCallback = packetAvailableCallback;
  }

  template <typename IN>
  IN read()
  {
    RF24NetworkHeader header;
    IN ev;
    uint8_t len = sizeof(IN);
    uint8_t buff[len];
    _network->read(header, buff, len);
    memcpy(&ev, &buff, len);
    return ev;
  }

  template <typename OUT>
  bool sendTo(uint8_t type, OUT data)
  {
    uint8_t len = sizeof(OUT);
    uint8_t bs[len];
    memcpy(bs, &data, len);
    // takes 3ms if OK, 30ms if not OK
    RF24NetworkHeader header(_to, type);
    _connected = _network->write(header, bs, len);

    if (_connectedStateChanged() && _connectionStateChangeCallback != NULL)
      _connectionStateChangeCallback();

    if (_sentEventCallback != NULL)
      _sentEventCallback(_connected);

    return _connected;
  }

  void setConnectedStateChangeCallback(ClientEventCallback cb)
  {
    _connectionStateChangeCallback = cb;
  }

  void setSentEventCallback(ClientEventWithBoolCallback cb)
  {
    _sentEventCallback = cb;
  }

  void update()
  {
    _network->update();
    if (_network->available())
    {
      RF24NetworkHeader header;
      _network->peek(header);
      if (header.from_node == _to)
      {
        _connected = true;
        _packetAvailableCallback(header.from_node, header.type);
        if (_connectedStateChanged() && _connectionStateChangeCallback != NULL)
          _connectionStateChangeCallback();
      }
    }
  }

  bool connected()
  {
    return _connected;
  }

private:
  RF24Network *_network;
  uint8_t _to;
  PacketAvailableCallback _packetAvailableCallback;
  ClientEventCallback _connectionStateChangeCallback;
  ClientEventWithBoolCallback _sentEventCallback;
  bool _connected = true, _oldConnected = false;

  bool _connectedStateChanged()
  {
    bool changed = _oldConnected != _connected;
    _oldConnected = _connected;
    return changed;
  }
};