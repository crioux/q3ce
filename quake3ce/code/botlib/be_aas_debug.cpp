/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*****************************************************************************
 * name:		be_aas_debug.c
 *
 * desc:		AAS debug code
 *
 * $Archive: /MissionPack/code/botlib/be_aas_debug.c $
 *
 *****************************************************************************/

#include"botlib_pch.h"


#define MAX_DEBUGLINES				1024
#define MAX_DEBUGPOLYGONS			8192

int debuglines[MAX_DEBUGLINES];
int debuglinevisible[MAX_DEBUGLINES];
int numdebuglines;

static int debugpolygons[MAX_DEBUGPOLYGONS];

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ClearShownPolygons(void)
{
	int i;
//*
	for (i = 0; i < MAX_DEBUGPOLYGONS; i++)
	{
		if (debugpolygons[i]) botimport.DebugPolygonDelete(debugpolygons[i]);
		debugpolygons[i] = 0;
	} //end for
//*/
/*
	for (i = 0; i < MAX_DEBUGPOLYGONS; i++)
	{
		botimport.DebugPolygonDelete(i);
		debugpolygons[i] = 0;
	} //end for
*/
} //end of the function AAS_ClearShownPolygons
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowPolygon(int color, int numpoints, bvec3_t *points)
{
	int i;

	for (i = 0; i < MAX_DEBUGPOLYGONS; i++)
	{
		if (!debugpolygons[i])
		{
			debugpolygons[i] = botimport.DebugPolygonCreate(color, numpoints, points);
			break;
		} //end if
	} //end for
} //end of the function AAS_ShowPolygon
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ClearShownDebugLines(void)
{
	int i;

	//make all lines invisible
	for (i = 0; i < MAX_DEBUGLINES; i++)
	{
		if (debuglines[i])
		{
			//botimport.DebugLineShow(debuglines[i], NULL, NULL, LINECOLOR_NONE);
			botimport.DebugLineDelete(debuglines[i]);
			debuglines[i] = 0;
			debuglinevisible[i] = qfalse;
		} //end if
	} //end for
} //end of the function AAS_ClearShownDebugLines
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DebugLine(bvec3_t start, bvec3_t end, int color)
{
	int line;

	for (line = 0; line < MAX_DEBUGLINES; line++)
	{
		if (!debuglines[line])
		{
			debuglines[line] = botimport.DebugLineCreate();
			debuglinevisible[line] = qfalse;
			numdebuglines++;
		} //end if
		if (!debuglinevisible[line])
		{
			botimport.DebugLineShow(debuglines[line], start, end, color);
			debuglinevisible[line] = qtrue;
			return;
		} //end else
	} //end for
} //end of the function AAS_DebugLine
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_PermanentLine(bvec3_t start, bvec3_t end, int color)
{
	int line;

	line = botimport.DebugLineCreate();
	botimport.DebugLineShow(line, start, end, color);
} //end of the function AAS_PermenentLine
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_DrawPermanentCross(bvec3_t origin, bfixed size, int color)
{
	int i, debugline;
	bvec3_t start, end;

	for (i = 0; i < 3; i++)
	{
		VectorCopy(origin, start);
		start[i] += size;
		VectorCopy(origin, end);
		end[i] -= size;
		AAS_DebugLine(start, end, color);
		debugline = botimport.DebugLineCreate();
		botimport.DebugLineShow(debugline, start, end, color);
	} //end for
} //end of the function AAS_DrawPermanentCross
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DrawPlaneCross(bvec3_t point, bvec3_t normal, bfixed dist, int type, int color)
{
	int n0, n1, n2, j, line, lines[2];
	bvec3_t start1, end1, start2, end2;

	//make a cross in the hit plane at the hit point
	VectorCopy(point, start1);
	VectorCopy(point, end1);
	VectorCopy(point, start2);
	VectorCopy(point, end2);

	n0 = type % 3;
	n1 = (type + 1) % 3;
	n2 = (type + 2) % 3;
	start1[n1] -= BFIXED(6,0);
	start1[n2] -= BFIXED(6,0);
	end1[n1] += BFIXED(6,0);
	end1[n2] += BFIXED(6,0);
	start2[n1] += BFIXED(6,0);
	start2[n2] -= BFIXED(6,0);
	end2[n1] -= BFIXED(6,0);
	end2[n2] += BFIXED(6,0);

	start1[n0] = (dist - (start1[n1] * normal[n1] +
				start1[n2] * normal[n2])) / normal[n0];
	end1[n0] = (dist - (end1[n1] * normal[n1] +
				end1[n2] * normal[n2])) / normal[n0];
	start2[n0] = (dist - (start2[n1] * normal[n1] +
				start2[n2] * normal[n2])) / normal[n0];
	end2[n0] = (dist - (end2[n1] * normal[n1] +
				end2[n2] * normal[n2])) / normal[n0];

	for (j = 0, line = 0; j < 2 && line < MAX_DEBUGLINES; line++)
	{
		if (!debuglines[line])
		{
			debuglines[line] = botimport.DebugLineCreate();
			lines[j++] = debuglines[line];
			debuglinevisible[line] = qtrue;
			numdebuglines++;
		} //end if
		else if (!debuglinevisible[line])
		{
			lines[j++] = debuglines[line];
			debuglinevisible[line] = qtrue;
		} //end else
	} //end for
	botimport.DebugLineShow(lines[0], start1, end1, color);
	botimport.DebugLineShow(lines[1], start2, end2, color);
} //end of the function AAS_DrawPlaneCross
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowBoundingBox(bvec3_t origin, bvec3_t mins, bvec3_t maxs)
{
	bvec3_t bboxcorners[8];
	int lines[3];
	int i, j, line;

	//upper corners
	bboxcorners[0][0] = origin[0] + maxs[0];
	bboxcorners[0][1] = origin[1] + maxs[1];
	bboxcorners[0][2] = origin[2] + maxs[2];
	//
	bboxcorners[1][0] = origin[0] + mins[0];
	bboxcorners[1][1] = origin[1] + maxs[1];
	bboxcorners[1][2] = origin[2] + maxs[2];
	//
	bboxcorners[2][0] = origin[0] + mins[0];
	bboxcorners[2][1] = origin[1] + mins[1];
	bboxcorners[2][2] = origin[2] + maxs[2];
	//
	bboxcorners[3][0] = origin[0] + maxs[0];
	bboxcorners[3][1] = origin[1] + mins[1];
	bboxcorners[3][2] = origin[2] + maxs[2];
	//lower corners
	Com_Memcpy(bboxcorners[4], bboxcorners[0], sizeof(bvec3_t) * 4);
	for (i = 0; i < 4; i++) bboxcorners[4 + i][2] = origin[2] + mins[2];
	//draw bounding box
	for (i = 0; i < 4; i++)
	{
		for (j = 0, line = 0; j < 3 && line < MAX_DEBUGLINES; line++)
		{
			if (!debuglines[line])
			{
				debuglines[line] = botimport.DebugLineCreate();
				lines[j++] = debuglines[line];
				debuglinevisible[line] = qtrue;
				numdebuglines++;
			} //end if
			else if (!debuglinevisible[line])
			{
				lines[j++] = debuglines[line];
				debuglinevisible[line] = qtrue;
			} //end else
		} //end for
		//top plane
		botimport.DebugLineShow(lines[0], bboxcorners[i],
									bboxcorners[(i+1)&3], LINECOLOR_RED);
		//bottom plane
		botimport.DebugLineShow(lines[1], bboxcorners[4+i],
									bboxcorners[4+((i+1)&3)], LINECOLOR_RED);
		//vertical lines
		botimport.DebugLineShow(lines[2], bboxcorners[i],
									bboxcorners[4+i], LINECOLOR_RED);
	} //end for
} //end of the function AAS_ShowBoundingBox
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowFace(int facenum)
{
	int i, color, edgenum;
	aas_edge_t *edge;
	aas_face_t *face;
	aas_plane_t *plane;
	bvec3_t start, end;

	color = LINECOLOR_YELLOW;
	//check if face number is in range
	if (facenum >= aasworld.numfaces)
	{
		botimport.Print(PRT_ERROR, "facenum %d out of range\n", facenum);
	} //end if
	face = &aasworld.faces[facenum];
	//walk through the edges of the face
	for (i = 0; i < face->numedges; i++)
	{
		//edge number
		edgenum = abs(aasworld.edgeindex[face->firstedge + i]);
		//check if edge number is in range
		if (edgenum >= aasworld.numedges)
		{
			botimport.Print(PRT_ERROR, "edgenum %d out of range\n", edgenum);
		} //end if
		edge = &aasworld.edges[edgenum];
		if (color == LINECOLOR_RED) color = LINECOLOR_GREEN;
		else if (color == LINECOLOR_GREEN) color = LINECOLOR_BLUE;
		else if (color == LINECOLOR_BLUE) color = LINECOLOR_YELLOW;
		else color = LINECOLOR_RED;
		AAS_DebugLine(aasworld.vertexes[edge->v[0]],
										aasworld.vertexes[edge->v[1]],
										color);
	} //end for
	plane = &aasworld.planes[face->planenum];
	edgenum = abs(aasworld.edgeindex[face->firstedge]);
	edge = &aasworld.edges[edgenum];
	VectorCopy(aasworld.vertexes[edge->v[0]], start);
	FIXED_VEC3MA_R(start, BFIXED(20,0), plane->normal, end);
	AAS_DebugLine(start, end, LINECOLOR_RED);
} //end of the function AAS_ShowFace
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowFacePolygon(int facenum, int color, int flip)
{
	int i, edgenum, numpoints;
	bvec3_t points[128];
	aas_edge_t *edge;
	aas_face_t *face;

	//check if face number is in range
	if (facenum >= aasworld.numfaces)
	{
		botimport.Print(PRT_ERROR, "facenum %d out of range\n", facenum);
	} //end if
	face = &aasworld.faces[facenum];
	//walk through the edges of the face
	numpoints = 0;
	if (flip)
	{
		for (i = face->numedges-1; i >= 0; i--)
		{
			//edge number
			edgenum = aasworld.edgeindex[face->firstedge + i];
			edge = &aasworld.edges[abs(edgenum)];
			VectorCopy(aasworld.vertexes[edge->v[edgenum < 0]], points[numpoints]);
			numpoints++;
		} //end for
	} //end if
	else
	{
		for (i = 0; i < face->numedges; i++)
		{
			//edge number
			edgenum = aasworld.edgeindex[face->firstedge + i];
			edge = &aasworld.edges[abs(edgenum)];
			VectorCopy(aasworld.vertexes[edge->v[edgenum < 0]], points[numpoints]);
			numpoints++;
		} //end for
	} //end else
	AAS_ShowPolygon(color, numpoints, points);
} //end of the function AAS_ShowFacePolygon
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowArea(int areanum, int groundfacesonly)
{
	int areaedges[MAX_DEBUGLINES];
	int numareaedges, i, j, n, color = 0, line;
	int facenum, edgenum;
	aas_area_t *area;
	aas_face_t *face;
	aas_edge_t *edge;

	//
	numareaedges = 0;
	//
	if (areanum < 0 || areanum >= aasworld.numareas)
	{
		botimport.Print(PRT_ERROR, "area %d out of range [0, %d]\n",
								areanum, aasworld.numareas);
		return;
	} //end if
	//pointer to the convex area
	area = &aasworld.areas[areanum];
	//walk through the faces of the area
	for (i = 0; i < area->numfaces; i++)
	{
		facenum = abs(aasworld.faceindex[area->firstface + i]);
		//check if face number is in range
		if (facenum >= aasworld.numfaces)
		{
			botimport.Print(PRT_ERROR, "facenum %d out of range\n", facenum);
		} //end if
		face = &aasworld.faces[facenum];
		//ground faces only
		if (groundfacesonly)
		{
			if (!(face->faceflags & (FACE_GROUND | FACE_LADDER))) continue;
		} //end if
		//walk through the edges of the face
		for (j = 0; j < face->numedges; j++)
		{
			//edge number
			edgenum = abs(aasworld.edgeindex[face->firstedge + j]);
			//check if edge number is in range
			if (edgenum >= aasworld.numedges)
			{
				botimport.Print(PRT_ERROR, "edgenum %d out of range\n", edgenum);
			} //end if
			//check if the edge is stored already
			for (n = 0; n < numareaedges; n++)
			{
				if (areaedges[n] == edgenum) break;
			} //end for
			if (n == numareaedges && numareaedges < MAX_DEBUGLINES)
			{
				areaedges[numareaedges++] = edgenum;
			} //end if
		} //end for
		//AAS_ShowFace(facenum);
	} //end for
	//draw all the edges
	for (n = 0; n < numareaedges; n++)
	{
		for (line = 0; line < MAX_DEBUGLINES; line++)
		{
			if (!debuglines[line])
			{
				debuglines[line] = botimport.DebugLineCreate();
				debuglinevisible[line] = qfalse;
				numdebuglines++;
			} //end if
			if (!debuglinevisible[line])
			{
				break;
			} //end else
		} //end for
		if (line >= MAX_DEBUGLINES) return;
		edge = &aasworld.edges[areaedges[n]];
		if (color == LINECOLOR_RED) color = LINECOLOR_BLUE;
		else if (color == LINECOLOR_BLUE) color = LINECOLOR_GREEN;
		else if (color == LINECOLOR_GREEN) color = LINECOLOR_YELLOW;
		else color = LINECOLOR_RED;
		botimport.DebugLineShow(debuglines[line],
									aasworld.vertexes[edge->v[0]],
									aasworld.vertexes[edge->v[1]],
									color);
		debuglinevisible[line] = qtrue;
	} //end for*/
} //end of the function AAS_ShowArea
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowAreaPolygons(int areanum, int color, int groundfacesonly)
{
	int i, facenum;
	aas_area_t *area;
	aas_face_t *face;

	//
	if (areanum < 0 || areanum >= aasworld.numareas)
	{
		botimport.Print(PRT_ERROR, "area %d out of range [0, %d]\n",
								areanum, aasworld.numareas);
		return;
	} //end if
	//pointer to the convex area
	area = &aasworld.areas[areanum];
	//walk through the faces of the area
	for (i = 0; i < area->numfaces; i++)
	{
		facenum = abs(aasworld.faceindex[area->firstface + i]);
		//check if face number is in range
		if (facenum >= aasworld.numfaces)
		{
			botimport.Print(PRT_ERROR, "facenum %d out of range\n", facenum);
		} //end if
		face = &aasworld.faces[facenum];
		//ground faces only
		if (groundfacesonly)
		{
			if (!(face->faceflags & (FACE_GROUND | FACE_LADDER))) continue;
		} //end if
		AAS_ShowFacePolygon(facenum, color, face->frontarea != areanum);
	} //end for
} //end of the function AAS_ShowAreaPolygons
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DrawCross(bvec3_t origin, bfixed size, int color)
{
	int i;
	bvec3_t start, end;

	for (i = 0; i < 3; i++)
	{
		VectorCopy(origin, start);
		start[i] += size;
		VectorCopy(origin, end);
		end[i] -= size;
		AAS_DebugLine(start, end, color);
	} //end for
} //end of the function AAS_DrawCross
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_PrintTravelType(int traveltype)
{
#ifdef DEBUG
	char *str;
	//
	switch(traveltype & TRAVELTYPE_MASK)
	{
		case TRAVEL_INVALID: str = "TRAVEL_INVALID"; break;
		case TRAVEL_WALK: str = "TRAVEL_WALK"; break;
		case TRAVEL_CROUCH: str = "TRAVEL_CROUCH"; break;
		case TRAVEL_BARRIERJUMP: str = "TRAVEL_BARRIERJUMP"; break;
		case TRAVEL_JUMP: str = "TRAVEL_JUMP"; break;
		case TRAVEL_LADDER: str = "TRAVEL_LADDER"; break;
		case TRAVEL_WALKOFFLEDGE: str = "TRAVEL_WALKOFFLEDGE"; break;
		case TRAVEL_SWIM: str = "TRAVEL_SWIM"; break;
		case TRAVEL_WATERJUMP: str = "TRAVEL_WATERJUMP"; break;
		case TRAVEL_TELEPORT: str = "TRAVEL_TELEPORT"; break;
		case TRAVEL_ELEVATOR: str = "TRAVEL_ELEVATOR"; break;
		case TRAVEL_ROCKETJUMP: str = "TRAVEL_ROCKETJUMP"; break;
		case TRAVEL_BFGJUMP: str = "TRAVEL_BFGJUMP"; break;
		case TRAVEL_GRAPPLEHOOK: str = "TRAVEL_GRAPPLEHOOK"; break;
		case TRAVEL_JUMPPAD: str = "TRAVEL_JUMPPAD"; break;
		case TRAVEL_FUNCBOB: str = "TRAVEL_FUNCBOB"; break;
		default: str = "UNKNOWN TRAVEL TYPE"; break;
	} //end switch
	botimport.Print(PRT_MESSAGE, "%s", str);
#endif
} //end of the function AAS_PrintTravelType
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DrawArrow(bvec3_t start, bvec3_t end, int linecolor, int arrowcolor)
{
	avec3_t dir, cross, up = {AFIXED_0, AFIXED_0, AFIXED_1};
	bvec3_t p1, p2, tmp;
	afixed dot;

	VectorSubtract(end, start, tmp);
	VectorNormalizeB2A(tmp,dir);
	dot = FIXED_VEC3DOT(dir, up);
	if (dot > AFIXED(0,99) || dot < -AFIXED(0,99)) VectorSet(cross, AFIXED_1, AFIXED_0, AFIXED_0);
	else CrossProduct(dir, up, cross);

	FIXED_VEC3MA_R(end, -BFIXED(6,0), dir, p1);
	VectorCopy(p1, p2);
	FIXED_VEC3MA_R(p1, BFIXED(6,0), cross, p1);
	FIXED_VEC3MA_R(p2, -BFIXED(6,0), cross, p2);

	AAS_DebugLine(start, end, linecolor);
	AAS_DebugLine(p1, end, arrowcolor);
	AAS_DebugLine(p2, end, arrowcolor);
} //end of the function AAS_DrawArrow
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowReachability(aas_reachability_t *reach)
{
	avec3_t dir;
	bvec3_t cmdmove, velocity, tmp;
	bfixed speed, zvel;
	aas_clientmove_t move;

	AAS_ShowAreaPolygons(reach->areanum, 5, qtrue);
	//AAS_ShowArea(reach->areanum, qtrue);
	AAS_DrawArrow(reach->start, reach->end, LINECOLOR_BLUE, LINECOLOR_YELLOW);
	//
	if ((reach->traveltype & TRAVELTYPE_MASK) == TRAVEL_JUMP ||
		(reach->traveltype & TRAVELTYPE_MASK) == TRAVEL_WALKOFFLEDGE)
	{
		AAS_HorizontalVelocityForJump(aassettings.phys_jumpvel, reach->start, reach->end, &speed);
		//
		VectorSubtract(reach->end, reach->start, tmp);
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,dir);
		//set the velocity
		FIXED_VEC3SCALE_R(dir, speed, velocity);
		//set the command movement
		VectorClear(cmdmove);
		cmdmove[2] = aassettings.phys_jumpvel;
		//
		AAS_PredictClientMovement(&move, -1, reach->start, PRESENCE_NORMAL, qtrue,
									velocity, cmdmove, 3, 30, GFIXED(0,1),
									PSE_HITGROUND|PSE_ENTERWATER|PSE_ENTERSLIME|
									PSE_ENTERLAVA|PSE_HITGROUNDDAMAGE, 0, qtrue);
		//
		if ((reach->traveltype & TRAVELTYPE_MASK) == TRAVEL_JUMP)
		{
			AAS_JumpReachRunStart(reach, tmp);
			AAS_DrawCross(tmp, BFIXED(4,0), LINECOLOR_BLUE);
		} //end if
	} //end if
	else if ((reach->traveltype & TRAVELTYPE_MASK) == TRAVEL_ROCKETJUMP)
	{
		zvel = AAS_RocketJumpZVelocity(reach->start);
		AAS_HorizontalVelocityForJump(zvel, reach->start, reach->end, &speed);
		//
		VectorSubtract(reach->end, reach->start, tmp);
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,dir);
		//get command movement
		FIXED_VEC3SCALE_R(dir, speed, cmdmove);
		VectorSet(velocity, BFIXED_0, BFIXED_0, zvel);
		//
		AAS_PredictClientMovement(&move, -1, reach->start, PRESENCE_NORMAL, qtrue,
									velocity, cmdmove, 30, 30, GFIXED(0,1),
									PSE_ENTERWATER|PSE_ENTERSLIME|
									PSE_ENTERLAVA|PSE_HITGROUNDDAMAGE|
									PSE_TOUCHJUMPPAD|PSE_HITGROUNDAREA, reach->areanum, qtrue);
	} //end else if
	else if ((reach->traveltype & TRAVELTYPE_MASK) == TRAVEL_JUMPPAD)
	{
		VectorSet(cmdmove, BFIXED_0, BFIXED_0, BFIXED_0);
		//
		VectorSubtract(reach->end, reach->start, tmp);
		tmp[2] = BFIXED_0;
		VectorNormalizeB2A(tmp,dir);
		//set the velocity
		//NOTE: the edgenum is the horizontal velocity
		FIXED_VEC3SCALE_R(dir, MAKE_BFIXED(reach->edgenum), velocity);
		//NOTE: the facenum is the Z velocity
		velocity[2] = MAKE_BFIXED(reach->facenum);
		//
		AAS_PredictClientMovement(&move, -1, reach->start, PRESENCE_NORMAL, qtrue,
									velocity, cmdmove, 30, 30, GFIXED(0,1),
									PSE_ENTERWATER|PSE_ENTERSLIME|
									PSE_ENTERLAVA|PSE_HITGROUNDDAMAGE|
									PSE_TOUCHJUMPPAD|PSE_HITGROUNDAREA, reach->areanum, qtrue);
	} //end else if
} //end of the function AAS_ShowReachability
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowReachableAreas(int areanum)
{
	aas_areasettings_t *settings;
	static aas_reachability_t reach;
	static int index, lastareanum;
	static gfixed lasttime;

	if (areanum != lastareanum)
	{
		index = 0;
		lastareanum = areanum;
	} //end if
	settings = &aasworld.areasettings[areanum];
	//
	if (!settings->numreachableareas) return;
	//
	if (index >= settings->numreachableareas) index = 0;
	//
	if (AAS_Time() - lasttime > GFIXED(1,5))
	{
		Com_Memcpy(&reach, &aasworld.reachability[settings->firstreachablearea + index], sizeof(aas_reachability_t));
		index++;
		lasttime = AAS_Time();
		AAS_PrintTravelType(reach.traveltype & TRAVELTYPE_MASK);
		botimport.Print(PRT_MESSAGE, "\n");
	} //end if
	AAS_ShowReachability(&reach);
} //end of the function ShowReachableAreas

