//
//   CS 134 - Midterm Starter File - Fall 2018
//
//
//   This file contains all the necessary startup code for the Midterm problem Part II
//   Please make sure to you the required data files installed in your $OF/data directory.
//
//	Student: Luis Pamintuan
//
//
//                                             (c) Kevin M. Smith  - 2018
#pragma once

#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "box.h"
#include "ray.h"
#include "Octree.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void loadVbo();


		/*-----------------------------------------------------------------------------*/
		//Imported from Terrain assignment
		void setCameraTarget();
		bool  doPointSelection();
		void drawBox(const Box &box);
		Box meshBounds(const ofMesh &);
		void subDivideBox8(const Box &b, vector<Box> & boxList);
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;

		bool bRoverLoaded;
		bool bTerrainSelected;

		ofVec3f selectedPoint;
		ofVec3f intersectPoint;
		float altitude;
		Octree oct;
		Octree octShip;


		const float selectionRange = 4.0;

		/*-----------------------------------------------------------------------------*/

		ofEasyCam cam;
		ofEasyCam landerCam;
		ofxAssimpModelLoader lander, mars; // Mars == Moon for the sake of consistency
		ofLight light;
		Box boundingBox;
		Box landerBox;
		vector<Box> level1, level2, level3;
		ofImage backgroundImage;
		ofCamera *theCam = NULL;
//		ofCamera topCam;
		ofCamera sideCam;
		ofCamera botCam;
		ofCamera farCam;
		ofCamera groundCam;

		ofLight sunLight;


		//Forces
		TurbulenceForce *turbForce;
		GravityForce *gravityForce;
		ImpulseRadialForce *radialForce;
		TurbulenceForce *turbForce1;
		ThrusterForce *thrustForce;
		ImpulseForce *impulseForce;

		ParticleEmitter emitter;
		ParticleSystem system;
		Particle x;
		

		//Exists in Terrain ofApp.h
		//bool bAltKeyDown;
		//bool bCtrlKeyDown;
		//bool bWireframe;
		//bool bDisplayPoints;
	
		bool bBackgroundLoaded = false;
		bool bLanderLoaded = false;
		bool forwardDir = false;
		bool backDir = false;

		//Octree oct;
		ofVec3f rayPoint;
		ofVec3f rayDir;
		Ray ray;

		//Sound Effects
		ofSoundPlayer thruster;
		
		//Textures and Shaders
		ofVbo vbo;
		ofShader shader;
		ofTexture particleTex;

};
