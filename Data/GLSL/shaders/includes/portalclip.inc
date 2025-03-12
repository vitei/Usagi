#ifndef _PORTAL_CLIP_INC
#define _PORTAL_CLIP_INC


// Transform a world-space position into local OOBB space
vec3 WorldToOOBB(vec3 worldPos)
{
    return vec4(worldPos, 1.0) * uPortal.mWorldToPortal;
}

// Check if a point is inside the OOBB
bool IsInsideOOBB(vec3 localPos)
{
    return all(lessThanEqual(abs(localPos), uPortal.portalOOBBHalfSize));
}

// Ray-OOBB intersection test (checks only for front/rear face exit)
bool RayExitsThroughFrontOrRear(vec3 rayOrigin, vec3 rayDir)
{
    vec3 tMin = (-uPortal.portalOOBBHalfSize - rayOrigin) / rayDir;
    vec3 tMax = ( uPortal.portalOOBBHalfSize - rayOrigin) / rayDir;
    
    float tNear = max(max(min(tMin.x, tMax.x), min(tMin.y, tMax.y)), min(tMin.z, tMax.z));
    float tFar  = min(min(max(tMin.x, tMax.x), max(tMin.y, tMax.y)), max(tMin.z, tMax.z));

    if (tNear > tFar || tFar < 0.0) return false; // No intersection

    // If the intersection happens along the Z-axis (front or rear), allow it
    vec3 exitPoint = rayOrigin + tFar * rayDir;
    return abs(exitPoint.z) > abs(exitPoint.x) && abs(exitPoint.z) > abs(exitPoint.y);
}

bool ShouldPortalDiscardFragment(vec3 vFragWorldPos)
{
    // Transform positions into local OOBB space

#ifndef SHADOW_PASS
    vec3 vLocalEyePos  = WorldToOOBB(vEyePos.xyz);
    
    // If the camera is inside the hangar, always render
    if (IsInsideOOBB(vLocalEyePos))
    {
        return false;
    }

#elif defined(OMNI_DEPTH)
    vec3 vLocalEyePos  = WorldToOOBB(lightPos.xyz);

    // If the camera is inside the hangar, always render
    if (IsInsideOOBB(vLocalEyePos))
    {
        return !uPortal.bIsInnerGeo;
    }
#else
    vec3 vLocalEyePos = -vec3(mViewMat[0][3], mViewMat[1][3], mViewMat[2][3]);

    // No need to test if in it for a diectional shadow
#endif

    vec3 vLocalFragPos = WorldToOOBB(vFragWorldPos);

    // Compute the view ray
    vec3 vRayDir = normalize(vLocalFragPos - vLocalEyePos);

    // If the ray exits through the front or rear, render
    if (RayExitsThroughFrontOrRear(vLocalEyePos, vRayDir))
    {
        return !uPortal.bIsInnerGeo;
    }

    return uPortal.bIsInnerGeo;
}

#endif
