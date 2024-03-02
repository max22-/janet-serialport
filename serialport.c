#include <janet.h>
#include <libserialport.h>

static Janet list_ports(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 0);
    struct sp_port **port_list;
    enum sp_return result = sp_list_ports(&port_list);
    if(result != SP_OK)
        return janet_wrap_array(janet_array(0));
    int port_count = 0;
    for(int i = 0; port_list[i] != NULL; i++)
        port_count++;
    JanetArray *ports_array = janet_array(port_count);
    for(int i = 0; port_list[i] != NULL; i++) {
        struct sp_port *port = port_list[i];
        char *port_name = sp_get_port_name(port);
        janet_array_push(ports_array, janet_cstringv(port_name));
    }
    return janet_wrap_array(ports_array);
}

static Janet port_info(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    if(!janet_checktype(argv[0], JANET_STRING))
        janet_panic("expected a string for the port name");
    const char *port_name = janet_unwrap_string(argv[0]);
    struct sp_port *port;
    enum sp_return result = sp_get_port_by_name(port_name, &port);
    if(result != SP_OK)
        janet_panicf("port \"%s\" not found", port_name);
    JanetTable *port_info_table = janet_table(4);
    janet_table_put(port_info_table, janet_ckeywordv("name"), janet_cstringv(port_name));
    const char *description = sp_get_port_description(port);
    janet_table_put(port_info_table, janet_ckeywordv("description"), janet_cstringv(description));
    enum sp_transport transport = sp_get_port_transport(port);
    if(transport == SP_TRANSPORT_NATIVE)
        janet_table_put(port_info_table, janet_ckeywordv("transport"), janet_ckeywordv("native"));
    else if(transport == SP_TRANSPORT_USB) {
        janet_table_put(port_info_table, janet_ckeywordv("transport"), janet_ckeywordv("usb"));
        JanetTable *usb_info_table = janet_table(7);
        const char *manufacturer = sp_get_port_usb_manufacturer(port);
        janet_table_put(usb_info_table, janet_ckeywordv("manufacturer"), janet_cstringv(manufacturer));
        const char *product = sp_get_port_usb_product(port);
        janet_table_put(usb_info_table, janet_ckeywordv("product"), janet_cstringv(product));
        const char *serial = sp_get_port_usb_serial(port);
        janet_table_put(usb_info_table, janet_ckeywordv("serial"), janet_cstringv(serial));
        int usb_vid, usb_pid;
        sp_get_port_usb_vid_pid(port, &usb_vid, &usb_pid);
        char buffer[5] = {0};
        sprintf(buffer, "%04x", usb_vid);
        janet_table_put(usb_info_table, janet_ckeywordv("vid"), janet_cstringv(buffer));
        sprintf(buffer, "%04x", usb_pid);
        janet_table_put(usb_info_table, janet_ckeywordv("pid"), janet_cstringv(buffer));
        int usb_bus, usb_address;
        sp_get_port_usb_bus_address(port, &usb_bus, &usb_address);
        janet_table_put(usb_info_table, janet_ckeywordv("usb-bus"), janet_wrap_integer(usb_bus));
        janet_table_put(usb_info_table, janet_ckeywordv("usb-address"), janet_wrap_integer(usb_address));
        janet_table_put(port_info_table, janet_ckeywordv("usb-info"), janet_wrap_table(usb_info_table));
    } else if(transport == SP_TRANSPORT_BLUETOOTH) {
        janet_table_put(port_info_table, janet_ckeywordv("transport"), janet_ckeywordv("bluetooth"));
        JanetTable *bluetooth_info_table = janet_table(1);
        const char *mac_address = sp_get_port_bluetooth_address(port);
        janet_table_put(bluetooth_info_table, janet_ckeywordv("mac"), janet_cstringv(mac_address));
        janet_table_put(port_info_table, janet_ckeywordv("bluetooth-info"), janet_wrap_table(bluetooth_info_table));
    }
    sp_free_port(port);
    return janet_wrap_table(port_info_table);
}

static const JanetReg cfuns[] = {
    {"list-ports", list_ports, "(serialport/list-ports)\n\nReturns an array of all the serial ports on the system."},
    {"port-info", port_info, "(serialport/port-info port-name)\n\nReturns a table of informations about a serial port."},
    {NULL, NULL, NULL}
};

JANET_MODULE_ENTRY(JanetTable *env) {
    janet_cfuns(env, "serialport", cfuns);
}