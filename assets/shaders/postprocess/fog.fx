

	float4 fogColour = float4(0.8f, 0.8f, 0.8f, 1.0f);
	
	float fogStart = 20.0f;fogStart = 0.0f;
	float fogRange = length(input.posw - scene.eyePos);
	float fogDensity = 0.005;
	float fog = 1.0f / exp((fogRange - fogStart) * fogDensity);

	// float c = 1.0f;
	// float b = 0.5f;
	// float d = length(input.posw - scene.eyePos);
	// float fog = 1.0f - c*exp(-scene.eyePos.y*b) * (1.0f - exp( -d*vdir.y*b )) / vdir.y;
	
	fog = clamp(fog, 0.0, 1.0);	
	
	colour = lerp(fogColour, colour, fog);