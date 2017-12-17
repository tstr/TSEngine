##TODO:

* Tools
	* Write Annotation parser for shader compiler (started).
	* Store shader reflection data properly.
	* Add importing of outside resource schemas to rcschema compiler.
	* Convert buildassets.bat script to automated CMake custom target.
	* Rewrite 3D model builder tool using new resource system.
	* Write a texture conversion tool using new resource system.
	* Refactor shader preprocessor.
* Graphics
	* Improve architecture of renderer (IRenderDriver -> GraphicsSystem -> GraphicsContext -> Scene).
	* Improve management of graphics object lifetime (use the Qt approach using an object tree).
	* Refactor IRender interface
		* Divide draw commands into smaller separate chunks (PSO's, Resource Lists).