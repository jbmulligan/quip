//#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "spink.h"

//
// Retrieve TL nodemap from interface
//
// *** NOTES ***
// Each interface has a nodemap that can be retrieved in order to access
// information about the interface itself, any devices connected, or
// addressing information if applicable.
//

#ifdef HAVE_LIBSPINNAKER
int _get_spink_map(QSP_ARG_DECL  spinInterface hInterface, spinNodeMapHandle *hMap_p)
{
	spinError err = SPINNAKER_ERR_SUCCESS;

	err = spinInterfaceGetTLNodeMap(hInterface, hMap_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinInterfaceGetTLNodeMap");
		return -1;
	}
	return 0;
}

int _get_spink_node(QSP_ARG_DECL  spinNodeMapHandle hMap, const char *tag, spinNodeHandle *hdl_p)
{
	spinError err;

	err = spinNodeMapGetNode(hMap, tag, hdl_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinNodeMapGetNode");
		return -1;
	}
	return 0;
}

int _spink_node_is_available(QSP_ARG_DECL  spinNodeHandle hdl)
{
	spinError err;
	bool8_t isAvailable = False;

	err = spinNodeIsAvailable(hdl, &isAvailable);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinNodeIsAvailable");
		return 0;
	}
	if( isAvailable )
		return 1;
	return 0;
}

int _spink_node_is_readable(QSP_ARG_DECL spinNodeHandle hdl)
{
	spinError err;
	bool8_t isReadable = False;

	err = spinNodeIsReadable(hdl, &isReadable);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinNodeIsReadable");
		return 0;
	}
	if( isReadable )
		return 1;
	return 0;
}

int _spink_node_is_writable(QSP_ARG_DECL spinNodeHandle hdl)
{
	spinError err;
	bool8_t pbWritable = False;

	err = spinNodeIsWritable(hNode, &pbWritable);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinNodeIsWritable");
		return 0;
	}
	if( isWritable )
		return 1;
	return 0;
}

int _spink_get_string(QSP_ARG_DECL spinNodeHandle hdl, char *buf, size_t *len_p)
{
	spinError err;

	err = spinStringGetValue(hdl, buf, len_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinStringGetValue");
		return -1;
	}
	return 0;
}

//
// Print interface display name
//

int _get_interface_name(QSP_ARG_DECL  char *buf, size_t buflen, spinInterface hInterface)
{
	spinNodeMapHandle hNodeMapInterface = NULL;
	spinNodeHandle hInterfaceDisplayName = NULL;

	if( get_spink_map(hInterface,&hNodeMapInterface) < 0 ) return -1;

	if( get_spink_node(hNodeMapInterface,"InterfaceDisplayName",&hInterfaceDisplayName) < 0 ) return -1;

	if( ! spink_node_is_available(hInterfaceDisplayName) ) return -1;
	if( ! spink_node_is_readable(hInterfaceDisplayName) ) return -1;

	if( spink_get_string(hInterfaceDisplayName,buf,&buflen) < 0 ) return -1;
	return 0;
}

void _print_interface_name(QSP_ARG_DECL  spinNodeHandle hInterfaceDisplayName)
{
	// Print
	char buf[MAX_BUFF_LEN];
	size_t len = MAX_BUFF_LEN;

	if( ! spink_node_is_available(hInterfaceDisplayName) ) return;
	if( ! spink_node_is_readable(hInterfaceDisplayName) ) return;

	if( spink_get_string(hInterfaceDisplayName,buf,&len) < 0 ) return;

	printf("Interface Display Name:  %s\n", buf);
}

//
// Retrieve list of cameras from the interface
//
// *** NOTES ***
// Camera lists can be retrieved from an interface or the system object.
// Camera lists retrieved from an interface, such as this one, only return
// cameras attached on that specific interface whereas camera lists
// retrieved from the system will return all cameras on all interfaces.
//
// *** LATER ***
// Camera lists must be cleared manually. This must be done prior to
// releasing the system and while the camera list is still in scope.
//

int _get_spink_cam_list(QSP_ARG_DECL spinInterface hInterface, spinCameraList *hCamList_p, size_t *num_p)
{
	spinError err;

	// Create empty camera list
	err = spinCameraListCreateEmpty(hCamList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinCameraListCreateEmpty");
		return -1;
	}

	// Retrieve cameras
	err = spinInterfaceGetCameras(hInterface, *hCamList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinInterfaceGetCameras");
		return -1;
	}

	// Retrieve number of cameras
	err = spinCameraListGetSize(*hCamList_p, num_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinCameraListGetSize");
		return -1;
	}

	// Return if no cameras detected
	if( *num_p == 0 ){
		printf("\tNo devices detected.\n\n");
		return release_spink_cam_list(hCamList_p);
	}
	return 0;
}

