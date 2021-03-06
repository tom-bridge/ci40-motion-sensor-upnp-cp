#include "control_point.h"

#define MEDIA_RENDERER 		"urn:schemas-upnp-org:device:MediaRenderer:1"
#define RENDERING_CONTROL 	"urn:schemas-upnp-org:service:RenderingControl"

#define FIND_ERR_NOT_FOUND	-1
#define FIND_ERR_DUPLICATE	-2

static GUPnPDeviceProxy		*aDevices[MAX_DEV_ENTRIES + 1] 	= {NULL};
static unsigned int 		ui32DeviceCount 				= 0;							
static GUPnPContextManager 	*context_manager;

int find( char* device )
{
	int	dev_cnt = ui32DeviceCount;
	int	entry 	= -1;
	int	entries	= MAX_DEV_ENTRIES;
	
	// While there are still devices
	while(dev_cnt > 0)
	{
		// If entry is an existing device
		if(aDevices[entries] != NULL)
		{		
			// Retrieve device name
			char* reg_device = gupnp_device_info_get_friendly_name(GUPNP_DEVICE_INFO(aDevices[entries]));
			
			// Compare against new device
			if(strcmp(reg_device, device) == 0)
			{
				entry = entries;
				break;
			}
			
			// Decrement the number of devices to look for
			dev_cnt--;
		}
		
		// Decrement the number of entries to examine
		entries--;
	}
	
	return entry;
}

GUPnPDeviceProxy* control_point_find_device( char* device )
{
	int entry = find(device);
	
	if(entry > FIND_ERR_NOT_FOUND)
	{
		return aDevices[entry];
	}
	else
	{
		return NULL;
	}
}

static void
dmr_proxy_available_cb (GUPnPControlPoint *cp,
                        GUPnPDeviceProxy  *proxy)
{	
    char* 				dev_name	= gupnp_device_info_get_friendly_name(GUPNP_DEVICE_INFO(proxy));
	int 				dev_cnt 	= ui32DeviceCount;
	int					entry 		= -1;
	int 				entries		= MAX_DEV_ENTRIES;
	
	// Determine how many devices have already been detected. If none, skip detection
	if(dev_cnt > 0)
	{
		// While there are still devices to compare
		while(entries >= 0)
		{
			// If entry is an existing device
			if(aDevices[entries] != NULL)
			{		
				// Retrieve device name
				GUPnPDeviceInfo* gupnp_device_info 	= GUPNP_DEVICE_INFO(aDevices[entries]);
				char* registered_device 			= gupnp_device_info_get_friendly_name(gupnp_device_info);
				
				// Compare against new device
				if(strcmp(registered_device, dev_name) == 0)
				{
					// Return error if a device of the same name is already found
					printf("Duplicate Device Detected: %s\n", dev_name);
					entry = -1;
					break;
				}
				
				// Decrement the number of devices to look for
				dev_cnt--;
			}
			else
			{
				// Set entry as the lowest available slot
				entry = dev_cnt;
			}
			
			// Decrement the number of entries to examine
			entries--;
		}
	}
	else
	{
		// If no entries exist first available slot will be 0
		entry = 0;
	}
	
	// Check an erro hasn't occured and a slot has been found
	if(entry > -1)
	{
		// Register device
		aDevices[entry] = proxy;
		printf("%s added at entry %d\n", dev_name, entry);
		ui32DeviceCount++;
	}
}

static void
dmr_proxy_unavailable_cb (GUPnPControlPoint *cp,
                          GUPnPDeviceProxy  *proxy)
{
	// Look up device to be removed
	char* 	dev_name	= gupnp_device_info_get_friendly_name(GUPNP_DEVICE_INFO(proxy));	
	int 	dev 		= find(dev_name);
	
	if( dev > FIND_ERR_NOT_FOUND )
	{
		aDevices[dev] = NULL;
		ui32DeviceCount--;
		printf("%s Removed from entry %d\n", dev_name, dev);
	}
}


