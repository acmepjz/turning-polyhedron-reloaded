// osgMyGUI, modified from osgRecipe <https://github.com/xarray/osgRecipes>
// Warning: this version of osgMyGUI only works under single-threaded mode.

#pragma once

#include <MYGUI/MyGUI.h>
#include <MYGUI/MyGUI_OpenGLPlatform.h>
#include <osg/Camera>
#include <osg/Drawable>
#include <osgGA/GUIEventHandler>
#include <osgViewer/GraphicsWindow>
#include <queue>

class MYGUIManager;

class MYGUIHandler : public osgGA::GUIEventHandler
{
public:
    MYGUIHandler( osg::Camera* c, MYGUIManager* m ) : _camera(c), _manager(m) {}
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );
    
protected:
    osg::observer_ptr<osg::Camera> _camera;
    MYGUIManager* _manager;
};

class MYGUIManager : public osg::Drawable, public MyGUI::OpenGLImageLoader
{
protected:
	~MYGUIManager();
public:
    MYGUIManager();
    MYGUIManager( const MYGUIManager& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    META_Object( osg, MYGUIManager )
    
    void setResourcePathFile( const std::string& file ) { _resourcePathFile = file; }
    const std::string& getResourcePathFile() const { return _resourcePathFile; }
    
    void setResourceCoreFile( const std::string& file ) { _resourceCoreFile = file; }
    const std::string& getResourceCoreFile() const { return _resourceCoreFile; }

	void setUIScale(float uiScale);
	float getUIScale() const { return _uiScale; }

	void setUseHWCursor(bool b);
	bool getUseHWCursor() const { return _useHWCursor; }
    
    void pushEvent( const osgGA::GUIEventAdapter* ea )
    { _eventsToHandle.push( ea ); }
    
    // image loader methods
    virtual void* loadImage( int& width, int& height, MyGUI::PixelFormat& format, const std::string& filename );
    virtual void saveImage( int width, int height, MyGUI::PixelFormat format, void* texture, const std::string& filename );
    
    // drawable methods
    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;
    virtual void releaseGLObjects( osg::State* state=0 ) const;

	void setGraphicsWindow(osgViewer::GraphicsWindow* gw) { _gw = gw; }

	static MYGUIManager* instance; //!< the global reference of the node which renders MyGUI
    
protected:
    virtual void updateEvents() const;
	virtual bool handleEvent(const osgGA::GUIEventAdapter& ea, bool async = false) const;
	virtual void setupResources();
	virtual void initializeControls();
    
    static MyGUI::MouseButton convertMouseButton( int button );
    static MyGUI::KeyCode convertKeyCode( int key );
    
    std::queue< osg::ref_ptr<const osgGA::GUIEventAdapter> > _eventsToHandle;
    MyGUI::Gui* _gui;
    MyGUI::OpenGLPlatform* _platform;
    std::string _resourcePathFile;
    std::string _resourceCoreFile;
    std::string _rootMedia;
    unsigned int _activeContextID;
    bool _initialized;
	osg::observer_ptr<osgViewer::GraphicsWindow> _gw;
	float _uiScale;
	bool _useHWCursor;

private:
	void notifyChangeMousePointer(const std::string& _name);

	friend class MYGUIHandler;
};
