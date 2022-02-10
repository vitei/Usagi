#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"


ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec3 ao_scale;
ATTRIB_LOC(2) in vec4 ao_color;
ATTRIB_LOC(3) in float ao_yaw;


ATTRIB_LOC(0) out vec3    vo_vScale;
ATTRIB_LOC(1) out vec4    vo_vColor;
ATTRIB_LOC(2) out float	  vo_fYaw;

void main(void)
{
    vo_vScale = ao_scale;
    vo_vColor = ao_color;
    vo_fYaw = ao_yaw;

    gl_Position = vec4(ao_position,1.0);
}
