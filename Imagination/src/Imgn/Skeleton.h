#pragma once
#include "ImgnMath.h"

namespace Imgn
{
	struct Joint
	{
		std::string name;
		int nodeIdx = -1, parentJoint = -1;
		mat4 inverseBindMatrix = Math::identity;
	};

	struct Skeleton
	{
		std::string name;
		int rootNode = -1;
		std::vector<Joint> joints;
		std::map<int, uint32_t> nodeToJoint;

		//void Update();
		//void UpdateJoint(int16_t pJoint);
	};
}