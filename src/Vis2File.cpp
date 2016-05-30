#include "vis2.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>


using namespace vis2;
using namespace boost::filesystem;
using namespace ci;
using namespace ci::app;
using namespace std;
using namespace gl;

/**
<summary> </summary>
*/
void vis2::Vis2App::loadModel()
{
	ci::fs::path file(mModelFile);

	if (!boost::filesystem::exists(mModelFile))
		return;

	(boost::filesystem::exists(mModelMtl) ? loadObj(loadFile(mModelFile), loadFile(mModelMtl)) : loadObj(loadFile(mModelFile)));
}


/**
<summary> Load .obj file without mtl(materials, textures, etc)</summary>
*/
void Vis2App::loadObj(const DataSourceRef &dataSource)
{
	try {

		ObjLoader loader(dataSource);

		//need trimesh for bounding box functionality
		mCurrentTriMesh = TriMesh::create(loader);
		mObjectBounds = mCurrentTriMesh->calcBoundingBox();
		mCurrentVboMesh = VboMesh::create(loader);

		mObjectBatch = gl::Batch::create(mCurrentVboMesh, *mCurrentShader);
	}
	catch (ci::Exception &ex)
	{


	}


}

/**
<summary> Load .obj and .mtl file and create mesh and vbo</summary>
*/
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
	try {

		ObjLoader loader(dataSource, dataSourceMtl);

		//need trimesh for bounding box functionality

		mCurrentTriMesh = TriMesh::create(loader);
		mCurrentVboMesh = VboMesh::create(loader);

		mObjectBatch = gl::Batch::create(mCurrentVboMesh, *mCurrentShader);
	}
	catch(ci::Exception &ex)
	{
		

	}

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
	
	//mVecBatchRef.push_back(batch);
	//mVecBatchRef.push_back(mObjectBatch);
}



/**
<summary> Callback for Load button

Opens an OS style file open dialog, to open a standard obj file
</summary>
*/
void vis2::Vis2App::selectObjFileDialog()
{
	std::string pathRef = ".";
	vector<std::string> fileExtension = { "obj" };

	ci::fs::path path = getOpenFilePath(pathRef, fileExtension);

	if (!path.empty())
	{
		this->mModelFile = this->mModelMtl = path.string();
		boost::replace_last(this->mModelMtl, ".obj", ".mtl");

		this->loadModel();
	}
}


/*
void Vis2App::saveToJson()
{
	mJsonTree = JsonTree();
	
	auto it = cutsLabelList.begin();
	while(it!=cutsLabelList.end())
	{ 
		int index = (it - cutsLabelList.begin());
		
		auto cut = cutsList.at(index);

		cut.camera.getAspectRatio();
		cut.camera.getEyePoint();
		cut.camera.getFov();
		cut.camera.getViewDirection();

	}
}


void Vis2App::loadJson()
{

}
*/