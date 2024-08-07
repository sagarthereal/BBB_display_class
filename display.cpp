#include"display.h"

#define CHECK_ERROR(cond, msg) do {\
	if (cond) { \
		perror(msg); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

int display_t::drm_fd = -1;
drmModeCrtc* display_t::crtc = NULL;
uint32_t display_t::buffer1_id = -1;
uint32_t display_t::buffer2_id = -1;
uint32_t display_t::current_buffer_id_in_use = -1;
cairo_t* display_t::cr1 = NULL;
cairo_t* display_t::cr2 = NULL;

	
display_t::display_t(string filename)
{
	struct drm_mode_map_dumb map_dumb={0};
	int i = 0;
	uint32_t handle[4] = {0};
	uint32_t pitch[4] = {0};
	uint32_t offsets[4];
	uint32_t offsets1[4];
	cairo_surface_t *cairo_surface1;
	cairo_surface_t *cairo_surface2;	
	
	drm_fd = open(filename.c_str(), O_RDWR | O_CLOEXEC); //drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);	
	CHECK_ERROR(drm_fd<0, "Failed to open DRM device");
	resources = drmModeGetResources(drm_fd);
	CHECK_ERROR(!resources, "drmModeGetResources failed");
	connector = NULL;
	for(i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
		if (connector->connection == DRM_MODE_CONNECTED) {
			break;
		}
		drmModeFreeConnector(connector);
		connector = NULL;
	}
	CHECK_ERROR(!connector, "No connected connector found");
	encoder = drmModeGetEncoder(drm_fd, connector->encoder_id);
	CHECK_ERROR(!encoder, "drmModeGetEncoder failed");
	crtc = drmModeGetCrtc(drm_fd, encoder->crtc_id);
	CHECK_ERROR(!crtc, "drmModeGetCrtc failed");
	orig_crtc = drmModeGetCrtc(drm_fd, crtc->crtc_id);
	mode = connector->modes[0];
	width = mode.hdisplay;
	height = mode.vdisplay;
	bpp = 16;

	
	create_dumb.width = width;
	create_dumb.height = 3*height;
	create_dumb.bpp = bpp;
	ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb);
	map_dumb.handle = create_dumb.handle;
	ioctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb);
	db_ptr = static_cast<uint16_t*>(mmap(0, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, map_dumb.offset));
	CHECK_ERROR(db_ptr == MAP_FAILED, "Failed to mmap framebuffer");
	
    	handle[0] = create_dumb.handle;
    	pitch[0]  = create_dumb.pitch;
	offsets[0] = 0;
	offsets1[0] = width*height*bpp/8;
	
	
	drmModeAddFB2(drm_fd, width, height, DRM_FORMAT_RGB565, handle, pitch, offsets, &buffer1_id, 0);
	drmModeAddFB2(drm_fd, width, height, DRM_FORMAT_RGB565, handle, pitch, offsets1, &buffer2_id, 0);
	drmModeSetCrtc(drm_fd, crtc->crtc_id, buffer1_id, 0, 0, &connector->connector_id, 1, &mode);
	current_buffer_id_in_use = buffer1_id;
	
	cairo_surface1 = cairo_image_surface_create_for_data(reinterpret_cast<unsigned char*>(db_ptr), CAIRO_FORMAT_RGB16_565, width, height, width*bpp/8);
	cr1 = cairo_create(cairo_surface1);
	cairo_surface2 = cairo_image_surface_create_for_data(reinterpret_cast<unsigned char*>(&(db_ptr[height*width])), CAIRO_FORMAT_RGB16_565, width, height, width*bpp/8);
	cr2 = cairo_create(cairo_surface2);
	
	
	CHECK_ERROR(buffer1_id < 0, "drmModeADDFB failed");
	CHECK_ERROR(buffer2_id < 0, "drmModeADDFB failed");
	
	timer.it_value.tv_sec = 5;
	timer.it_value.tv_usec = 0 ;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = del_t;
	setitimer(ITIMER_REAL, &timer, NULL);
}

void display_t::timer_on(void (*handle_timer)(int))
{
	signal(SIGALRM, handle_timer);
}

display_t::~display_t()
{
	drmModeSetCrtc(drm_fd, orig_crtc->crtc_id, orig_crtc->buffer_id, orig_crtc->x, orig_crtc->y, &connector->connector_id, 1, &orig_crtc->mode);
	drmModeRmFB(drm_fd, buffer1_id);
	drmModeRmFB(drm_fd, buffer2_id);
	drmModeFreeCrtc(crtc);
	drmModeFreeEncoder(encoder);
	drmModeFreeConnector(connector);
	drmModeFreeResources(resources);
	munmap(db_ptr, create_dumb.size);
	struct drm_mode_destroy_dumb destroy_dumb = {0};
	destroy_dumb.handle = create_dumb.handle;
	ioctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_dumb);
	drmModeFreeCrtc(orig_crtc);	
	close(drm_fd);
}


