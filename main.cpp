
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <ctime>

#include <Ogre.h>
#include <OgreFrameListener.h>
#include <OIS/OIS.h>
#include <SdkCameraMan.h>
#include <SdkTrays.h>

using namespace std;
using namespace Ogre;


static const size_t Nparticles = 2000;
static SceneNode* particle_nodes[Nparticles];
static Entity* particle_entities[Nparticles];

static float *x, *y, *z,
			 *vx, *vy, *vz;

extern void process_particles(float* x, float* y, float* z,
	float* vx, float* vy, float*vz, size_t count, float tau);

class App : public FrameListener
{
public:
	App(OIS::Keyboard* keyboard,  OIS::Mouse* mouse,
		OgreBites::SdkCameraMan* cameraMan, OgreBites::SdkTrayManager* trayMgr) :
		 mKeyboard(keyboard), mMouse(mouse), mCameraMan(cameraMan), mTrayMgr(trayMgr) { }

	bool frameStarted(const FrameEvent& evt)
	{
		//update the input devices
		mKeyboard->capture();
		mMouse->capture();
		//exit if escape pressed
		if(mKeyboard->isKeyDown(OIS::KC_ESCAPE))
			return false;
		return true;
	}

	bool frameRenderingQueued (const FrameEvent &evt)
	{
		mCameraMan->frameRenderingQueued(evt);
		mTrayMgr->frameRenderingQueued(evt);
		process_particles(x, y, z, vx, vy, vz, Nparticles, 1e-1f);
		Vector3 v;
		for(int i = 0; i < Nparticles; i++)
		{
			v.x = x[i]; v.y = y[i]; v.z = z[i];
			particle_nodes[i]->setPosition(v);
		}
		return true;
	}

private:
	OIS::Keyboard* mKeyboard;
	OIS::Mouse* mMouse;
	OgreBites::SdkCameraMan* mCameraMan;
	OgreBites::SdkTrayManager* mTrayMgr;
};

class KeyListener : public OIS::KeyListener
{
public:
	KeyListener(OgreBites::SdkCameraMan* cameraMan) : mCameraMan(cameraMan) {}

	bool keyPressed(const OIS::KeyEvent& e)
	{
		mCameraMan->injectKeyDown(e);
		return true;
	}

	bool keyReleased(const OIS::KeyEvent& e)
	{
		mCameraMan->injectKeyUp(e);
		return true;
	}
private:
	OgreBites::SdkCameraMan* mCameraMan;
};

class MouseListener : public OIS::MouseListener
{
public:
	MouseListener(OgreBites::SdkCameraMan* cameraMan) : mCameraMan(cameraMan) {}

	bool mouseMoved(const OIS::MouseEvent& e)
	{
		mCameraMan->injectMouseMove(e);
		return true;
	}

	bool mousePressed(const OIS::MouseEvent& e, OIS::MouseButtonID id)
	{
		mCameraMan->injectMouseDown(e, id);
		return true;
	}

	bool mouseReleased(const OIS::MouseEvent& e, OIS::MouseButtonID id)
	{
		mCameraMan->injectMouseUp(e, id);
		return true;
	}

private:
	OgreBites::SdkCameraMan* mCameraMan;
};


