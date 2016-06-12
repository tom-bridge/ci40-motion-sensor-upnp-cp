/***************************************************************************************************
 * Copyright (c) 2015, Imagination Technologies Limited
 * All rights reserved.
 *
 * Redistribution and use of the Software in source and binary forms, with or
 * without modification, are permitted provided that the following conditions are met:
 *
 *     1. The Software (including after any modifications that you make to it) must
 *        support the FlowCloud Web Service API provided by Licensor and accessible
 *        at  http://ws-uat.flowworld.com and/or some other location(s) that we specify.
 *
 *     2. Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *
 *     3. Redistributions in binary form must reproduce the above copyright notice, this
 *        list of conditions and the following disclaimer in the documentation and/or
 *        other materials provided with the distribution.
 *
 *     4. Neither the name of the copyright holder nor the names of its contributors may
 *        be used to endorse or promote products derived from this Software without
 *        specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 **************************************************************************************************/

/**
 * @file flow_button_gateway.c
 * @brief Flow button gateway application to observe a button press on constrained device,
 *        and set the led on another. Also send a flow message to user for change in LED state.
 *        It uses FlowDeviceManagment Server SDK for communicating with lwm2m client on constrained
 *        devices and Client SDK for communicating with lwm2m server on flowcloud.
 *        NOTE: the provisioning app must have been run prior to this application.
 */

/***************************************************************************************************
 * Includes
 **************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "server_low.h"
#include "client_low.h"
#include "flow_interface.h"
#include "flow/core/flow_time.h"
#include "flow/core/flow_memalloc.h"
#include "log.h"
#include "control_point.h"
#include <pthread.h>
#include "timeout.h"

/***************************************************************************************************
 * Definitions
 **************************************************************************************************/

/** Calculate size of array. */
#define ARRAY_SIZE(x) ((sizeof x) / (sizeof *x))
/** Buffer size for receiving resource data. */
#define BUFF_SIZE			(4096)

//! @cond Doxygen_Suppress
#define IPC_SERVER_PORT			(54321)
#define IPC_CLIENT_PORT			(12345)
#define RESOURCE_INSTANCE_ID		(0)
#define FLOW_ACCESS_OBJECT_ID		(20001)
#define FLOW_OBJECT_INSTANCE_ID		(0)
#define BUTTON_OBJECT_ID	(3200)
#define BUTTON_RESOURCE_ID	(5560)
#define BUTTON_STR			"button"
#define BUTTON2_STR			"button2"
#define SPEAKER1_STR		"ewc_1"
#define SPEAKER2_STR		"ewc_2"

//! @endcond

/***************************************************************************************************
 * Typedef
 **************************************************************************************************/

/**
 * A structure to contain resource information.
 */
typedef struct
{
	/*@{*/
	ResourceIDType resourceID; /**< resource ID */
	ResourceInstanceIDType resourceInstanceID; /**< resource instance ID */
	FlowDeviceMgmtResourceType resourceType; /**< type of resource e.g. bool, string, integer etc. */
	bool doObserve; /**< should we observe on this resource? */
	const char *resourceName; /**< resource name */
	/*@}*/
}RESOURCE_T;

/**
 * A structure to contain objects information.
 */
typedef struct
{
	/*@{*/
	char *clientID; /**< client ID */
	ObjectIDType objectID; /**< object ID */
	ObjectInstanceIDType objectInstanceID; /**< object instance ID */
	const char *objectName; /**< object name */
	unsigned int numResources; /**< number of resource under this object */
	RESOURCE_T *resources; /**< resource information */
	/*@}*/
}OBJECT_T;

/***************************************************************************************************
 * Globals
 **************************************************************************************************/

/** Variable storing device registration status. */
static bool isDeviceRegistered = false;
/** Interrupt signal have been issued. */
static bool receivedSignal = false;
/** Callback function, called when button state gets updated. */
static int ButtonStateChangeCallback(FlowDeviceMgmtHandle * handle);
static int ButtonStateChangeCallback2(FlowDeviceMgmtHandle * handle);
/** Set default debug level to info. */
int debug_level = LOG_INFO;
static TIMEOUT_S stimeout[2];
static int devices = 0;

