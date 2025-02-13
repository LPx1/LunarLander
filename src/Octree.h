#pragma once
#include "ofMain.h"
#include "box.h"
#include "ray.h"


class TreeNode {
public:
	Box box;
	vector<int> points;
	vector<TreeNode> children;
};

class Octree {
public:
	
	void create(const ofMesh & mesh, int numLevels);
	void subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level);
	bool intersect(const Ray &, const TreeNode & node, TreeNode & nodeRtn);
	void draw(TreeNode & node, int numLevels, int level,int color);
	void draw(int numLevels, int level) {
		draw(root, numLevels, level,0);
	}
	void drawLeafNodes(TreeNode & node);
	static void drawBox(const Box &box);
	static Box meshBounds(const ofMesh &);
	int getMeshPointsInBox(const ofMesh &mesh, const vector<int> & points, Box & box, vector<int> & pointsRtn);
	void subDivideBox8(const Box &b, vector<Box> & boxList);
	vector<int> getIndtoInt(ofMesh m);

	ofMesh mesh;
	TreeNode root;

	//**MODIFIED
	ofColor colors[5] = { ofColor::blue, ofColor::red, ofColor::green,
		ofColor::purple, ofColor::gold };
};