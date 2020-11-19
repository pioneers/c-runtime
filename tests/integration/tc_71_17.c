#include "../test.h"

/**
 * This test ensures that received device data on UDP from the
 * perspective of Dawn is as expected when devices are connected
 * and disocnnected by the student.
 */

#define UID1 0x1234
#define UID2 0x2341
#define UID3 0x3412
#define UID4 0x4123

// We don't check the contents of the arriving data, we only check that
// the devices that we expect to be present are coming in.

int main() {
    int port1, port2, port3;

    // setup
    start_test("receive device data, general", "", "", NO_REGEX);

    // poke the system
    // send gamepad state so net_handler starts sending device data packets
    uint32_t buttons = 0;
    float joystick_vals[] = {0.0, 0.0, 0.0, 0.0};
    send_gamepad_state(buttons, joystick_vals);
    sleep(1);
    print_next_dev_data();

    // connect two devices
    port1 = connect_virtual_device("SimpleTestDevice", UID1);
    sleep(1);
    check_device_connected(UID1);

    port2 = connect_virtual_device("SimpleTestDevice", UID2);
    sleep(1);
    check_device_connected(UID2);

    // print device data
    sleep(1);
    print_next_dev_data();
    // check in device data that those are the two devices we see
    add_ordered_string_output("Device No. 0:\ttype = SimpleTestDevice, uid = 4660, itype = 62\n");
    add_ordered_string_output("Device No. 1:\ttype = SimpleTestDevice, uid = 9025, itype = 62\n");

    // disconnect first device (Port1=Empty, Port2=UID2)
    disconnect_virtual_device(port1);
    sleep(1);
    check_device_not_connected(UID1);

    // print device data
    sleep(1);
    print_next_dev_data();
    // check in device data that second device only is reported
    add_ordered_string_output("Device No. 0:\ttype = SimpleTestDevice, uid = 9025, itype = 62\n");

    // connect two more devices (Port1=UID3, Port2=UID2, Port3=UID4)
    port1 = connect_virtual_device("SimpleTestDevice", UID3);
    sleep(1);
    check_device_connected(UID3);
    port3 = connect_virtual_device("SimpleTestDevice", UID4);
    sleep(1);
    check_device_connected(UID4);

    // print device data
    sleep(1);
    print_next_dev_data();
    // check that we see three devices in output
    add_ordered_string_output("Device No. 0:\ttype = SimpleTestDevice, uid = 13330, itype = 62\n");
    add_ordered_string_output("Device No. 1:\ttype = SimpleTestDevice, uid = 9025, itype = 62\n");
    add_ordered_string_output("Device No. 2:\ttype = SimpleTestDevice, uid = 16675, itype = 62\n");
    sleep(1);

    // disconnect all devices
    disconnect_virtual_device(port1);
    sleep(1);
    check_device_not_connected(UID3);

    disconnect_virtual_device(port2);
    sleep(1);
    check_device_not_connected(UID2);

    disconnect_virtual_device(port3);
    sleep(1);
    check_device_not_connected(UID4);

    // print device data
    sleep(1);
    print_next_dev_data();
    // check that last device data has only the custom data device
    char check_15_output[] =
        "Device No. 0:\ttype = CustomData, uid = 2020, itype = 32\n"
        "\tParams:\n"
        "\t\tparam \"time_ms\" has type INT with value";
    add_ordered_string_output(check_15_output);

    // stop the system and check the output strings
    end_test();
    return 0;
}
