#pragma once
#include <iostream>
#include <vector>

using namespace std;

struct SHPHeader {
	int32_t fileCode;
	int32_t fileLen;
	int32_t version;
	int32_t SHPType;

	double Xmin;
	double Ymin;
	double Xmax;
	double Ymax;
	double Zmin;
	double Zmax;
	double Mmin;
	double Mmax;
};
