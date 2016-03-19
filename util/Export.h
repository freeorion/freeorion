#ifndef _Export_h_
#define _Export_h_

#if FREEORION_BUILD_COMMON && __GNUC__
#   define FO_COMMON_API __attribute__((__visibility__("default")))
#else
#   define FO_COMMON_API
#endif


#endif // _Export_h_
