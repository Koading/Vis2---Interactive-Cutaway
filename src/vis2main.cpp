#include "vis2.h"
#include <iostream>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace vis2;
using namespace gl;

Vis2App::Vis2App() :
	mNumGridCells(20),
	mEnablePlaneCut(false),
	mSpaceParamU(0.0f),
	mSpaceParamV(0.0f),
	mSpaceParamW(0.0f),
	mSpaceX(0.0f),
	mSpaceY(0.0f),
	mSpaceZ(0.0f),
	mMousePos(vec2(0.0f, 0.0f)),
	mEnableSelect(false),
	mBackgroundColor(0.1f, 0.1f, 0.1f),
	mPlaneCutParams(vec4(0.0f, 0.0f, 1.0f, 0.0f)),
	mEnableFaceCulling(true),
	mShowGrid(true),
	mSpacePos(vec4(0.0, 0.0, 0.0, 1.0)),
	mCutAlpha(0.25),
	//mModelFile(".//..//assets//8lbs.obj"),
	mModelFile(".//..//assets//house.obj"),
	mModelMtl(".//..//assets//house.mtl"),
	mJsonFile(mModelFile + ".json"),
	mPickedPoint(vec3(0.0f, 0.0f, 0.0f)),
	mTextureType(TexType::CHECKERED),
	mCutType(cutType::BOX),
	mCutLabel("default"),
	cutsLabelList({}),
	cutsList({})
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

	geom::WireSphere();
	geom::WireCube;

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

void Vis2App::setupUI()
{

	const static vector<string> vecCutTypes = { "none", "box", "ball", "plane" };
	const static vector<string> texturingModes = { "none" , "checkered", "texture", "normals" };

	int sizeX = 200;
	int sizeY = 300;

	int stdMargin = 20;
	int offsetXLeft = stdMargin;
	int offsetYTop = stdMargin;

	int offsetXRight = getWindowSize().x - sizeX - stdMargin;

	//mCameraSettings = params::InterfaceGl::create(getWindow(), "Camera", toPixels(ivec2(200, 200)));	

	mCutSettings = params::InterfaceGl::create(getWindow(), "Cut Settings", 
		toPixels(ivec2(sizeX, sizeY)));

	//mCutSettings->addParam("EnablePlaneCut", &mEnablePlaneCut);
	mCutSettings->addSeparator();
	//mCutSettings->addParam("Plane Params", &(mPlaneCutParams.x));

	mCutSettings->addParam("Cut Mode", vecCutTypes, (int*)&mCutType);

	mCutSettings->addParam("U", &mSpaceParamU).min(0.0f).step(0.1f);
	mCutSettings->addParam("V", &mSpaceParamV).min(0.0f).step(0.1f);
	mCutSettings->addParam("W", &mSpaceParamW).min(0.0f).step(0.1f);

	mCutSettings->addParam("PosX", &mSpaceX).step(0.1f);
	mCutSettings->addParam("PosY", &mSpaceY).step(0.1f);
	mCutSettings->addParam("PosZ", &mSpaceZ).step(0.1f);

	mCutSettings->addParam("uCutAlpha", &mCutAlpha).min(0.0f).max(1.0f).step(0.05f);

	mCutSettings->addSeparator();

	mCutSettings->addParam("Selection", &mEnableSelect, true);
	
	mCutSettings->addButton("Select cut", 
		std::bind(&Vis2App::enableSelect, this)
		);
	
	mOptions = params::InterfaceGl::create(getWindow(), "General Options", 
		toPixels(ivec2(sizeX, sizeY)));
	mOptions->addParam("Face Culling", 
		&mEnableFaceCulling).
		updateFn([this] { gl::enableFaceCulling(mEnableFaceCulling); });


	mOptions->addSeparator();
	mOptions->addParam("Draw Grid", &mShowGrid);
	
	mOptions->setPosition(vec2(stdMargin, stdMargin));
	//mOptions->addParam("")
		
		
	mOptions->addParam("Texturing Mode", texturingModes, (int*)&mTextureType);

	mOptions->addButton("Open File",
		std::bind(&Vis2App::selectObjFileDialog, this)
		);

	mRigs = params::InterfaceGl::create(getWindow(), "Rigs", 
		toPixels(ivec2(sizeX, sizeY)));

	mRigs->addParam("LabelName", &mCutLabel);
	//mRigs->addParam("Rig", cutsLabelList, &mCutSelection);
	mRigs->addButton("Safe Cut", std::bind(&Vis2App::createCut, this));
	setupLabelList();
	mRigs->addButton("Load Cut", std::bind(&Vis2App::loadCut, this));
	mCutSettings->setPosition(vec2(offsetXRight, offsetYTop));
	mRigs->setPosition(
		vec2(mCutSettings->getPosition().x,
			mCutSettings->getPosition().y + mCutSettings->getHeight() + offsetYTop

			));



}

