#ifndef _PORTAL_CLIP_INC
#define _PORTAL_CLIP_INC

// Transform a world-space position into local OOBB space.
// (Note: Adjust the multiplication order if your matrix convention differs.)
vec3 WorldToOOBB(vec3 worldPos)
{
    return (vec4(worldPos, 1.0) * uPortal.mWorldToPortal).xyz;
}

// Check if a point is inside the OOBB.
// Assumes the box is centered at the origin in OOBB space.
bool IsInsideOOBB(vec3 localPos)
{
    return all(lessThanEqual(abs(localPos), uPortal.portalOOBBHalfSize));
}

// Calculate the local eye position for different cases (camera/omni light)
vec3 GetEyePosition()
{
#ifndef SHADOW_PASS
    return WorldToOOBB(vEyePos.xyz);  // Camera view
#elif defined(OMNI_DEPTH)
    return WorldToOOBB(lightPos.xyz);  // Light view (for depth pass)
#else
    return -vec3(mViewMat[0][3], mViewMat[1][3], mViewMat[2][3]);  // Default (camera)
#endif
}

bool RayPassesThroughDoor(vec3 eyePos, vec3 rayDir, float doorZ)
{
    float epsilon = 0.0001;
    if (abs(rayDir.z) < epsilon)
        return false;
        
    // Compute t for intersection with the door plane.
    float t = (doorZ - eyePos.z) / rayDir.z;
    if (t < 0.0)
        return false; // Intersection behind the eye.
        
    vec3 p = eyePos + t * rayDir;
    // Check if the intersection is within the door's x/y extents.
    return (abs(p.x) <= uPortal.portalOOBBHalfSize.x &&
            abs(p.y) <= uPortal.portalOOBBHalfSize.y);
}

bool ShouldPortalDiscardFragment(vec3 vFragWorldPos)
{
    // Transform positions into portal (OOBB) local space.
    vec3 vLocalEyePos  = GetEyePosition();
    vec3 vLocalFragPos = WorldToOOBB(vFragWorldPos);
    vec3 vRayDir       = normalize(vLocalFragPos - vLocalEyePos);

    // Define door plane positions in portal space.
    float doorNeg = -uPortal.portalOOBBHalfSize.z;
    float doorPos =  uPortal.portalOOBBHalfSize.z;
    
    bool inside = IsInsideOOBB(vLocalEyePos);
    
    if (inside)
    {
        if (uPortal.bIsInnerGeo)
        {
            // When inside, interior geometry is always visible.
            return false;
        }
        else
        {
            // When inside, treat both doors as exits.
            bool throughNegExit = RayPassesThroughDoor(vLocalEyePos, vRayDir, doorNeg);
            bool throughPosExit = RayPassesThroughDoor(vLocalEyePos, vRayDir, doorPos);
            return !(throughNegExit || throughPosExit);
        }
    }
    else
    {
        // Outside: determine entrance (nearest door) and exit (farthest door)
        float distNeg = abs(vLocalEyePos.z - doorNeg);
        float distPos = abs(vLocalEyePos.z - doorPos);
        float entranceZ = (distNeg < distPos) ? doorNeg : doorPos;
        float exitZ     = (distNeg < distPos) ? doorPos : doorNeg;

        bool passesEntrance = RayPassesThroughDoor(vLocalEyePos, vRayDir, entranceZ);
        bool passesExit     = RayPassesThroughDoor(vLocalEyePos, vRayDir, exitZ);
        
        if (uPortal.bIsInnerGeo)
        {
            // Outside: interior geometry is visible only if the ray passes through the entrance.
            return !passesEntrance;
        }
        else
        {
            // For external geometry:
            // If the ray passes through the entrance but not the exit,
            // then check if the fragment is behind the entrance relative to the eye.
            if (passesEntrance && !passesExit)
            {
                bool geoInFrontOfEntrance;
                // "In front" means on the same side of the entrance plane as the eye.
                if (vLocalEyePos.z > entranceZ)
                    geoInFrontOfEntrance = (vLocalFragPos.z > entranceZ);
                else
                    geoInFrontOfEntrance = (vLocalFragPos.z < entranceZ);
                    
                // Only discard if the fragment is behind the entrance.
                if (!geoInFrontOfEntrance)
                    return true;
                else
                    return false;
            }
            else
            {
                // Otherwise, external geometry is rendered.
                return false;
            }
        }
    }
}


#endif

