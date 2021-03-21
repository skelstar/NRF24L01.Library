
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

SemaphoreHandle_t mutex;

VescData data;

void packetAvailable_cb(uint16_t from_id, uint8_t t)
{
  Serial.printf("Packet available from %d\n", from_id);
}

// runs every test
void setUp()
{
#define PRINT_NRF24L01_DETAILS 0
  nrf24.begin(&radio, &network, 1);

  mutex = xSemaphoreCreateMutex();

  testClient.begin(&network, packetAvailable_cb, mutex);
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

void test_send_able_to_take_free_mutex()
{
  bool success = testClient.sendTo(1, data);

  TEST_ASSERT_TRUE(success);
}

void test_send_unable_to_take_busy_mutex()
{
  xSemaphoreTake(mutex, (TickType_t)50);

  bool success = testClient.sendTo(1, data);

  TEST_ASSERT_FALSE(success);
}

void test_update_able_to_take_free_mutex()
{
  bool success = testClient.update();

  TEST_ASSERT_TRUE(success);
}

void test_update_unable_to_take_busy_mutex()
{
  xSemaphoreTake(mutex, (TickType_t)50);

  bool success = testClient.update();

  xSemaphoreGive(mutex);

  TEST_ASSERT_FALSE(success);
}

void setup()
{
  delay(2000);
  UNITY_BEGIN();

  RUN_TEST(test_send_able_to_take_free_mutex);
  RUN_TEST(test_send_unable_to_take_busy_mutex);
  RUN_TEST(test_update_able_to_take_free_mutex);
  RUN_TEST(test_update_unable_to_take_busy_mutex);

  UNITY_END();
}

void loop()
{
}