void vis2::Vis2App::setupLabelList()
{
	mRigs->addParam("Rig", cutsLabelList, &mCutSelection);
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

void Vis2App::keyDown(KeyEvent event)
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

		//possibly move camera
	case KeyEvent::KEY_DOWN:
		
		break;
	case KeyEvent::KEY_UP:

		break;
	case KeyEvent::KEY_LEFT:

		break;
	case KeyEvent::KEY_RIGHT:

		break;
	case KeyEvent::KEY_ESCAPE:
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

	if(!mEnableSelect)
		mCamUi.mouseDown(event);
	else
	{
		this->mSpaceX = mPickedPoint.x;
		this->mSpaceY = mPickedPoint.y;
		this->mSpaceZ = mPickedPoint.z;

		//mEnableSelect = false;
	}

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
		CI_LOG_E( gl::getErrorString(err));
		
	}
}

void vis2::Vis2App::mouseUp(MouseEvent event)
{
	mEnableSelect = false;
}

void Vis2App::mouseWheel(MouseEvent event)
{
	mCamUi.mouseWheel(event);
}

void Vis2App::mouseDrag(MouseEvent event)
{
	if(!mEnableSelect)
		mCamUi.mouseDrag(event);
	else
	{
		CI_LOG_D("Mousedrag cursor");
		vec3 pickedNormal;
		if (performPicking(&mPickedPoint, &pickedNormal)) {
			CI_LOG_D("pickpoint 2");
			gl::ScopedColor color(Color::black());

			// Draw an arrow to the picked point along its normal.
			gl::ScopedGlslProg shader(gl::getStockShader(gl::ShaderDef().color().lambert()));
			gl::drawVector(mPickedPoint + pickedNormal, mPickedPoint);
		}

		this->mSpaceParamU = abs(mPickedPoint.x - this->mSpaceX);
		this->mSpaceParamV = abs(mPickedPoint.y - this->mSpaceY);
		this->mSpaceParamW = abs(mPickedPoint.z - this->mSpaceZ);


	}

}

void Vis2App::loadObj(const DataSourceRef &dataSource, const DataSourceRef &dataSourceMtl)
{
	//throws a build error in a non related namespace
	/*
	//get the current model, if one is loaded and delete

	auto batchIterator = std::find(mVecBatchRef.begin(), mVecBatchRef.end(), mCurrentObjecMesh);

	if (mCurrentObjecMesh 
		&& batchIterator != mVecBatchRef.end())
	{
		mVecBatchRef.erase(batchIterator);
	}
	*/
	//ci::fs::path path(dataSource);
	//ObjLoader * loader;
	/*
	if(dataSourceMtl->isFilePath())
		loader = & ObjLoader(dataSource, dataSourceMtl);
	else
		loader = & ObjLoader(dataSource);
	*/

	ObjLoader loader(dataSource, dataSourceMtl);

	//need trimesh for bounding box functionality
	
	mCurrentTriMesh = TriMesh::create(loader);
	mCurrentVboMesh = VboMesh::create(loader);
	
	
	/*
	mCurrentTriMesh = TriMesh::create(*(loader));
	mCurrentVboMesh = VboMesh::create(*(loader));
	*/
	//if (!loader.getAvailableAttribs().count(geom::NORMAL))
		//mCurrentTriMesh->recalculateNormals();

	//choose a renderer
	//auto batch = gl::Batch::create(*mCurrentObjecMesh, mPhongShader);
	//auto batch = 
	
	//mObjectBatch = gl::Batch::create(*mCurrentTriMesh, *mCurrentShader);
	mObjectBatch = gl::Batch::create(mCurrentVboMesh, *mCurrentShader);
	//mVecBatchRef.push_back(batch);
	//mVecBatchRef.push_back(mObjectBatch);
}


bool Vis2App::testPlaneCut(gl::BatchRef batchRef )
{
	//return (mEnablePlaneCut ? true : false);
	if (!mEnablePlaneCut)
		return true;

	return false;
	//VboMeshRef mesh batchRef->getVboMesh();
	
}