//
// Clear and destroy camera list before losing scope
//
// *** NOTES ***
// Camera lists do not automatically clean themselves up. This must be done
// manually. The same is true of interface lists.
//

int _release_spink_cam_list(QSP_ARG_DECL  spinCameraList *hCamList_p )
{
	spinError err;

DEBUG_MSG(release_spin_cam_list BEGIN)
	if( *hCamList_p == NULL ){
		fprintf(stderr,"release_spink_cam_list:  null list!?\n");
		return -1;
	}

	err = spinCameraListClear(*hCamList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinCameraListClear");
		return -1;
	}

	err = spinCameraListDestroy(*hCamList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinCameraListDestroy");
		return -1;
	}

	*hCamList_p = NULL;
DEBUG_MSG(release_spin_cam_list DONE)
	return 0;
}

int _release_spink_interface_list(QSP_ARG_DECL  spinInterfaceList *hInterfaceList_p )
{
	spinError err;

DEBUG_MSG(release_spink_interfacelist BEGIN)
	if( *hInterfaceList_p == NULL ){
		fprintf(stderr,"release_spink_interface_list:  null list!?\n");
		return -1;
	}

	// Clear and destroy interface list before releasing system
	err = spinInterfaceListClear(*hInterfaceList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinInterfaceListClear");
		return -1;
	}

	err = spinInterfaceListDestroy(*hInterfaceList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinInterfaceListDestroy");
		return -1;
	}
	*hInterfaceList_p = NULL;
DEBUG_MSG(release_spink_interfacelist DONE)
	return 0;
}

int _release_spink_interface(QSP_ARG_DECL spinInterface hInterface)
{
	spinError err;

	// Release interface
	err = spinInterfaceRelease(hInterface);
	if (err != SPINNAKER_ERR_SUCCESS){
		report_spink_error(err,"spinInterfaceRelease");
		return -1;
	}
	return 0;
}

//
// Select camera
//
// *** NOTES ***
// Each camera is retrieved from a camera list with an index. If the
// index is out of range, an exception is thrown.
//
// *** LATER ***
// Each camera handle needs to be released before losing scope or the
// system is released.
//

int _get_spink_cam_from_list(QSP_ARG_DECL  spinCamera *hCam_p, spinCameraList hCameraList, int idx )
{
	spinError err;

	err = spinCameraListGet(hCameraList, idx, hCam_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinCameraListGet");
		return -1;
	}
	return 0;
}

int _get_spink_interface_from_list(QSP_ARG_DECL spinInterface *hInterface_p, spinInterfaceList hInterfaceList, int idx )
{
	spinError err;

	err = spinInterfaceListGet(hInterfaceList, idx, hInterface_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinInterfaceListGet");
		return -1;
	}
	return 0;
}

// Retrieve TL device nodemap; please see NodeMapInfo_C example for
// additional comments on transport layer nodemaps.

int _get_spink_transport_level_map(QSP_ARG_DECL  spinNodeMapHandle *mapHdl_p, spinCamera hCam )
{
	spinError err;

	err = spinCameraGetTLDeviceNodeMap(hCam, mapHdl_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinCameraGetTLDeviceNodeMap");
		return -1;
	}
	return 0;
}

//
// Retrieve device vendor name
//
// *** NOTES ***
// Grabbing node information requires first retrieving the node and
// then retrieving its information. There are two things to keep in
// mind. First, a node is distinguished by type, which is related
// to its value's data type.  Second, nodes should be checked for
// availability and readability/writability prior to making an
// attempt to read from or write to the node.
//

int _get_spink_vendor_name( QSP_ARG_DECL  spinNodeMapHandle hNodeMapTLDevice, char *buf, size_t *len_p )
{
	spinNodeHandle hDeviceVendorName = NULL;

	if( get_spink_node(hNodeMapTLDevice,"DeviceVendorName",&hDeviceVendorName) < 0 ) return -1;
	if( ! spink_node_is_available(hDeviceVendorName) ) return -1;
	if( ! spink_node_is_readable(hDeviceVendorName) ) return -1;

	if( spink_get_string(hDeviceVendorName,buf,len_p) < 0 ) return -1;
	return 0;
}


