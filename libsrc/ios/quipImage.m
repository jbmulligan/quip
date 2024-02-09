//
//  quipImage.m
//

#include "quipImage.h"
#include "quip_prot.h"
#include "viewer.h"

/* we want little-endian for images we synthesize on the iPad...
 * but when we write and then read a png file, the bytes are swapped!?
 * What a mess...
 */

QUIP_IMAGE_TYPE *objc_img_for_dp(Data_Obj *dp, int little_endian_flag)
{
	QUIP_IMAGE_TYPE *theImage;
	CGImageRef myimg;
	CGColorSpaceRef colorSpace;

	if( OBJ_PREC(dp) != PREC_UBY ){
		snprintf(DEFAULT_ERROR_STRING,LLEN,
			"cgimg_for_dp:  object %s (%s) must be u_byte",
			OBJ_NAME(dp),OBJ_PREC_NAME(dp));
		NWARN(DEFAULT_ERROR_STRING);
		return NULL;
	}
	if( OBJ_COMPS(dp) != 4 ){
		snprintf(DEFAULT_ERROR_STRING,LLEN,
			"cgimg_for_dp:  object %s (%d) must have 4 components",
			OBJ_NAME(dp),OBJ_COMPS(dp));
		NWARN(DEFAULT_ERROR_STRING);
		return NULL;
	}

	colorSpace = CGColorSpaceCreateDeviceRGB();
//fprintf(stderr,"objc_img_for_dp:  little_endian_flag = %d\n",little_endian_flag);
	CGContextRef cref = CGBitmapContextCreateWithData(OBJ_DATA_PTR(dp),
		OBJ_COLS(dp), OBJ_ROWS(dp), 8, 4* OBJ_COLS(dp) ,
		colorSpace,
		(
#ifdef BUILD_FOR_IOS
		(little_endian_flag==0?0:kCGBitmapByteOrder32Little) |
#endif // BUILD_FOR_IOS
		kCGImageAlphaPremultipliedFirst
		/*kCGImageAlphaPremultipliedLast*/
		/*kCGImageAlphaFirst*/	// not compatible with other flags???
		/*kCGImageAlphaLast*/	// docs say have to be premultiplied???
		),

		NULL,		// release callback
		NULL			// release callback data arg
		);
	CGColorSpaceRelease(colorSpace);


	if ( cref == NULL ){
		printf("error creating bitmap context!?\n");
		return NULL;
	}

	myimg = CGBitmapContextCreateImage(cref);
	CGContextRelease(cref);

#ifdef BUILD_FOR_IOS
	// We could also use imageWithData, and it might be a lot simpler???
	theImage = [QUIP_IMAGE_TYPE imageWithCGImage:myimg];
#endif // BUILD_FOR_IOS

#ifdef BUILD_FOR_MACOS
	theImage = [[QUIP_IMAGE_TYPE alloc]
		initWithCGImage:myimg
		size:NSZeroSize ];
#endif // BUILD_FOR_MACOS
    
	// image is retained by the property setting above,
	// so we can release the original
	// jbm:  DO WE NEED THAT WITH ARC???
	
	// static analyzer complains, so we remove for now...
	// but that broke things!
	// It appears that CG stuff needs releases, even with ARC...
	CGImageRelease(myimg);

	return theImage;
}



