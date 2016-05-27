/**
	Main class for vis2 assignment 

	Holds the declaration of "Vis2" class

	A lot of code was inspired by cinder samples (geometry, loadobj,...) to get started on the project

	Name definition follow by the example projects (mostly semi-hungarian notation, with the m- prefix meaning memberObject
	Other hungarian notation is ignored, full descriptive variable and constant names should be sufficient.

*/
#pragma once

//#include "..\\..\\..\\Cinder\include\cinder\Cinder.h"
//essential
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/gl.h"



#include "cinder/Camera.h"
#include "cinder/GeomIo.h"
#include "cinder/ImageIo.h"
#include "cinder/CameraUi.h"


#include "cinder/gl/Batch.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/params/Params.h"
#include "cinder/Log.h"
#include "cinder/Json.h"

//predefined obj loader
//
#include "cinder/ObjLoader.h"

#include <vector>
#include <algorithm> // contains search functions for vector classes
#include <iostream>
#include <iostream>
#include <boost/filesystem.hpp>

#include "vis2enums.h"



using namespace boost::filesystem;
using namespace ci;
using namespace ci::app;
using namespace std;

namespace vis2 {

	//explicit App to make clear where App came from (cinder, not Windows RT SDK)
	class Vis2App : public ci::app::App {

	public:
		Vis2App();


		void setup() override;
		void setupUI();
		void setupLabelList();
		void setupGLSL();
		
		void resize() override;
		void update() override;
		void draw() override;


		void mouseDown(MouseEvent event) override;
		void mouseUp(MouseEvent event) override;

		void mouseDrag(MouseEvent event) override;
		void mouseWheel(MouseEvent event) override;
		void keyDown(KeyEvent event) override;
		void mouseMove(MouseEvent event) override;

		vec3				mCameraTarget, mCameraLerpTarget, mCameraViewDirection;

		//for multiple meshes/objects to render
		vector<gl::BatchRef> mVecBatchRef;
		vector<gl::VertBatchRef> mVecVertBatchRef;
		
		gl::VertBatchRef	mObjectVertBatch;
		gl::BatchRef		mObjectBatch;



		CameraPersp			mCamera;
		CameraUi			mCamUi;
	
		gl::BatchRef		mPrimitive;
		gl::BatchRef		mPrimitiveWire;
		gl::BatchRef		mPrimitiveWireframe;
		gl::BatchRef		mPrimitiveNormalLines, mPrimitiveTangentLines;

		

		gl::GlslProgRef		mPhongShader;
		gl::GlslProgRef		mWireShader;
		gl::GlslProgRef		mWireframeShader;

		gl::GlslProgRef		*mCurrentShader;

		void loadObj(const DataSourceRef &dataSourceObj, const DataSourceRef &dataSourceMtl);
		
		
		gl::GlslProgRef		mFlatShader;

		enum shaderSetting
		{
			PHONG, FLAT, WIRE
		};

		struct sCut
		{
			//std::string label = "DEFUALTLABELCHANGETHIS";

			CameraPersp camera;
			float u = 0.0;
			float v = 0.0;
			float w = 0.0;

			float x = 0.0;
			float y = 0.0;
			float z = 0.0;

			bool enabeled;

			float alpha = 0.0;
			cutType type;
		} ;


		string mCutLabel;

		int mCutSelection;
		vector<string> cutsLabelList;
		vector<sCut> cutsList;


		void createCut();
		void loadCut();


		//UI Windows
		params::InterfaceGlRef	mOptions;
		params::InterfaceGlRef	mCameraSettings;
		params::InterfaceGlRef  mCutSettings;
		params::InterfaceGlRef  mViewSettings;
		
		params::InterfaceGlRef  mRigs;

		void updateViewInterface();
		
		shaderSetting mShaderSetting;

		int mNumGridCells;

		TriMeshRef mCurrentTriMesh;
		
		cinder::gl::VboMeshRef mCurrentVboMesh;

		//parameters for UI Settings
		bool mEnablePlaneCut;

		//simple test cut methods:
		vec4 mPlaneCutParams;
		bool testPlaneCut(gl::BatchRef batchRef);

		vec3 mPickedPoint;
		void enableSelect();

		float mSpaceParamU;
		float mSpaceParamV;
		float mSpaceParamW;

		float mSpaceX;
		float mSpaceY;
		float mSpaceZ;

		vec4 mSpacePos;

		float mCutAlpha; 

		bool mEnableSelect;
		bool performPicking(vec3 *pickedPoint, vec3 *pickedNormal);
		vec2 mMousePos;

		string mCurrentModelPath;
		void selectObjFileDialog();
		void loadModel();

		Color mBackgroundColor;

		bool mEnableFaceCulling;
		bool mShowGrid;

		void buttonLoadModel();
		
		std::string mModelFile;
		std::string mModelMtl;
		std::string mJsonFile;

		void saveToJson();
		void loadJson();

		JsonTree mJsonTree;

		gl::VertBatchRef mGridLoop;
		void createGridLoop();
		
		gl::VertBatchRef mGrid;
		void createGrid();

		TexType mTextureType;
		cutType mCutType;
		
		void saveCurrentCutAndView();

		void resetCam();
		void moveCameraPosLinear(CameraPersp newCam);
	};
}

