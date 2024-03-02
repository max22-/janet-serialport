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

static const JanetReg cfuns[] = {
    {"list-ports", list_ports, "(serialport/list-ports)\n\nReturns an array of all the serial ports on the system."},
    {NULL, NULL, NULL}
};

JANET_MODULE_ENTRY(JanetTable *env) {
    janet_cfuns(env, "serialport", cfuns);
}