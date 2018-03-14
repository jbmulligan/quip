
SPINK_WRAPPER_THREE_ARG(get_enumeration_entry_by_name,EnumerationGetEntryByName,spinNodeHandle,hEnum,const char *, tag,spinNodeHandle *,hdl_p)
SPINK_WRAPPER_TWO_ARG(get_enumeration_int_val,EnumerationEntryGetIntValue,spinNodeHandle,hNode,int64_t *, int_ptr)

SPINK_WRAPPER_TWO_ARG(set_enumeration_int_val,EnumerationSetIntValue,spinNodeHandle,hNode,int64_t , v)
SPINK_WRAPPER_ONE_ARG(release_spink_image,ImageRelease,spinImage,hImage)
SPINK_WRAPPER_TWO_ARG(get_next_image,CameraGetNextImage,spinCamera,hCam,spinImage *,img_p)
SPINK_WRAPPER_TWO_ARG(image_is_incomplete,ImageIsIncomplete,spinImage,hImg,bool8_t *,flag_p)
SPINK_WRAPPER_TWO_ARG(get_image_status,ImageGetStatus,spinImage,hImg,spinImageStatus *,status_p)
SPINK_WRAPPER_TWO_ARG(get_image_width,ImageGetWidth,spinImage,hImg,size_t *,width_p)
SPINK_WRAPPER_TWO_ARG(get_image_height,ImageGetHeight,spinImage,hImg,size_t *,height_p)
SPINK_WRAPPER_ONE_ARG(create_empty_image,ImageCreateEmpty,spinImage *,hImg_p)
SPINK_WRAPPER_THREE_ARG(convert_spink_image,ImageConvert,spinImage,hSrcImg,spinPixelFormatEnums,fmt,spinImage,hDestImg)
SPINK_WRAPPER_ONE_ARG(destroy_spink_image,ImageDestroy,spinImage,hImg)
SPINK_WRAPPER_ONE_ARG(begin_acquisition,CameraBeginAcquisition,spinCamera,hCam)
SPINK_WRAPPER_ONE_ARG(end_acquisition,CameraEndAcquisition,spinCamera,hCam)
SPINK_WRAPPER_TWO_ARG(get_iface_map,InterfaceGetTLNodeMap,spinInterface,hIface,spinNodeMapHandle *,hMap_p)
SPINK_WRAPPER_THREE_ARG(fetch_spink_node,NodeMapGetNode,spinNodeMapHandle,hMap,const char *,tag,spinNodeHandle *,hNode_p)

SPINK_WRAPPER_TWO_ARG(node_is_implemented,NodeIsImplemented,spinNodeHandle,hNode,bool8_t *,flag_p)
SPINK_WRAPPER_TWO_ARG(node_is_available,NodeIsAvailable,spinNodeHandle,hNode,bool8_t *,flag_p)
SPINK_WRAPPER_TWO_ARG(node_is_readable,NodeIsReadable,spinNodeHandle,hNode,bool8_t *,flag_p)
SPINK_WRAPPER_TWO_ARG(node_is_writeable,NodeIsWritable,spinNodeHandle,hNode,bool8_t *,flag_p)

SPINK_WRAPPER_THREE_ARG(spink_get_string,StringGetValue,spinNodeHandle,hNode,char *,buf,size_t *,len_p)

SPINK_WRAPPER_ONE_ARG(create_empty_cam_list,CameraListCreateEmpty,spinCameraList *,hCamList_p)
SPINK_WRAPPER_TWO_ARG(get_iface_cameras,InterfaceGetCameras,spinInterface,hIface,spinCameraList *,hCamList_p)
SPINK_WRAPPER_TWO_ARG(get_n_cameras,CameraListGetSize,spinCameraList,hCamList,size_t *,num_p)

SPINK_WRAPPER_ONE_ARG(clear_cam_list,CameraListClear,spinCameraList,hCamList)
SPINK_WRAPPER_ONE_ARG(destroy_cam_list,CameraListDestroy,spinCameraList,hCamList)

SPINK_WRAPPER_ONE_ARG(clear_iface_list,InterfaceListClear,spinInterfaceList,hIfaceList)
SPINK_WRAPPER_ONE_ARG(destroy_iface_list,InterfaceListDestroy,spinInterfaceList,hIfaceList)
SPINK_WRAPPER_TWO_ARG(get_n_interfaces,InterfaceListGetSize,spinInterfaceList,hIfaceList,size_t *,num_p)

