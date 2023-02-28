#include "..\\shapefil.h"
#include <iostream>

using namespace std;

int main()
{
	// Shapefile을 읽어들이는 예시 코드
	const char* pszPath = "C:\\example\\TN_BULD_부산광역시_연제구.shp";
	SHPHandle hSHP = SHPOpen(pszPath, "rb");

	int nShapeCount;
	SHPGetInfo(hSHP, &nShapeCount, NULL, NULL, NULL);
	std::cout << "nShapeCount: " << nShapeCount << endl;

	int cnt = 0;
	int maxIdx = 0;
	int maxVert = 0;
	
	for (size_t i = 0; i < nShapeCount; ++i) {
		SHPObject* psShape = SHPReadObject(hSHP, i);

		if (psShape->nVertices != 8) {
			++cnt;
			if (std::max(psShape->nVertices, maxVert) > maxVert) {
				maxVert = psShape->nVertices;
				maxIdx = i;
			}
		}

		SHPDestroyObject(psShape);
		continue;

		//{
		//	std::cout << "nSHPType: " << psShape->nSHPType << std::endl;
		//	std::cout << "nShapeId: " << psShape->nShapeId << std::endl;
		//	std::cout << "nParts: " << psShape->nParts << std::endl;
		//	std::cout << "nVertices: " << psShape->nVertices << std::endl;

		//	for (int i = 0; i < psShape->nVertices; i++) {
		//		std::cout << "X: " << psShape->padfX[i] << " ";
		//		std::cout << "Y: " << psShape->padfY[i] << " ";
		//		std::cout << "Z: " << psShape->padfZ[i] << " ";
		//		std::cout << "M: " << psShape->padfM[i] << std::endl;
		//	}

		//	for (int i = 0; i < psShape->nParts; i++) {
		//		std::cout << "Part " << i << " (Start Vertex Index): " << psShape->panPartStart[i] << std::endl;
		//	}
		//}

		//SHPDestroyObject(psShape);
	}
	

	SHPObject* psShape = SHPReadObject(hSHP, maxIdx);
	
	for (int i = 0; i < psShape->nParts; i++) {
		std::cout << "Part " << i << " (Start Vertex Index): " << psShape->panPartStart[i] << std::endl;
	}std::cout << cnt << "/" << maxVert << endl;

	SHPDestroyObject(psShape);

	SHPClose(hSHP);
	return 0;
}
