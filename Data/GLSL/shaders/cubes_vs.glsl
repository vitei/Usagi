#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"


in vec3 ao_position;
in vec3 ao_scale;
in vec4 ao_color;
in float ao_yaw;


out VertexData
{
    vec3    vo_vScale;
    vec4    vo_vColor;
    float	vo_fYaw;
} vertexData;

void main(void)
{
    vertexData.vo_vScale = ao_scale;
    vertexData.vo_vColor = ao_color;
    vertexData.vo_fYaw = ao_yaw;

    gl_Position = vec4(ao_position,1.0);
}
