#include "vis2.h"

using namespace vis2;
using namespace boost::filesystem;
using namespace ci;
using namespace ci::app;
using namespace std;
using namespace gl;



void vis2::Vis2App::loadModel()
{
	ci::fs::path file(mModelFile);
	//string_type ext = file.extension();

	//string mtlFile = mModelFile.substr(0, mModelFile.size() - ext) + ".mtl";
	loadObj(loadFile(mModelFile), loadFile(mModelMtl));
	//loadObj(loadFile(mModelFile), loadFile(mtlFile));
	//loadJson();
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
