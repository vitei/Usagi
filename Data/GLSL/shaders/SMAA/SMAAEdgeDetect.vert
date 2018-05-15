/**
 * Copyright (C) 2013 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2013 Jose I. Echevarria (joseignacioechevarria@gmail.com)
 * Copyright (C) 2013 Belen Masia (bmasia@unizar.es)
 * Copyright (C) 2013 Fernando Navarro (fernandn@microsoft.com)
 * Copyright (C) 2013 Diego Gutierrez (diegog@unizar.es)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. As clarification, there
 * is no requirement that the copyright notice and permission be included in
 * binary distributions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "../includes/platformdefines.inc"
#include "../includes/smaa.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

// Output attributes
out vec4 vo_vPosition;
out vec2 vo_vTexCoord;
out vec4 vo_vOffset[3];

void main(void)
{
	vec4 vPosition = vec4( ao_position.xy, -0.5, 1.0);
 	
 	gl_Position		= vPosition;	

	vo_vTexCoord = GetRTUV( vec2(0.5, 0.5) * vPosition.xy + vec2(0.5, 0.5) ); 

    vo_vOffset[0] = mad(vScreenMetrics.xyxy, vec4(-1.0, 0.0, 0.0, -1.0), vo_vTexCoord.xyxy);
    vo_vOffset[1] = mad(vScreenMetrics.xyxy, vec4( 1.0, 0.0, 0.0,  1.0), vo_vTexCoord.xyxy);
    vo_vOffset[2] = mad(vScreenMetrics.xyxy, vec4(-2.0, 0.0, 0.0, -2.0), vo_vTexCoord.xyxy);
}
