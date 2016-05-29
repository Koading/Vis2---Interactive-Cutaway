#include "vis2.h"
#include <iostream>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace vis2;
using namespace gl;

Vis2App::Vis2App() : 
	mCurrentShader(nullptr),                 
	mCutLabel("default"),
	mCutSelection(0),
	cutsLabelList({}),
	cutsList({}), 
	mShaderSetting(),                 
	mNumGridCells(20),
	mEnablePlaneCut(false),
	mPlaneCutParams(vec4(0.0f, 0.0f, 1.0f, 0.0f)),
	mPickedPoint(vec3(0.0f, 0.0f, 0.0f)),
	mSpaceParamU(0.0f),
	mSpaceParamV(0.0f),
	mSpaceParamW(0.0f),
	mSpaceX(0.0f),
	mSpaceY(0.0f),
	mSpaceZ(0.0f),
	mSpacePos(vec4(0.0, 0.0, 0.0, 1.0)),
	mCutAlpha(0.25),
	//mModelFile(".//..//assets//8lbs.obj"),
	mEnableSelect(false),
	mMousePos(vec2(0.0f, 0.0f)),
	mBackgroundColor(0.1f, 0.1f, 0.1f),
	mEnableFaceCulling(true),
	mShowGrid(true),
	mModelFile(".//..//assets//house.obj"),
	mModelMtl(".//..//assets//house.mtl"), mJsonFile(mModelFile + ".json"),
	mTextureType(TexType::CHECKERED),
	mCutType(cutType::BOX),
	mMouseDown(false),
	mFont(Font("Arial", 12.0f)),
	mFps("0.0")
{
	
}

void Vis2App::setup()
{
	//grid setup
	createGrid();
	createGridLoop();

	//camera setup
	mCamera.lookAt(normalize(vec3(60, 60, 120)) * 100.0f, mCameraTarget); 
	mCamUi = CameraUi(&mCamera);

	this->setupGLSL();
	this->setupUI();

	loadModel();

	/*
	geom::WireSphere();
	geom::WireCube;
	*/
	TriMesh::Format fmt = TriMesh::Format().positions().normals().texCoords().tangents();
	TriMesh mesh((geom::WireCube(vec3(1.0)), fmt));

	mCutWire = mesh;
	mCutWireBatch = Batch::create(mCutWire, gl::context()->getStockShader(gl::ShaderDef().color()));
	
	//create default cut
	this->createCut();
}

void Vis2App::setupGLSL()
{
	//gl::getStockShader

	try {
		mPhongShader = gl::GlslProg::create(
			/*
			loadAsset("phong.vert"), 
			loadAsset("phong.frag")
			*/
			loadFile(".//..//assets//phong.vert"),
			loadFile(".//..//assets//phong.frag")
			);

		mCurrentShader = &mPhongShader;
	}
	catch (cinder::Exception &ex)
	{
		CI_LOG_E("error loading shader: " << ex.what());
		mCurrentShader = &(gl::context()->getStockShader(gl::ShaderDef().color()));
	}

	try {
		mWireShader = gl::GlslProg::create(
			loadFile(".//..//assets//wireframe.vert"),
			loadFile(".//..//assets//wireframe.frag"), 
			loadFile(".//..//assets//wireframe.geom")
			);

	}
	catch (cinder::Exception &ex)
	{
		CI_LOG_E("error loading shader: " << ex.what());
		mWireShader = (gl::context()->getStockShader(gl::ShaderDef().color()));
		
	}

	

}




void Vis2App::resize()
{
	mCamera.setAspectRatio(getWindowAspectRatio());

	if (mWireframeShader)
		mWireframeShader->uniform("uViewportSize", vec2(getWindowSize()));	

	//setupUI();
}


void Vis2App::update()
{

}


void Vis2App::resetCam()
{
	mCameraLerpTarget = normalize(vec3(60, 60, 120)) * 100.0f;
	//mCamera.lookAt(normalize(vec3(60, 60, 120)) * 100.0f, mCameraTarget);
	
	// After creating a new primitive, gradually move the camera to get a good view.
	{
		float distance = glm::distance(mCamera.getEyePoint(), mCameraLerpTarget);
		vec3 eye = mCameraLerpTarget - lerp(distance, 5.0f, 0.25f) * mCameraViewDirection;
		mCameraTarget = lerp(mCameraTarget, mCameraLerpTarget, 0.25f);
		mCamera.lookAt(eye, mCameraTarget);
	}

	mCamUi = CameraUi(&mCamera);

}

void Vis2App::moveCameraPosLinear(CameraPersp newCam)
{
	float distance = glm::distance(mCamera.getEyePoint(), newCam.getViewDirection());
	vec3 eye = newCam.getViewDirection() - lerp(distance, 5.0f, 0.25f) * mCameraViewDirection;
	mCameraTarget = lerp(mCameraTarget, newCam.getViewDirection(), 0.25f);
	mCamera.lookAt(eye, mCameraTarget);
}




