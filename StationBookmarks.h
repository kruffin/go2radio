#pragma once

#include <vector>
#include <string>

class StationBookmarks {
public:
	std::string file_path;

	StationBookmarks(std::string file);
	~StationBookmarks();

	bool isMarked(int bmark);
	void add(int bmark);
	void remove(int bmark);
	int size();
	int indexOf(int bmark);
	int get(int idx);

	int next(int bmark);
	int prev(int bmark);

	void save();
	bool load();
private:
	std::vector<int> bookmarks;
};