

vec3 GetPosFromLinDepth3DFS(in float fZBuffer, in vec3 vFrustumRay)
{
	float fLinDepth = fZBuffer;

	return (fLinDepth * vFrustumRay);
}


vec3 GetPosFromLinDepth3D(in float fZBuffer, in vec3 vViewDir)
{
	float fLinDepth = fZBuffer;//GetLinearDepth(fZBuffer);
	vec3 vViewRay = vec3(vViewDir.xy * (vNearFar.y / vViewDir.z), vNearFar.y);
	//vViewRay = normalize(vViewRay);

	return vViewRay * fLinDepth;
}