bool Vis2App::testPlaneCut(gl::BatchRef batchRef )
{
	//return (mEnablePlaneCut ? true : false);
	if (!mEnablePlaneCut)
		return true;

	return false;
	//VboMeshRef mesh batchRef->getVboMesh();
	
}






//creates a cut struct and adds it to the collection of cuts
void Vis2App::createCut()
{

	auto iterator = std::find(cutsLabelList.begin(), cutsLabelList.end(), mCutLabel);
	
	/*
	if ( iterator != cutsLabelList.end())
	{

			
		//print error message duplicate label
		return;
	}
	*/
	
	sCut cut;
	cut.camera = CameraPersp(this->mCamera);
	cut.u = this->mSpaceParamU;
	cut.v = this->mSpaceParamV;
	cut.w = this->mSpaceParamW;

	cut.x = this->mSpaceX;
	cut.y = this->mSpaceY;
	cut.z = this->mSpaceZ;

	cut.alpha = this->mCutAlpha;

	cut.type = mCutType;

	if(iterator != cutsLabelList.end())
	{

		cutsList.at(iterator - cutsLabelList.begin()) = cut;

	}
	else{
		this->cutsList.push_back(cut);
		this->cutsLabelList.push_back(mCutLabel);
	}
	this->setupLabelList();
	
}

void Vis2App::loadCut()
{
	
	sCut cut = cutsList.at(mCutSelection);

	this->mCamera = CameraPersp(cut.camera);

	this->mSpaceParamU = cut.u;
	this->mSpaceParamV = cut.v;
	this->mSpaceParamW = cut.w;

	this->mSpaceX = cut.x;
	this->mSpaceY = cut.y;
	this->mSpaceZ = cut.z;

	this->mCutAlpha = cut.alpha;

	this->mCutType = cut.type;

}


void Vis2App::buttonLoadModel()
{
	selectObjFileDialog();
	loadModel();

	//loadViewSettings
}

/**
*/
void Vis2App::updateViewInterface() const
{
	if (mViewSettings)
		mViewSettings->clear();

}

void Vis2App::enableSelect()
{
	mEnableSelect = true;
}

