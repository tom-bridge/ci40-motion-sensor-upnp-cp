#include <libgupnp/gupnp-control-point.h>
#include <libgupnp-av/gupnp-av.h>

#define MAX_DEV_ENTRIES 	99

void control_point_init_and_run();

void control_point_list_devices(GUPnPDeviceProxy** output_array, int* count);

char* control_point_get_device_name(GUPnPDeviceProxy* device);

GUPnPDeviceProxy* control_point_find_device( char* device );

int control_point_set_volume(char* device, int volume);

int control_point_set_mute(char* device, int mute);
