#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Vis2App : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void Vis2App::setup()
{
}

void Vis2App::mouseDown( MouseEvent event )
{
}

void Vis2App::update()
{
}

void Vis2App::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( Vis2App, RendererGl )