//
// Retrieve device model name
//
// *** NOTES ***
// Because C has no try-catch blocks, each function returns an error
// code to suggest whether an error has occurred. Errors can be
// sufficiently handled with these return codes. Checking availability
// and readability/writability makes for safer and more complete code;
// however, keeping in mind example conciseness and legibility, only
// this example and NodeMapInfo_C demonstrate checking node
// availability and readability/writability while other examples
// handle errors with error codes alone.
//

int _get_spink_model_name( QSP_ARG_DECL  spinNodeMapHandle hNodeMapTLDevice, char *buf, size_t *len_p )
{
	spinNodeHandle hDeviceModelName = NULL;

	if( get_spink_node(hNodeMapTLDevice,"DeviceModelName",&hDeviceModelName) < 0 ) return -1;
	if( ! spink_node_is_available(hDeviceModelName) ) return -1;
	if( ! spink_node_is_readable(hDeviceModelName) ) return -1;
	if( spink_get_string(hDeviceModelName,buf,len_p) < 0 ) return -1;
	return 0;
}

int _get_camera_model_name(QSP_ARG_DECL  char *buf, size_t buflen, spinNodeMapHandle hNodeMapTLDevice)
{
	spinNodeHandle hDeviceModelName = NULL;

	if( get_spink_node(hNodeMapTLDevice,"DeviceModelName",&hDeviceModelName) < 0 ) return -1;
	if( ! spink_node_is_available(hDeviceModelName) ) return -1;
	if( ! spink_node_is_readable(hDeviceModelName) ) return -1;
	if( spink_get_string(hDeviceModelName,buf,&buflen) < 0 ) return -1;

	return 0;
}

int _get_camera_vendor_name(QSP_ARG_DECL  char *buf, size_t buflen, spinCamera hCam)
{
	spinNodeMapHandle hNodeMapTLDevice = NULL;
	spinNodeHandle hDeviceVendorName = NULL;

	if( get_spink_transport_level_map(&hNodeMapTLDevice,hCam) < 0 ) return -1;

	if( get_spink_node(hNodeMapTLDevice,"DeviceVendorName",&hDeviceVendorName) < 0 ) return -1;
	if( ! spink_node_is_available(hDeviceVendorName) ) return -1;
	if( ! spink_node_is_readable(hDeviceVendorName) ) return -1;
	if( spink_get_string(hDeviceVendorName,buf,&buflen) < 0 ) return -1;

	return 0;
}

int _print_indexed_spink_cam_info( QSP_ARG_DECL  spinCameraList hCameraList, int idx )
{
	spinCamera hCam = NULL;
	//spinNodeMapHandle hNodeMapTLDevice = NULL;
	char deviceVendorName[MAX_BUFF_LEN];
	char deviceModelName[MAX_BUFF_LEN];
	size_t vendor_len = MAX_BUFF_LEN;
	size_t model_len = MAX_BUFF_LEN;

	if( get_spink_cam_from_list(&hCam,hCameraList,idx) < 0 ) return -1;

	if( get_camera_vendor_name(deviceVendorName,vendor_len,hCam) < 0 ) return -1;
	if( get_camera_model_name(deviceModelName,model_len,hCam) < 0 ) return -1;

	/*
	if( get_spink_transport_level_map(&hNodeMapTLDevice,hCam) < 0 ) return -1;

	if( get_spink_vendor_name(hNodeMapTLDevice,deviceVendorName,&vendor_len) < 0 ) return -1;
	if( get_spink_model_name(hNodeMapTLDevice,deviceModelName,&model_len) < 0 ) return -1;
	*/

	printf("\tDevice %d %s %s\n\n", idx, deviceVendorName, deviceModelName);

	// release the camera?
	if( release_spink_cam(hCam) < 0 ) return -1;

	return 0;
}

//
// Release camera before losing scope
//
// *** NOTES ***
// Every handle that is created for a camera must be released before
// the system is released or an exception will be thrown.
//

int _release_spink_cam(QSP_ARG_DECL spinCamera hCam)
{
	spinError err;

	err = spinCameraRelease(hCam);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinCameraRelease");
		return -1;
	}
	return 0;
}

int _release_spink_system(QSP_ARG_DECL spinSystem hSystem)
{
	spinError err;

DEBUG_MSG(release_spink_system BEGIN)
	err = spinSystemReleaseInstance(hSystem);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinSystemReleaseInstance");
		return -1;
	}
DEBUG_MSG(release_spink_system DONE)
	return 0;
}