void AAS_FloodAreas_r(int areanum, int cluster, int *done)
{
	int nextareanum, i, facenum;
	aas_area_t *area;
	aas_face_t *face;
	aas_areasettings_t *settings;
	aas_reachability_t *reach;

	AAS_ShowAreaPolygons(areanum, 1, qtrue);
	//pointer to the convex area
	area = &aasworld.areas[areanum];
	settings = &aasworld.areasettings[areanum];
	//walk through the faces of the area
	for (i = 0; i < area->numfaces; i++)
	{
		facenum = abs(aasworld.faceindex[area->firstface + i]);
		face = &aasworld.faces[facenum];
		if (face->frontarea == areanum)
			nextareanum = face->backarea;
		else
			nextareanum = face->frontarea;
		if (!nextareanum)
			continue;
		if (done[nextareanum])
			continue;
		done[nextareanum] = qtrue;
		if (aasworld.areasettings[nextareanum].contents & AREACONTENTS_VIEWPORTAL)
			continue;
		if (AAS_AreaCluster(nextareanum) != cluster)
			continue;
		AAS_FloodAreas_r(nextareanum, cluster, done);
	} //end for
	//
	for (i = 0; i < settings->numreachableareas; i++)
	{
		reach = &aasworld.reachability[settings->firstreachablearea + i];
		nextareanum = reach->areanum;
		if (!nextareanum)
			continue;
		if (done[nextareanum])
			continue;
		done[nextareanum] = qtrue;
		if (aasworld.areasettings[nextareanum].contents & AREACONTENTS_VIEWPORTAL)
			continue;
		if (AAS_AreaCluster(nextareanum) != cluster)
			continue;
		/*
		if ((reach->traveltype & TRAVELTYPE_MASK) == TRAVEL_WALKOFFLEDGE)
		{
			AAS_DebugLine(reach->start, reach->end, 1);
		}
		*/
		AAS_FloodAreas_r(nextareanum, cluster, done);
	}
}

void AAS_FloodAreas(bvec3_t origin)
{
	int areanum, cluster, *done;

	done = (int *) GetClearedMemory(aasworld.numareas * sizeof(int));
	areanum = AAS_PointAreaNum(origin);
	cluster = AAS_AreaCluster(areanum);
	AAS_FloodAreas_r(areanum, cluster, done);
}
