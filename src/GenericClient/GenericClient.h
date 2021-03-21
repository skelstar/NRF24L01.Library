

typedef void (*PacketAvailableCallback)(uint16_t from, uint8_t type);
typedef void (*ClientEventCallback)(void);
typedef void (*ClientEventWithBoolCallback)(bool success);

bool radioInitialised = false;

template <typename OUT, typename IN>
class GenericClient
{
  typedef void (*ClientSentPacketCallback)(OUT out);
  typedef void (*ClientReadPacketCallback)(IN in);

public:
  GenericClient(uint8_t to)
  {
    _to = to;
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
    RF24NetworkHeader header;
    IN ev;
    uint8_t len = sizeof(IN);
    uint8_t buff[len];

    bool taken = _mutex == nullptr || xSemaphoreTake(_mutex, (TickType_t)10);
    if (taken)
    {
      _network->read(header, buff, len);
      xSemaphoreGive(_mutex);
      memcpy(&ev, &buff, len);
    }
    else
      Serial.printf("ERROR: Generic client unable to take mutex (read)\n");

    if (_readPacketCallback != nullptr)
      _readPacketCallback(ev);
    return ev;
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
      xSemaphoreGive(_mutex);
      memcpy(&ev, &buff, len);
    }
    else
      Serial.printf("ERROR: Generic client unable to take mutex (readAlt)\n");
    return ev;
  }

  // template <typename OUT>
  bool sendTo(uint8_t type, OUT data)
  {
    uint8_t len = sizeof(OUT);
    uint8_t bs[len];
    memcpy(bs, &data, len);
    // takes 3ms if OK, 30ms if not OK
    RF24NetworkHeader header(_to, type);

    bool taken = _mutex == nullptr || xSemaphoreTake(_mutex, (TickType_t)10);
    if (taken)
    {
      _connected = _network->write(header, bs, len);
      xSemaphoreGive(_mutex);
    }

    if (_sentPacketCallback != nullptr)
      _sentPacketCallback(data);

    if (_connectedStateChanged() && _connectionStateChangeCallback != nullptr)
      _connectionStateChangeCallback();

    if (_sentEventCallback != nullptr)
      _sentEventCallback(_connected);

    return _connected && taken;
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
      xSemaphoreGive(_mutex);
    }

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

  bool update()
  {
    bool taken = _mutex == nullptr || xSemaphoreTake(_mutex, (TickType_t)10);

    if (taken)
    {
      _network->update();

      if (_network->available())
      {
        RF24NetworkHeader header;
        _network->peek(header);

        if (_mutex != nullptr)
          // give semaphore so _packetAvailableCallback can use it
          xSemaphoreGive(_mutex);

        if (header.from_node == _to)
        {
          _connected = true;
          _packetAvailableCallback(header.from_node, header.type);
          if (_connectedStateChanged() && _connectionStateChangeCallback != nullptr)
            _connectionStateChangeCallback();
        }
      }
      if (xSemaphoreGetMutexHolder(_mutex) != NULL)
        xSemaphoreGive(_mutex);
    }
    return taken;
  }

  bool connected()
  {
    return _connected;
  }

private:
  RF24Network *_network;
  uint8_t _to;
  SemaphoreHandle_t _mutex;
  PacketAvailableCallback _packetAvailableCallback = nullptr;
  ClientEventCallback _connectionStateChangeCallback = nullptr;
  ClientEventWithBoolCallback _sentEventCallback = nullptr;
  ClientSentPacketCallback _sentPacketCallback = nullptr;
  ClientReadPacketCallback _readPacketCallback = nullptr;

  bool _connected = true, _oldConnected = false;

  bool _connectedStateChanged()
  {
    bool changed = _oldConnected != _connected;
    _oldConnected = _connected;
    return changed;
  }
};