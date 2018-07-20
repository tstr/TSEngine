/*
    Material reader
*/

#include <tscore/pathutil.h>

#include "../util/INIReader.h"
#include "MaterialReader.h"

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////

MaterialReader::MaterialReader(GraphicsSystem* gfx, const Path& fileName)
{
	INIReader matReader(fileName);

	vector<String> materialSections;
	vector<String> materialProperies;
	String propValue;
	matReader.getSections(materialSections);

	Path root = fileName.getParent();

	//Helper function
	auto getVectorProperty = [](const String& value) -> Vector
	{
		if (trim(value) == "")
			return Vector();

		Vector v;
		vector<string> tokens = split(value, ',');

		if (tokens.size() > 0) v.x() = stof(tokens[0]);
		if (tokens.size() > 1) v.y() = stof(tokens[1]);
		if (tokens.size() > 2) v.z() = stof(tokens[2]);
		if (tokens.size() > 3) v.w() = stof(tokens[3]);

		return v;
	};

	auto getImageProperty = [&](const String& value) -> ImageView
	{
		if (trim(value) == "")
			return ImageView();

		Path imgPath(root);
		imgPath.addDirectories(value);

		if (isFile(imgPath))
		{
			return gfx->getImage(imgPath).getView();
		}

		return ImageView();
	};

	for (const auto& section : materialSections)
	{
		PhongMaterial matInfo;

		//Format key helper
		auto fmtKey = [&section](auto key)
		{
			return (String)section + "." + key;
		};

		/*
			Properties
		*/

		matReader.getProperty(fmtKey("alpha"), matInfo.enableAlpha);
		matReader.getProperty(fmtKey("shininess"), matInfo.specularPower);

		if (matReader.getProperty(fmtKey("diffuseColour"), propValue))
			matInfo.diffuseColour = getVectorProperty(propValue);
		propValue.clear();
		if (matReader.getProperty(fmtKey("ambientColour"), propValue))
			matInfo.ambientColour = getVectorProperty(propValue);
		propValue.clear();
		if (matReader.getProperty(fmtKey("emissiveColour"), propValue))
			matInfo.emissiveColour = getVectorProperty(propValue);
		propValue.clear();

		/*
			Images
		*/

		if (matReader.getProperty(fmtKey("diffuseMap"), propValue))
			matInfo.diffuseMap = getImageProperty(propValue);
		propValue.clear();

		if (matReader.getProperty(fmtKey("normalMap"), propValue))
			matInfo.normalMap = getImageProperty(propValue);
		propValue.clear();

		if (matReader.getProperty(fmtKey("specularMap"), propValue))
			matInfo.specularMap = getImageProperty(propValue);
		propValue.clear();

		if (matReader.getProperty(fmtKey("displacementMap"), propValue))
			matInfo.displacementMap = getImageProperty(propValue);
		propValue.clear();

		String name(section);
		toLower(name);
		m_infoMap[name] = matInfo;
	}
}

PhongMaterial MaterialReader::find(const String& name) const
{
    String _name(name);
    toLower(_name);
    
    auto it = m_infoMap.find(_name);
    
    return (it != m_infoMap.end()) ? it->second : PhongMaterial();
}

bool MaterialReader::has(const String& name) const
{
	String _name(name);
	toLower(_name);

	auto it = m_infoMap.find(_name);

	return (it != m_infoMap.end());
}

///////////////////////////////////////////////////////////////////////////////