/** Initializing objects. */
static OBJECT_T objects[] =
{
	{
		"ButtonDevice",
		BUTTON_OBJECT_ID,
		0,
		"Digital Input",
		1,
		(RESOURCE_T []){
							{
								BUTTON_RESOURCE_ID,
								0,
								FlowDeviceMgmtResourceType_TypeBoolean,
								true,
								BUTTON_STR
							},
						}
	},
	{
		"ButtonDevice2",
		BUTTON_OBJECT_ID,
		1,
		"Digital Input",
		1,
		(RESOURCE_T []){
							{
								BUTTON_RESOURCE_ID,
								0,
								FlowDeviceMgmtResourceType_TypeBoolean,
								true,
								BUTTON2_STR
							},
						}
	},
};

/***************************************************************************************************
 * Implementation
 **************************************************************************************************/

/**
 * @brief Signal interrupt handler.
 * @param sig signal ID.
 */
static void INThandler(int sig)
{
	receivedSignal = true;
	signal(sig, SIG_IGN);
}

/**
 * @brief Cancel observing all observed resources.
 */
static void CancelObserve(void)
{
	unsigned int i, j;

	for (i = 0; i < devices; i++)
	{
		for (j = 0; j < objects[i].numResources; j++)
		{
			if (objects[i].resources[j].doObserve)
			{
				FlowDeviceMgmtKey key = { {0} };

				key = FlowDeviceMgmtServer_ToResourceKey(objects[i].clientID,
															objects[i].objectID,
															objects[i].objectInstanceID,
															objects[i].resources[j].resourceID);

				if (FlowDeviceMgmtServer_CancelObserve(key) != 0)
				{
					FlowDeviceMgmtServer_PError("FlowDeviceMgmtServer_CancelObserve failed");
				}
			}
		}
	}
}

/**
 * @brief Start observing a resource.
 *        Register a callback function, which gets called on resource value change.
 */
static void StartObserving(void)
{
	unsigned int i, j;

	for (i = 0; i < devices; i++)
	{
		for (j = 0; j < objects[i].numResources; j++)
		{
			if (objects[i].resources[j].doObserve)
			{
				FlowDeviceMgmtKey key = { {0} };

				FlowDeviceMgmt_Process(1 /*second*/);

				printf("Pull registration %d %d\n", i,objects[i].objectInstanceID);

				if (FlowDeviceMgmtServer_PullRegistration(objects[i].objectID))
				{
					FlowDeviceMgmt_PError("FlowDeviceMgmtServer_PullRegistration failed");
					return;
				}

				key = FlowDeviceMgmtServer_ToResourceKey(objects[i].clientID,
															objects[i].objectID,
															objects[i].objectInstanceID,
															objects[i].resources[j].resourceID);

				if (FlowDeviceMgmtServer_Observe(key, ((i)?ButtonStateChangeCallback2:ButtonStateChangeCallback)))
				{
					FlowDeviceMgmtServer_PError("FlowDeviceMgmtServer_Observe failed");
				}
			}
		}
	}
	
	// catch CTRL-C to ensure clean-up
	signal(SIGINT, INThandler);

	while(receivedSignal == false)
	{
		FlowDeviceMgmt_Process(1 /*second*/);
	}

	CancelObserve();
}

/**
 * @brief Register all objects and its resources with client daemon.
 * @return true if object is successfully registered on client, else false.
 */
static bool RegisterObjectsAsClient(void)
{
	bool success = true;
	unsigned int i, j;
	FlowDeviceMgmtFlags flags = FlowDeviceMgmt_ToFlags(FlowDeviceMgmtOperations_RW,
														FlowDeviceMgmtMandatory_Mandatory);
	/* Register objects */
	for (i = 0; (i < ARRAY_SIZE(objects)) && success; i++)
	{
		/* Check if object is registered or not */
		if (FlowDeviceMgmt_PullRegistration(objects[i].objectID) != 0)
		{
			/* Defining object */
			if (FlowDeviceMgmt_RegisterObjectType(objects[i].objectName,
													objects[i].objectID,
													true,
													flags) == -1)
			{
				FlowDeviceMgmt_PError("Registering object failed with");
				success = false;
			}
			else
			{
				/* Object defined successfully. Now define all its resources */
				for (j = 0; j < objects[i].numResources; j++)
				{
					if (FlowDeviceMgmt_RegisterResourceType(objects[i].resources[j].resourceName,
															objects[i].objectID,
															objects[i].resources[j].resourceID,
															objects[i].resources[j].resourceType,
															true,
															flags))
					{
						FlowDeviceMgmt_PError("Registering resource failed with");
						success = false;
					}
				}
			}

			/* Register objects and all its resources */
			if (success)
			{
				if (FlowDeviceMgmt_PushRegistration(objects[i].objectID))
				{
					FlowDeviceMgmt_PError("FlowDeviceMgmt_PushRegistration() failed");
					success = false;
				}
			}
		}
	}
	return success;
}

