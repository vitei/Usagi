#include "../includes/platformdefines.inc"


BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	// Materials
	float fFade;

} uPSMaterial;

layout(location = 0) out vec4 colorOut;

void main(void)
{
    colorOut = vec4(0, 0, 0, uPSMaterial.fFade);
}

