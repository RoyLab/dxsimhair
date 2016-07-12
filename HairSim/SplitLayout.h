#pragma once
#include "XRwy_h.h"

#define MAX_SPLIT_LAYOUT_NUMBER 4

namespace XRwy
{
	class SplitLayout
	{
	public:
		SplitLayout(int n);
		~SplitLayout();

		int GetFrameNumber() const { return nFrame; }
		void BeginLayout();
		void EndLayout();
		void SetupFrame(int i);

	private:
		int nFrame = 0;
	};
}
