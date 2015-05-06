#include <iostream>
#include <sstream>

#ifdef _WIN32
	#include <Windows.h>
#else

#endif

#include "BCReader.h"

void init()
{
#ifdef _WIN32
	SetConsoleCP(1251);
#else

#endif
}


//void process_business_card_bulk(string path)
//{
//	for (int i = 12; i <= 50; i++)
//	{
//		std::ostringstream oss;
//		oss << "./test3/(" << i << ").jpg";
//		auto bcr = bcr::BCReader(oss.str());
//		bcr.process();
//
//		BCR_DEBUG_HEADER_LINE
//		bcr.getBusinessCard().print();
//		BCR_DEBUG_HEADER_LINE
//		bcr.getBusinessCard().show();
//	}
//}


void process_business_card(string path)
{
	auto bcr = bcr::BCReader(path);
	bcr.process();

	BCR_DEBUG_HEADER_LINE
	bcr.getBusinessCard().print();
	BCR_DEBUG_HEADER_LINE
	bcr.getBusinessCard().show();
}



int main(int argc, char *argv[])
{
	init();

	if (argc < 2)
		std::cout << "Pass path to photo as first argument" << endl;
	else
		process_business_card(argv[1]);
	
	//process_business_card_bulk("");

	return 0;
}