// This function queries an interface for its cameras and then prints out
// device information.

int _query_spink_interface(QSP_ARG_DECL  spinInterface hInterface)
{
	char buf[MAX_BUFF_LEN];
	size_t len = MAX_BUFF_LEN;

	if( get_interface_name(buf,len,hInterface) < 0 ) return -1;

	/*
	spinNodeMapHandle hNodeMapInterface = NULL;
	spinNodeHandle hInterfaceDisplayName = NULL;

	if( get_spink_map(hInterface,&hNodeMapInterface) < 0 ) return -1;

	if( get_spink_node(hNodeMapInterface,"InterfaceDisplayName",&hInterfaceDisplayName) < 0 ) return -1;

	print_interface_name(hInterfaceDisplayName);
	*/

	return 0;
}

int _get_spink_interface_cameras(QSP_ARG_DECL  spinInterface hInterface)
{
	unsigned int i = 0;

	spinCameraList hCameraList = NULL;
	size_t numCameras = 0;

	if( get_spink_cam_list(hInterface, &hCameraList, &numCameras) < 0 )
		return -1;

	if( numCameras == 0 ) return 0;

	// Print device vendor and model name for each camera on the interface
	for (i = 0; i < numCameras; i++) {
		if( print_indexed_spink_cam_info(hCameraList,i) < 0 )
			return -1;
	}

	if( release_spink_cam_list(&hCameraList) < 0 ) return -1;
	
	return 0;
}

//
// Retrieve singleton reference to system object
//
// *** NOTES ***
// Everything originates with the system object. It is important to notice
// that it has a singleton implementation, so it is impossible to have
// multiple system objects at the same time.
//
// *** LATER ***
// The system object should be cleared prior to program completion.  If not
// released explicitly, it will be released automatically.
//

int _get_spink_system(QSP_ARG_DECL spinSystem *hSystem_p)
{
	spinError err;

	err = spinSystemGetInstance(hSystem_p);
	if (err != SPINNAKER_ERR_SUCCESS)
	{
		report_spink_error(err,"spinSystemGetInstance");
		return -1;
	}
	return 0;
}

//
// Retrieve list of interfaces from the system
//
// *** NOTES ***
// Interface lists are retrieved from the system object.
//
// *** LATER ***
// Interface lists must be cleared and destroyed manually. This must be
// done prior to releasing the system and while the interface list is still
// in scope.
//

int _get_spink_interfaces(QSP_ARG_DECL spinSystem hSystem, spinInterfaceList *hInterfaceList_p, size_t *numInterfaces_p)
{
	spinError err;

	//spinInterfaceList hInterfaceList = NULL;
	//size_t numInterfaces = 0;

	// Create empty interface list
	err = spinInterfaceListCreateEmpty(hInterfaceList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinInterfaceListCreateEmpty");
		return -1;
	}

	// Retrieve interfaces from system
	err = spinSystemGetInterfaces(hSystem, *hInterfaceList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinSystemGetInterfaces");
		return -1;
	}

	// Retrieve number of interfaces
	err = spinInterfaceListGetSize(*hInterfaceList_p, numInterfaces_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinInterfaceListGetSize");
		return -1;
	}
	return 0;
}

//
// Retrieve list of cameras from the system
//
// *** NOTES ***
// Camera lists can be retrieved from an interface or the system object.
// Camera lists retrieved from the system, such as this one, return all
// cameras available on the system.
//
// *** LATER ***
// Camera lists must be cleared and destroyed manually. This must be done
// prior to releasing the system and while the camera list is still in
// scope.
//

int _get_spink_cameras(QSP_ARG_DECL spinSystem hSystem, spinCameraList *hCameraList_p, size_t *num_p )
{
	spinError err;


	// Create empty camera list
	err = spinCameraListCreateEmpty(hCameraList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinCameraListCreateEmpty");
		return -1;
	}

	// Retrieve cameras from system
	err = spinSystemGetCameras(hSystem, *hCameraList_p);
	if (err != SPINNAKER_ERR_SUCCESS) {
		report_spink_error(err,"spinSystemGetCameras");
		return -1;
	}

	// Retrieve number of cameras
	err = spinCameraListGetSize(*hCameraList_p, num_p);
	if (err != SPINNAKER_ERR_SUCCESS)
	{
		report_spink_error(err,"spinCameraListGetSize");
		return -1;
	}
	return 0;
}

#endif // HAVE_LIBSPINNAKER
