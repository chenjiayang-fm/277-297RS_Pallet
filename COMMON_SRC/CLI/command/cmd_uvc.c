#include "cmd.h"
#ifndef S2019A
#include "USBH_API.h"

int32_t cmd_uvc_init(int argc, char* argv[])
{
	return Cmd_uvc_init(argc,argv);
}
int32_t cmd_uvc_get_info(int argc, char* argv[])
{
	return Cmd_uvc_get_info(argc,argv);
}
int32_t cmd_uvc_start(int argc, char* argv[])
{
	return Cmd_uvc_start(argc,argv);
}
int32_t cmd_uvc_stop(int argc, char* argv[])
{
	return Cmd_uvc_stop(argc,argv);
}

#endif //! End of #ifndef S2019A
