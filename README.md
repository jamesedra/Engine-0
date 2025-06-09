# Engine-0
This is an OpenGL graphics engine made for learning how graphics engines can be built and structured.
![cover](https://github.com/user-attachments/assets/c66d94d6-789a-4c7e-b344-a765250ad3c4)



## Features
### System Architecture
The system uses a custom variant of the entity component system (ECS) architecture. Modules are listed below:
- Entity: Simple ID
- Components: Data modules (eg. Transform, Shader, Asset components)
- Systems: Handles logic based on passed component combinations
- Managers: Stores type/s of components with its entity
- Factories: Static functions that creates sets of components to be tied to a created entity
- Libraries: Static libraries that stores references of instanced classes which can be retrieved globally
- Loaders: Static functions that create classes for said components

### Deferred Rendering:
   1. Geometry pass
   2. PBR lighting
   3. Image-based lighting (IBL)
   4. Skybox
   5. Directional shadows (VSM)
   7. Post-processing (HDR, SSAO, Gamma, Tone-mapping, SSAO)

<img src="https://github.com/user-attachments/assets/cc4ca711-54e8-43b2-91e7-a4f1689d1b46" width="100%">
<img src="https://github.com/user-attachments/assets/bf19ac3c-a4e0-47b0-8c3c-ee9ef8c8e602" width="100%">

### Material Pipeline
Material pipeline is mainly used in the geometry pass, includes albedo, roughness, metallic, normal, and AO maps. 
These are configurable (eg. terrain uses multiple maps for the auto material) as long as a proper geometry shader is used and implemented.

### Asset Loading
Assimp is used to load assets and parsed as a mesh data set. Due to the material pipeline, textures are also packaged and used as default values based on the texture type.

<img src="https://github.com/user-attachments/assets/4f9ef92e-c12e-47a2-a82f-c85313c88bdb" width="50%"><img src="https://github.com/user-attachments/assets/3e31f426-c5cd-489e-81ff-19e6a38dad20" width="50%">

### Terrain System
Mainly uses Geomipmapping with patch-based LOD based on world-space camera distance and per patch bounding box.

Other features include frustum culling and difference height data generation types (heightmap loading, fault generation, midpoint displacement)

Brute force and tessellation based terrain were also implemented.

### Environment Probe System
Used mainly for IBL via nearest probes selection blending

<img src="https://github.com/user-attachments/assets/11ca78fa-84aa-4e66-a5c9-e5a7f08b670e" width="50%"><img src="https://github.com/user-attachments/assets/d2d8005f-5dae-4a59-a281-ac16eb8bea33" width="50%">


### Tiled Shading for Lighting System

### GUI - ImGUI
