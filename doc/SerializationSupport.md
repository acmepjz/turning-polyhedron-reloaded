[TOC]

The "second-generation" OSG format is introduced at the beginning of the year 2010, which is serialized, easy to extend, cross-format, and will be continuous updated to support all native OSG libraries.

The format reader writer plugin is located at `src/osgPlugins/osg/ReaderWriterOSG2.cpp` and wrappers at `src/osgWrappers/serializers`.

It supports two extensions at present:
 * `.osgb` The binary format
 * `.osgt` The ascii format

Supported options are:
  * `WriteImageHint=<hint>` (Export option) Hint of writing image to stream.
  * `<IncludeData>` writes Image::data() directly;
  * `<IncludeFile>` writes the image file itself to stream;
  * `<UseExternal>` writes only the filename;
  * `<WriteOut>` writes Image::data() to disk as external file.
The first two hints only affect binary formats. By default, the image writer will check `osg::Image::getWriteHint()` to decide if save data or only the filename.

 * `Compressor=<name>` (Export option) Use an inbuilt or user-defined compressor to encode the stream. These only affect binary formats.

 * `SchemaFile=<file>` (Import/Export option) Record an ascii schema file of writing properties orders, and use it while reading if necessary.

 * `ForceReadingImage` (Import option) Load an empty image with specified filename instead if required file missed. This is useful when converting from other formats. Image information won't be erased even without the external reference.

 * `Ascii` (Import/Export option) By default, the reading/writing format is guessed from the extension, and this option will force using the ascii format directly.

Main features:
 * **Serialization I/O**: Serialization is used for writing objects into memory buffer and "resurrected" them in the same or another computer environment, enabling simple and common input/output interfaces to be utilized. See `osgDB/Serializer` header for details.

 * **Binary/ascii file format**: A dual binary/ascii format is supported based on different implementations of I/O classes in the `osgDB/StreamOperator` header. These implementations could be found at `osgPlugins/osg` directory. The cross-format compatibility is provided by the object wrappers and their reader/writer classes.

 * **Object wrappers**: Every class in the scene graph should have a corresponding wrapper, in which a series of templated serializers are used to bind reading and writing members. The wrapper should record all necessary setting/getting members of the object class, as well as the I/O order of these properties. See `osgDB/ObjectWrapper` header for details.

 * **Compressors and decompressors**: The compressor is used while the whole scene graph is already recorded, to reduce data size and encrypt the result buffer for variant reasons. A decompressor method should also be provided for decoding. See the `osgDB/ObjectWrapper` header again. And the total output/input managers are declared in `osgDB/OutputStream` and `osgDB/InputStream` headers.

 * **Extendability**: The object wrappers and compressors are always extendable. You may easily write wrappers for user-customized class derived from `osg::Object` base class and load them from applications or dynamic libraries. The wrapper may contain a series of predefined and custom serializers.

 * **Schema Definitions**: The sequence of a class' properties is recorded in its wrapper class. These sequences could be write out as a "schema" list, which indicates the reading/writing orders of all existing classes in current OSG version. The schema may be used to force resort wrappers in another environment. According to the schema, any unrecognizable property name and disrupted orders will be automatically fixed while reading an external file. These will help applications to be backwards and forward compatible.

