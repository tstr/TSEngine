/*
    Material reader
*/

#include "../util/INIReader.h"
#include "MaterialReader.h"

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////

MaterialReader::MaterialReader(const Path& fileName)
{
	INIReader matReader(fileName);

	vector<String> materialSections;
	vector<String> materialProperies;
	String propKey;
	matReader.getSections(materialSections);

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

	for (const auto& section : materialSections)
	{
		MaterialCreateInfo matInfo;

		//Format key helper
		auto fmtKey = [&section](auto key)
		{
			return (String)section + "." + key;
		};

		float alpha;
		float shininess;
		matReader.getProperty(fmtKey("alpha"), alpha);
		matReader.getProperty(fmtKey("shininess"), shininess);

		if (matReader.getProperty(fmtKey("diffuseColour"), propKey))
			matInfo.constants.diffuseColour = getVectorProperty(propKey);
		propKey.clear();
		if (matReader.getProperty(fmtKey("ambientColour"), propKey))
			matInfo.constants.ambientColour = getVectorProperty(propKey);
		propKey.clear();
		if (matReader.getProperty(fmtKey("emissiveColour"), propKey))
			matInfo.constants.emissiveColour = getVectorProperty(propKey);
		propKey.clear();

		for (const char* imageKey : {
			"diffuseMap",
			"normalMap",
			"specularMap",
			"displacementMap"
		})
		{
			propKey.clear();

			if (matReader.getProperty(fmtKey(imageKey), propKey))
			{
				if (propKey != "")
				{
					//Resolve image path
					Path a(matReader.getPath().getParent());
					a.addDirectories(propKey);

					matInfo.images[imageKey] = a;
				}
			}
		}

		String name(section);
		toLower(name);
		m_infoMap[name] = matInfo;
	}
}

MaterialCreateInfo MaterialReader::find(const String& name) const
{
    String _name(name);
    toLower(_name);
    
    auto it = m_infoMap.find(_name);
    
    return (it != m_infoMap.end()) ? it->second : MaterialCreateInfo();
}

bool MaterialReader::has(const String& name) const
{
	String _name(name);
	toLower(_name);

	auto it = m_infoMap.find(_name);

	return (it != m_infoMap.end());
}

///////////////////////////////////////////////////////////////////////////////
