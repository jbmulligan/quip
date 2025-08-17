#ifndef _MTL_PLATFORM_H_
#define _MTL_PLATFORM_H_

#ifdef HAVE_METAL

#ifdef __OBJC__
#import <Metal/Metal.h>
#endif // __OBJC__

struct mtl_platform_data {
	// BUG - needs to include device pointer!
	const char *		mpd_version;
	const char *		mpd_extensions;
//	id<MTLDevice>		mtl_device;
};

struct mtl_dev_info {
#ifdef __OBJC__
	// implementation-specific stuff here...
	id<MTLDevice>		mdi_device;
#endif // __OBJC__
};

struct mtl_stream_info {
	int foobar;
};

#endif // HAVE_METAL

#endif // _MTL_PLATFORM_H_

