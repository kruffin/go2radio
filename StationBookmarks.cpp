#include <iostream>
#include <fstream>
#include <algorithm>

#include "StationBookmarks.h"


StationBookmarks::StationBookmarks(std::string file) {
	this->file_path = file;
};
StationBookmarks::~StationBookmarks() {

};

bool StationBookmarks::isMarked(int bmark) {
	int idx = this->indexOf(bmark);
	if (-1 != idx) {
		return true;
	} else {
		return false;
	}
};

void StationBookmarks::add(int bmark) {
	if (this->isMarked(bmark)) {
		return; // don't add it again.
	}

	this->bookmarks.push_back(bmark);

	std::sort(this->bookmarks.begin(), this->bookmarks.end());
};
void StationBookmarks::remove(int bmark) {
	int idx = this->indexOf(bmark);
	if (-1 != idx) {
		this->bookmarks.erase(this->bookmarks.begin() + idx);
	}
};

int StationBookmarks::size() {
	return this->bookmarks.size();
};

int StationBookmarks::indexOf(int bmark) {
	for (int i = 0; i < this->bookmarks.size(); ++i) {
		if (this->bookmarks[i] == bmark) {
			return i;
		}
	}
	return -1;
};

int StationBookmarks::get(int idx) {
	return this->bookmarks[idx];
};

int StationBookmarks::next(int bmark) {
	int idx = this->indexOf(bmark);
	if (-1 == idx) {
		// find closest
		int shortest_dist = -1;
		int shortest_mark = -1;
		for (int i = 0; i < this->bookmarks.size(); ++i) {
			int cur_dist = abs(this->bookmarks[i] - bmark);
			if (cur_dist < shortest_dist || shortest_mark == -1) {
				shortest_mark = this->bookmarks[i];
				shortest_dist = cur_dist;
			}
		}

		return shortest_mark;
	}

	++idx;
	if (idx >= this->bookmarks.size()) {
		idx = 0; // wrap around
	}
	return this->bookmarks[idx];
};
int StationBookmarks::prev(int bmark) {
	int idx = this->indexOf(bmark);
	if (-1 == idx) {
		// find closest
		int shortest_dist = -1;
		int shortest_mark = -1;
		for (int i = 0; i < this->bookmarks.size(); ++i) {
			int cur_dist = abs(this->bookmarks[i] - bmark);
			if (cur_dist < shortest_dist || shortest_mark == -1) {
				shortest_mark = this->bookmarks[i];
				shortest_dist = cur_dist;
			}
		}

		return shortest_mark;
	}
	
	--idx;
	if (idx < 0) {
		idx = this->bookmarks.size() - 1; // wrap around
	}

	return this->bookmarks[idx];
};

void StationBookmarks::save() {
	std::ofstream bmarksFile;
	bmarksFile.open(this->file_path);

	for (int i = 0; i < this->size(); ++i) {
		bmarksFile << this->get(i) << std::endl;
	}

	bmarksFile.close();
};
bool StationBookmarks::load() {
	std::ifstream bmarksFile;
	bmarksFile.open(this->file_path);
	if (bmarksFile.fail()) {
		return false;
	}

	std::string line;
	while (std::getline(bmarksFile, line)) {
		this->add(std::stoi(line));
	}
	bmarksFile.close();

	return true;
};

// std::vector<int> bookmarks;
// std::string file_path;