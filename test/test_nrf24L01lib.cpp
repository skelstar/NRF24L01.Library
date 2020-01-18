#include <Arduino.h>
#include <unity.h>

#include <VescData.h>
#include <RF24Network.h>
#include <NRF24L01Lib.h>
  
#define SPI_CE 33
#define SPI_CS 26

#define SEND_TO_BOARD_MS  3000
#define NUM_RETRIES       2

RF24 radio(SPI_CE, SPI_CS);
RF24Network network(radio);
NRF24L01Lib nrf24;

void setUp()
{
}

void tearDown()
{
}
void board_packet_available_cb(uint16_t from_id, uint8_t type)
{
}

void test_comms_ok_with_board()
{ 
  nrf24.begin(&radio, &network, /*address*/ 0, board_packet_available_cb);

  Serial.printf("starting..");
  VescData data;
  data.reason = ReasonType::REQUESTED;

  uint8_t bs[sizeof(VescData)];
  memcpy(bs, &data, sizeof(VescData));
  Serial.printf("starting.2.");

  Serial.printf("starting.2.2.");
  uint8_t retries = nrf24.send_with_retries(/*to*/0, /*type*/0 , /*data*/bs, sizeof(VescData), /*num_retries*/5);

  Serial.printf("starting.3.");
  TEST_ASSERT_EQUAL(/*expected*/ 1, retries);
}

void setup()
{
  UNITY_BEGIN();
}

void loop()
{
  Serial.begin(115200);
  RUN_TEST(test_comms_ok_with_board);

  UNITY_END();
}
