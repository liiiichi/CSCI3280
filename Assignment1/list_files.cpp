#include "stdio.h"
#include <filesystem>
#include <iostream>
#include "list_files.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;

void list_files(string const &directory, string const &ext, vector<string> &file_path_vector, bool verbose=false)
{
	if(!std::filesystem::is_directory(directory))
	{
		cout << "Directory " << directory << " does not exist";
		return;
	}
	for (auto& p : std::filesystem::recursive_directory_iterator(directory))
	{
		if (p.path().extension() == ext)
		{
			if(verbose)
				cout << p << endl;
			file_path_vector.push_back(p.path().string());
		}
	}
	return;
}