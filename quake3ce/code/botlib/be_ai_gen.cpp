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
 * name:		be_ai_gen.c
 *
 * desc:		genetic selection
 *
 * $Archive: /MissionPack/code/botlib/be_ai_gen.c $
 *
 *****************************************************************************/

#include"botlib_pch.h"

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int GeneticSelection(int numranks, gfixed *rankings)
{
	gfixed sum, select;
	int i, index;

	sum = GFIXED_0;
	for (i = 0; i < numranks; i++)
	{
		if (rankings[i] < GFIXED_0) continue;
		sum += rankings[i];
	} //end for
	if (sum > GFIXED_0)
	{
		//select a bot where the ones with the higest rankings have
		//the highest chance of being selected
		select = random() * sum;
		for (i = 0; i < numranks; i++)
		{
			if (rankings[i] < GFIXED_0) continue;
			sum -= rankings[i];
			if (sum <=GFIXED_0) return i;
		} //end for
	} //end if
	//select a bot randomly
	index = rand() % numranks;
	for (i = 0; i < numranks; i++)
	{
		if (rankings[index] >= GFIXED_0) return index;
		index = (index + 1) % numranks;
	} //end for
	return 0;
} //end of the function GeneticSelection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int GeneticParentsAndChildSelection(int numranks, gfixed *ranks, int *parent1, int *parent2, int *child)
{
	gfixed rankings[256], max;
	int i;

	if (numranks > 256)
	{
		botimport.Print(PRT_WARNING, "GeneticParentsAndChildSelection: too many bots\n");
		*parent1 = *parent2 = *child = 0;
		return qfalse;
	} //end if
	for (max = GFIXED_0, i = 0; i < numranks; i++)
	{
		if (ranks[i] < GFIXED_0) continue;
		max++;
	} //end for
	if (max < GFIXED(3,0))
	{
		botimport.Print(PRT_WARNING, "GeneticParentsAndChildSelection: too few valid bots\n");
		*parent1 = *parent2 = *child = 0;
		return qfalse;
	} //end if
	Com_Memcpy(rankings, ranks, sizeof(gfixed) * numranks);
	//select first parent
	*parent1 = GeneticSelection(numranks, rankings);
	rankings[*parent1] = -GFIXED_1;
	//select second parent
	*parent2 = GeneticSelection(numranks, rankings);
	rankings[*parent2] = -GFIXED_1;
	//reverse the rankings
	max = GFIXED_0;
	for (i = 0; i < numranks; i++)
	{
		if (rankings[i] < GFIXED_0) continue;
		if (rankings[i] > max) max = rankings[i];
	} //end for
	for (i = 0; i < numranks; i++)
	{
		if (rankings[i] < GFIXED_0) continue;
		rankings[i] = max - rankings[i];
	} //end for
	//select child
	*child = GeneticSelection(numranks, rankings);
	return qtrue;
} //end of the function GeneticParentsAndChildSelection