/**
 * @brief Register all objects and its resources with server deamon.
 * @return true if object is successfully registered on server, else false.
 */
static bool RegisterObjectsAsServer(void)
{
	bool success = true;
	unsigned int i, j;
	FlowDeviceMgmtFlags flags = {0};

	for (i = 0; (i < ARRAY_SIZE(objects)) && success; i++)
	{
		/* Check if object is registered or not */
		if (FlowDeviceMgmtServer_PullRegistration(objects[i].objectID) != 0)
		{
			/* Defining object */
			if (FlowDeviceMgmtServer_RegisterObjectType(objects[i].objectName,
														objects[i].objectID,
														true,
														flags) == -1)
			{
				FlowDeviceMgmtServer_PError("Registering object failed with");
				success = false;
			}
			else
			{
				/* Object defined successfully. Now define all its resources */
				for (j = 0; j < objects[i].numResources; j++)
				{
					if (FlowDeviceMgmtServer_RegisterResourceType(
															objects[i].resources[j].resourceName,
															objects[i].objectID,
															objects[i].resources[j].resourceID,
															objects[i].resources[j].resourceType,
															true,
															flags))
					{
						FlowDeviceMgmtServer_PError("Registering resource failed with");
						success = false;
					}
				}
			}

			/* Register objects and all its resources */
			if (success)
			{
				if (FlowDeviceMgmtServer_PushRegistration(objects[i].objectID))
				{
					FlowDeviceMgmtServer_PError("FlowDeviceMgmtServer_PushRegistration() failed");
					success = false;
				}
			}
		}
	}
	return success;
}

/**
 * @brief Callback function, called when button state gets updated.
 */
static int ButtonStateChangeCallback(FlowDeviceMgmtHandle * handle)
{
	unsigned int i, j;
	const int bufferLen = BUFF_SIZE;
	bool buttonState = false, ledState = false;
	void *buffer = malloc(bufferLen);
	FlowDeviceMgmtValue ledResourceValue = { 0 };

	printf("MOtion Call back called\n");

	FlowDeviceMgmtValue buttonResourceValue = FlowDeviceMgmtServer_ValueBuffer(buffer,
																				0,
																				bufferLen);

	// perform the GET operation
	if (FlowDeviceMgmtServer_GetValue(handle, &buttonResourceValue) != 0)
	{
		FlowDeviceMgmt_PError("FlowDeviceMgmt_GetValue() failed");
		free(buffer);
		return -1;
	}

	buttonState = FlowDeviceMgmtServer_ExtractBoolean(buttonResourceValue);

	if(!buttonState)
	{
		printf("Detected 	- Count Down Disabled\n");
		stimeout[0].b_elapsed = 1;
		control_point_set_mute(SPEAKER1_STR, 0);	
	}
	else
	{
		printf("No Acitivty	- Count Down Resuming\n");
		timeout_reset(&stimeout[0]);
	}


	return 0;
}

/**
 * @brief Callback function, called when button state gets updated.
 */
static int ButtonStateChangeCallback2(FlowDeviceMgmtHandle * handle)
{
	unsigned int i, j;
	const int bufferLen = BUFF_SIZE;
	bool buttonState = false, ledState = false;
	void *buffer = malloc(bufferLen);
	FlowDeviceMgmtValue ledResourceValue = { 0 };

	FlowDeviceMgmtValue buttonResourceValue = FlowDeviceMgmtServer_ValueBuffer(buffer,
																				0,
																				bufferLen);
	// perform the GET operation
	if (FlowDeviceMgmtServer_GetValue(handle, &buttonResourceValue) != 0)
	{
		FlowDeviceMgmt_PError("FlowDeviceMgmt_GetValue() failed");
		free(buffer);
		return -1;
	}

	buttonState = FlowDeviceMgmtServer_ExtractBoolean(buttonResourceValue);

	if(!buttonState)
	{
		printf("Detected 	- Count Down Disabled\n");
		stimeout[1].b_elapsed = 1;
		control_point_set_mute(SPEAKER2_STR, 0);	
	}
	else
	{
		printf("No Acitivty	- Count Down Resuming\n");
		timeout_reset(&stimeout[1]);
	}

	free(buffer);

	return 0;
}

