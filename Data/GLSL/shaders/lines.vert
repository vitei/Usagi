// Includes.
#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"

ATTRIB_LOC(0) in vec3 ao_vStart;
ATTRIB_LOC(1) in vec3 ao_vEnd;
ATTRIB_LOC(2) in vec2 ao_vWidth;
ATTRIB_LOC(3) in vec4 ao_vColor;

out VertexData
{
	ATTRIB_LOC(0) vec3 vo_vStart;
	ATTRIB_LOC(1) vec3 vo_vEnd;
	ATTRIB_LOC(2) vec2 vo_vWidth;
	ATTRIB_LOC(3) vec4 vo_vColor;
} vertexData;

void main(void)
{
	vertexData.vo_vStart = ao_vStart;;
	vertexData.vo_vEnd = ao_vEnd;
	vertexData.vo_vWidth = ao_vWidth;
	vertexData.vo_vColor = ao_vColor;

	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