static void
on_context_available (GUPnPContextManager *context_manager,
                      GUPnPContext        *context,
                      gpointer             user_data)
{
    GUPnPControlPoint *dmr_cp;

    dmr_cp = gupnp_control_point_new (context, MEDIA_RENDERER);


    g_signal_connect (dmr_cp,
                      "device-proxy-available",
                      G_CALLBACK (dmr_proxy_available_cb),
                      NULL);

    g_signal_connect (dmr_cp,
                      "device-proxy-unavailable",
                      G_CALLBACK (dmr_proxy_unavailable_cb),
                      NULL);

    gssdp_resource_browser_set_active (GSSDP_RESOURCE_BROWSER (dmr_cp),
                                       TRUE);

    /* Let context manager take care of the control point life cycle */
    gupnp_context_manager_manage_control_point (context_manager, dmr_cp);

    /* We don't need to keep our own references to the control points */
    g_object_unref (dmr_cp);
}

static void
set_volume_cb (GUPnPServiceProxy       *rendering_control,
               GUPnPServiceProxyAction *action,
               gpointer                 user_data)
{
	GError *error;

    error = NULL;
    if (!gupnp_service_proxy_end_action (rendering_control,
											action,
											&error,
											NULL)) 
	{
		const char *udn;
	
		udn = gupnp_service_info_get_udn
				(GUPNP_SERVICE_INFO (rendering_control));
	
		g_warning ("Action Failed: %s: %s",
				udn,
				error->message);
	
		g_error_free (error);
    }

	g_object_unref (rendering_control);
}

static GUPnPServiceProxy *
get_rendering_control (GUPnPDeviceProxy *proxy)
{
    GUPnPDeviceInfo  *info;
    GUPnPServiceInfo *rendering_control;
	
    info = GUPNP_DEVICE_INFO (proxy);
    
    rendering_control = gupnp_device_info_get_service (info,
                                                       RENDERING_CONTROL);
    return GUPNP_SERVICE_PROXY (rendering_control);
}

char* control_point_get_device_name(GUPnPDeviceProxy* device)
{
	// Retrieve device name
	GUPnPDeviceInfo* gupnp_device_info 	= GUPNP_DEVICE_INFO(device);
	return gupnp_device_info_get_friendly_name(gupnp_device_info);
}

void control_point_list_devices(GUPnPDeviceProxy** output_array, int* count)
{
	int	dev_cnt = ui32DeviceCount;
	int	entries	= MAX_DEV_ENTRIES;
	
	// While there are still devices
	while(dev_cnt > 0)
	{
		// If entry is an existing device
		if(aDevices[entries] != NULL)
		{	
			// Decrement the number of devices to look for
			dev_cnt--;
				
			output_array[dev_cnt] = aDevices[entries];
		}
		
		// Decrement the number of entries to examine
		entries--;
	}
	
	*count = ui32DeviceCount;
}

int control_point_set_volume(char* device, int volume)
{
	GUPnPDeviceProxy* cp = control_point_find_device( device );
				
	// If the device has been found
	if(cp)
	{	
		guint desired_volume = volume;

		// Issue UPNP command
		gupnp_service_proxy_begin_action (get_rendering_control(cp),
								"SetVolume",
								set_volume_cb,
								NULL,
								"InstanceID",
								G_TYPE_UINT,
								0,
								"Channel",
								G_TYPE_STRING,
								"Master",
								"DesiredVolume",
								G_TYPE_UINT,
								desired_volume,
								NULL);	
		return 1;
	}
	else
	{
		return 0;
	}
}

int control_point_set_mute(char* device, int mute)
{
	GUPnPDeviceProxy* cp = control_point_find_device( device );
						
	// If the device has been found
	if(cp)	
	{
		gupnp_service_proxy_begin_action (
								get_rendering_control(cp),
								"SetMute",
								set_volume_cb,
								NULL,
								"InstanceID",
								G_TYPE_UINT,
								0,
								"Channel",
								G_TYPE_STRING,
								"Master",
								"DesiredMute",
								G_TYPE_UINT,
								mute,
								NULL);		
		return 1;	
	}
	else
	{
		return 0;
	}
}

void control_point_init_and_run()
{
	GMainLoop *main_loop;
	
    context_manager = gupnp_context_manager_new (NULL, 0);
    g_assert (context_manager != NULL);

    g_signal_connect (context_manager,
                      "context-available",
                      G_CALLBACK (on_context_available),
                      NULL);
                      
	/* Run the main loop */
	main_loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (main_loop);

	/* Cleanup */
	g_main_loop_unref (main_loop);
}
