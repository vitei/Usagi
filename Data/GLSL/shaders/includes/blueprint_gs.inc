#ifndef _BLUEPRINT_INC
#define _BLUEPRINT_INC

// Input points, output two triangles using 4 vertices in a triangle strip.
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


// Inputs from VS.
in vec2   vo_texcoordTL[];
in vec2   vo_texcoordTR[];
in vec2   vo_texcoordBL[];
in vec2   vo_texcoordBR[];
in vec4   vo_color[];
in mat3x4 vo_world[];
in vec2   vo_scale[];

// Outputs to FS.
out vec2 go_texcoord0;
out vec4 go_color;

void CreateVertex( vec4 center, vec4 scale, vec2 tex )
{
	vec4 pos = vec4(center.xyz + scale.xyz, 1.0f);
	// vec4 viewPos = vec4(pos.xyz, 1.0f);
	// use the pane's z-coordinate as a multiplier
	// for the parallax effect
	pos.x = pos.x + (parallax * pos.z);

	gl_Position  = pos * proj;

	go_texcoord0 = tex;
	go_color     = vo_color[0];
	EmitVertex();
}

void CreateQuadScaled( vec4 center, vec2 scale )
{
	// Expand and output four vertices:
	//
	//       +
	//   a-------b
	//   |   | / |
	// - |---o---| +
	//   | / |   |
	//   c-------d
	//       -
	//
	//     X    Y  |  X  Y
	// a= (-1, +1) | (0, 1)
	// b= (+1, +1) | (1, 1)
	// c= (-1, -1) | (0, 0)
	// d= (+1, -1) | (1, 0)
	//
	// Tri0 = { c, d, a }
	// Tri1 = { a, d, b }

	// Multiply the scaling offsets by the world matrix so that everything is in screen-world space.
	// vec4 firstScale  = vo_world[0] * vec4(-scale.x,  scale.y, 0.0f, 1.0f);
	// vec4 secondScale = vo_world[0] * vec4( scale.x,  scale.y, 0.0f, 1.0f);
	// vec4 thirdScale  = vo_world[0] * vec4(-scale.x, -scale.y, 0.0f, 1.0f);
	// vec4 fourthScale = vo_world[0] * vec4( scale.x, -scale.y, 0.0f, 1.0f);
	vec4 firstScale  = vec4(vec4(-scale.x,  scale.y, 1.0f, 1.0f) * vo_world[0], 1.0f);
	vec4 secondScale = vec4(vec4( scale.x,  scale.y, 1.0f, 1.0f) * vo_world[0], 1.0f);
	vec4 thirdScale  = vec4(vec4(-scale.x, -scale.y, 1.0f, 1.0f) * vo_world[0], 1.0f);
	vec4 fourthScale = vec4(vec4( scale.x, -scale.y, 1.0f, 1.0f) * vo_world[0], 1.0f);

	CreateVertex(center, thirdScale, vo_texcoordBL[0]);
	CreateVertex(center, fourthScale, vo_texcoordBR[0]);
	CreateVertex(center, firstScale, vo_texcoordTL[0]);
	CreateVertex(center, secondScale, vo_texcoordTR[0]);

	EndPrimitive();
}

#endif
