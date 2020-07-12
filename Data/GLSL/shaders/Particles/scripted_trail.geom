#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"


layout(lines_adjacency) in;	// 4 vertices
layout(triangle_strip, max_vertices = 4) out;


BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	vec4 	vStartColor;				// color of the outside of the beam
	vec4 	vEndColor;				// color of the outside of the beam
	float 	fLineWidth;					// size in world space of the beam
	float	fInvLinePersist;
	float	fElapsedTime;
};


// Line adjacency so we should have 4 points
ATTRIB_LOC(0) in vec3  vo_viewPos[];
ATTRIB_LOC(1) in float vo_fCreateTime[];
ATTRIB_LOC(2) in float vo_fLength[];


out GeometryData
{
    INT_LOC(0) vec4 	go_vColor;
    INT_LOC(1) vec2	go_vTexCoord;

} geometryData;



vec2 CalculateTexCoord()
{
	vec2 vCoord = vec2(0.0, 0.0);
	if(vo_fCreateTime[0] < 0.0f)
		vCoord.x = 1.0;

	if(vo_fCreateTime[3] < 0.0f)
		vCoord.y = 1.0;

	return vCoord;
}

float CalculateRatio(in int vecId)
{
	float fTime = max(fElapsedTime - vo_fCreateTime[vecId], 0.0);
	return 1.0f-min(fTime * fInvLinePersist, 1.0);
}

void CreateVertex(vec3 vPos, vec4 vColor, vec2 vTexCoord)
{
	//vec4 pos;
  
	//pos = vec4(vPos, 0.0, 1.0);
	geometryData.go_vTexCoord = vTexCoord;
	geometryData.go_vColor = vColor;
	gl_Position = vec4(vPos, 1.0) * mProjMat;

	EmitVertex();
}

void CalculateForwardRight(in int firstVec, out vec3 vForward, out vec3 vRight )
{
	// vs stands for view space
	vec3 vs_start = vo_viewPos[firstVec];
	vec3 vs_end = vo_viewPos[firstVec+1];
	vForward = normalize(vs_end - vs_start);
	vec3 vToCamera = normalize(vs_start);
	
	vRight = fLineWidth * normalize(cross(vForward,/*vec3(0,0,1)*/vToCamera));
}

void main(void)
{
	vec3 vs_start = vo_viewPos[1];
	vec3 vs_end = vo_viewPos[2];

	vec3 vForward, vRight, vRight2;
	CalculateForwardRight(1, vForward, vRight);

	float fRatioBack = CalculateRatio(1);
	float fRatioFront = CalculateRatio(2);

	/*if(vertexData[3].vo_fCreateTime > 0.0)
	{
		vec3 vForwardTmp;
		CalculateForwardRight(2, vForwardTmp, vRight);
	}
	else*/
	{
		vRight2 = vRight;
	}

	vec2 vTexCoord = CalculateTexCoord();
	
	vec4 vColorBk = mix(vEndColor, vStartColor, fRatioBack);
	vec4 vColorFr = mix(vEndColor, vStartColor, fRatioFront);

	CreateVertex( vs_start - vRight, vColorBk, vec2(-1.0, vTexCoord.x) );
	CreateVertex( vs_start + vRight, vColorBk, vec2(1.0, vTexCoord.x) );
	CreateVertex( vs_end - vRight2, vColorFr, vec2(-1.0, vTexCoord.y) );
	CreateVertex( vs_end + vRight2, vColorFr, vec2(1.0, vTexCoord.y) );

	 
	EndPrimitive();

}
