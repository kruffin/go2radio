#include <go2/input.h>
#include <go2/display.h>
#include <drm/drm_fourcc.h>

#include <cstring>
#include <stdlib.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <filesystem>
namespace fs = std::filesystem::__cxx11;

#include "lib/ugui/ugui.h"

void initGo2();
void destroyGo2();
void initUgui();
void destroyUgui();
void go2_present();
void drawScreen();
void tuneRadio(int freq);
void killRadio();

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

bool dirty_display = true;
int MAX_FREQ = 1070;
int MIN_FREQ = 881;
int frequency = 881;
int frequency_temp = 881;

// For handling button inputs
clock_t last_press;
clock_t current_press;
clock_t held_since;

clock_t run_time;

char fm_program_relative[] = "lib/ngsoftfm/build/softfm";
char fm_program_args[] = "-t rtlsdr -c %d00000";
pid_t fm_pid = -1;
const char *fm_program;

int main(int argc, char * argv[]) {
	fs::path progPath = fs::path(argv[0]);
	progPath.remove_filename();
	progPath += fm_program_relative;
	fm_program = progPath.c_str();

	// std::cout << argv[0] << "|" << fm_program << std::endl;
	initGo2();
	initUgui();
	last_press = clock();

	while(1) {
		go2_input_gamepad_read(input,&outGamepadState);
		if (outGamepadState.buttons.f1) {
			std::cout << "f1";
			killRadio();
			destroyGo2();
			destroyUgui();
			return 0;
		}
		current_press = clock();
		double diff = double(current_press - last_press) / double(CLOCKS_PER_SEC);
		if (diff > 0.15) {
			int increment = 1;
			if (held_since != -1 && double(current_press - held_since) / double(CLOCKS_PER_SEC) > 2.0) {
				increment = 10;
			}
			if (outGamepadState.dpad.left) {
				frequency_temp -= increment;
				if (frequency_temp < MIN_FREQ) {
					frequency_temp = MAX_FREQ;
				}
				dirty_display = true;
				last_press = current_press;
				if (held_since == -1) {
					held_since = last_press;
				}
			}
			if (outGamepadState.dpad.right) {
				frequency_temp += increment;
				if (frequency_temp > MAX_FREQ) {
					frequency_temp = MIN_FREQ;
				}
				dirty_display = true;
				last_press = current_press;
				if (held_since == -1) {
					held_since = last_press;
				}
			}

			if (held_since != -1 && current_press != last_press) {
				// No buttons held
				held_since = -1;
				//tuneRadio(frequency);
				// std::cout << "held since reset c(" << current_press << ") l(" << last_press << ")" << std::endl;
			}

			if (outGamepadState.buttons.a) {
				frequency = frequency_temp;
				tuneRadio(frequency);
				dirty_display = true;
			}
			if (outGamepadState.buttons.b) {
				frequency_temp = frequency;
				dirty_display = true;
			}
			if (outGamepadState.buttons.x) {
				killRadio();
				dirty_display = true;
			}
		}

		if (dirty_display) {
			drawScreen();
			go2_present();
			dirty_display = false;
		}
	}
}

void killRadio() {
	if (fm_pid != -1) {
		// Kill it
		while( double(clock() - run_time)/double(CLOCKS_PER_SEC) < 5.0 || -1 == kill(fm_pid, SIGKILL) ) {
			// std::cerr << "Failed to kill process [" << fm_pid << "] " << std::strerror(errno);
		}
		fm_pid = -1;
	}
}

