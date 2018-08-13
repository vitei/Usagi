// Includes.
#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
#include "includes/depth_write.inc"

in GeometryData
{
	ATTRIB_LOC(0) vec4 vo_vColor;
} geometryData;

layout(location = 0) out vec4 colorOut;

void main()
{
	vec4 clrOut = geometryData.vo_vColor;
	//OutputLinearDepth(clrOut.w);
	colorOut = clrOut;
}
