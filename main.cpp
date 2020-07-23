#include <go2/input.h>
#include <go2/display.h>
#include <drm/drm_fourcc.h>

#include <cstring>
#include <stdlib.h>
#include <iostream>
#include "lib/ugui/ugui.h"

void initGo2();
void destroyGo2();
void initUgui();
void destroyUgui();
void go2_present();

// libgo2 stuff
go2_gamepad_state_t outGamepadState;
go2_display_t* display;
go2_surface_t* surface;
go2_presenter_t* presenter;
go2_input_t* input;

uint32_t color_format;
int height;
int width;
int bytes_per_pixel;

// ugui stuff
UG_GUI gui;
UG_WINDOW mainWindow;

bool dirty_display = false;

int main() {

	initGo2();
	initUgui();

	while(1) {
		go2_input_gamepad_read(input,&outGamepadState);
		if (outGamepadState.buttons.f1) {
			std::cout << "f1";
			destroyGo2();
			destroyUgui();
			return 0;
		}

		if (dirty_display) {
			go2_present();
			dirty_display = false;
		}
	}
}

void go2_present() {
	go2_presenter_post(presenter, surface, 
						0, 0, width, height,
						0, 0, width, height,
						GO2_ROTATION_DEGREES_270);
	std::cout << "drawing buffer" << std::endl;
}

void initGo2() {
	input = go2_input_create();

	display = go2_display_create();
	presenter = go2_presenter_create(display, DRM_FORMAT_RGB565, 0xff00ff00); // ABGR
	// presenter = go2_presenter_create(display, DRM_FORMAT_RGB888, 0xff00ff00); // ABGR
	height = go2_display_height_get(display);
	width = go2_display_width_get(display);
    surface = go2_surface_create(display, width, height, DRM_FORMAT_RGB565);
    // surface = go2_surface_create(display, width, height, DRM_FORMAT_RGB888);

    bytes_per_pixel = go2_drm_format_get_bpp(go2_surface_format_get(surface)) / 8;

    go2_display_backlight_set(display, (uint32_t)50);
}

void destroyGo2() {
	go2_input_destroy(input);
	//go2_surface_destroy(surface);
	go2_presenter_destroy(presenter);
	go2_display_destroy(display);
}

void go2SetPixel(UG_S16 x, UG_S16 y, UG_COLOR c) {
	uint8_t* dst = (uint8_t*)go2_surface_map(surface);
	
	memcpy(dst + (y * go2_surface_stride_get(surface) + x*bytes_per_pixel), (unsigned char*)&c, sizeof(c));

	dirty_display = true;

	// std::cout << "drawing pixel " << c << std::endl;
}

void initUgui() {
	std::cout << "screen width: " << width << ", height: " << height << std::endl;
	UG_Init(&gui, go2SetPixel, width, height);
	UG_FontSelect(&FONT_22X36);
	UG_FillScreen(C_DARK_GOLDEN_ROD);

	std::string str = "Go 2 Radio";
	char c[str.size() + 1];
	str.copy(c, str.size() + 1);
	c[str.size()] = '\0';

	UG_SetForecolor(C_DARK_OLIVE_GREEN);
	UG_SetBackcolor(C_DARK_GOLDEN_ROD);
	UG_PutString(20, 20, c);

	UG_FontSelect(&FONT_8X14);
	UG_SetForecolor(C_BLACK);
	char exit[] = "F1 = Exit";
	UG_PutString(20, height-34, exit);

}

void destroyUgui() {

}