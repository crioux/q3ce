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
 * name:		be_aas_sample.h
 *
 * desc:		AAS
 *
 * $Archive: /source/code/botlib/be_aas_sample.h $
 *
 *****************************************************************************/

#ifdef AASINTERN
void AAS_InitAASLinkHeap(void);
void AAS_InitAASLinkedEntities(void);
void AAS_FreeAASLinkHeap(void);
void AAS_FreeAASLinkedEntities(void);
aas_face_t *AAS_AreaGroundFace(int areanum, bvec3_t point);
aas_face_t *AAS_TraceEndFace(aas_trace_t *trace);
aas_plane_t *AAS_PlaneFromNum(int planenum);
aas_link_t *AAS_AASLinkEntity(bvec3_t absmins, bvec3_t absmaxs, int entnum);
aas_link_t *AAS_LinkEntityClientBBox(bvec3_t absmins, bvec3_t absmaxs, int entnum, int presencetype);
qboolean AAS_PointInsideFace(int facenum, bvec3_t point, bfixed epsilon);
qboolean AAS_InsideFace(aas_face_t *face, avec3_t pnormal, bvec3_t point, bfixed epsilon);
void AAS_UnlinkFromAreas(aas_link_t *areas);
#endif //AASINTERN

//returns the mins and maxs of the bounding box for the given presence type
void AAS_PresenceTypeBoundingBox(int presencetype, bvec3_t mins, bvec3_t maxs);
//returns the cluster the area is in (negative portal number if the area is a portal)
int AAS_AreaCluster(int areanum);
//returns the presence type(s) of the area
int AAS_AreaPresenceType(int areanum);
//returns the presence type(s) at the given point
int AAS_PointPresenceType(bvec3_t point);
//returns the result of the trace of a client bbox
aas_trace_t AAS_TraceClientBBox(bvec3_t start, bvec3_t end, int presencetype, int passent);
//stores the areas the trace went through and returns the number of passed areas
int AAS_TraceAreas(bvec3_t start, bvec3_t end, int *areas, bvec3_t *points, int maxareas);
//returns the areas the bounding box is in
int AAS_BBoxAreas(bvec3_t absmins, bvec3_t absmaxs, int *areas, int maxareas);
//return area information
int AAS_AreaInfo( int areanum, aas_areainfo_t *info );
//returns the area the point is in
int AAS_PointAreaNum(bvec3_t point);
//
int AAS_PointReachabilityAreaIndex( bvec3_t point );
//returns the plane the given face is in
void AAS_FacePlane(int facenum, avec3_t normal, bfixed *dist);

