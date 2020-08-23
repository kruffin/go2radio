#include <string>

class Config {
public:
	std::string file_path;
	double idle_time;
	int brightness_idle;
	int brightness_active;
	int frequency_min;
	int frequency_max;
	int frequency_start;
	std::string frequency_display_format;
	double tune_kill_sleep_time;
	double tune_increment_trans_time;
	int tune_increment_normal;
	int tune_increment_fast;
	std::string softfm_path;
	std::string softfm_args;

	int bookmark_margin;
	int bookmark_cols;
	int bookmark_separation;

	Config(std::string file);
	~Config();
	void save();
	bool load();
	void print_config();
};