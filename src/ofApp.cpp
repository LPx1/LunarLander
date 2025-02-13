//
//   CS 134 - Final MoonLander Project - Fall 2018
//
//
//   This file contains all the necessary startup code for the Midterm problem Part II
//   Please make sure to you the required data files installed in your $OF/data directory.
//
//                                             (c) Kevin M. Smith  - 2018
//
//
//	Student: Luis Pamintuan
// 
//


#include "ofApp.h"

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){

	ofDisableArbTex();

	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;

	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	cam.disableMouseInput();

	landerCam.setDistance(10);
	landerCam.setNearClip(.1);
	landerCam.setFov(65.5);
	
	farCam.setPosition(ofVec3f(80, -20, -50));
	farCam.lookAt(ofVec3f(-70, 0, 50));

	groundCam.setPosition(ofVec3f(10, -30, -50));
	groundCam.lookAt(ofVec3f(10, -40, 0));
	groundCam.setNearClip(.1);


	//Camera set up to look down to the surface or whats under the lander (F3)
	botCam.setNearClip(.1);
	botCam.setFov(65.5);   
	botCam.setPosition(0, -3, 0);
	botCam.lookAt(glm::vec3(0, -10, 0));

	//Camera set up to look at a side view of the lander (F2)
	sideCam.setNearClip(.1);
	sideCam.setFov(65.5);
	sideCam.setPosition(0, 0, -3);
	sideCam.lookAt(glm::vec3(0, 0, 0));

	sunLight.setup();
	sunLight.enable();
	sunLight.setAreaLight(2, 2);
	sunLight.setAmbientColor(ofFloatColor(10, 10, 10));
//	sunLight.setDiffuseColor(ofFloatColor(100, 100, 100));
//	sunLight.setSpecularColor(ofFloatColor(100, 100, 00));
//	sunLight.set

	// set current camera;
	theCam = &cam;
	
	ofSetVerticalSync(true);
	ofEnableSmoothing();
	ofEnableDepthTest();

	//load textures
	//
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	// load the shader
//
#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
#else
	shader.load("shaders/shader");
#endif


	// load BG image
	//
	bBackgroundLoaded = backgroundImage.load("images/starfield-plain.jpg");

	// Load sound
	
	if (thruster.load("sounds/atmosphere3.mp3")) {

	}
	else {
		printf("Error: Unable to load  'atmosphere3.mp3' \n");
	}

	// load moon terrain
	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	// 	The maximum number of stack frames supported by Visual Studio has been exceeded.
	// (mars.loadModel("geo/moon-houdini.obj"))

	if (mars.loadModel("geo/moon-crater-v1.obj")) {
		mars.setScaleNormalization(false);
		mars.setRotation(0, 180, 0, 0, 1);
		mars.setPosition(0, 0, 0);
	}
	else
	{
		printf("Error: Unable to load model 'geo/moon-crater-v1.obj' \n");
		ofExit(0);
	}

	// load lander model
	//
	if (lander.loadModel("geo/lander.obj")) {
		lander.setScaleNormalization(false);
		lander.setScale(.5, .5, .5); 
		lander.setRotation(0, -180, 1, 0, 0);
		lander.setPosition(0,0,0);
		
		bLanderLoaded = true;
	}
	else {
		cout << "Error: Can't load model" << "geo/lander.obj" << endl;
		ofExit(0);
	}

	//Bounding boxes
	boundingBox = meshBounds(mars.getMesh(0));
	landerBox = meshBounds(mars.getMesh(0));
	oct.create(mars.getMesh(0),6);
	octShip.create(lander.getMesh(0), 6);

	//Initialize Forces
	turbForce = new TurbulenceForce(ofVec3f(-10, -10, -10), ofVec3f(10, 10, 10));
	gravityForce = new GravityForce(ofVec3f(0, -.098, 0)); //Moon's gravity -1.6
	radialForce = new ImpulseRadialForce(20.0);
	thrustForce = new ThrusterForce();
	turbForce1 = new TurbulenceForce(ofVec3f(-1, -1, -1), ofVec3f(1, 1, 1));
	impulseForce = new ImpulseForce();

	//Create particle to drive the model and add forces on particle for ship
	x.lifespan = 10000000000;
	system.add(x);
	system.addForce(turbForce1);
	system.addForce(thrustForce);
	system.addForce(impulseForce);
	system.addForce(gravityForce);


//	emitter.sys->addForce(turbForce);
	emitter.sys->addForce(gravityForce);
	emitter.sys->addForce(radialForce);
//	emitter.sys->addForce(thrustForce);
	emitter.setVelocity(lander.getPosition());
	emitter.setEmitterType(DiscEmitter);
	emitter.setGroupSize(100);
	emitter.setParticleRadius(.02);
	emitter.setOneShot(true);
	emitter.particleColor = ofColor::red;

}

// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo() {
	if (emitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < emitter.sys->particles.size(); i++) {
		points.push_back(emitter.sys->particles[i].position);
		//sizes.push_back(ofVec3f(radius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}


void ofApp::update() {
	
	// Draw ray from Ship to a Point on the surface
	rayPoint = ofVec3f(botCam.getPosition().x,
		botCam.getPosition().y - 100,
		botCam.getPosition().z);
	rayDir = rayPoint - botCam.getPosition();
	rayDir.normalize();
	ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	TreeNode intersect;
	oct.intersect(ray, oct.root, intersect);
	if (intersect.points.size() > 0) {
		selectedPoint = mars.getMesh(0).getVertex(intersect.points[0]);
	//	bPointSelected = true;
		
		//Environment flipped?
		altitude = 100 - (selectedPoint.y - rayPoint.y);
		//printf_s("%f\n", 100 - (selectedPoint.y - rayPoint.y));

		//When landed remove all forces
		if ((selectedPoint.y - rayPoint.y) > 100) {
			ofVec3f power = system.particles[0].velocity;
		//	printf("Velocity: %f \n", power);
			impulseForce->apply(-ofGetFrameRate() * power);
			
			//Remove particle forces when landed
			gravityForce->set(ofVec3f(0, 0, 0));
		}
		//After a certain altitude remove turbulance
		else if (100 - (selectedPoint.y - rayPoint.y) < 1 && 
			100 -(selectedPoint.y - rayPoint.y) > 10) {
			turbForce1->set(ofVec3f(0, 0, 0), ofVec3f(0, 0, 0));

		}
		else {
			//Reset forces 
			gravityForce->set(ofVec3f(0, -0.098, 0));
			turbForce1->set(ofVec3f(-1, -1, -1), ofVec3f(1, 1, 1));
		}
	}
	else {
		//When lander is off map 
		altitude = 0;
	}


	//Updates the positions of cameras following the lander

	ofVec3f place = lander.getPosition();
	groundCam.lookAt(ofVec3f(place.x, -50 -(place.y),place.z));

	botCam.setPosition(system.particles[0].position.x,
		system.particles[0].position.y ,
		system.particles[0].position.z);

	sideCam.setPosition(system.particles[0].position.x,
		system.particles[0].position.y,
		system.particles[0].position.z - 3);



	landerCam.setPosition(lander.getPosition().x,
		lander.getPosition().y + 3,
		lander.getPosition().z + 5);
	landerCam.setTarget(lander.getPosition());


	//Plane is flipped 
	cam.setTarget(ofVec3f(place.x, (place.y), place.z));
	

	//Updates the position of the lander following a particle with forces enacted on it

	lander.setPosition(system.particles[0].position.x,
		system.particles[0].position.y,
		system.particles[0].position.z);


	emitter.setPosition(glm::vec3(system.particles[0].position.x,
		system.particles[0].position.y,
		system.particles[0].position.z));

	x.position = ofVec3f(system.particles[0].position.x,
		system.particles[0].position.y ,
		system.particles[0].position.z);

	emitter.update();
	system.update();
}

//--------------------------------------------------------------
void ofApp::draw() {
	loadVbo();

	//	ofBackgroundGradient(ofColor(20), ofColor(0));   // pick your own backgroujnd
	//	ofBackground(ofColor::black);
	if (bBackgroundLoaded) {
		ofPushMatrix();
		ofDisableDepthTest();
		ofSetColor(50, 50, 50);
		ofScale(2, 2);
		backgroundImage.draw(-200, -100);
		ofEnableDepthTest();
		ofPopMatrix();
	}

	ofSetColor(255, 100, 90);
//	ofSetColor(ofColor::red);

	// this makes everything look glowy :)
	//
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();

//	shader.begin();
	theCam->begin();
	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		if (bLanderLoaded) {
			lander.drawWireframe();
		}
	}
	else {
		ofEnableLighting();              // shaded mode
		if (bLanderLoaded) {
			lander.drawFaces();

		}
	}

	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();

		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();

		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}


	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}

	ofNoFill();
	ofSetColor(ofColor::white);

	
	ofPopMatrix();

//	emitter.draw();

	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)emitter.sys->particles.size());
	particleTex.unbind();
//	system.draw();

//	oct.draw(oct.root, 3, 1, 0);

//	x.draw();
//	ofDrawLine(rayPoint, rayDir);
	
	ofPopMatrix();
	theCam->end();
//	shader.end();

	ofDisablePointSprites();
	ofDisableBlendMode();
	ofEnableAlphaBlending();

	// draw screen data
	//
	string str;
	str += "Frame Rate: " + std::to_string(ofGetFrameRate());
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);

	string alt;
	alt += "Altitude (AGL): " + std::to_string(altitude);
	ofSetColor(ofColor::white);
	ofDrawBitmapString(alt, ofGetWindowWidth() - 200, 30);

}


// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'P':
	case 'p':
		{
		ofVec3f pos = ofVec3f(lander.getPosition().x + 2,
		lander.getPosition().y + 2,
		lander.getPosition().z);
		cam.setPosition(pos);
		cam.lookAt(ofVec3f(pos.x - 2, pos.y - 2, pos.z));
		break; }
		
	case 'r':
		cam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 'D':
	case 'd':

	case 't':
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
//	case 'A':
//	case 'a':
//	{
//		ofVec3f power = system.particles[0].velocity;
//		printf("Velocity: %f \n", power);
//		impulseForce->apply(-60 * power);
////		thrustForce->up = true;
//		break;
//	}
	case 'w':
		toggleWireframeMode();
		break;
	case OF_KEY_F1:
		theCam = &cam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &botCam;
		break;
	case OF_KEY_F4:
		theCam = &landerCam;
		break;
	case OF_KEY_F5:
		theCam = &farCam;
		break;
	case OF_KEY_F6:
		theCam = &groundCam;
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		forwardDir = true;
		backDir = true;
		break;
	case OF_KEY_DEL:
		break;
	case OF_KEY_UP:		
		if (backDir == true)
			thrustForce->back = true;
		else {
			thrustForce->up = true;
			emitter.sys->reset();
			emitter.start();
			if (thrustForce->up == true) {
				thruster.setLoop(true);
				if (thruster.getIsPlaying()) {

				}
				else {
					thruster.play();
				}
			}
		}

		break;
	case OF_KEY_DOWN:
		if (forwardDir == true) {
			thrustForce->forward = true;
		}
		else
			thrustForce->down = true;
		break;
	case OF_KEY_LEFT:
		thrustForce->left = true;
		break;
	case OF_KEY_RIGHT:
		thrustForce->right = true;
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}


void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	case OF_KEY_UP:
		thrustForce->up = false;
		thrustForce->back= false;
		thruster.setLoop(false);
		thruster.stop();
		thrustForce->clear();
		break;
	case OF_KEY_DOWN:
		thrustForce->down = false;
		thrustForce->forward = false;
		thrustForce->clear();
		break;
	case OF_KEY_LEFT:
		thrustForce->left = false;
		thrustForce->clear();
		break;
	case OF_KEY_RIGHT:
		thrustForce->right = false;
		thrustForce->clear();
		break;
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		forwardDir = false;
		backDir = false;
		break;
	//case 'A':
	//case 'a':
	//{
	//	thrustForce->up = false;
	//	break;
	//}
	default:
		break;

	}
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {


}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {


}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ .7f, .7f, .7f, 1.0f };

	static float position[] =
	{20.0, 20.0, 20.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
//	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
//	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

//TERRAIN IMPORTED METHODS
/*--------------------------------------------------------------------------------*/

//
//  ScreenSpace Selection Method: 
//  This is not the octree method, but will give you an idea of comparison
//  of speed between octree and screenspace.
//
//  Select Target Point on Terrain by comparing distance of mouse to 
//  vertice points projected onto screenspace.
//  if a point is selected, return true, else return false;
//
bool ofApp::doPointSelection() {

	ofMesh mesh = mars.getMesh(0);
	int n = mesh.getNumVertices();
	float nearestDistance = 0;
	int nearestIndex = 0;

	bPointSelected = false;

	ofVec2f mouse(mouseX, mouseY);
	vector<ofVec3f> selection;

	// We check through the mesh vertices to see which ones
	// are "close" to the mouse point in screen space.  If we find 
	// points that are close, we store them in a vector (dynamic array)
	//
	for (int i = 0; i < n; i++) {
		ofVec3f vert = mesh.getVertex(i);
		ofVec3f posScreen = cam.worldToScreen(vert);
		float distance = posScreen.distance(mouse);
		if (distance < selectionRange) {
			selection.push_back(vert);
			bPointSelected = true;
		}
	}

	//  if we found selected points, we need to determine which
	//  one is closest to the eye (camera). That one is our selected target.
	//
	if (bPointSelected) {
		float distance = 0;
		for (int i = 0; i < selection.size(); i++) {
			ofVec3f point = cam.worldToCamera(selection[i]);

			// In camera space, the camera is at (0,0,0), so distance from 
			// the camera is simply the length of the point vector
			//
			float curDist = point.length();

			if (i == 0 || curDist < distance) {
				distance = curDist;
				selectedPoint = selection[i];
			}
		}
	}
	return bPointSelected;
}

//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void ofApp::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}


//draw a box from a "Box" class  
//
void ofApp::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}

// return a Mesh Bounding Box for the entire Mesh
//
Box ofApp::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}


//bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
//	glm::vec3 mouse(mouseX, mouseY, 0);
//	ofVec3f rayPoint = cam.screenToWorld(mouse);
//	ofVec3f rayDir = rayPoint - cam.getPosition();
//	rayDir.normalize();
//	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
//}