A quick start guide {#a}
===

1.1 Make use of osgconv {#b}
---

The osgconv will help us quickly realize all the features of the new OSG2 format. The commands below will generate and render a binary cow model:
~~~
# ./osgconv cow.osg cow.osgb
# ./osgviewer cow.osgb
~~~

And the ascii one:
~~~
# ./osgconv cow.osg cow.osgt
# ./osgviewer cow.osgt
~~~

To write out with specified writing image hint:
~~~
# ./osgconv cow.osg cow.osgb -O WriteImageHint=IncludeData
# ./osgviewer cow.osgb
~~~

Use an inbuilt zlib compressor:
~~~
# ./osgconv cow.osg cow.osgb -O Compressor=zlib
# ./osgviewer cow.osgb
~~~

Write out the schema at the same time, and use it to force resort reading orders:
~~~
# ./osgconv cow.osg cow.osgb -O SchemaFile=osg_schema_2.9.7.txt
# ./osgviewer cow.osgb -O SchemaFile=osg_schema_2.9.7.txt
~~~

1.2 In your applications {#c}
---

The required plugin is `osgdb_osg2.so` (`.dll`), and related wrappers are recorded in different `osgdb_serializers_*` libraries. And user compressors and decompressors may be saved in a `osgdb_compressor_*` library.

To read node file directly:
~~~{.cpp}
osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile("cow.osgb");
~~~

To write the scene into a binary file, with the "!WriteImageHint" and "Compressor" option:
~~~{.cpp}
osgDB::writeNodeFile(*node, "cow.osgb", new osgDB::Options("WriteImageHint=IncludeData Compressor=zlib"));
~~~

You may also use the stream I/O functions:
~~~{.cpp}
osgDB::ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension("osgt");
if (rw)
{
    osgDB::ReaderWriter::ReadResult rr = rw->readNode(istream);
    return rr.takeNode();
}
~~~

Note, to write out an ascii stream, you should manually add the "Ascii" option:
~~~{.cpp}
osgDB::ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension("osgt");
if (rw)
{
    rw->writeNode(*node, ostream, new osgDB::Options("Ascii"));
}
~~~

How to write extended wrappers {#d}
===

2.1 Basic structure {#e}
---

A wrapper completely and correctly records all necessary properties of a scene object and its proto. In the reading process, the wrapper will create a clone from the proto, reread properties and add it to the proper position in the scene graph.

A number of serializers in order are used to implement setting/getting methods of each recordable property. The class inheritance is provided to make sure that members of parent classes are also recorded.

The basic structure of creating a wrapper is:
~~~{.cpp}
REGISTER_OBJECT_WRAPPER( Node,                      // The unique wrapper name
                         new osg::Node,             // The proto
                         osg::Node,                 // The class typename
                         "osg::Object osg::Node" )  // The inheritance relations
{
    // Serializers for different members
    ADD_OBJECT_SERIALIZER( UpdateCallback, osg::NodeCallback, NULL );
    ADD_BOOL_SERIALIZER( CullingActive, true );
    ADD_HEXINT_SERIALIZER( NodeMask, 0xffffffff );
    ...
}
~~~

A lot of macro definitions are used here to provide an easy to read structure. `REGISTER_OBJECT_WRAPPER` will automatically add the wrapper to a global manager at initial time. And the `ADD_*_SERIALIZER` statements will bring different serializers for reading/writing class members in sequence of names, which may be output as part of schema file.

Notice that the "osg::" prefix in the inheritance string is important. It indicates the system that `osgdb_serializers_osg.so` library should be loaded and all its wrappers will be put into use. A different namespace, like `osgParticle`, will cause a different library to be loaded and made use of.

2.2 Predefined serializers {#f}
---

Obviously, the `ADD_BOOL_SERIALIZER(CullingActive)` above is sure to call `setCullingActive()` and `getCullingActive()` methods of `osg::Node` class to set/get a boolean property. and `ADD_HEXINT_SERIALIZER(NodeMask)` similarly calls `setNodeMask()` and `getNodeMask()` to operate on an integer one. There are some more predefined serializers to do such repeated work, on the premise that the class methods' naming styles are acceptable.

| Code | Description |
|------|-------------|
| `ADD_BOOL_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(bool)`</b> and <b>`bool getNAME() const`</b> methods. DEF is the default value of the proto, which will not be saved into files. |
| `ADD_SHORT_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(short)`</b> and <b>`short getNAME() const`</b> methods. |
| `ADD_USHORT_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(unsigned short)`</b> and <b>`unsigned short getNAME() const`</b> methods. |
| `ADD_HEXSHORT_SERIALIZER( NAME, DEF )` | Same as ADD_USHORT_SERIALIZER, but use hex values instead. |
| `ADD_USHORT_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(unsigned short)`</b> and <b>`unsigned short getNAME() const`</b> methods. |
| `ADD_INT_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(int)`</b> and <b>`int getNAME() const`</b> methods. |
| `ADD_UINT_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(unsigned int)`</b> and <b>`unsigned int getNAME() const`</b> methods. |
| `ADD_HEXINT_SERIALIZER( NAME, DEF )` | Same as ADD_UINT_SERIALIZER, but use hex values instead. |
| `ADD_FLOAT_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(float)`</b> and <b>`float getNAME() const`</b> methods. |
| `ADD_DOUBLE_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(double)`</b> and <b>`double getNAME() const`</b> methods. |
| `ADD_VEC3F_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const Vec3f&)`</b> and <b>`const Vec3f& getNAME() const`</b> methods. |
| `ADD_VEC3D_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const Vec3d&)`</b> and <b>`const Vec3d& getNAME() const`</b> methods. |
| `ADD_VEC3_SERIALIZER( NAME, DEF )` | Same as ADD_VEC3F_SERIALIZER. |
| `ADD_VEC4F_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const Vec4f&)`</b> and <b>`const Vec4f& getNAME() const`</b> methods. |
| `ADD_VEC4D_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const Vec4d&)`</b> and <b>`const Vec4d& getNAME() const`</b> methods. |
| `ADD_VEC4_SERIALIZER( NAME, DEF )` | Same as ADD_VEC4F_SERIALIZER. |
| `ADD_QUAT_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const Quat&)`</b> and <b>`const Quat& getNAME() const`</b> methods. |
| `ADD_PLANE_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const Plane&)`</b> and <b>`const Plane& getNAME() const`</b> methods. |
| `ADD_MATRIXF_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const Matrixf&)`</b> and <b>`const Matrixf& getNAME() const`</b> methods. |
| `ADD_MATRIXD_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const Matrixd&)`</b> and <b>`const Matrixd& getNAME() const`</b> methods. |
| `ADD_MATRIX_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const Matrix&)`</b> and <b>`const Matrix& getNAME() const`</b> methods. |
| `ADD_STRING_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(const std::string&)`</b> and <b>`const std::string& getNAME() const`</b> methods. |
| `ADD_GLENUM_SERIALIZER( NAME, TYPE, DEF )` | Input/output with <b>`void setNAME(TYPE)`</b> and <b>`TYPE getNAME() const`</b> methods. TYPE here could be `GLenum`, `GLbitfield`, `GLint` and so on, to fit different method parameters. In ascii format, this serializer gets numerical values and saves corresponding OpenGL enumeration names to the buffer, and read it back in the opposite way. For example, it will map `GL_NEVER` to the string "NEVER", and vice versa. |
| `ADD_OBJECT_SERIALIZER( NAME, TYPE, DEF )` | Input/output with <b>`void setNAME(TYPE*)`</b> and <b>`const TYPE* getNAME() const`</b> methods. This serializer is used to record another object attached, that is, the wrapper of another object class will be called inside current reading/writing function and cause iteration of functions. |
| `ADD_IMAGE_SERIALIZER( NAME, TYPE, DEF )` | Same as ADD_OBJECT_SERIALIZER, but only read osg::Image* and inherited instances. |
| `ADD_LIST_SERIALIZER( NAME, TYPE )` | Input/output with <b>`void setNAME(const TYPE&)`</b> and <b>`const TYPE& getNAME() const`</b> methods. TYPE should be a `std::vector` like typename, because the serializer will assume a TYPE::const_iterator internal to traverse all elements. |
| `ADD_USER_SERIALIZER( NAME )` | Add a user-customizied serializer, with at least 3 static user functions for checking, reading and writing properties. See Chapter 2.3 for details. |
| `BEGIN_ENUM_SERIALIZER( NAME, DEF )` | Input/output with <b>`void setNAME(NAME)`</b> and <b>`NAME getNAME() const`</b> methods. This is used only for enum values, and the enum name and methods' names should strictly obey the naming rules. Another two macros `ADD_ENUM_VALUE` and `END_ENUM_SERIALIZER` will be also used to form a complete serializer. |

An example of `BEGIN_ENUM_SERIALIZER` is in the `osg::Object` wrapper:
~~~{.cpp}
BEGIN_ENUM_SERIALIZER( DataVariance, UNSPECIFIED );
    ADD_ENUM_VALUE( STATIC );
    ADD_ENUM_VALUE( DYNAMIC );
    ADD_ENUM_VALUE( UNSPECIFIED );
END_ENUM_SERIALIZER();
~~~

The enum type `osg::Object::DataVariance` has 3 values: `STATIC`, `DYNAMIC` and `UNSPECIFIED` (default). They are all recorded in the serializer and in ascii format, will be automatically mapped to strings "STATIC", "DYNAMIC" and "UNSPECIFIED".

Sometimes the enum type is not declared in the form of `Class::NAME`, and the `BEGIN_ENUM_SERIALIZER` will failed then. Use `BEGIN_ENUM_SERIALIZER2` instead at this time, for example:
~~~{.cpp}
BEGIN_ENUM_SERIALIZER2( Hint, osg::Multisample::Mode, DONT_CARE );
    ADD_ENUM_VALUE( FASTEST );
    ADD_ENUM_VALUE( NICEST );
    ADD_ENUM_VALUE( DONT_CARE );
END_ENUM_SERIALIZER();
~~~

Here the bound methods are <b>`void setHint(osg::Multisample::Mode)`</b> and <b>`osg::Multisample::Mode getHint() const`</b>.

With the predefinied serializers, we may easily add wrappers for most simple classes. A simple example is listed below:
~~~{.cpp}
REGISTER_OBJECT_WRAPPER( Box,
                         new osg::Box,
                         osg::Box,
                         "osg::Object osg::Shape osg::Box" )
{
    ADD_VEC3_SERIALIZER( Center, osg::Vec3() );  // _center
    ADD_VEC3_SERIALIZER( HalfLengths, osg::Vec3() );  // _halfLengths
    ADD_QUAT_SERIALIZER( Rotation, osg::Quat() );  // _rotation
}
~~~

With no more than 10 lines, the `osg::Box` class wrapper is finished! In binary mode, it will be saved as a series of float and double values in bits. In ascii mode, the output text may look like:
~~~{.cpp}
osg::Box {
    ...
    Center 10 0 0
    HalfLengths 1 1 1
    Rotation 1 0 0 1
    ...
}
~~~

The properties of `osg::Shape` and `osg::Object` will also be recorded, unless they are not motioned in the inheritance string.

2.3 Custom serializers {#g}
---

There are often some member methods that should be recorded but don't obey all above naming rules for some reason. For instance, the `setTextureAttribute()` and `getTextureAttribute()` pairs of the `osg::StateSet` class. Both methods have an extra incoming parameter "unit" and can't be accepted by any prebuilt serializers. In this case, `ADD_USER_SERIALIZER` will be required to help.

Take the `osg::Group` wrapper as an example. A `Group` has multiple child nodes, but it doesn't have a `setChildren()` or `getChildren()` method. So a custom serializer writing children into files and reading them back is generated as below:
~~~{.cpp}
static bool checkChildren( const osg::Group& node );
static bool writeChildren( osgDB::OutputStream& os, const osg::Group& node );
static bool readChildren( osgDB::InputStream& is, osg::Group& node );

REGISTER_OBJECT_WRAPPER( Group,
                         new osg::Group,
                         osg::Group,
                         "osg::Object osg::Node osg::Group" )
{
    ADD_USER_SERIALIZER( Children );  // _children
}
~~~

The `ADD_USER_SERIALIZER` macro, named "Children" and work for `osg::Group` class, will look for 3 static global functions in compile-time:
 * <b>`bool checkChildren(const osg::Group&)`</b> is used to check if the property should be recorded or not this time. Null pointers, default initial values, and 0 sized lists could always be ignored and not written to files. To tell the serializer to continue writing the bound property, return TRUE, otherwise FALSE.
~~~{.cpp}
return node.getNumChildren()>0;  // Continue only if there is any child node to write
~~~

 * <b>`bool writeChildren(osgDB::!OutputStream&, const osg::Group&)`</b> is going to save the property to buffer with the `OutputStream` manager. It would usually call the getChild() function and write out child objects in a loop. Return TRUE if all is normal.
~~~{.cpp}
unsigned int size = node.getNumChildren();
os << size << osgDB::BEGIN_BRACKET << std::endl;
for ( unsigned int i=0; i<size; ++i )
{
    os.writeObject( node.getChild(i) );
}
os << osgDB::END_BRACKET << std::endl;
return true;
~~~

 * <b>`bool readChildren(osgDB::!InputStream&, osg::Group&)`</b> is going to read data from the `InputStream` manager and set to the `Group` instance, using the `addChild()` method here. Return `TRUE` if there is nothing to warn during reading.
~~~{.cpp}
unsigned int size = 0; is >> size >> osgDB::BEGIN_BRACKET;
for ( unsigned int i=0; i<size; ++i )
{
    osg::Node* child = dynamic_cast<osg::Node*>( is.readObject() );
    if ( child ) node.addChild( child );
}
is >> osgDB::END_BRACKET;
return true;
~~~

The `OutputStream` accepts `<<` operators on common data types directly. And you may use `writeObject()`, `writeImage()`, `writePrimitiveSet()` and `writeArray()` to apply specified OSG objects. Similarly, the `InputStream` could use `>>` operators, `readObject()`, `readImage()`, `readPrimitiveSet()`` and `readArray()` to reread data from files and data buffer.

You will also notice that `BEGIN_BRACKET` and `END_BRACKET` macrodefinitions are used here. Actually, they tell there will be a bracket to indicate indentation and following subitems. Another useful helper macro is `PROPERTY`, which is used to mark out a property name and automatically check it while reading. `PROPERTY` is also planned to be used in XML mode later.

In ascii mode, such an output:
~~~{.cpp}
os << osgDB::PROPERTY("Account") << osgDB::BEGIN_BRACKET << std::endl;
os << osgDB::PROPERTY("ID") << (int)1 << std::endl;
os << osgDB::PROPERTY("Name"); os.writeWrappedString("Wang Rui"); os << std::endl;
os << osgDB::PROPERTY("Salary") << (float)25.5 << std::endl;
os << osgDB::END_BRACKET << std::endl;
~~~

Will result in:
~~~{.cpp}
Account {
    ID 1
    Name "Wang Rui"
    Salary 25.5
}
~~~

And to read the information back, just invert the operators and omit the endl symbol:
~~~{.cpp}
std::string name; int id; float salary;
is >> osgDB::PROPERTY("Account") >> osgDB::BEGIN_BRACKET;
is >> osgDB::PROPERTY("ID") >> id;
is >> osgDB::PROPERTY("Name"); is.readWrappedString(name);
is >> osgDB::PROPERTY("Salary") >> salary;
is >> osgDB::END_BRACKET;
~~~

**Caution:** the `PROPERTY` and `std::string` contents should not have any space inside, if work with `<<` and `>>` operators. That is because the input stream will use spaces as separators and thus breaks the string itself and the reading order. Use `writeWrappedString()` and `readWrappedString()` instead.

The `BEGIN_BRACKET`, `END_BRACKET` and `PROPERTY` macros have no effects in binary mode.

There are some more convenience functions and macros for custom serializer programmers:
 * <b>`InputStream::matchString(const std::string&)`</b> checks if next token in the input stream matches its parameter. If not, it will rollback. This method only works in ascii mode.

 * <b>`InputStream::advanceToCurrentEndBracket()`</b> will keep reading and discarding data until it meets a `END_BRACKET` which is believed to end current block. This method only works in ascii mode.

 * <b>`InputStream::throwException(const std::string&)`</b> and <b>`OutputStream::throwException(const std::string&)`</b> will throw a fatal exception and stop the reading/writing processes.

 * <b>`BEGIN_USER_TABLE`</b> macro is useful for user serializers to obtain a similar capacity of `BEGIN_ENUM_SERIALIZER`. You may define an enumeration table, a global read function and a write function, and use them to map values to strings in ascii mode. For example,
~~~{.cpp}
BEGIN_USER_TABLE( Mode, osg::PolygonMode );
    ADD_USER_VALUE( POINT );
    ADD_USER_VALUE( LINE );
    ADD_USER_VALUE( FILL );
END_USER_TABLE()

USER_READ_FUNC( Mode, readModeValue )
USER_WRITE_FUNC( Mode, writeModeValue )
~~~

To write out the polygon mode:
~~~{.cpp}
writeModeValue(os, (int)pm.getMode(osg::PolygonMode::FRONT));
~~~

And to read it back in user serializers:
~~~{.cpp}
int value = readModeValue(is);
pm.setMode(osg::PolygonMode::FRONT, static_cast<osg::PolygonMode::Mode>(value));
~~~

2.4 Custom compressor/decompressor {#h}
---

We have already had a zlib compressor, which is defined in `src/osgDB/Compressor.cpp`. But we could easily extend user compressors, with only two virtual functions overrided.

The example source code below will simply add an author information at the first of each result file. Note that `REGISTER_COMPRESSOR` macro should be declared somewhere.
~~~{.cpp}
class TestCompressor : public osgDB::BaseCompressor
{
public:
    TestCompressor() {}
    
    virtual bool compress( std::ostream& fout, const std::string& src )
    {
        std::string info("Written by Wang Rui, (C) 2010");
        int infoSize = info.size();
        fout.write( (char*)&infoSize, INT_SIZE );
        fout.write( info.c_str(), infoSize );
        
        int size = src.size();
        fout.write( (char*)&size, INT_SIZE );
        fout.write( src.c_str(), src.size() );
        return true;
    }
    
    virtual bool decompress( std::istream& fin, std::string& target )
    {
        std::string info;
        int infoSize = 0; fin.read( (char*)&infoSize, INT_SIZE );
        if ( infoSize )
        {
            info.resize( infoSize );
            fin.read( (char*)info.c_str(), infoSize );
            osg::notify(osg::INFO) << info << std::endl;
        }
        
        int size = 0; fin.read( (char*)&size, INT_SIZE );
        if ( size )
        {
            target.resize( size );
            fin.read( (char*)target.c_str(), size );
        }
        return true;
    }
};

REGISTER_COMPRESSOR( "test", TestCompressor )
~~~

The new `TestCompressor` class could either be placed in user applications or an `osgdb_compressor_test.so` library. Use the command to work with binary formats.
~~~
# ./osgconv cow.osg cow.osgb -O Compressor=test
# ./osgviewer cow.osgb
~~~

Todo {#i}
===

 1. Go on finish all other osg core class wrappers, besides osg, osgText and osgParticle. And do comprehensive tests to put the new mechanism and plugin into public use as soon as possible.

 2. Add support for XML, without big changes to current class wrappers.

 3. Compress float arrays and integer arrays to reduce file sizes, if necessary.

 4. Write wrapper properties schema to the header of binary files if needed. This will improve the compatibilities of files created by different !OpenSceneGraph versions. We could also keep inbuilt schema for each stable osg releases and match the binary file version automatically.

 5. Consider a method to replace parts of the functionalities of `osgIntrospection`, which provides an introspection/reflection framework for runtime querying and calling of class properties. This may be done by serializers in a slightly different way now.

 6. So, what is next? :)