/**
* Draws the scene
*
*/
void Vis2App::draw()
{
	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableDepth();
	
	gl::enableFaceCulling(mEnableFaceCulling);
	
	gl::setMatrices(mCamera);

	//gl::clear(Color(.20f, .20f, .20f));
	gl::clear(Color(mBackgroundColor));

	//gl::drawString("Framerate: " + toString(getAverageFps()), vec2(10, 10), Color::white(), mFont);
	
	mFps = toString(getAverageFps());

	if (this->mShowGrid && this->mGridLoop)
		mGridLoop->draw();

	//this is the part where generic geometry is drawn
	//basically everything from an objfile which can be drawn as is will be drawn here:

	/*
	if (!mVecVertBatchRef.empty())
	{
		for (gl::VertBatchRef element : mVecVertBatchRef)
		{
			element->draw();
		}
	}


	if (!mVecBatchRef.empty())
	{
		for (gl::BatchRef element : mVecBatchRef)
		{
			if (this->testPlaneCut(element))
				element->draw();
		}
	}
	*/


	gl::enableBlending(true);
	//glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	//glBlendEquation(GL_MAX);


	if (mPhongShader)
	{


		/*

		uniform int			uNumCuts;
		uniform booleanCut	uCutArray[10];
		uniform sampler2D	uTex0;
		uniform int			uTexturingMode;
		uniform ivec2       uFreq;
		uniform int			uLightAll;
		uniform vec4		uSpaceParams;
		uniform vec4		uSpacePos;
		uniform float		uCutAlpha;
		uniform bool		uBackfaceCulling;


		uniform int			uNumCuts;
		uniform booleanCut	uCutArray[10];
		uniform vec4		uSpaceParams;

		*/

		//test for simple uvw parameter space
		mPhongShader->uniform("uSpaceParams",
			vec4(mSpaceParamU, mSpaceParamV, mSpaceParamW, 1.0f));

		//mPhongShader->uniform("uSpacePos", mSpacePos);
		mPhongShader->uniform("uSpacePos", vec4(mSpaceX, mSpaceY, mSpaceZ, 1.0));
		mPhongShader->uniform("uCutAlpha", mCutAlpha);

		mPhongShader->uniform("uTexturingMode", int(mTextureType));

		mPhongShader->uniform("uFreq", ivec2(80));
		mPhongShader->uniform("uBackfaceCulling", mEnableFaceCulling);
		mPhongShader->uniform("uCutMode", int(mCutType));
		mPhongShader->uniform("uCutEnable", mCutEnabled);

		auto size = cutsLabelList.size();

		auto it = cutsLabelList.begin();
		
		mPhongShader->uniform("uNumCuts", int(size));

		//vec3 *cutCenter = new vec3[size];
		vector<vec3> cutCenter;
		vector<vec3> cutUVW;
		vector<int>  cutType;
		vector<bool> cutEnabled;
		vector<float> cutAlpha;

		while (it != cutsLabelList.end())
		{
			sCut cut = cutsList.at(it - cutsLabelList.begin());

			vec3 v3Pos = vec3(cut.x, cut.y, cut.z);
			vec3 v3UVW = vec3(cut.u, cut.v, cut.w);
			
			cutCenter.push_back(v3Pos);
			cutUVW.push_back(v3UVW);
			cutType.push_back(int(cut.type));
			cutEnabled.push_back(cut.enabled);
			cutAlpha.push_back(cut.alpha);

			++it;
		}

		mPhongShader->uniform("uCutCenter",		(&cutCenter));
		mPhongShader->uniform("uCutUVW",		(&cutUVW));
		mPhongShader->uniform("uCutType",		(&cutType));
		mPhongShader->uniform("uCutEnabled",	(&cutEnabled));
		mPhongShader->uniform("uCutAlphas",		(&cutAlpha));
		//structs as uniform?
		
	}

	if (mObjectBatch)
		mObjectBatch->draw();

	gl::enableBlending(false);

	//draw settings last:
	//from geometry sample:
	/*
	if (mParams) {
	mParams->draw();
	}
	*/

	if (mOptions)
		mOptions->draw();

	if (mRigs)
		mRigs->draw();
	
	if (mCutSettings)
		mCutSettings->draw();

	
	if (mCutWireBatch)
	{
		gl::pushMatrices();
		gl::scale(this->mSpaceParamU, this->mSpaceParamV, this->mSpaceParamW);
		//gl::draw(mCutWireBatch)
		mCutWireBatch->draw();
		
		gl::popMatrices();
	}
		//Picking to select center of cut
	if (mEnableSelect && !mMouseDown)
	{
		if (performPicking(&mPickedPoint, &mPickedNormal)) {
			
			gl::ScopedColor color(Color::white());
			
			// Draw an arrow to the picked point along its normal.
			gl::ScopedGlslProg shader(gl::getStockShader(gl::ShaderDef().color().lambert()));
			gl::drawVector(mPickedPoint + mPickedNormal, mPickedPoint);
		}
	}
	
	if (mMouseDown && mEnableSelect)
	{
		
		if (performPicking(&mMouseDrawPoint, &mMouseDrawPickedNormal)) {

			gl::ScopedColor color(Color(0.0f,1.0f,0.0f));

			// Draw an arrow to the picked point along its normal.
			gl::ScopedGlslProg shader(gl::getStockShader(gl::ShaderDef().color().lambert()));
			gl::drawVector(mMouseDrawPoint + mMouseDrawPickedNormal, mMouseDrawPoint);
			
			gl::drawVector(mPickedPoint + mPickedNormal, mPickedPoint);
		}
	}
	
	
}


void vis2::Vis2App::createGridLoop()
{
	mGridLoop = gl::VertBatch::create(GL_LINE_LOOP);
	mGridLoop->begin(GL_LINE_LOOP);

	for (auto i = -mNumGridCells; i <= 0; ++i) {

		mGridLoop->color(Color(0.25f, 0.25f, 0.25f));

		mGridLoop->vertex(float(i), 0.0f, float(-mNumGridCells));
		mGridLoop->vertex(float(i), 0.0f, float(mNumGridCells));
		mGridLoop->vertex(float(-mNumGridCells), 0.0f, float(i));
		mGridLoop->vertex(float(mNumGridCells), 0.0f, float(i));
	}

	for (auto i = 1; i <= mNumGridCells; ++i) {

		mGridLoop->color(Color(0.25f, 0.25f, 0.25f));

		mGridLoop->vertex(float(i), 0.0f, float(-mNumGridCells));
		mGridLoop->vertex(float(i), 0.0f, float(mNumGridCells));
		mGridLoop->vertex(float(-mNumGridCells), 0.0f, float(i));
		mGridLoop->vertex(float(mNumGridCells), 0.0f, float(i));
	}

	/*
	mGrid->color(Color(0.0, 0.0, 1.0));

	mGrid->vertex(0.0f, float(-mNumGridCells), 0.0f);
	mGrid->vertex(0.0f, float(mNumGridCells), 0.0f);

	mGrid->vertex(0.0f, 0.0f, float(-mNumGridCells));
	mGrid->vertex(0.0f, 0.0f, float(mNumGridCells));

	mGrid->vertex(0.0f, 0.0f, float(-mNumGridCells));
	mGrid->vertex(0.0f, 0.0f, float(mNumGridCells));

	mGrid->color(Color(1.0, 0.0, 0.0));
	mGrid->vertex(float(-mNumGridCells), 0.0f, float(0));
	mGrid->vertex(float(mNumGridCells), 0.0f, float(0));
	*/

	mGridLoop->end();

}

