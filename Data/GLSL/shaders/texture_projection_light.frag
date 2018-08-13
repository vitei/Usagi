#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
#include "includes/lighting_structs.inc"

BUFFER_LAYOUT(1, UBO_MATERIAL_1_ID) uniform Material1
{
	// Materials
	float fAlpha;

} uPSMaterial1;

// The projection texture
SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

ATTRIB_LOC(0) in vec4 vo_vTexCoord;

layout(location = 0) out vec4 colorOut;

void main(void)
{
	vec4 texRead = vec4( 0.0 );
	if( vo_vTexCoord.w > 0.0 ) {
		vec2 uv = vo_vTexCoord.xy / vo_vTexCoord.w;
		texRead = texture( sampler0, GetUV( uv ) );
	}

	colorOut = vec4( texRead.rgb, texRead.a * uPSMaterial1.fAlpha );
}
