#pragma once
#include "ibl_generator.h"

class ProbeLibrary
{
public:
	static void Register(
		const std::string& path, 
		uint32_t envSize = 512, 
		uint32_t irradianceSize = 32, 
		uint32_t prefilterSize = 128, 
		uint32_t brdfLUTSize = 512, 
		uint32_t maxMipLevels = 5)
	{
		if (GetLibrary().empty())
			InitializeLibrary();
		IBLSettings settings{ 
			envSize, 
			irradianceSize, 
			prefilterSize, 
			brdfLUTSize, 
			maxMipLevels, 
			path };

		GetLibrary()[path] = settings;
	}

	static IBLSettings& GetSettings(const std::string& key)
	{
		if (GetLibrary().empty())
			InitializeLibrary();
		auto it = GetLibrary().find(key);
		if (it != GetLibrary().end())
			return it->second;
		else
			throw std::runtime_error("IBL setting not found: " + key);
	}

	static std::vector<const char*> GetLibraryKeys()
	{
		auto& lib = GetLibrary();
		std::vector<const char*> keys;
		keys.reserve(lib.size());
		for (const auto& pair : lib)
		{
			keys.push_back(pair.first.c_str());
		}
		return keys;
	}

private:
	static std::unordered_map<std::string, IBLSettings>& GetLibrary()
	{
		static std::unordered_map<std::string, IBLSettings> library;
		return library;
	}

	static void InitializeLibrary()
	{
		IBLSettings PresetA{};
		PresetA.eqrMapPath = "resources/textures/eqr_maps/autumn_field_puresky_2k.hdr";
		IBLSettings PresetB{};
		PresetB.eqrMapPath = "resources/textures/eqr_maps/kloofendal_43d_clear_puresky_2k.hdr";
		IBLSettings PresetC{};
		PresetC.eqrMapPath = "resources/textures/eqr_maps/kloppenheim_06_puresky_2k.hdr";
		IBLSettings PresetD{};
		PresetD.eqrMapPath = "resources/textures/eqr_maps/newport_loft.hdr";
		IBLSettings PresetE{};
		PresetE.eqrMapPath = "resources/textures/eqr_maps/rogland_clear_night_2k.hdr";
		IBLSettings PresetF{};
		PresetF.eqrMapPath = "resources/textures/eqr_maps/nightsky.hdr";

		GetLibrary().emplace(PresetA.eqrMapPath, std::move(PresetA));
		GetLibrary().emplace(PresetB.eqrMapPath, std::move(PresetB));
		GetLibrary().emplace(PresetC.eqrMapPath, std::move(PresetC));
		GetLibrary().emplace(PresetD.eqrMapPath, std::move(PresetD));
		GetLibrary().emplace(PresetE.eqrMapPath, std::move(PresetE));
		GetLibrary().emplace(PresetF.eqrMapPath, std::move(PresetF));
	}
};