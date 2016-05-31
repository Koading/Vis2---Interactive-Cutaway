#include "vis2.h"

//using namespace vis2;
using namespace ci;

namespace vis2{
/**
<summary>User Interface

Handles positioning, lists
</summary>
*/
void Vis2App::setupUI()
{

	const static vector<string> vecCutTypes = { "none", "box", "ball", "plane" };
	const static vector<string> texturingModes = { "none" , "material","checkered", "texture", "normals" };

	int sizeX = 200;
	int sizeY = 300;

	int stdMargin = 20;
	int offsetXLeft = stdMargin;
	int offsetYTop = stdMargin;

	int offsetXRight = getWindowSize().x - sizeX - stdMargin;

	//mCameraSettings = params::InterfaceGl::create(getWindow(), "Camera", toPixels(ivec2(200, 200)));	


	//Cutsettings
	mCutSettings = params::InterfaceGl::create(getWindow(), "Cut Settings",
		toPixels(ivec2(sizeX, sizeY)));

	mCutSettings->addSeparator();
	
	mCutSettings->addParam("Cut Mode", vecCutTypes, reinterpret_cast<int*>(&mCutType));
	/*
	mCutSettings->addParam("U", &mSpaceParamU).min(0.0f).step(0.1f);
	mCutSettings->addParam("V", &mSpaceParamV).min(0.0f).step(0.1f);
	mCutSettings->addParam("W", &mSpaceParamW).min(0.0f).step(0.1f);
	*/

	mCutSettings->addParam("U", &mSpaceParamU).step(0.1f);
	mCutSettings->addParam("V", &mSpaceParamV).step(0.1f);
	mCutSettings->addParam("W", &mSpaceParamW).step(0.1f);

	mCutSettings->addParam("PosX", &mSpaceX).step(0.1f);
	mCutSettings->addParam("PosY", &mSpaceY).step(0.1f);
	mCutSettings->addParam("PosZ", &mSpaceZ).step(0.1f);

	mCutSettings->addParam("uCutAlpha", &mCutAlpha).min(0.0f).max(1.0f).step(0.05f);

	mCutSettings->addSeparator();

	mCutSettings->addParam("Selection", &mEnableSelect, true);

	mCutSettings->addButton("Select cut",
		std::bind(&Vis2App::enableSelect, this)
		);



	/////
	//Options
	mOptions = params::InterfaceGl::create(getWindow(), "General Options",
		toPixels(ivec2(sizeX, sizeY)));
	mOptions->addParam("Framerate", &mFps, true);
	mOptions->addParam("Face Culling",
		&mEnableFaceCulling).
		updateFn([this] { gl::enableFaceCulling(mEnableFaceCulling); });


	mOptions->addSeparator();
	mOptions->addParam("Draw Grid", &mShowGrid);

	mOptions->setPosition(vec2(stdMargin, stdMargin));
	//mOptions->addParam("")

	mOptions->addParam("Texturing Mode", texturingModes, reinterpret_cast<int*>(&mTextureType));

	mOptions->addButton("Open File",
		std::bind(&Vis2App::selectObjFileDialog, this)
		);

	///////////7
	//rigs
	//
	mRigs = params::InterfaceGl::create(getWindow(), "Rigs",
		toPixels(ivec2(sizeX, sizeY)));

	mRigs->addParam("LabelName", &mCutLabel);
	mRigs->addParam("Enabled", &mCutEnabled);
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


/**
<summary>

(Re)sets the dropdown list for cuts. 

</summary>
*/
void vis2::Vis2App::setupLabelList()
{
	mRigs->addParam("Rig", cutsLabelList, &mCutSelection);
}

/**

<summary> 

Performs a raycast from the mouse.xy screen coordinates
Returns true if the ray hits a precalculated bounding box of the object, holds position and normal as a pointer

</summary>

*/
bool Vis2App::performPicking(vec3 *pickedPoint, vec3 *pickedNormal) const
{
	if (!mCurrentTriMesh)
		return false;
	// Generate a ray from the camera into our world. Note that we have to
	// flip the vertical coordinate.
	float u = mMousePos.x / static_cast<float>(getWindowWidth());
	float v = mMousePos.y / static_cast<float>(getWindowHeight());
	auto ray = mCamera.generateRay(u, 1.0f - v, mCamera.getAspectRatio());

	// The coordinates of the bounding box are in object space, not world space,
	// so if the model was translated, rotated or scaled, the bounding box would not
	// reflect that. One solution would be to pass the transformation to the calcBoundingBox() function:
	//AxisAlignedBox worldBoundsExact = mTriMesh->calcBoundingBox(mTransform); // slow
	AxisAlignedBox worldBoundsExact = mCurrentTriMesh->calcBoundingBox();
	
	// But if you already have an object space bounding box, it's much faster to
	// approximate the world space bounding box like this:
	//mat4 mTransform(1.0f);
	// ReSharper disable once CppUseAuto
	//AxisAlignedBox worldBoundsApprox = mObjectBounds; // fast


	// Perform fast detection first - test against the bounding box itself.
	if (!worldBoundsExact.intersects(ray))
	//if(!worldBoundsApprox.intersects(ray))
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
			if (distance < result) 
			{
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
}