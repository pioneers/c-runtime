/**
 * Hotplugging
 * Verifies that a ForeignTestDevice does not get connected to shared memory.
 * A ForeignTestDevice does not send an ACK, and sends only random bytes
 */
#include "../test.h"

#define UID 0x123

#define ORDERED_STRINGS 0
#define UNORDERED_STRINGS 0

int main() {
    // Setup
    start_test("Hotplug ForeignTestDevice", "", "", ORDERED_STRINGS, UNORDERED_STRINGS);

    // Connect a ForeignTestDevice
    check_device_not_connected(UID);
    connect_virtual_device("ForeignTestDevice", UID);
    sleep(2);
    check_device_not_connected(UID);

    // Clean up
    end_test();
    return 0;
}
