#include "../includes/platformdefines.inc"
#include "../includes/global_2d.inc"

// <<GENERATED_CODE>>

// Input/ output definitions
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


ATTRIB_LOC(0) in vec4    vo_vColor[];
ATTRIB_LOC(1) in vec4    vo_vTexCoordRng[];
ATTRIB_LOC(2) in vec2    vo_vSize[];


ATTRIB_LOC(0) out vec4 	go_vColor;
ATTRIB_LOC(1) out vec2 	go_vTexcoord;



void CreateVertex(int ii, vec2 vTexCoord, vec4 pos)
{
	//vec4 pos;
  
	//pos = vec4(vPos, 0.0, 1.0);
	go_vColor = vo_vColor[ii];

	go_vTexcoord = vTexCoord;


	gl_Position = pos * proj;

	EmitVertex();
}

void main(void)
{
    for (int ii = 0; ii < gl_in.length(); ii++)
    {
        vec2 vCenterPos = gl_in[ii].gl_Position.xy;
        vec4 vSize = vec4(0.0, 0.0, vo_vSize[ii]);

        vec4 vPos = vec4(vCenterPos, 0.0, 1.0);

        CreateVertex(ii, vo_vTexCoordRng[ii].xw, vPos + vec4(vSize.xw, 0.0, 0.0));
        CreateVertex(ii, vo_vTexCoordRng[ii].zw, vPos + vec4(vSize.zw, 0.0, 0.0));  
        CreateVertex(ii, vo_vTexCoordRng[ii].xy, vPos + vec4(vSize.xy, 0.0, 0.0));
        CreateVertex(ii, vo_vTexCoordRng[ii].zy, vPos + vec4(vSize.zy, 0.0, 0.0));
        
        EndPrimitive();
    }
}
