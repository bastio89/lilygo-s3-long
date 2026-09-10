#include <stdint.h>

#include <unity.h>

#include "services/govee_protocol.h"

void test_decodes_h5074_payload(void) {
    const uint8_t payload[] = {0x00, 0xE6, 0x09, 0xBC, 0x12, 0x64, 0x02};
    services::GoveeH5074Reading reading;

    TEST_ASSERT_TRUE(services::decodeGoveeH5074(payload, sizeof(payload), reading));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.34f, reading.temperatureC);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 47.96f, reading.humidityPct);
    TEST_ASSERT_EQUAL_UINT8(100, reading.batteryPct);
}

void test_decodes_h5074_manufacturer_header_and_negative_temperature(void) {
    const uint8_t manufacturerData[] = {0x88, 0xEC, 0x00, 0x38, 0xFF,
                                        0x10, 0x27, 0x2A, 0x00};
    services::GoveeH5074Reading reading;

    TEST_ASSERT_TRUE(
        services::decodeGoveeH5074(manufacturerData, sizeof(manufacturerData), reading));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -2.0f, reading.temperatureC);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, reading.humidityPct);
    TEST_ASSERT_EQUAL_UINT8(42, reading.batteryPct);
}

void test_rejects_non_h5074_payload(void) {
    const uint8_t payload[] = {0x00, 0x03, 0x41, 0xC2, 0x64, 0x00};
    services::GoveeH5074Reading reading;

    TEST_ASSERT_FALSE(services::decodeGoveeH5074(payload, sizeof(payload), reading));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_decodes_h5074_payload);
    RUN_TEST(test_decodes_h5074_manufacturer_header_and_negative_temperature);
    RUN_TEST(test_rejects_non_h5074_payload);
    return UNITY_END();
}

void setUp(void) {}
void tearDown(void) {}