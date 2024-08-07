#include"display.h"
#include<string>
using namespace std;

void display_t::handle_timer(int sig)
{
	if (display_t::current_buffer_id_in_use == display_t::buffer1_id)
	{	
		cairo_rectangle(display_t::cr2, 100, 100, 50, 50);
		drmModePageFlip(display_t::drm_fd, display_t::crtc->crtc_id, display_t::buffer2_id, 0, NULL);
		display_t::current_buffer_id_in_use = display_t::buffer2_id;
	}
	else
	{
		cairo_rectangle(display_t::cr1, 100, 100, 50, 50);
		drmModePageFlip(display_t::drm_fd, display_t::crtc->crtc_id, display_t::buffer1_id, 0, NULL);
		display_t::current_buffer_id_in_use = display_t::buffer1_id;		
	}
}

int main()
{
	string display_driver_name = "/dev/dri/card0";
	display_t display(display_driver_name);
	display.timer_on(display_t::handle_timer);
	while(1);
}
