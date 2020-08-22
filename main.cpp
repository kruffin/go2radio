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
#include "Config.h"
#include "StationBookmarks.h"

void initGo2();
void destroyGo2();
void initUgui();
void destroyUgui();
void go2_present();
void drawScreen();
void tuneRadio(int freq);
void killRadio();
void volumeUp();
void volumeDown();
void alterBookmarks();
void tempNextBookmark();
void tempPrevBookmark();

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
bool show_help = false;

bool dirty_display = true;
int frequency;
int frequency_temp;

int volume_increment = 2; // Amount to increment the volume in percent.

// For handling button inputs
clock_t last_press;
clock_t current_press;
clock_t held_since;
clock_t idle_since;

clock_t run_time;

pid_t fm_pid = -1;

// Configuration
Config config = Config("");
StationBookmarks bmarks = StationBookmarks("");


int main(int argc, char * argv[]) {
	char dir[1024];
	sprintf(dir, "%s/.go2radio/config", std::getenv("HOME"));
	config.file_path = std::string(dir);

	sprintf(dir, "%s/.go2radio/stations", std::getenv("HOME"));
	bmarks.file_path = std::string(dir);

	system("mkdir -p ~/.go2radio");
	if (!config.load()) {
		// If it doesn't exist create it.
		config.save();
	}
	config.print_config();

	if (!bmarks.load()) {
		std::cout << "No saved bookmarks found: " << bmarks.file_path << std::endl;
	}

	if (config.softfm_path.find('/') != 0) {
		// It's a relative path, turn into absolute.
		fs::path progPath = fs::path(argv[0]);
		progPath.remove_filename();
		progPath += config.softfm_path;
		config.softfm_path = progPath;
	}

	frequency = frequency_temp = config.frequency_start;

	std::cout << "program: " << config.softfm_path << std::endl;
	initGo2();
	initUgui();
	last_press = clock();
	idle_since = clock();

	while(1) {
		go2_input_gamepad_read(input,&outGamepadState);
		if (outGamepadState.buttons.f1) {
			std::cout << "f1";
			killRadio();
			go2_display_backlight_set(display, (uint32_t)config.brightness_active);
			destroyGo2();
			destroyUgui();
			return 0;
		}
		current_press = clock();
		double diff = double(current_press - last_press) / double(CLOCKS_PER_SEC);
		if (diff > 0.15) {
			int increment = config.tune_increment_normal;
			if (held_since != -1 && double(current_press - held_since) / double(CLOCKS_PER_SEC) > config.tune_increment_trans_time) {
				increment = config.tune_increment_fast;
			}
			if (outGamepadState.dpad.left) {
				frequency_temp -= increment;
				if (frequency_temp < config.frequency_min) {
					frequency_temp = config.frequency_max;
				}
				dirty_display = true;
				last_press = current_press;
				idle_since = current_press;
				if (held_since == -1) {
					held_since = last_press;
				}
			}
			if (outGamepadState.dpad.right) {
				frequency_temp += increment;
				if (frequency_temp > config.frequency_max) {
					frequency_temp = config.frequency_min;
				}
				dirty_display = true;
				last_press = current_press;
				idle_since = current_press;
				if (held_since == -1) {
					held_since = last_press;
				}
			}
			if (outGamepadState.dpad.up) {
				volumeUp();
				last_press = current_press;
				idle_since = current_press;
			}
			if (outGamepadState.dpad.down) {
				volumeDown();
				last_press = current_press;
				idle_since = current_press;
			}


			if (outGamepadState.buttons.a) {
				if (! (frequency == frequency_temp && -1 != fm_pid)) {
					// It's already tuned to this frequency.
					frequency = frequency_temp;
					tuneRadio(frequency);
					dirty_display = true;
					last_press = current_press;
				}
				idle_since = current_press;
			}
			if (outGamepadState.buttons.b) {
				frequency_temp = frequency;
				dirty_display = true;
				last_press = current_press;
				idle_since = current_press;
			}
			if (outGamepadState.buttons.x) {
				killRadio();
				dirty_display = true;
				last_press = current_press;
				idle_since = current_press;
			}
			if (outGamepadState.buttons.y) {
				if (-1 == held_since) {
					alterBookmarks();
					dirty_display = true;
					held_since = current_press;
				}
				
				last_press = current_press;
				idle_since = current_press;
			}

			if (outGamepadState.buttons.top_left) {
				if (-1 == held_since) {
					tempPrevBookmark();
					dirty_display = true;
					held_since = current_press;
				}
				
				last_press = current_press;
				idle_since = current_press;
			}

			if (outGamepadState.buttons.top_right) {
				if (-1 == held_since) {
					tempNextBookmark();
					dirty_display = true;
					held_since = current_press;
				}
				
				last_press = current_press;
				idle_since = current_press;
			}

			if (outGamepadState.buttons.f2) {
				if (-1 == held_since) {
					show_help = !show_help;
					dirty_display = true;
					held_since = current_press;
				}
				
				last_press = current_press;
				idle_since = current_press;
			}

			if (held_since != -1 && current_press != last_press) {
				// No buttons held
				held_since = -1;
				//tuneRadio(frequency);
				// std::cout << "held since reset c(" << current_press << ") l(" << last_press << ")" << std::endl;
			}
		}
		if (double(clock() - idle_since) / double(CLOCKS_PER_SEC) > config.idle_time) {
			// Change the backlight if idle
			go2_display_backlight_set(display, (uint32_t)config.brightness_idle);
		} else if (current_press == idle_since) {
			// It just came out of idle
			go2_display_backlight_set(display, (uint32_t)config.brightness_active);
		}

		if (dirty_display) {
			drawScreen();
			go2_present();
			dirty_display = false;
		}
	}
}

