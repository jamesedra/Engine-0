# Engine-0
This is an OpenGL graphics engine made for learning how graphics engines can be built and structured.

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
   5. Directional shadows (VSM)
   7. Post-processing (HDR, SSAO, Gamma, Tone-mapping, SSAO)
      
### Material Pipeline
Material pipeline is mainly used in the geometry pass, includes albedo, roughness, metallic, normal, and AO maps. 
These are configurable (eg. terrain uses multiple maps for the auto material) as long as a proper geometry shader is used and implemented.

### Asset Loading
Assimp is used to load assets and parsed as a mesh data set. Due to the material pipeline, textures are also packaged and used as default values based on the texture type.

### Terrain System
Mainly uses Geomipmapping with patch-based LOD based on world-space camera distance and per patch bounding box.

Other features include frustum culling and difference height data generation types (heightmap loading, fault generation, midpoint displacement)

Brute force and tessellation based terrain were also implemented.

### Environment Probe System
Used mainly for IBL via nearest probes selection blending

### Tiled Shading for Lighting System

### GUI - ImGUI
