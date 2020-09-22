#pragma once

#include <fstream>
#include "List.h"

class LogFileWriter {

public:
	void Log(List* list) {
		while (list->CountNode() > 0) {
			output << list->PopNode() << std::endl;
		}
	}

private:
	std::ofstream output{ "output.txt" };
};
