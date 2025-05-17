#include "../public/terrain.h"

bool Terrain::LoadHeightMap(const char* filename)
{
	int width, depth, nChannels;
	unsigned char* img = stbi_load(filename, &width, &depth, &nChannels, STBI_grey);
	if (!img)
	{
		std::cerr << "Error: internal format not supported." << std::endl;
		return false;
	}

	// if there is a height data, unload it first
	if (!heightData.data.empty()) UnloadHeightData();

	SetHeightDataDimensions(width, depth);
	heightData.data.resize(width * depth);
	for (int i = 0; i < width * depth; i++)
	{
		heightData.data[i] = img[i] / 255.0f; // normalize to 0 to 1
	}
	stbi_image_free(img);
	
	return true;
}

bool Terrain::GenerateFaultHeightData(int iterations, float filter, int width, int depth)
{
	if (!heightData.data.empty()) UnloadHeightData();

	if (width > 0 && depth > 0) SetHeightDataDimensions(width, depth);
	else SetHeightDataDimensions();

	// populate placeholder values on data
	heightData.data.assign(heightData.width * heightData.depth, 0.0f);

	for (int i = 0; i < iterations; i++)
	{
		float iterationRatio = ((float)i / (float)iterations);
		float height = 1.0f - iterationRatio;

		glm::ivec2 p1, p2;
		p1.x = rand() % heightData.width;
		p1.y = rand() % heightData.depth; // treat y as the z axis

		int count = 0;
		do
		{
			p2.x = rand() % heightData.width;
			p2.y = rand() % heightData.depth;
			if (count++ == 1000)
			{
				std::cerr << "Endless loop detected" << std::endl;
				assert(0);
			}
		} while (p1.x == p2.x && p1.y == p2.y);

		int dx = p2.x - p1.x;
		int dz = p2.y - p1.y;

		for (int z = 0; z < heightData.depth; z++)
		{
			for (int x = 0; x < heightData.width; x++)
			{
				int dx_in = x - p1.x;
				int dz_in = z - p1.y;

				int cross = dx_in * dz - dx * dz_in;

				// increment height if in the positive side
				if (cross > 0)
				{
					float currHeight = GetTrueHeightAtPoint(x, z);
					SetHeightAtPoint(currHeight + height, x, z);
				}
			}
		}
	}
	ApplyFIRFilter(filter);
	NormalizeHeightData();

	return true;
}

bool Terrain::SaveHeightMap(const char* filename)
{
	// TODO:
	return false;
}

bool Terrain::UnloadHeightData()
{
	if (!heightData.data.empty()) heightData.unload();
	return true;
}

void Terrain::ApplyIIRFilter(float filter)
{
	if (heightData.data.empty())
	{
		std::cerr << "Terrain::ApplyFIRFilter called when data is empty." << std::endl;
		return;
	}

	// left to right
	for (int z = 0; z < heightData.depth; z++)
	{
		float prevVal = GetTrueHeightAtPoint(0, z);
		for (int x = 1; x < heightData.width; x++)
		{
			float currVal = GetTrueHeightAtPoint(x, z);
			float newVal = filter * prevVal + (1.0f - filter) * currVal;
			SetHeightAtPoint(newVal, x, z);

			prevVal = newVal;
		}
	}

	// right to left
	for (int z = 0; z < heightData.depth; z++)
	{
		float prevVal = GetTrueHeightAtPoint(0, heightData.depth - 1);
		for (int x = heightData.width - 2; x >= 0; x--)
		{
			float currVal = GetTrueHeightAtPoint(x, z);
			float newVal = filter * prevVal + (1.0f - filter) * currVal;
			SetHeightAtPoint(newVal, x, z);

			prevVal = newVal;
		}
	}

	// bottom to top
	for (int x = 0; x < heightData.width; x++)
	{
		float prevVal = GetTrueHeightAtPoint(x, 0);
		for (int z = 1; z < heightData.depth; z++)
		{
			float currVal = GetTrueHeightAtPoint(x, z);
			float newVal = filter * prevVal + (1.0f - filter) * currVal;
			SetHeightAtPoint(newVal, x, z);

			prevVal = newVal;
		}
	}

	// top to bottom
	for (int x = 0; x < heightData.width; x++)
	{
		float prevVal = GetTrueHeightAtPoint(heightData.width - 1, 0);
		for (int z = heightData.depth - 2; z >= 0; z--)
		{
			float currVal = GetTrueHeightAtPoint(x, z);
			float newVal = filter * prevVal + (1.0f - filter) * currVal;
			SetHeightAtPoint(newVal, x, z);

			prevVal = newVal;
		}
	}
}

void Terrain::NormalizeHeightData()
{
	if (heightData.data.empty())
	{
		std::cerr << "Terrain::NormalizeHeightData called when data is empty." << std::endl;
		return;
	}

	auto mm = std::minmax_element(heightData.data.begin(), heightData.data.end());
	float curMin = *mm.first;
	float curMax = *mm.second;

	float delta = float(curMax) - float(curMin);
	if (delta <= 0.0f) return;

	for (auto& d : heightData.data)
		d = (float(d) - float(curMin)) / delta;		// normalized coordinates
}
