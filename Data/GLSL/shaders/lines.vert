// Includes.
#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"

ATTRIB_LOC(0) in vec3 ao_vStart;
ATTRIB_LOC(1) in vec3 ao_vEnd;
ATTRIB_LOC(2) in vec2 ao_vWidth;
ATTRIB_LOC(3) in vec4 ao_vColor;

out VertexData
{
	vec3 vo_vStart;
	vec3 vo_vEnd;
	vec2 vo_vWidth;
	vec4 vo_vColor;
} vertexData;

void main(void)
{
	vertexData.vo_vStart = ao_vStart;;
	vertexData.vo_vEnd = ao_vEnd;
	vertexData.vo_vWidth = ao_vWidth;
	vertexData.vo_vColor = ao_vColor;

	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
