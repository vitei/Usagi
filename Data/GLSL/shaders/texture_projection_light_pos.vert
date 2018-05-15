#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
#include "includes/lighting_structs.inc"

struct TextureProjection
{
	mat4 projectorMatrix;
};

BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	TextureProjection texProj;
};

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

// Output attributes
out vec4 vo_vTexCoord;

void main(void)
{
	vo_vTexCoord = vec4( ao_position, 1.0 ) * texProj.projectorMatrix;

	gl_Position = vec4( ao_position, 1.0 ) * mViewProjMat;
}
