
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "lib/sensors.h"

#include "lwm2m_core.h"
#include "lwm2m_bootstrap.h"
#include "lwm2m_registration.h"
#include "lwm2m_connectivity.h"
#include "lwm2m_security.h"
#include "lwm2m_device.h"
#include "lwm2m_object_store.h"
#include "coap_abstraction.h"
#include "lwm2m_acl.h"
#include "lwm2m_object_defs.h"
#include "lwm2m_types.h"
#include "motion-sensor.h"
#include "dev/leds.h"

#define BOOTSTRAP_SERVER_URL          "coap://[fe80::1]:15685"
#define COAP_PORT                     6000
#define ENDPOINT_NAME                 "ButtonDevice"
#define LOG_LEVEL                     DebugLevel_Debug

#define BUTTON_OBJECT_STR             "Digital Input"
#define BUTTON_RESOURCE_STR           "button"

#define BUTTON_OBJECT_ID              3200
#define BUTTON_OBJECT_INSTANCE_ID     0
#define BUTTON_RESOURCE_ID            5560
#define BUTTON_RESOURCE_INSTANCE_ID   0

static int button = 0;
PROCESS(lwm2m_button_client, "LWM2M Button Client");

AUTOSTART_PROCESSES(&lwm2m_button_client);

/*---------------------------------------------------------------------------*/
static void
register_button_object(ObjectStore *store)
{
  ObjectStore_RegisterObjectType(store, BUTTON_OBJECT_STR, BUTTON_OBJECT_ID,
    MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory);
  ObjectStore_RegisterResourceType(store, BUTTON_RESOURCE_STR,
    BUTTON_OBJECT_ID, BUTTON_RESOURCE_ID, ResourceTypeEnum_TypeBoolean,
    MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, Operations_RW);
}

/*---------------------------------------------------------------------------*/
static void
setup_button_object(ObjectStore *store)
{
  ObjectStore_SetResourceInstanceValue(store, BUTTON_OBJECT_ID,
    BUTTON_OBJECT_INSTANCE_ID, BUTTON_RESOURCE_ID, BUTTON_RESOURCE_INSTANCE_ID,
    &button, sizeof(button));
}

/*---------------------------------------------------------------------------*/
static Lwm2mContextType*
lwm2m_client_start()
{
  Lwm2m_SetOutput(stdout);
  Lwm2m_SetLogLevel(LOG_LEVEL);
  Lwm2m_PrintBanner();

  CoapInfo* coap = coap_Init("0.0.0.0", COAP_PORT, LOG_LEVEL);
  Lwm2mContextType *context = Lwm2mCore_Init(coap, ENDPOINT_NAME, NULL);

  /* Construct Object Tree */
  Lwm2m_Debug("Construct object tree\n");
  Lwm2m_RegisterObjectTypes(context->Store);
  register_button_object(context->Store);

  LWM2M_example_security(context->Store, BOOTSTRAP_SERVER_URL);
  LWM2M_device_example(context->Store);
  setup_button_object(context->Store);

  return context;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(lwm2m_button_client, ev, data)
{
  PROCESS_BEGIN();
  static struct etimer et;
  static Lwm2mContextType *context;
  int wait_time;

  context = lwm2m_client_start();

  /* Define application-specific events here. */
  while(1) {
    wait_time = Lwm2mCore_Process(context);
    etimer_set(&et, (wait_time * CLOCK_SECOND) / 100);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et) || (ev == sensors_event));
    
    if(data == &motion_sensor)
	{
		printf("Sensor active\n");
		if(motion_sensor.value(0) == 1) 
		{
			button = 1;
		}
		else
		{
		 	button = 0;
		}
		
		ObjectStore_SetResourceInstanceValue(context->Store, BUTTON_OBJECT_ID,
        BUTTON_OBJECT_INSTANCE_ID, BUTTON_RESOURCE_ID,
        BUTTON_RESOURCE_INSTANCE_ID, &button, sizeof(button));
	}
  }

  PROCESS_END();
}