static void init_particles(SceneManager* sceneManager, size_t count, float r0 = 0, float delta_r = 0, float v0 = 0, float delta_v = 0)
{
	printf("Initializing particles... ");
	srand(unsigned(time(NULL)));

	x = new float[count];
	memset(x, 0, count * sizeof(float));
	y = new float[count];
	memset(y, 0, count * sizeof(float));
	z = new float[count];
	memset(z, 0, count * sizeof(float));
	vx = new float[count];
	memset(vx, 0, count * sizeof(float));
	vy = new float[count];
	memset(vy, 0, count * sizeof(float));
	vz = new float[count];
	memset(vz, 0, count * sizeof(float));
	for(size_t i = 0; i < count; i++)
	{
		x[i]  = r0 + delta_r * (rand() - RAND_MAX / 2) / float(RAND_MAX);
		y[i]  =      delta_r * (rand() - RAND_MAX / 2) / float(RAND_MAX);
		z[i]  =      delta_r * (rand() - RAND_MAX / 2) / float(RAND_MAX);
		vx[i] =      delta_v * (rand() - RAND_MAX / 2) / float(RAND_MAX);
		vy[i] = v0 + delta_v * (rand() - RAND_MAX / 2) / float(RAND_MAX);
		vz[i] =      0;//delta_v * (rand() - RAND_MAX / 2) / float(RAND_MAX);
		particle_entities[i] = sceneManager->createEntity(StringConverter::toString(i), SceneManager::PT_CUBE);
		particle_nodes[i] = sceneManager->getRootSceneNode()->createChildSceneNode(StringConverter::toString(i));
		particle_nodes[i]->setScale(0.01f, 0.01f, 0.01f);
		particle_nodes[i]->attachObject(particle_entities[i]);
	}
	printf("done. \n");
}


int main()
{
	Root* root = new Root;
	ResourceGroupManager::getSingleton().addResourceLocation(
		"data", "FileSystem");
	ResourceGroupManager::getSingleton().addResourceLocation(
		"data/SdkTrays.zip", "Zip");
	if(!root->restoreConfig() && !root->showConfigDialog())
	{
		delete root;
		return EXIT_SUCCESS;
	}

	RenderWindow* window = root->initialise(true, "Particles");
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	SceneManager* sceneMgr = root->createSceneManager(ST_GENERIC);
	Camera* camera = sceneMgr->createCamera("Camera");
	camera->setNearClipDistance(0.01f);
	camera->setPosition(Ogre::Vector3(0,10,500));
	camera->lookAt(Ogre::Vector3(0,0,0));
	OgreBites::SdkCameraMan* cameraMan = new OgreBites::SdkCameraMan(camera);
	cameraMan->setStyle(OgreBites::CS_ORBIT);
	Viewport* viewPort = window->addViewport(camera);

	init_particles(sceneMgr, Nparticles, 40, 10, 8, 2);

	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;
	window->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
	OIS::InputManager* inputManager = OIS::InputManager::createInputSystem(pl);
	OIS::Keyboard* keyboard = static_cast<OIS::Keyboard*>(inputManager->createInputObject(OIS::OISKeyboard, true));
	OIS::Mouse* mouse = static_cast<OIS::Mouse*>(inputManager->createInputObject(OIS::OISMouse, true));
	unsigned int width, height, depth;
	int top, left;
	window->getMetrics(width, height, depth, left, top);
	const OIS::MouseState &ms = mouse->getMouseState();
	ms.width = width;
	ms.height = height;

	OgreBites::SdkTrayManager* trayMgr = new OgreBites::SdkTrayManager("TrayMgr", window, mouse);
	trayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
	trayMgr->hideCursor();

	KeyListener* keyListener = new KeyListener(cameraMan);
	keyboard->setEventCallback(keyListener);
	MouseListener* mouseListener = new MouseListener(cameraMan);
	mouse->setEventCallback(mouseListener);
	App* frameListener = new App(keyboard, mouse, cameraMan, trayMgr);
	root->addFrameListener(frameListener);

	Entity* zero = sceneMgr->createEntity(Ogre::SceneManager::PT_SPHERE);
	zero->setMaterialName("Red");
	SceneNode* zero_node = sceneMgr->getRootSceneNode()->createChildSceneNode();
	zero_node->setScale(0.02f, 0.02f, 0.02f);
	zero_node->attachObject(zero);
	cameraMan->setTarget(zero_node);

	sceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
	sceneMgr->createLight("MainLight")->setPosition(20,80,50);

	root->startRendering();

	inputManager->destroyInputObject(mouse);
	inputManager->destroyInputObject(keyboard);
	OIS::InputManager::destroyInputSystem(inputManager);
	delete frameListener;
	delete root;

	return EXIT_SUCCESS;
}

