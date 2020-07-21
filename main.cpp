#include <go2/input.h>
#include <go2/display.h>
#include <drm/drm_fourcc.h>

#include <stdlib.h>
#include <iostream>
#include "lib/ugui/ugui.h"

void initGo2();
void destroyGo2();

// libgo2 stuff
go2_gamepad_state_t outGamepadState;
go2_display_t* display;
go2_surface_t* surface;
go2_presenter_t* presenter;
go2_input_t* input;

uint32_t color_format;
int height;
int width;
int bits_per_pixel;

// ugui stuff
UG_WINDOW mainWindow;

int main() {

	initGo2();

	while(1) {
		go2_input_gamepad_read(input,&outGamepadState);
		if (outGamepadState.buttons.f1) {
			std::cout << "f1";
			destroyGo2();
			return 0;
		}
	}
}

void initGo2() {
	input = go2_input_create();

	display = go2_display_create();
	presenter = go2_presenter_create(display, DRM_FORMAT_RGB565, 0xff080808); // ABGR
	height = go2_display_height_get(display);
	width = go2_display_width_get(display);
    surface = go2_surface_create(display, width, height, color_format);

    bits_per_pixel = go2_drm_format_get_bpp(go2_surface_format_get(surface)) / 8;
}

void destroyGo2() {
	go2_input_destroy(input);
	//go2_surface_destroy(surface);
	go2_presenter_destroy(presenter);
	go2_display_destroy(display);
}

void go2SetPixel(UG_S16 x, UG_S16 y, UG_COLOR c) {

	uint8_t* dst = (uint8_t*)go2_surface_map(surface);
	// TODO: figure out how to turn this ug color into rgb565...
	dst[bits_per_pixel * (y * width + x)] = 0;
}