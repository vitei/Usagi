#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"


ATTRIB_LOC(0) in mat3x4 ao_matrix;
ATTRIB_LOC(3) in vec4 ao_color;


out VertexData
{
	INT_LOC(0) mat3x4	vo_matrix;
	INT_LOC(3) vec4     vo_vColor;
} vertexData;

void main(void)
{
    vertexData.vo_matrix = ao_matrix;
    vertexData.vo_vColor = ao_color;

    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
