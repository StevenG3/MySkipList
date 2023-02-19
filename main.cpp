#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dump_file"

int main() {
	srand ((unsigned int)(time(NULL)));
	SkipList skip_list(6);
	skip_list.InsertElement("Steven", 1);
	skip_list.InsertElement("Jobs", 3);
	skip_list.InsertElement("Justin", 7);
	skip_list.InsertElement("Bibber", 8);
	skip_list.InsertElement("Bibber", 8);
	skip_list.InsertElement("Abraham", 9);
	skip_list.InsertElement("Lincoln", 19);
	skip_list.InsertElement("Benjamin", 19);
	skip_list.InsertElement("Franklin", 20);

	std::cout << "skip_list length: " << skip_list.Length() << std::endl;

	skip_list.DumpFile();

    skip_list.SearchElement("Bibber", 8);
    skip_list.SearchElement("Benjamin", 20);

	skip_list.DisplayList();

#if 1
    skip_list.DeleteElement("Bibber", 8);
    skip_list.DeleteElement("Benjamin", 20);

    std::cout << "skip_list length: " << skip_list.Length() << std::endl;

    skip_list.DisplayList();
#endif

	return 0;
}