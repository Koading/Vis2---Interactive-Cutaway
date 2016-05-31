#include "vis2.h"
#include <iostream>

using namespace ci;
using namespace ci::app;
using namespace std;
//using namespace vis2;
using namespace gl;

namespace vis2 {

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
		mEnableSelect(false),
		mMousePos(vec2(0.0f, 0.0f)),
		mBackgroundColor(0.1f, 0.1f, 0.1f),
		mEnableFaceCulling(true),
		mShowGrid(true),
		mModelFile(".//..//assets//house.obj"),
		mModelMtl(".//..//assets//house.mtl"), mJsonFile(mModelFile + ".json"),
		mTextureType(TexType::MATERIAL),
		mCutType(cutType::BOX),
		mMouseDown(false),
		mFont(Font("Arial", 12.0f)),
		mFps("0.0")
	{

	}

	/**
	<summary>

		Initializes grid, camera and model on app startup

	</summary>
	*/
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

	/**
	<summary> Compile and create shaders </summary>
	*/
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

	/**
	<summary> Set correct aspect ratio after resizing</summary>
	*/
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




	/**
	<summary> Save button callback. Creates a cut structure instance and saves it in a vector</summary>
	*/

	void Vis2App::createCut()
	{

		auto iterator = std::find(cutsLabelList.begin(), cutsLabelList.end(), mCutLabel);

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
		cut.enabled = this->mCutEnabled;

		if (iterator != cutsLabelList.end())
		{

			cutsList.at(iterator - cutsLabelList.begin()) = cut;

		}
		else {
			this->cutsList.push_back(cut);
			this->cutsLabelList.push_back(mCutLabel);
		}
		this->setupLabelList();

	}

	/**
	<summary> Load Button Callback: takes current selection, looks it up in the cut vector and sets parameters</summary>
	*/
	void Vis2App::loadCut()
	{

		sCut cut = cutsList.at(mCutSelection);


		this->mCutLabel = cutsLabelList.at(mCutSelection);

		this->mCutEnabled = cut.enabled;
		
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


	/**
	<summary> Load Model Button Callback</summary>
	*/
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


	/**
	<summary> Pick Cut Callback</summary>
	*/
	void Vis2App::enableSelect()
	{
		mEnableSelect = true;
	}

	/**
	* <summary>Draws the scene, grid and picker arrows</summary>
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
		//mFps = toString(ci::app::AppBase::getFrameRate());
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

			mPhongShader->uniform("uRenderEdges", true);


			auto size = cutsLabelList.size();

			auto it = cutsLabelList.begin();

			mPhongShader->uniform("uNumCuts", int(size));

			//vec3 *cutCenter = new vec3[size];
			vector<vec4> cutCenter;
			vector<vec4> cutUVW;
			vector<int>  cutType;
			vector<int> cutEnabled;
			vector<float> cutAlpha;

			while (it != cutsLabelList.end())
			{
				sCut cut = cutsList.at(it - cutsLabelList.begin());

				vec4 v3Pos = vec4(cut.x, cut.y, cut.z, 1.0f);
				vec4 v3UVW = vec4(cut.u, cut.v, cut.w, 1.0f);

				cutCenter.push_back(v3Pos);
				cutUVW.push_back(v3UVW);
				cutType.push_back(int(cut.type));
				cutEnabled.push_back(cut.enabled);
				cutAlpha.push_back(cut.alpha);

				++it;
			}

			mPhongShader->uniform("uCutCenter", &cutCenter[0], size);
			mPhongShader->uniform("uCutUVW", &cutUVW[0], size);
			mPhongShader->uniform("uCutType", &cutType[0], size);
			mPhongShader->uniform("uCutEnabled", (&cutEnabled[0]), size);
			mPhongShader->uniform("uCutAlphas", (&cutAlpha[0]), size);
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

				gl::ScopedColor color(Color(0.0f, 1.0f, 0.0f));

				// Draw an arrow to the picked point along its normal.
				gl::ScopedGlslProg shader(gl::getStockShader(gl::ShaderDef().color().lambert()));
				gl::drawVector(mMouseDrawPoint + mMouseDrawPickedNormal, mMouseDrawPoint);

				gl::drawVector(mPickedPoint + mPickedNormal, mPickedPoint);
			}
		}


	}

	/**<summary> Creates reference to draw gridloop</summary>
	*/
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





	/////////////////////////////////////////
	//User input
	////////////

	/**
	<summary> Keyboard Input handling

	enabled keys:

		c - enables picking cut position
		d - resets uvw to 1.0
		ESC - cancel picking or quit application

		</summary>
	*/
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

	/**
	<summary>Tracks mousposition for picking feature</summary>
	*/
	void Vis2App::mouseMove(MouseEvent event)
	{
		this->mMousePos = event.getPos();
	}

	/**
	<summary>Tracks mouseclicks:

		Moves the camera along a centred ball and enables drag and drop setting for uvw parameters
		</summary>
	*/
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

		int err = gl::getError();
		if (err != GL_NO_ERROR)
		{
			CI_LOG_E(gl::getErrorString(err));

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

	}


	/**
	<summary>Tracks MouseUp: terminates uvw drag and drop </summary>
	*/
	void vis2::Vis2App::mouseUp(MouseEvent event)
	{
		this->mMousePos = event.getPos();
		mEnableSelect = false;
		mMouseDown = false;
	}


	/**
	<summary>Tracks MouseWheel event: Scene zooming and camera pan on mouse3</summary>
	*/
	void Vis2App::mouseWheel(MouseEvent event)
	{
		this->mMousePos = event.getPos();
		mCamUi.mouseWheel(event);
	}


	/**
	<summary>Tracks MouseDrag event: move camera and drag and drop mechanism for basic 4sided window cut</summary>
	*/
	void Vis2App::mouseDrag(MouseEvent event)
	{
		this->mMousePos = event.getPos();
		if (!mEnableSelect)
			mCamUi.mouseDrag(event);
		else
		{
			this->mSpaceParamU = max(abs(mPickedPoint.x - this->mMouseDrawPoint.x), 0.1f);
			this->mSpaceParamV = max(abs(mPickedPoint.y - this->mMouseDrawPoint.y), 0.1f);
			this->mSpaceParamW = max(abs(mPickedPoint.z - this->mMouseDrawPoint.z), 0.1f);

		}

	}





	/*
	void vis2::Vis2App::saveCurrentCutAndView()
	{

	}
	*/

	/*

	bool Vis2App::testPlaneCut(gl::BatchRef batchRef )
	{
	return (mEnablePlaneCut ? true : false);
	if (!mEnablePlaneCut)
	return true;

	return false;
	VboMeshRef mesh batchRef->getVboMesh();

	}

	*/


}

/**
<summary> This macro is the main entry point </summary>
*/
CINDER_APP(vis2::Vis2App, RendererGl, [&](App::Settings *settings) {


	//settings->setFullScreen(true);
	settings->disableFrameRate();
	settings->setWindowSize(1366, 768);

	vector<string> args = settings->getCommandLineArgs();

	//if there should ever be intend to use program arguments
	for (string s : args)
	{
		std::cout << s << std::endl;
		CI_LOG_D(s);
	}

});