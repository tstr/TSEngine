


inline constexpr uint32 fmtSig(char a, char b, char c, char d)
{
	return (d << 24) | (c << 16) | (b << 8) | (a << 0);
}

static const uint32 SVTX = fmtSig('S', 'V', 'T', 'X');
static const uint32 SGEO = fmtSig('S', 'G', 'E', 'O');
static const uint32 STEC = fmtSig('S', 'T', 'E', 'C');
static const uint32 STEV = fmtSig('S', 'T', 'E', 'V');
static const uint32 SPIX = fmtSig('S', 'P', 'I', 'X');


EShaderManagerStatus CShaderManager::Manager::loadProgramStage(HShader& hShader, EShaderStage stageEnum, const tsr::ShaderStage& stage)
{
	uint32 signature = 0;
    
    
    

	switch (stageEnum)
	{
	case EShaderStage::eShaderStageVertex:
		signature = 
		break;
	case EShaderStage::eShaderStageGeometry:
		signature = formatSignature;
		break;
	case EShaderStage::eShaderStageTessCtrl:
		signature = formatSignature;
		break;
	case EShaderStage::eShaderStageTessEval:
		signature = formatSignature;
		break;
	case EShaderStage::eShaderStagePixel:
		signature = ;
		break;
	}

	//Determine which compiled stages to load
	const EGraphicsAPIID apiid = system->getApiID();

	//Verify stage signature
	if (signature != stage.signature())
	{
		return eShaderManagerStatus_StageCorrupt;
	}

	switch (apiid)
	{
	case eGraphicsAPI_D3D11:

		//If stage is present
		if (stage.has_code_hlslSM5())
		{
			//Create stage
			ERenderStatus status = system->getApi()->createShader(
				hShader,
				stage.code_hlslSM5().data(),
				(uint32)stage.code_hlslSM5().length(),
				stageEnum
			);

			if (status)
			{
				return eShaderManagerStatus_StageCorrupt;
			}
		}
		else
		{
			//Exit failure
			return eShaderManagerStatus_StageCorrupt;
		}

		break;

	default:
		return eShaderManagerStatus_StageNotFound;
	}

	return eShaderManagerStatus_Ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Manager methods

EShaderManagerStatus CShaderManager::Manager::loadProgram(const string& shaderName, ShaderId& id)
{
	Path shaderFile = shaderPath;
	shaderFile.addDirectories(shaderName + ".shader");

	auto it = programMap.find(shaderName);

	if (it == programMap.end())
	{
		if (!isFile(shaderFile))
		{
			return eShaderManagerStatus_ProgramNotFound;
		}

		rc::ResourceLoader loader(ifstream(shaderFile.str(), ios::binary));

		if (loader.fail())
		{
			return eShaderManagerStatus_Fail;
		}

		tsr::Shader& shaderRsc = loader.deserialize<tsr::Shader>();

		//Verify signature
		if (shaderRsc.signature() != formatSignature('T', 'S', 'S', 'H'))
		{
			return eShaderManagerStatus_StageCorrupt;
		}

		//Load each stage if they exist
		SShaderProgram program;

		if (shaderRsc.has_vertex())
		{
			if (auto err = loadProgramStage(program.hVertex, EShaderStage::eShaderStageVertex, shaderRsc.vertex()))
				return err;
		}

		if (shaderRsc.has_tessControl())
		{
			if (auto err = loadProgramStage(program.hHull, EShaderStage::eShaderStageTessCtrl, shaderRsc.tessControl()))
				return err;
		}

		if (shaderRsc.has_tessEval())
		{
			if (auto err = loadProgramStage(program.hDomain, EShaderStage::eShaderStageTessEval, shaderRsc.tessEval()))
				return err;
		}

		if (shaderRsc.has_geometry())
		{
			if (auto err = loadProgramStage(program.hGeometry, EShaderStage::eShaderStageGeometry, shaderRsc.geometry()))
				return err;
		}

		if (shaderRsc.has_pixel())
		{
			if (auto err = loadProgramStage(program.hPixel, EShaderStage::eShaderStagePixel, shaderRsc.pixel()))
				return err;
		}

		programs.push_back(program);
		id = (uint32)programs.size(); //ShaderId is equal to the index of the shader instance + 1 ie. the new size of the program cache
		programMap.insert(make_pair(shaderName, id));
	}
	else
	{
		id = it->second;
	}

	return eShaderManagerStatus_Ok;
}