/**
 * @brief Checks whether flow access object is registerd or not,
 *        which shows the privisioning status of device.
 */
static void WaitForProvisioning(void)
{
	LOG(LOG_INFO, "Wait until device is provisioned");

	while (FlowDeviceMgmt_PullRegistration(FLOW_ACCESS_OBJECT_ID) != 0)
	{
		FlowDeviceMgmt_Process(1 /*second*/);
	}
	LOG(LOG_INFO, "Device is provisioned");
}

/**
 * @brief Check to see if a constrained device by the name CLIENT_ID has registered
 *        itself with the server on the gateway or not.
 * @param *endPointName pointer to client name.
 * @return true if constrained device is in client list i.e. registered, else false.
 */
static bool CheckConstrainedRegistered(const char *endPointName)
{
	int i;

	FlowDeviceMgmtClientList * clientList = FlowDeviceMgmtServer_GetClientList();
	if (clientList == NULL)
	{
		return false;
	}

	for (i = 0; i < clientList->NumClients; i++)
	{
		if (!strcmp(endPointName, clientList->Client[i].ClientID))
		{
			LOG(LOG_INFO, "Constrained device %s registered", endPointName);
			FlowDeviceMgmtServer_FreeClientList(clientList);
			return true;
		}
	}

	FlowDeviceMgmtServer_FreeClientList(clientList);

	return false;
}

int cb(void* stimeout)
{
	printf("Time has elapsed!\n");
	control_point_set_mute(SPEAKER1_STR, 1);
	return 0;
}

int cb2(void* stimeout)
{
	printf("Time has elapsed!\n");
	control_point_set_mute(SPEAKER2_STR, 1);
	return 0;
}

/**
 * @brief Flow button gateway application to observe a button press on constrained device,
 *        and set the led on another. Also send a flow message to user for change in LED state.
 *        NOTE: the provisioning app must have been run prior to this application.
 */
int main(int argc, char ** argv)
{
	int i;
	pthread_t control_point_thread;

	LOG(LOG_INFO, "Flow Control Application");
	LOG(LOG_INFO, "------------------------\n");

	if (FlowDeviceMgmt_Initialise(IPC_CLIENT_PORT))
	{
		FlowDeviceMgmt_PError("FlowDeviceMgmt_Initialise() failed");
		return -1;
	}
    
	if (FlowDeviceMgmtServer_Initialise(IPC_SERVER_PORT))
	{
		FlowDeviceMgmt_PError("FlowDeviceMgmtServer_Initialise() failed");
		return -1;
	}

	//WaitForProvisioning();

	//isDeviceRegistered = InitializeAndRegisterFlowDevice();

	if (RegisterObjectsAsServer() && RegisterObjectsAsClient())
	{
		unsigned int poll = 10;
		
		while(1)
		{
			if(devices < ARRAY_SIZE(objects))
			{
				int res = CheckConstrainedRegistered(objects[devices].clientID);
				
				if(res)
				{
					printf("Device found\n");
					
					// If fisrt device call cb otherwise cb2. TEMPORARY
					stimeout[devices].elapsed_cb 	= (devices)? cb2 : cb;
					stimeout[devices].sec_timeout 	= 10;
		
					if(devices == 0)
					{
						pthread_create( &control_point_thread, NULL, ((void *)control_point_init_and_run), NULL);
					}

					timeout_init_and_run(&stimeout[devices]);
					
					devices++;
					poll = 30;				
				}
				else
				{
					if(poll == 0)
					{
						break;
					}
					else
					{
						printf("Waiting ...\n");
						FlowDeviceMgmt_Process(1 /*second*/);
						poll--;
					}		
				}	
			}
			else
			{
				break;
			}
		}
		
		if(devices == 0)
		{
			printf("Zero Devices registered");
			
			return -1;
		}
		
		printf("Begin Observing\n");
		StartObserving();
	}
	
	return -1;
}
