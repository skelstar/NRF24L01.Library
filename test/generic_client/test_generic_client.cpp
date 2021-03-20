
#include <Arduino.h>
#include <unity.h>

#include <RF24.h>
#include <RF24Network.h>
#include <NRF24L01Lib.h>

#include <GenericClient.h>
#include <VescData.h>

NRF24L01Lib nrf24;

RF24 radio(SPI_CE, SPI_CS);
RF24Network network(radio);

GenericClient<VescData, ControllerData> testClient(00);

void packetAvailable_cb(uint16_t from_id, uint8_t t)
{
  Serial.printf("Packet available from %d\n", from_id);
}

// runs every test
void setUp()
{
  nrf24.begin(&radio, &network, 00);

  testClient.begin(&network, packetAvailable_cb);
  // testClient.setConnectedStateChangeCallback([] {
  //   if (PRINT_BOARD_CLIENT_CONNECTED_CHANGED)
  //     Serial.printf(BOARD_CLIENT_CONNECTED_FORMAT, testClient.connected() ? "CONNECTED" : "DISCONNECTED");
  // });
  // testClient.setSentPacketCallback(printSentToBoard_cb);
  // testClient.setReadPacketCallback(printRecvFromBoard_cb);
}

// runs every test
void tearDown()
{
}

void test_not_failing()
{
  float temp = 330.0;

  TEST_ASSERT_EQUAL(330.0, temp);
}

void setup()
{
  delay(2000);
  UNITY_BEGIN();

  RUN_TEST(test_not_failing);

  UNITY_END();
}

void loop()
{
}