void tuneRadio(int freq) {
	killRadio();
	run_time = clock();

	// int filedes[2];
	// if (pipe(filedes) == -1) {
	// 	std::cerr << "Failed to create a pipe." << std::endl;
	// 	exit(1);
	// }

	fm_pid = fork();
	switch (fm_pid) {
		case -1:
			std::cerr << "Failed to fork process." << std::endl;
			exit(1);
		case 0:
			// while( (dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR) ) {}
			// close(filedes[1]);
			// close(filedes[0]);
			char freqStr[50];
			sprintf(freqStr, "freq=%d00000", freq);
			// std::cout << "euid: " << geteuid() << "uid: " << getuid()  << " frequency: " << freqStr << std::endl;
			execl(fm_program, "softfm", "-t", "rtlsdr", "-q", "-c", freqStr, (char *) NULL);
			std::cerr << "execl() failed to run: " << fm_program << std::endl;
			exit(1);
		// default:
			// close(filedes[1]);
			// char buff[4096];
			// clock_t start = clock();
			// while ( double(clock() - start) / double(CLOCKS_PER_SEC) < 5.0) {
			// 	ssize_t count = read(filedes[0], buff, sizeof(buff));
			// 	if (count == -1 && errno != EINTR) {
			// 		break;
			// 	} else if (count > 0) {
			// 		std::cout << buff;
			// 		break;
			// 	}
			// }
			// close(filedes[0]);
	}
}

void go2_present() {
	go2_presenter_post(presenter, surface, 
						0, 0, width, height,
						0, 0, width, height,
						GO2_ROTATION_DEGREES_0);
	// std::cout << "drawing buffer" << std::endl;
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
	
	// swap the x and y while translating x
	// →[*][ ][ ][ ]
	//  [ ][ ][ ][ ]
	// to:
	//  [ ][*]
	//  [ ][ ]
	//  [ ][ ]
	//  [ ][ ]
	//      ↑

	UG_S16 yfinal = height - 1 - x;
	UG_S16 xfinal = y;
	// std::cout << "drawing pixel og(" << x << "," << y << ") trans(" << xfinal << "," << yfinal << ")" << std::endl;
	memcpy(dst + (yfinal * go2_surface_stride_get(surface) + xfinal*bytes_per_pixel), (unsigned char*)&c, sizeof(c));

	dirty_display = true;

	// std::cout << "drawing pixel " << c << std::endl;
}

void format_frequency(int freq, char *output) {
	int len = sprintf(output, "%d", freq);
	// Shift chars by 1 and add a '.'
	output[len] = output[len-1];
	output[len-1] = '.';
	output[len+1] = '\0';
}

void drawScreen() {
	UG_FontSelect(&FONT_22X36);
	UG_FillScreen(C_DARK_GOLDEN_ROD);

	char title[] = "Go 2 Radio";
	UG_SetForecolor(C_DARK_OLIVE_GREEN);
	UG_SetBackcolor(C_DARK_GOLDEN_ROD);
	UG_PutString(20, 20, title);

	UG_FontSelect(&FONT_32X53);
	char freq[6];
	format_frequency(frequency, (char *)freq);
	UG_PutString(20, width/2-53, freq);

	if (frequency_temp != frequency) {
		// Only draw if attempting to tune
		UG_FontSelect(&FONT_8X14);
		format_frequency(frequency_temp, (char *)freq);
		UG_PutString(25, width/2, freq);
	}

	if (-1 != fm_pid) {
		UG_FontSelect(&FONT_22X36);
		char music[2];
		sprintf(music, "%c", char(14));
		UG_PutString(height-36 - strlen(music)*36, 20, music);
	}

	UG_FontSelect(&FONT_8X8);
	UG_SetForecolor(C_BLACK);
	char exit[] = "f1=exit";
	UG_PutString(20, width-28, exit);

	char tuneText[8];
	sprintf(tuneText, "%ctune%c", char(27), char(26));
	// std::cout << "! = " << int('!') << std::endl;
	UG_PutString(40 + strlen(exit) * 8, width-28, tuneText);

	char selectText[] = "a=select  b=cancel  x=stop tune";
	UG_PutString(60 + (strlen(exit) + strlen(tuneText)) * 8, width-28, selectText);
}

void initUgui() {
	// std::cout << "screen width: " << width << ", height: " << height << std::endl;
	// swap dimensions so ui surface is correct for rotated screen
	UG_Init(&gui, go2SetPixel, height, width); 
}

void destroyUgui() {

}