#ifndef DISPLAY_H
#define DISPLAY_H

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm.h>
#include <libdrm/drm.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/input.h>
#include <libdrm/drm_fourcc.h>
#include <signal.h>
#include <sys/time.h>
#include <cairo.h>
using namespace std;

class display_t
{
	public:
		static int drm_fd;
		drmModeRes* resources;
		drmModeConnector* connector;
		drmModeEncoder* encoder;	
		static drmModeCrtc *crtc;
		drmModeCrtc *orig_crtc;
		drmModeModeInfo mode;
		
		uint16_t *db_ptr;
		static uint32_t buffer1_id;
		static uint32_t buffer2_id;
		static uint32_t current_buffer_id_in_use;
		uint32_t width;
		uint32_t height;
		uint16_t bpp;
		struct drm_mode_create_dumb create_dumb = {0};
		static cairo_t *cr1;
		static cairo_t *cr2;
		
		struct itimerval timer;
		float del_t = 40000;
		
		display_t(string filename);
		~display_t();
		static void handle_timer(int sig);
		void timer_on(void (*handle_timer)(int));
};

#endif
