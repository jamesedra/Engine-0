#include "../public/model.h"

void Model::loadModel(std::string path)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		// add to mesh data array
		meshDataList.push_back(processMeshData(mesh, scene));
	}

	// recurse through its children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

MeshData Model::processMeshData(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<TextureMetadata> textures;

	// populate vertices vector
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		// mvertices is assimps vertex position
		vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		if (mesh->HasNormals())
			vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		// check if mesh has texture coordinates we only check the index 0 since assimp meshes 
		// can store 8 different texture coordinates
		if (mesh->mTextureCoords[0]) {
			vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		
		// check if mesh has tangents
		if (mesh->HasTangentsAndBitangents()) {
			vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}

		vertices.push_back(vertex);
	}

	// populate indices vector
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// populate textures vector
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<TextureMetadata> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<TextureMetadata> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<TextureMetadata> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	}

	return MeshData{
		Mesh(std::move(vertices), std::move(indices)), std::move(textures)
	};
}

std::vector<TextureMetadata> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
	std::vector<TextureMetadata> textures;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string filename = str.C_Str();
		std::string fullpath = directory + "/" + filename;

		unsigned int texID;
		int width, height;

		try
		{
			Texture& cached = TextureLibrary::GetTexture(fullpath);
			texID = cached.id;
			width = cached.width;
			height = cached.height;
		}
		catch (const std::runtime_error&)
		{
			// texture wasn't in the library. Load and register it.
			TextureColorSpace space =
				(type == aiTextureType_DIFFUSE || type == aiTextureType_AMBIENT)
				? TextureColorSpace::sRGB
				: TextureColorSpace::Linear;

			texID = TextureFromFile(fullpath.c_str(), directory, width, height, space);
			TextureLibrary::Register(fullpath, texID, width, height);
		}
		TextureMetadata texture;
		texture.path = fullpath;
		texture.type = typeName;
		texture.id = texID;
		texture.width = width;
		texture.height = height;
		textures.push_back(texture);
	}
	return textures;
}

unsigned int Model::TextureFromFile(const char* path, const std::string& directory, int& width, int& height, TextureColorSpace space) {
	std::string filename = std::string(path);

	unsigned int textureID;
	glGenTextures(1, &textureID);
	stbi_set_flip_vertically_on_load(true); // test
	int nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		// check count of nrComponents from the data
		GLenum baseFormat;
		if (nrComponents == 1) baseFormat = GL_RED;
		else if (nrComponents == 3) baseFormat = GL_RGB;
		else if (nrComponents == 4) baseFormat = GL_RGBA;

		GLenum internalFormat = baseFormat;
		if (space == TextureColorSpace::sRGB) {
			if (baseFormat == GL_RGB)
				internalFormat = GL_SRGB;
			else if (baseFormat == GL_RGBA)
				internalFormat = GL_SRGB_ALPHA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, baseFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
	}
	stbi_image_free(data);
	return textureID;
}
