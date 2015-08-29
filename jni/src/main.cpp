#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

int main(int argc,char** argv){
	osgViewer::Viewer viewer;

	viewer.setUpViewInWindow(64, 64, 800, 600);

	return viewer.run();
}
