#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"


ATTRIB_LOC(0) in mat3x4 ao_matrix;
ATTRIB_LOC(3) in vec4 ao_color;


ATTRIB_LOC(0) out mat3x4	vo_matrix;
ATTRIB_LOC(3) out vec4      vo_vColor;

void main(void)
{
    vo_matrix = ao_matrix;
    vo_vColor = ao_color;

    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
