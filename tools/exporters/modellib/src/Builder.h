/*
    Model builder tool
*/

#pragma once

#include <string>
#include <ostream>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

class Model
{
private:

	//Assimp importer
    Assimp::Importer m_imp;

	//Supported file extensions
	std::unordered_set<std::string> m_extensions;

    const aiScene* m_scene;
	std::string m_sceneFile;

    void attachAILogger(bool verbose);
    bool exportModel(std::ostream& output);
    bool exportMaterials(std::ostream& output);

public:

    Model();

    /*
        Import model file
    */
    bool imp(const std::string& modelFileName);

    /*
        Export model to output directory
    */
    bool exp(const std::string& outputDir);
    
    const std::unordered_set<std::string>& Model::getExtensions()
    {
        return m_extensions;
    }

    const std::string getErrorString()
    {
		return m_imp.GetErrorString();
    }
};