bool Vis2App::performPicking(vec3 *pickedPoint, vec3 *pickedNormal)
{
	if (!mCurrentTriMesh)
		return false;
	// Generate a ray from the camera into our world. Note that we have to
	// flip the vertical coordinate.
	float u = mMousePos.x / (float)getWindowWidth();
	float v = mMousePos.y / (float)getWindowHeight();
	Ray ray = mCamera.generateRay(u, 1.0f - v, mCamera.getAspectRatio());
	
	// The coordinates of the bounding box are in object space, not world space,
	// so if the model was translated, rotated or scaled, the bounding box would not
	// reflect that. One solution would be to pass the transformation to the calcBoundingBox() function:
	//AxisAlignedBox worldBoundsExact = mTriMesh->calcBoundingBox(mTransform); // slow
	AxisAlignedBox worldBoundsExact = mCurrentTriMesh->calcBoundingBox();
																			 // But if you already have an object space bounding box, it's much faster to
																	 // approximate the world space bounding box like this:
	//mat4 mTransform(1.0f);
	//AxisAlignedBox worldBoundsApprox = mObjectBounds.transformed(mTransform); // fast

																			  // Draw the object space bounding box in yellow. It will not animate,
																			  // because animation is done in world space.
	//drawCube(mObjectBounds, Color(1, 1, 0));

	// Draw the exact bounding box in orange.
	//drawCube(worldBoundsExact, Color(1, 0.5f, 0));

	// Draw the approximated bounding box in cyan.
	//drawCube(worldBoundsApprox, Color(0, 1, 1));
	
	// Perform fast detection first - test against the bounding box itself.
	if (!worldBoundsExact.intersects(ray))
		return false;

	// Set initial distance to something far, far away.
	float result = FLT_MAX;

	// Traverse triangle list and find the closest intersecting triangle.
	const size_t polycount = mCurrentTriMesh->getNumTriangles();

	float distance = 0.0f;
	for (size_t i = 0; i < polycount; ++i) {
		// Get a single triangle from the mesh.
		vec3 v0, v1, v2;
		mCurrentTriMesh->getTriangleVertices(i, &v0, &v1, &v2);
		
		mat4 transform(1.0f);

		// Transform triangle to world space.
		v0 = vec3(transform * vec4(v0, 1.0));
		v1 = vec3(transform * vec4(v1, 1.0));
		v2 = vec3(transform * vec4(v2, 1.0));

		// Test to see if the ray intersects this triangle.
		if (ray.calcTriangleIntersection(v0, v1, v2, &distance)) {
			// Keep the result if it's closer than any intersection we've had so far.
			if (distance < result) {
				result = distance;

				// Assuming this is the closest triangle, we'll calculate our normal
				// while we've got all the points handy.
				*pickedNormal = normalize(cross(v1 - v0, v2 - v0));
			}
		}
	}

	// Did we have a hit?
	if (distance > 0) {
		// Calculate the exact position of the hit.
		*pickedPoint = ray.calcPosition(result);

		return true;
	}
	else
		return false;
}

void vis2::Vis2App::selectObjFileDialog()
{
	std::string pathRef = ".";
	vector<std::string> fileExtension = { "obj", "3ds" };

	//std::tr2::sys::path filePath;
	
	//boost
	ci::fs::path path = getOpenFilePath(pathRef, fileExtension);

	if (!path.empty())
	{
		//this->mModelFile = path->
		this->mModelFile = path.string();
		this->loadModel();
	}
}


//creates a cut struct and adds it to the collection of cuts
void Vis2App::createCut()
{
	if (std::find(cutsLabelList.begin(), cutsLabelList.end(), mCutLabel) != cutsLabelList.end())
	{
		//print error message duplicate label
		return;
	}

	
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

	this->cutsList.push_back(cut);
	this->cutsLabelList.push_back(mCutLabel);

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

void vis2::Vis2App::loadModel()
{
	ci::fs::path file(mModelFile);
	//string_type ext = file.extension();

	//string mtlFile = mModelFile.substr(0, mModelFile.size() - ext) + ".mtl";
	loadObj(loadFile(mModelFile), loadFile(mModelMtl));
	//loadObj(loadFile(mModelFile), loadFile(mtlFile));
	//loadJson();
}

void Vis2App::buttonLoadModel()
{
	selectObjFileDialog();
	loadModel();

	//loadViewSettings
}

/**
*/
void Vis2App::updateViewInterface()
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

		mPhongShader->uniform("uTexturingMode", (int)mTextureType);

		mPhongShader->uniform("uFreq", ivec2(80));
		mPhongShader->uniform("uBackfaceCulling", mEnableFaceCulling);
		mPhongShader->uniform("uCutMode", (int)mCutType);



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


	//Picking to select center of cut
	if (mEnableSelect) 
	{
		
		vec3 pickedNormal;
		if (performPicking(&mPickedPoint, &pickedNormal)) {
			
			gl::ScopedColor color(Color::white());
			
			// Draw an arrow to the picked point along its normal.
			gl::ScopedGlslProg shader(gl::getStockShader(gl::ShaderDef().color().lambert()));
			gl::drawVector(mPickedPoint + pickedNormal, mPickedPoint);
		}
	}
}

void Vis2App::saveToJson()
{
	//if(())
	{
		mJsonTree = JsonTree();
	}
	this->mCamera.getAspectRatio();
	this->mCamera.getEyePoint();
	
}


void Vis2App::loadJson()
{


}

void vis2::Vis2App::createGridLoop()
{
	mGridLoop = gl::VertBatch::create(GL_LINE_LOOP);
	mGridLoop->begin(GL_LINE_LOOP);

	for (int i = -mNumGridCells; i <= 0; ++i) {

		mGridLoop->color(Color(0.25f, 0.25f, 0.25f));

		mGridLoop->vertex(float(i), 0.0f, float(-mNumGridCells));
		mGridLoop->vertex(float(i), 0.0f, float(mNumGridCells));
		mGridLoop->vertex(float(-mNumGridCells), 0.0f, float(i));
		mGridLoop->vertex(float(mNumGridCells), 0.0f, float(i));
	}

	for (int i = 1; i <= mNumGridCells; ++i) {

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