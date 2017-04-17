#include "stdafx.h"
#include "HairConfiguration.h"
#include "HairAnim2Simulator.h"
#include "HairLoader.h"
#include "XRwy_h.h"

using namespace XRwy;

namespace HairCore {
	HairAnim2Simulator::HairAnim2Simulator(const HairConfiguration &conf): HairSimulator(conf) {
		//use some tricky way to avoid code change
		g_paramDict["particleperstrand"] = "25";
		g_paramDict["hairsample"] = "1";
		g_paramDict["hairsamplegroup"] = "0";
		g_paramDict["hairsamplegroupseed"] = "0";
		anim_loader.loadFile(conf.get_ref_file_path().c_str(), &hair);
	}

	void HairAnim2Simulator::on_frame(float trans[16]) {
		anim_loader.nextFrame();
	}
}