void vis2::Vis2App::createGrid()
{
	mGrid = gl::VertBatch::create(GL_LINE);

	mGrid->begin(GL_LINE);

	//draw red line for positive x axis
	mGrid->color(Color(1.00f, 0.25f, 0.25f));
	mGrid->vertex(0.00, 0.00, 0.00f);
	mGrid->vertex(0.00, float(mNumGridCells), 0.00f);
	mGrid->end();

	this->mVecVertBatchRef.push_back(mGrid);
}

void vis2::Vis2App::saveCurrentCutAndView()
{

}



CINDER_APP(Vis2App, RendererGl , [&](App::Settings *settings) {
	
	
	//settings->setFullScreen(true);
	//settings->disableFrameRate();
	settings->setWindowSize(1366,768);
	
	vector<string> args = settings->getCommandLineArgs();

	//if there should ever be intend to use program arguments
	for (string s : args)
	{
		std::cout << s << std::endl;
		CI_LOG_D(s);
	}

});


/////////////////////////////////////////
//User input
////////////

void vis2::Vis2App::keyDown(KeyEvent event)
{
	switch (event.getCode())
	{
	case KeyEvent::KEY_c:
		this->enableSelect();
		break;
	case KeyEvent::KEY_d:
		this->mSpaceParamU = 1.0f;
		this->mSpaceParamV = 1.0f;
		this->mSpaceParamW = 1.0f;
		break;
	case KeyEvent::KEY_r:
		//resetCam();
		break;


		
//move camera
	case KeyEvent::KEY_DOWN:
		
		break;
	case KeyEvent::KEY_UP:

		break;
	case KeyEvent::KEY_LEFT:

		break;
	case KeyEvent::KEY_RIGHT:

		break;
	case KeyEvent::KEY_ESCAPE:
		if (mEnableSelect) {
			mEnableSelect = false;
			mMouseDown = false;
		}
		else
			this->quit();
	default:
		break;

	}
}

void Vis2App::mouseMove(MouseEvent event)
{
	this->mMousePos = event.getPos();
}

void Vis2App::mouseDown(MouseEvent event)
{
	this->mMousePos = event.getPos();

	if (!mEnableSelect)
		mCamUi.mouseDown(event);
	else
	{
		this->mSpaceX = mPickedPoint.x;
		this->mSpaceY = mPickedPoint.y;
		this->mSpaceZ = mPickedPoint.z;

		//mEnableSelect = false;
	}

	mMouseDown = true;

	/*
	auto ray = mCamera.generateRay(event.getPos(), getWindowSize());

	ray.calcPlaneIntersection();
	*/

	//automates mouse control:
	//mousedown enables moving camera along a ball

	//sort for alpha blending


	/*
	if (!mVecVertBatchRef.empty())
	{
	std::sort(mVecVertBatchRef.begin(), mVecBatchRef.end()
	, [](const VertBatchRef &lhs, const VertBatchRef &rhs)
	{
	return lhs.get()->
	}

	for (gl::VertBatchRef element : mVecVertBatchRef)
	{
	element->draw();
	}

	// something like this
	vector<ObjectType> objects;
	std::sort(objects.begin(), objects.end(),
	[](const ObjectType &lhs, const ObjectType &rhs)
	{ return lhs.z < rhs.z; });


	}
	*/
	int err = gl::getError();
	if (err != GL_NO_ERROR)
	{
		CI_LOG_E(gl::getErrorString(err));

	}
}


void vis2::Vis2App::mouseUp(MouseEvent event)
{
	this->mMousePos = event.getPos();
	mEnableSelect = false;
	mMouseDown = false;
}


void Vis2App::mouseWheel(MouseEvent event)
{
	this->mMousePos = event.getPos();
	mCamUi.mouseWheel(event);
}

void Vis2App::mouseDrag(MouseEvent event)
{
	this->mMousePos = event.getPos();
	if (!mEnableSelect)
		mCamUi.mouseDrag(event);
	else
	{
		this->mSpaceParamU = max(abs(mPickedPoint.x - this->mMouseDrawPoint.x),0.1f);
		this->mSpaceParamV = max(abs(mPickedPoint.y - this->mMouseDrawPoint.y),0.1f);
		this->mSpaceParamW = max(abs(mPickedPoint.z - this->mMouseDrawPoint.z),0.1f);

	}
	
}