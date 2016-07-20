#include "ReconsReader.h"

namespace XRwy
{
	namespace
	{
		enum { iGuideHair, iInitialFrame, iWeights, iGroup, iNeighboring, iInterpolation };
	}

	ReconsReader::ReconsReader()
	{
		set_nFrame(-1);
		set_curFrame(-1);
	}

	ReconsReader::~ReconsReader()
	{
		if (file && file.is_open())
			file.close();
	}

	bool ReconsReader::loadFile(const char* fileName, SkinningInfo* skinHair)
	{
		hair = skinHair;
		file = std::ifstream(fileName, std::ios::binary);

		if (!file.is_open()) throw std::exception("file not found!");

		hair->restState = new HairGeometry;
		hair->guidances = new BlendHairGeometry;

		readHead(hair->restState);
		readHead2(hair->guidances);

		readShortcut();

		setupGuideHair();
		setupRestState();
		setupWeights();

		if (fileShortcut[iGroup] > 0)
			setupGroupSection();

		if (fileShortcut[iNeighboring] > 0)
			setupNeighboringSection();

		if (fileShortcut[iInterpolation] > 0)
			setupInterpolationSection();
	}

}