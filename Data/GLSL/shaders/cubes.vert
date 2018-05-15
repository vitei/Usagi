#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"


ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec3 ao_scale;
ATTRIB_LOC(2) in vec4 ao_color;
ATTRIB_LOC(3) in float ao_yaw;


out VertexData
{
    ATTRIB_LOC(0) vec3    vo_vScale;
    ATTRIB_LOC(1) vec4    vo_vColor;
    ATTRIB_LOC(2) float	vo_fYaw;
} vertexData;

void main(void)
{
    vertexData.vo_vScale = ao_scale;
    vertexData.vo_vColor = ao_color;
    vertexData.vo_fYaw = ao_yaw;

    gl_Position = vec4(ao_position,1.0);
}