SPINK_WRAPPER_ONE_ARG(release_spink_interface,InterfaceRelease,spinInterface,hIface)

SPINK_WRAPPER_THREE_ARG(get_cam_from_list,CameraListGet,spinCameraList,hCamList,int,idx,spinCamera *,hCam_p)
SPINK_WRAPPER_THREE_ARG(get_iface_from_list,InterfaceListGet,spinInterfaceList,hIfaceList,int,idx,spinInterface *,hIface_p)

SPINK_WRAPPER_TWO_ARG(get_transport_level_map,CameraGetTLDeviceNodeMap,spinCamera,hCam,spinNodeMapHandle *,hMap_p)

SPINK_WRAPPER_ONE_ARG(release_spink_cam,CameraRelease,spinCamera,hCam)
SPINK_WRAPPER_ONE_ARG(release_spink_system,SystemReleaseInstance,spinSystem,hSystem)
SPINK_WRAPPER_ONE_ARG(get_spink_system,SystemGetInstance,spinSystem *,hSystem_p)

SPINK_WRAPPER_ONE_ARG(create_empty_iface_list,InterfaceListCreateEmpty,spinInterfaceList *,hIfaceList_p)
SPINK_WRAPPER_TWO_ARG(get_iface_list,SystemGetInterfaces,spinSystem,hSystem,spinInterfaceList,hIfaceList)

SPINK_WRAPPER_TWO_ARG(get_cameras_from_system,SystemGetCameras,spinSystem,hSystem,spinCameraList,hCamList)

SPINK_WRAPPER_TWO_ARG(get_node_type,NodeGetType,spinNodeHandle,hNode,spinNodeType *,type_p)

SPINK_WRAPPER_THREE_ARG(node_to_string,NodeToString,spinNodeHandle,hNode,char *,buf,size_t *,num_p)
SPINK_WRAPPER_THREE_ARG(get_string_value,StringGetValue,spinNodeHandle,hNode,char *,buf,size_t *,num_p)
SPINK_WRAPPER_TWO_ARG(get_int_value,IntegerGetValue,spinNodeHandle,hNode,int64_t *,intptr)
SPINK_WRAPPER_TWO_ARG(get_float_value,FloatGetValue,spinNodeHandle,hNode,double *,fltptr)
SPINK_WRAPPER_TWO_ARG(get_bool_value,BooleanGetValue,spinNodeHandle,hNode,bool8_t *,flag_p)
SPINK_WRAPPER_THREE_ARG(get_tip_value,NodeGetToolTip,spinNodeHandle,hNode,char *,buf,size_t *,buflen_p)

SPINK_WRAPPER_TWO_ARG(get_current_entry,EnumerationGetCurrentEntry,spinNodeHandle,hNode,spinNodeHandle *,hNode_p)
SPINK_WRAPPER_THREE_ARG(get_entry_symbolic,EnumerationEntryGetSymbolic,spinNodeHandle,hNode,char *,buf,size_t *,buflen_p)

SPINK_WRAPPER_THREE_ARG(get_node_display_name,NodeGetDisplayName,spinNodeHandle,hNode,char *,buf,size_t *,buflen_p)
SPINK_WRAPPER_THREE_ARG(get_node_short_name,NodeGetName,spinNodeHandle,hNode,char *,buf,size_t *,buflen_p)

SPINK_WRAPPER_TWO_ARG(get_n_features,CategoryGetNumFeatures,spinNodeHandle,hNode,size_t *,num_p)
SPINK_WRAPPER_THREE_ARG(get_feature_by_index,CategoryGetFeatureByIndex,spinNodeHandle,hNode,int,idx,spinNodeHandle *,hNode_p)

SPINK_WRAPPER_TWO_ARG(get_device_node_map,CameraGetTLDeviceNodeMap,spinCamera,hCam,spinNodeMapHandle *,hMap_p)
SPINK_WRAPPER_TWO_ARG(get_stream_node_map,CameraGetTLStreamNodeMap,spinCamera,hCam,spinNodeMapHandle *,hMap_p)
SPINK_WRAPPER_TWO_ARG(get_camera_node_map,CameraGetNodeMap,spinCamera,hCam,spinNodeMapHandle *,hMap_p)

SPINK_WRAPPER_ONE_ARG(camera_deinit,CameraDeInit,spinCamera,hCam)
SPINK_WRAPPER_ONE_ARG(connect_spink_cam,CameraInit,spinCamera,hCam)

