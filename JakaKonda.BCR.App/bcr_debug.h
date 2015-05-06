#ifndef BCR_DEBUG_H
#define BCR_DEBUG_H

#define BCR_DEBUG_OFF			0x0000
#define BCR_DEBUG_EDGES			0x0001
#define BCR_DEBUG_LINES			0x0002
#define BCR_DEBUG_POINTS		0x0004
#define BCR_DEBUG_QUADS			0x0008
#define BCR_DEBUG_PERSP_TRANF	0x0010
#define BCR_DEBUG_TEXT_BOXES	0x0020
#define BCR_DEBUG_OCR_RES		0x0040
#define BCR_DEBUG_TXT_GROUPS	0x0080
#define BCR_DEBUG_NR_GROUPS		0x0100

#define BCR_DEBUG_HEADER_LINE	std::cout << "==============================" << std::endl;
#define BCR_DEBUG_HEADER_HASH	std::cout << "##############################" << std::endl;
#define BCR_DEBUG_HEADER(X)	\
		{ \
		std::cout << std::endl << std::endl; \
		BCR_DEBUG_HEADER_HASH \
		std::cout << "# DEBUG: " << X << std::endl; \
		BCR_DEBUG_HEADER_HASH \
		}

#define BCR_DEBUG (0)

#endif // BCR_DEBUG_H