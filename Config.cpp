#include <iostream>
#include <fstream>
#include <regex>

#include "Config.h"

Config::Config(std::string file) {
	this->file_path = file;
	// Set some defaults.
	this->idle_time = 30.0;
	this->brightness_idle = 1;
	this->brightness_active = 50;
	this->frequency_min = 88100000;
	this->frequency_max = 107000000;
	this->frequency_display_format = std::string("%.1f");
	this->tune_kill_sleep_time = 5.0;
	this->tune_increment_trans_time = 2.0;
	this->tune_increment_normal = 100000;
	this->tune_increment_fast = 1000000;
	this->softfm_path = std::string("lib/ngsoftfm/build/softfm");
	this->softfm_args = std::string("freq=%d gain=auto");
}

Config::~Config() {

}

void Config::save() {
	std::ofstream configFile;
	configFile.open(this->file_path);

	configFile << "# Describes how long to wait on inactivity before changing" << std::endl;
	configFile << "# the brightness level." << std::endl;
	configFile << "idle_time=" << this->idle_time << std::endl;
	configFile << "# A level of 0 means no backlight and 100 is full." << std::endl;
	configFile << "brightness_idle=" << this->brightness_idle << std::endl;
	configFile << "brightness_active=" << this->brightness_active << std::endl;
	configFile << "# The min/max frequencies in Hz." << std::endl;
	configFile << "frequency_min=" << this->frequency_min << std::endl;
	configFile << "frequency_max=" << this->frequency_max << std::endl;
	configFile << "# How to display the frequency in MHz." << std::endl;
	configFile << "frequency_display_format=" << this->frequency_display_format << std::endl;
	configFile << "# How long to let the softfm program run before attempting" << std::endl;
	configFile << "# to kill it in seconds." << std::endl;
	configFile << "tune_kill_sleep_time=" << this->tune_kill_sleep_time << std::endl;
	configFile << "# How long the tune button must be held before it" << std::endl;
	configFile << "# transitions to the fast increment speed." << std::endl;
	configFile << "tune_increment_trans_time=" << this->tune_increment_trans_time << std::endl;
	configFile << "# The amount to increment in Hz." << std::endl;
	configFile << "tune_increment_normal=" << this->tune_increment_normal << std::endl;
	configFile << "tune_increment_fast=" << this->tune_increment_fast << std::endl;
	configFile << "# The path to the softfm program." << std::endl;
	configFile << "softfm_path=" << this->softfm_path << std::endl;
	configFile << "# The arguments to pass to the softfm program specific to" << std::endl;
	configFile << "# the RTL-SDR inputs; at a minimum it must have:" << std::endl;
	configFile << "# 'freq=%d' for the correct frequency to be passed." << std::endl;
	configFile << "softfm_args=" << this->softfm_args << std::endl;

	configFile.close();
}

bool Config::load() {
	std::ifstream configFile;
	configFile.open(this->file_path);
	if (configFile.fail()) {
		return false;
	}

	std::string line;
	while (std::getline(configFile, line)) {
		std::size_t pos  = line.find('#');
		if (pos == 0) {
			// Ignore a commented line
			continue;
		}
		pos = line.find('=');
		if (pos == std::string::npos) {
			// keep going, this line is malformed.
			continue;
		}
		std::string attr_name = line.substr(0, pos);
		std::string attr_value = line.substr(pos+1);

		if (attr_name == "idle_time") {
			this->idle_time = std::stod(attr_value);
		} else if (attr_name == "brightness_idle") {
			this->brightness_idle = std::stoi(attr_value);
		} else if (attr_name == "brightness_active") {
			this->brightness_active = std::stoi(attr_value);
		} else if (attr_name == "frequency_min") {
			this->frequency_min = std::stoi(attr_value);
		} else if (attr_name == "frequency_max") {
			this->frequency_max = std::stoi(attr_value);
		} else if (attr_name == "frequency_display_format") {
			this->frequency_display_format = attr_value;
		} else if (attr_name == "tune_kill_sleep_time") {
			this->tune_kill_sleep_time = std::stod(attr_value);
		} else if (attr_name == "tune_increment_trans_time") {
			this->tune_increment_trans_time = std::stod(attr_value);
		} else if (attr_name == "tune_increment_normal") {
			this->tune_increment_normal = std::stoi(attr_value);
		} else if (attr_name == "tune_increment_fast") {
			this->tune_increment_fast = std::stoi(attr_value);
		} else if (attr_name == "softfm_path") {
			this->softfm_path = attr_value;
		} else if (attr_name == "softfm_args") {
			this->softfm_args = attr_value;
		} else {
			std::cout << "No match for (" << attr_name << ") = (" << attr_value << ")" << std::endl;
		}
	}
	configFile.close();

	return true;
}

void Config::print_config() {
	std::cout << "======Beg Config=====" << std::endl;
	std::cout << "idle_time=" << this->idle_time << std::endl;
	std::cout << "brightness_idle=" << this->brightness_idle << std::endl;
	std::cout << "brightness_active=" << this->brightness_active << std::endl;
	std::cout << "frequency_min=" << this->frequency_min << std::endl;
	std::cout << "frequency_max=" << this->frequency_max << std::endl;
	std::cout << "frequency_display_format=" << this->frequency_display_format << std::endl;
	std::cout << "tune_kill_sleep_time=" << this->tune_kill_sleep_time << std::endl;
	std::cout << "tune_increment_trans_time=" << this->tune_increment_trans_time << std::endl;
	std::cout << "tune_increment_normal=" << this->tune_increment_normal << std::endl;
	std::cout << "tune_increment_fast=" << this->tune_increment_fast << std::endl;
	std::cout << "softfm_path=" << this->softfm_path << std::endl;
	std::cout << "softfm_args=" << this->softfm_args << std::endl;
	std::cout << "======End Config=====" << std::endl;
}