void volumeUp() {
	char cmd[50];
	sprintf(cmd, "/usr/bin/amixer -q sset Playback %d%%+ ", volume_increment);
	system(cmd);
}

void volumeDown() {
	char cmd[50];
	sprintf(cmd, "/usr/bin/amixer -q sset Playback %d%%- ", volume_increment);
	system(cmd);
}

void killRadio() {
	if (fm_pid != -1) {
		// Kill it, but make sure softfm has had time to register it's sigterm handler.
		while( double(clock() - run_time)/double(CLOCKS_PER_SEC) < config.tune_kill_sleep_time || -1 == kill(fm_pid, SIGKILL) ) {
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
			sprintf(freqStr, config.softfm_args.c_str(), freq);
			// std::cout << "euid: " << geteuid() << "uid: " << getuid()  << " frequency: " << freqStr << std::endl;
			execl(config.softfm_path.c_str(), "softfm", "-t", "rtlsdr", "-q", "-c", freqStr, (char *) NULL);
			std::cerr << "execl() failed to run: " << config.softfm_path << std::endl;
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

	// Take this out since it has the potential to infinite loop if not lucky like I was.
	//dirty_display = true;

	// std::cout << "drawing pixel " << c << std::endl;
}

void format_frequency(int freq, char *output) {
	double tmp = freq/1000000.0;
	int len = sprintf(output, config.frequency_display_format.c_str(), tmp);
	// Shift chars by 1 and add a '.'
	// output[len] = output[len-1];
	// output[len-1] = '.';
	// output[len+1] = '\0';
}

void drawScreen() {
	UG_FontSelect(&FONT_22X36);
	UG_FillScreen(C_DARK_GOLDEN_ROD);

	char title[] = "Go 2 Radio";
	UG_SetForecolor(C_DARK_OLIVE_GREEN);
	UG_SetBackcolor(C_DARK_GOLDEN_ROD);
	UG_PutString(20, 20, title);

	UG_FontSelect(&FONT_32X53);
	char freq[30];
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

	// Draw bookmarks
	// std::cout << "Number of stations: " << bmarks.size() << std::endl;
	int startx = 20;
	int starty = width/2 + 30;

	UG_FontSelect(&FONT_8X14);
	UG_COLOR bg;
	UG_COLOR fg;
	for (int i = 0; i < bmarks.size(); ++i) {
		int b_freq = bmarks.get(i);
		if (frequency_temp == b_freq) {
			fg = C_BLACK;
			bg = C_OLIVE_DRAB;
		} else {
			fg = C_DARK_OLIVE_GREEN;
			bg = C_OLIVE;
		}
		UG_SetForecolor(fg);
		UG_SetBackcolor(bg);
		format_frequency(bmarks.get(i), (char *)freq);
		int row = i / 4;
		int col = i - row * 4;
		int curx = col * 110 + startx;
		int cury = row * 20 + starty;
		// std::cout << "Printing (" << col << "," << row << "): " << freq << std::endl;
		UG_FillRoundFrame(curx - 2, cury - 1, curx + 90, cury + 14, 5, bg);
		UG_PutString(curx, cury, freq);
	}

	UG_FontSelect(&FONT_8X8);
	UG_SetForecolor(C_BLACK);
	UG_SetBackcolor(C_DARK_GOLDEN_ROD);	

	char helpText[] = "f2=help";
	UG_PutString(height - strlen(helpText) * 9, width-9, helpText);

	if (show_help) {
		UG_FontSelect(&FONT_8X8);
		UG_SetForecolor(C_BLACK);
		UG_SetBackcolor(C_DARK_GOLDEN_ROD);
		char exit[] = "f1=exit";
		UG_PutString(20, width-28, exit);

		char volText[8];
		sprintf(volText, " %cvol", char(18));
		UG_PutString(40 + strlen(exit) * 8, width-38, volText);

		char tuneText[8];
		sprintf(tuneText, "%ctune%c", char(27), char(26));
		// std::cout << "! = " << int('!') << std::endl;
		UG_PutString(40 + strlen(exit) * 8, width-28, tuneText);

		char selectText[] = "a=select  b=cancel  x=stop tune";
		UG_PutString(60 + (strlen(exit) + strlen(tuneText)) * 8, width-28, selectText);

		if (bmarks.size() > 0) {
			char bookmarkText[65];
			sprintf(bookmarkText, "bookmarks: %cleft trigger,right trigger%c,y=add/remove", char(27), char(26));
			UG_PutString(0, width - 48, bookmarkText);
		}
	}
}

void initUgui() {
	// std::cout << "screen width: " << width << ", height: " << height << std::endl;
	// swap dimensions so ui surface is correct for rotated screen
	UG_Init(&gui, go2SetPixel, height, width); 
}

void destroyUgui() {

}

void alterBookmarks() {
	if (bmarks.isMarked(frequency)) {
		bmarks.remove(frequency);
		bmarks.save();
	} else {
		bmarks.add(frequency);
		bmarks.save();
	}
}

void tempNextBookmark() { 
	int tmp = bmarks.next(frequency_temp);
	if (-1 != tmp) {
		frequency_temp = tmp;
	}
}

void tempPrevBookmark() {
	int tmp = bmarks.prev(frequency_temp);
	if (-1 != tmp) {
		frequency_temp = tmp;
	}
}