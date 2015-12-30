#pragma once
#include "linmath.h"


struct wrLevelsetOctreeNode
{
	size_t					level;
	vec3						corners[2];
	// index?
	wrLevelsetOctreeNode*	pParent;
	wrLevelsetOctreeNode*	children[8];
};

class wrLevelsetOctree
{
public:
	wrLevelsetOctree();
	~wrLevelsetOctree();

private:

	wrLevelsetOctreeNode*	pRoot = nullptr;
	size_t					nMaxLevel = 0;
	// cell list
	// vertex list
};

