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
#include"splines_pch.h"


idCameraDef splineList;
idCameraDef *g_splineList = &splineList;

idVec3_t idSplineList::zero(GFIXED_0,GFIXED_0,GFIXED_0);
/*
void glLabeledPoint(idVec3_t &color, idVec3_t &point, gfixed size, const char *label) {
	glColor3fv(color);
	glPointSize(size);
	glBegin(GL_POINTS);
	glVertex3fv(point);
	glEnd();
	idVec3_t v = point;
	v.x += 1;
	v.y += 1;
	v.z += 1;
	glRasterPos3fv (v);
	glCallLists (strlen(label), GL_UNSIGNED_BYTE, label);
}


void glBox(idVec3_t &color, idVec3_t &point, gfixed size) {
	idVec3_t mins(point);
	idVec3_t maxs(point);
	mins[0] -= size;
	mins[1] += size;
	mins[2] -= size;
	maxs[0] += size;
	maxs[1] -= size;
	maxs[2] += size;
	glColor3fv(color);
	glBegin(GL_LINE_LOOP);
	glVertex3f(mins[0],mins[1],mins[2]);
	glVertex3f(maxs[0],mins[1],mins[2]);
	glVertex3f(maxs[0],maxs[1],mins[2]);
	glVertex3f(mins[0],maxs[1],mins[2]);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(mins[0],mins[1],maxs[2]);
	glVertex3f(maxs[0],mins[1],maxs[2]);
	glVertex3f(maxs[0],maxs[1],maxs[2]);
	glVertex3f(mins[0],maxs[1],maxs[2]);
	glEnd();

	glBegin(GL_LINES);
  	glVertex3f(mins[0],mins[1],mins[2]);
	glVertex3f(mins[0],mins[1],maxs[2]);
	glVertex3f(mins[0],maxs[1],maxs[2]);
	glVertex3f(mins[0],maxs[1],mins[2]);
	glVertex3f(maxs[0],mins[1],mins[2]);
	glVertex3f(maxs[0],mins[1],maxs[2]);
	glVertex3f(maxs[0],maxs[1],maxs[2]);
	glVertex3f(maxs[0],maxs[1],mins[2]);
	glEnd();

}

void splineTest() {
	//g_splineList->load("p:/doom/base/maps/test_base1.camera");
}

void splineDraw() {
	//g_splineList->addToRenderer();
}


//extern void D_DebugLine( const idVec3_t &color, const idVec3_t &start, const idVec3_t &end );

void debugLine(idVec3_t &color, gfixed x, gfixed y, gfixed z, gfixed x2, gfixed y2, gfixed z2) {
	//idVec3_t from(x, y, z);
	//idVec3_t to(x2, y2, z2);
	//D_DebugLine(color, from, to);
}


void idSplineList::addToRenderer() {

	if (controlPoints.Num() == 0) {
		return;
	}

	idVec3_t mins, maxs;
	idVec3_t yellow(GFIXED_1, GFIXED_1, 0);
	idVec3_t white(GFIXED_1, GFIXED_1, GFIXED_1);
        int i;
        
	for(i = 0; i < controlPoints.Num(); i++) {
		VectorCopy(*controlPoints[i], mins);
		VectorCopy(mins, maxs);
		mins[0] -= 8;
		mins[1] += 8;
		mins[2] -= 8;
		maxs[0] += 8;
		maxs[1] -= 8;
		maxs[2] += 8;
		debugLine( yellow, mins[0], mins[1], mins[2], maxs[0], mins[1], mins[2]);
		debugLine( yellow, maxs[0], mins[1], mins[2], maxs[0], maxs[1], mins[2]);
		debugLine( yellow, maxs[0], maxs[1], mins[2], mins[0], maxs[1], mins[2]);
		debugLine( yellow, mins[0], maxs[1], mins[2], mins[0], mins[1], mins[2]);
		
		debugLine( yellow, mins[0], mins[1], maxs[2], maxs[0], mins[1], maxs[2]);
		debugLine( yellow, maxs[0], mins[1], maxs[2], maxs[0], maxs[1], maxs[2]);
		debugLine( yellow, maxs[0], maxs[1], maxs[2], mins[0], maxs[1], maxs[2]);
		debugLine( yellow, mins[0], maxs[1], maxs[2], mins[0], mins[1], maxs[2]);
	    
	}

	int step = 0;
	idVec3_t step1;
	for(i = 3; i < controlPoints.Num(); i++) {
		for (gfixed tension = GFIXED_0; tension < GFIXED(1,001); tension += GFIXED(0,1)) {
			gfixed x = 0;
			gfixed y = 0;
			gfixed z = 0;
			for (int j = 0; j < 4; j++) {
				x += controlPoints[i - (3 - j)]->x * calcSpline(j, tension);
				y += controlPoints[i - (3 - j)]->y * calcSpline(j, tension);
				z += controlPoints[i - (3 - j)]->z * calcSpline(j, tension);
			}
			if (step == 0) {
				step1[0] = x;
				step1[1] = y;
				step1[2] = z;
				step = 1;
			} else {
				debugLine( white, step1[0], step1[1], step1[2], x, y, z);
				step = 0;
			}

		}
	}
}
*/

void idSplineList::buildSpline() {
	//int start = Sys_Milliseconds();
	clearSpline();
	for(int i = 3; i < controlPoints.Num(); i++) {
		for (gfixed tension = GFIXED_0; tension < GFIXED(1,001); tension += granularity) {
			gfixed x = GFIXED_0;
			gfixed y = GFIXED_0;
			gfixed z = GFIXED_0;
			for (int j = 0; j < 4; j++) {
				x += controlPoints[i - (3 - j)]->x * calcSpline(j, tension);
				y += controlPoints[i - (3 - j)]->y * calcSpline(j, tension);
				z += controlPoints[i - (3 - j)]->z * calcSpline(j, tension);
			}
			splinePoints.Append(new idVec3_t(x, y, z));
		}
	}
	dirty = false;
	//Com_Printf("Spline build took %f seconds\n", MAKE_GFIXED(Sys_Milliseconds() - start) / 1000.0f);
}

/*
void idSplineList::draw(bool editMode) {
	int i;
	mvec4_t yellow(1, 1, 0, 1);
        
	if (controlPoints.Num() == 0) {
		return;
	}

	if (dirty) {
		buildSpline();
	}


	glColor3fv(controlColor);
	glPointSize(5);
	
	glBegin(GL_POINTS);
	for (i = 0; i < controlPoints.Num(); i++) {
		glVertex3fv(*controlPoints[i]);
	}
	glEnd();
	
	if (editMode) {
		for(i = 0; i < controlPoints.Num(); i++) {
			glBox(activeColor, *controlPoints[i], 4);
		}
	}

	//Draw the curve
	glColor3fv(pathColor);
	glBegin(GL_LINE_STRIP);
	int count = splinePoints.Num();
	for (i = 0; i < count; i++) {
		glVertex3fv(*splinePoints[i]);
	}
	glEnd();

	if (editMode) {
		glColor3fv(segmentColor);
		glPointSize(3);
		glBegin(GL_POINTS);
		for (i = 0; i < count; i++) {
			glVertex3fv(*splinePoints[i]);
		}
		glEnd();
	}
	if (count > 0) {
		//assert(activeSegment >=0 && activeSegment < count);
		if (activeSegment >=0 && activeSegment < count) {
			glBox(activeColor, *splinePoints[activeSegment], 6);
			glBox(yellow, *splinePoints[activeSegment], 8);
		}
	}

}
*/

gfixed idSplineList::totalDistance() {

	if (controlPoints.Num() == 0) {
		return GFIXED_0;
	}

	if (dirty) {
		buildSpline();
	}

	gfixed dist = GFIXED_0;
	idVec3_t temp;
	int count = splinePoints.Num();
	for(int i = 1; i < count; i++) {
		temp = *splinePoints[i-1];
		temp -= *splinePoints[i];
		dist += temp.Length();
	}
	return dist;
}

void idSplineList::initPosition(long bt, long totalTime) {

	if (dirty) {
		buildSpline();
	}

	if (splinePoints.Num() == 0) {
		return;
	}

	baseTime = bt;
	time = totalTime;

	// calc distance to travel ( this will soon be broken into time segments )
	splineTime.Clear();
	splineTime.Append(MAKE_LFIXED(bt));
	lfixed dist = MAKE_LFIXED(totalDistance());
	lfixed distSoFar = LFIXED_0;
	idVec3_t temp;
	int count = splinePoints.Num();
	//for(int i = 2; i < count - 1; i++) {
	for(int i = 1; i < count; i++) {
		temp = *splinePoints[i-1];
		temp -= *splinePoints[i];
		distSoFar += MAKE_LFIXED(temp.Length());
		lfixed percent = distSoFar / dist;
		percent *= MAKE_LFIXED(totalTime);
		splineTime.Append(percent + MAKE_LFIXED(bt));
	}
	assert(splineTime.Num() == splinePoints.Num());
	activeSegment = 0;
}



gfixed idSplineList::calcSpline(int step, gfixed tension) {
	switch(step) {
		case 0:	return (FIXED_POW(GFIXED_1 - tension, GFIXED(3,0))) / GFIXED(6,0);
		case 1:	return (GFIXED(3,0) * FIXED_POW(tension, GFIXED(3,0)) - GFIXED(6,0) * FIXED_POW(tension, GFIXED(2,0)) + GFIXED(4,0)) / GFIXED(6,0);
		case 2:	return (-GFIXED(3,0) * FIXED_POW(tension, GFIXED(3,0)) + GFIXED(3,0) * FIXED_POW(tension, GFIXED(2,0)) + GFIXED(3,0) * tension + GFIXED_1) / GFIXED(6,0);
		case 3:	return FIXED_POW(tension, GFIXED(3,0)) / GFIXED(6,0);
	}
	return GFIXED_0;
}



void idSplineList::updateSelection(const idVec3_t &move) {
	if (selected) {
		dirty = true;
		VectorAdd((gfixed *)selected, (gfixed *)&move, (gfixed *)selected);
	}
}


void idSplineList::setSelectedPoint(idVec3_t *p) {
	if (p) {
		p->Snap();
		for(int i = 0; i < controlPoints.Num(); i++) {
			if (*p == *controlPoints[i]) {
				selected = controlPoints[i];
			}
		}
	} else {
		selected = NULL;
	}
}

const idVec3_t *idSplineList::getPosition(long t) {
	static idVec3_t interpolatedPos;
	//static long lastTime = -1;

	int count = splineTime.Num();
	if (count == 0) {
		return &zero;
	}

	Com_Printf("Time: %d\n", t);
	assert(splineTime.Num() == splinePoints.Num());

	while (activeSegment < count) {
		if (splineTime[activeSegment] >= MAKE_LFIXED(t)) {
			if (activeSegment > 0 && activeSegment < count - 1) {
				lfixed timeHi = splineTime[activeSegment + 1];
				lfixed timeLo = splineTime[activeSegment - 1];
				lfixed percent = (timeHi - MAKE_LFIXED(t)) / (timeHi - timeLo); 
				// pick two bounding points
				idVec3_t v1 = *splinePoints[activeSegment-1];
				idVec3_t v2 = *splinePoints[activeSegment+1];
				v2 *= MAKE_GFIXED(LFIXED_1 - percent);
				v1 *= MAKE_GFIXED(percent);
				v2 += v1;
				interpolatedPos = v2;
				return &interpolatedPos;
			}
			return splinePoints[activeSegment];
		} else {
			activeSegment++;
		}
	}
	return splinePoints[count-1];
}

void idSplineList::parse(const char *(*text)  ) {
	const char *token;
	//Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !Q_stricmp (token, "}") ) {
			break;
		}

		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !Q_stricmp (token, "(") || !Q_stricmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "granularity") == 0) {
				granularity = MAKE_GFIXED(atof(token));
			} else if (Q_stricmp(key.c_str(), "name") == 0) {
				name = token;
			}
			token = Com_Parse(text);

		} while (1);

		if ( !Q_stricmp (token, "}") ) {
			break;
		}

		Com_UngetToken();
		// read the control point
		idVec3_t point;
		Com_Parse1DMatrix( text, 3, point );
		addPoint(point.x, point.y, point.z);
	} while (1);
 
	//Com_UngetToken();
	//Com_MatchToken( text, "}" );
	dirty = true;
}

void idSplineList::write(fileHandle_t file, const char *p) {
	idStr s = va("\t\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	//s = va("\t\tname %s\n", name.c_str());
	//FS_Write(s.c_str(), s.length(), file);
	s = va("\t\t\tgranularity %f\n", FIXED_TO_DOUBLE(granularity));
	FS_Write(s.c_str(), s.length(), file);
	int count = controlPoints.Num();
	for (int i = 0; i < count; i++) {
		s = va("\t\t\t( %f %f %f )\n", FIXED_TO_DOUBLE(controlPoints[i]->x), FIXED_TO_DOUBLE(controlPoints[i]->y), FIXED_TO_DOUBLE(controlPoints[i]->z));
		FS_Write(s.c_str(), s.length(), file);
	}
	s = "\t\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}


void idCameraDef::getActiveSegmentInfo(int segment, idVec3_t &origin, idVec3_t &direction, gfixed *fov) {
#if 0
	if (!cameraSpline.validTime()) {
		buildCamera();
	}
	lfixed d = MAKE_LFIXEDsegment / numSegments();
	getCameraInfo(d * totalTime * 1000, origin, direction, fov);
#endif
/*
	if (!cameraSpline.validTime()) {
		buildCamera();
	}
	origin = *cameraSpline.getSegmentPoint(segment);
	

	idVec3_t temp;

	int numTargets = getTargetSpline()->controlPoints.Num();
	int count = cameraSpline.splineTime.Num();
	if (numTargets == 0) {
		// follow the path
		if (cameraSpline.getActiveSegment() < count - 1) {
			temp = *cameraSpline.splinePoints[cameraSpline.getActiveSegment()+1];
		}
	} else if (numTargets == 1) {
		temp = *getTargetSpline()->controlPoints[0];
	} else {
		temp = *getTargetSpline()->getSegmentPoint(segment);
	}

	temp -= origin;
	temp.Normalize();
	direction = temp;
*/
}

bool idCameraDef::getCameraInfo(long time, idVec3_t &origin, idVec3_t &direction, gfixed *fv) {


	if (MAKE_GFIXED((time - startTime) / 1000) > totalTime) {
		return false;
	}


	for (int i = 0; i < events.Num(); i++) {
		if (time >= startTime + events[i]->getTime() && !events[i]->getTriggered()) {
			events[i]->setTriggered(true);
			if (events[i]->getType() == idCameraEvent::EVENT_TARGET) {
				setActiveTargetByName(events[i]->getParam());
				getActiveTarget()->start(startTime + events[i]->getTime());
				//Com_Printf("Triggered event switch to target: %s\n",events[i]->getParam());
			} else if (events[i]->getType() == idCameraEvent::EVENT_TRIGGER) {
				//idEntity *ent = NULL;
				//ent = level.FindTarget( ent, events[i]->getParam());
				//if (ent) {
				//	ent->signal( SIG_TRIGGER );
				//	ent->ProcessEvent( &EV_Activate, world );
				//}
			} else if (events[i]->getType() == idCameraEvent::EVENT_FOV) {
				//*fv = fov = atof(events[i]->getParam());
			} else if (events[i]->getType() == idCameraEvent::EVENT_STOP) {
				return false;
			}
		}
	}

	origin = *cameraPosition->getPosition(time);
	
	*fv = fov.getFOV(time);

	idVec3_t temp = origin;

	int numTargets = targetPositions.Num();
	if (numTargets == 0) {
/*
		// follow the path
		if (cameraSpline.getActiveSegment() < count - 1) {
			temp = *cameraSpline.splinePoints[cameraSpline.getActiveSegment()+1];
			if (temp == origin) {
				int index = cameraSpline.getActiveSegment() + 2;
				while (temp == origin && index < count - 1) {
					temp = *cameraSpline.splinePoints[index++];
				}
			}
		}
*/
	} else {
		temp = *getActiveTarget()->getPosition(time);
	}
	
	temp -= origin;
	temp.Normalize();
	direction = temp;

	return true;
}

bool idCameraDef::waitEvent(int index) {
	//for (int i = 0; i < events.Num(); i++) {
	//	if (events[i]->getSegment() == index && events[i]->getType() == idCameraEvent::EVENT_WAIT) {
	//		return true;
	//	}
    //}
	return false;
}


#define NUM_CCELERATION_SEGS 10
#define CELL_AMT 5

void idCameraDef::buildCamera() {
	int i;
	//int lastSwitch = 0;
	idList<gfixed> waits;
	idList<int> targets;

	totalTime = baseTime;
	cameraPosition->setTime(FIXED_TO_INT(totalTime * GFIXED(1000,0)));
	// we have a base time layout for the path and the target path
	// now we need to layer on any wait or speed changes
	for (i = 0; i < events.Num(); i++) {
		//idCameraEvent *ev = events[i];
		events[i]->setTriggered(false);
		switch (events[i]->getType()) {
			case idCameraEvent::EVENT_TARGET : {
				targets.Append(i);
				break;
			}
			case idCameraEvent::EVENT_WAIT : {
				waits.Append(MAKE_GFIXED(atof(events[i]->getParam())));
				cameraPosition->addVelocity(events[i]->getTime(),(long)(atof(events[i]->getParam()) * 1000), GFIXED_0);
				break;
			}
			case idCameraEvent::EVENT_TARGETWAIT : {
				//targetWaits.Append(i);
				break;
			}
			case idCameraEvent::EVENT_SPEED : {
/*
				// take the average delay between up to the next five segments
				gfixed adjust = atof(events[i]->getParam());
				int index = events[i]->getSegment();
				total = 0;
				count = 0;

				// get total amount of time over the remainder of the segment
				for (j = index; j < cameraSpline.numSegments() - 1; j++) {
					total += cameraSpline.getSegmentTime(j + 1) - cameraSpline.getSegmentTime(j);
					count++;
				}

				// multiply that by the adjustment
				lfixed newTotal = total * adjust;
				// what is the difference.. 
				newTotal -= total;
				totalTime += newTotal / 1000;

				// per segment difference
				newTotal /= count;
				int additive = newTotal;

				// now propogate that difference out to each segment
				for (j = index; j < cameraSpline.numSegments(); j++) {
					cameraSpline.addSegmentTime(j, additive);
					additive += newTotal;
				}
				break;
*/
			}
    default: break; // FIXME: what about other idCameraEvent?
		}
	}


	for (i = 0; i < waits.Num(); i++) {
		totalTime += waits[i];
	}

	// on a new target switch, we need to take time to this point ( since last target switch ) 
	// and allocate it across the active target, then reset time to this point
	long timeSoFar = 0;
	long total = FIXED_TO_INT(totalTime * GFIXED(1000,0));
	for (i = 0; i < targets.Num(); i++) {
		long t;
		if (i < targets.Num() - 1) {
			t = events[targets[i+1]]->getTime();
		} else {
			t = total - timeSoFar;
		}
		// t is how much time to use for this target
		setActiveTargetByName(events[targets[i]]->getParam());
		getActiveTarget()->setTime(t);
		timeSoFar += t;
	}

	
}

void idCameraDef::startCamera(long t) {
	buildCamera();
	cameraPosition->start(t);
	//for (int i = 0; i < targetPositions.Num(); i++) {
	//	targetPositions[i]->
	//}
	startTime = t;
	cameraRunning = true;
}


void idCameraDef::parse(const char *(*text)  ) {

	const char	*token;
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !Q_stricmp (token, "}") ) {
			break;
		}

		if (Q_stricmp(token, "time") == 0) {
			baseTime = Com_ParseFloat(text);
		}

		if (Q_stricmp(token, "camera_fixed") == 0) {
			cameraPosition = new idFixedPosition();
			cameraPosition->parse(text);
		}

		if (Q_stricmp(token, "camera_interpolated") == 0) {
			cameraPosition = new idInterpolatedPosition();
			cameraPosition->parse(text);
		}

		if (Q_stricmp(token, "camera_spline") == 0) {
			cameraPosition = new idSplinePosition();
			cameraPosition->parse(text);
		}

		if (Q_stricmp(token, "target_fixed") == 0) {
			idFixedPosition *pos = new idFixedPosition();
			pos->parse(text);
			targetPositions.Append(pos);
		}
		
		if (Q_stricmp(token, "target_interpolated") == 0) {
			idInterpolatedPosition *pos = new idInterpolatedPosition();
			pos->parse(text);
			targetPositions.Append(pos);
		}

		if (Q_stricmp(token, "target_spline") == 0) {
			idSplinePosition *pos = new idSplinePosition();
			pos->parse(text);
			targetPositions.Append(pos);
		}

		if (Q_stricmp(token, "fov") == 0) {
			fov.parse(text);
		}

		if (Q_stricmp(token, "event") == 0) {
			idCameraEvent *event = new idCameraEvent();
			event->parse(text);
			addEvent(event);
		}


	} while (1);

	Com_UngetToken();
	Com_MatchToken( text, "}" );

}

qboolean idCameraDef::load(const char *filename) {
	char *buf;
	const char *buf_p;
	//int length = 
  FS_ReadFile( filename, (void **)&buf );
	if ( !buf ) {
		return qfalse;
	}

	clear();
	Com_BeginParseSession( filename );
	buf_p = buf;
	parse(&buf_p);
	Com_EndParseSession();
	FS_FreeFile( buf );

	return qtrue;
}

void idCameraDef::save(const char *filename) {
	fileHandle_t file = FS_FOpenFileWrite(filename);
	if (file) {
		int i;
		idStr s = "cameraPathDef { \n"; 
		FS_Write(s.c_str(), s.length(), file);
		s = va("\ttime %f\n", FIXED_TO_DOUBLE(baseTime));
		FS_Write(s.c_str(), s.length(), file);

		cameraPosition->write(file, va("camera_%s",cameraPosition->typeStr()));

		for (i = 0; i < numTargets(); i++) {
			targetPositions[i]->write(file, va("target_%s", targetPositions[i]->typeStr()));
		}

		for (i = 0; i < events.Num(); i++) {
			events[i]->write(file, "event");
		}

		fov.write(file, "fov");

		s = "}\n";
		FS_Write(s.c_str(), s.length(), file);
	}
	FS_FCloseFile(file);
}

int idCameraDef::sortEvents(const void *p1, const void *p2) {
	idCameraEvent *ev1 = (idCameraEvent*)(p1);
	idCameraEvent *ev2 = (idCameraEvent*)(p2);

	if (ev1->getTime() > ev2->getTime()) {
		return -1;
	}
	if (ev1->getTime() < ev2->getTime()) {
		return 1;
	}
	return 0; 
}

void idCameraDef::addEvent(idCameraEvent *event) {
	events.Append(event);
	//events.Sort(&sortEvents);

}
void idCameraDef::addEvent(idCameraEvent::eventType t, const char *param, long time) {
	addEvent(new idCameraEvent(t, param, time));
	buildCamera();
}


const char *idCameraEvent::eventStr[] = {
	"NA",
	"WAIT",
	"TARGETWAIT",
	"SPEED",
	"TARGET",
	"SNAPTARGET",
	"FOV",
	"SCRIPT",
	"TRIGGER",
	"STOP"
};

void idCameraEvent::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "type") == 0) {
				type = (idCameraEvent::eventType)(atoi(token));
			} else if (Q_stricmp(key.c_str(), "param") == 0) {
				paramStr = token;
			} else if (Q_stricmp(key.c_str(), "time") == 0) {
				time = atoi(token);
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}

void idCameraEvent::write(fileHandle_t file, const char *name) {
	idStr s = va("\t%s {\n", name);
	FS_Write(s.c_str(), s.length(), file);
	s = va("\t\ttype %d\n", (int)(type));
	FS_Write(s.c_str(), s.length(), file);
	s = va("\t\tparam %s\n", paramStr.c_str());
	FS_Write(s.c_str(), s.length(), file);
	s = va("\t\ttime %d\n", time);
	FS_Write(s.c_str(), s.length(), file);
	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}


const char *idCameraPosition::positionStr[] = {
	"Fixed",
	"Interpolated",
	"Spline",
};



const idVec3_t *idInterpolatedPosition::getPosition(long t) { 
	static idVec3_t interpolatedPos;

	gfixed velocity = getVelocity(t);
	gfixed timePassed = MAKE_GFIXED(t - lastTime);
	lastTime = t;

	// convert to seconds	
	timePassed /= GFIXED(1000,0);

	gfixed distToTravel = timePassed *= velocity;

	idVec3_t temp = startPos;
	temp -= endPos;
	gfixed distance = temp.Length();

	distSoFar += distToTravel;
	gfixed percent = MAKE_GFIXED(distSoFar) / distance;

	if (percent > GFIXED_1) {
		percent = GFIXED_1;
	} else if (percent < GFIXED_0) {
		percent = GFIXED_0;
	}

	// the following line does a straigt calc on percentage of time
	// gfixed percent = MAKE_GFIXED(startTime + time - t) / time;

	idVec3_t v1 = startPos;
	idVec3_t v2 = endPos;
	v1 *= (GFIXED_1 - percent);
	v2 *= percent;
	v1 += v2;
	interpolatedPos = v1;
	return &interpolatedPos;
}


void idCameraFOV::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "fov") == 0) {
				fov = MAKE_GFIXED(atof(token));
			} else if (Q_stricmp(key.c_str(), "startFOV") == 0) {
				startFOV = MAKE_GFIXED(atof(token));
			} else if (Q_stricmp(key.c_str(), "endFOV") == 0) {
				endFOV = MAKE_GFIXED(atof(token));
			} else if (Q_stricmp(key.c_str(), "time") == 0) {
				time = atoi(token);
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}

bool idCameraPosition::parseToken(const char *key, const char *(*text)) {
	const char *token = Com_Parse(text);
	if (Q_stricmp(key, "time") == 0) {
		time = atol(token);
		return true;
	} else if (Q_stricmp(key, "type") == 0) {
		type = (idCameraPosition::positionType)(atoi(token));
		return true;
	} else if (Q_stricmp(key, "velocity") == 0) {
		long t = atol(token);
		token = Com_Parse(text);
		long d = atol(token);
		token = Com_Parse(text);
		addVelocity(t, d, MAKE_GFIXED(atof(token)));
		return true;
	} else if (Q_stricmp(key, "baseVelocity") == 0) {
		baseVelocity = MAKE_GFIXED(atof(token));
		return true;
	} else if (Q_stricmp(key, "name") == 0) {
		name = token;
		return true;
	} else if (Q_stricmp(key, "time") == 0) {
		time = atoi(token);
		return true;
	}
	Com_UngetToken();
	return false;
}



void idFixedPosition::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "pos") == 0) {
				Com_UngetToken();
				Com_Parse1DMatrix( text, 3, pos );
			} else {
				Com_UngetToken();
				idCameraPosition::parseToken(key.c_str(), text);	
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}

void idInterpolatedPosition::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "startPos") == 0) {
				Com_UngetToken();
				Com_Parse1DMatrix( text, 3, startPos );
			} else if (Q_stricmp(key.c_str(), "endPos") == 0) {
				Com_UngetToken();
				Com_Parse1DMatrix( text, 3, endPos );
			} else {
				Com_UngetToken();
				idCameraPosition::parseToken(key.c_str(), text);	
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}


void idSplinePosition::parse(const char *(*text)  ) {
	const char *token;
	Com_MatchToken( text, "{" );
	do {
		token = Com_Parse( text );
	
		if ( !token[0] ) {
			break;
		}
		if ( !strcmp (token, "}") ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !strcmp (token, "(") || !strcmp(token, "}")) {
				break;
			}

			Com_UngetToken();
			idStr key = Com_ParseOnLine(text);
			
			const char *token = Com_Parse(text);
			if (Q_stricmp(key.c_str(), "target") == 0) {
				target.parse(text);
			} else {
				Com_UngetToken();
				idCameraPosition::parseToken(key.c_str(), text);	
			}
			token = Com_Parse(text);

		} while (1);

		if ( !strcmp (token, "}") ) {
			break;
		}

	} while (1);
 
	Com_UngetToken();
	Com_MatchToken( text, "}" );
}



void idCameraFOV::write(fileHandle_t file, const char *p) {
	idStr s = va("\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	
	s = va("\t\tfov %f\n", FIXED_TO_DOUBLE(fov));
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\tstartFOV %f\n", FIXED_TO_DOUBLE(startFOV));
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\tendFOV %f\n", FIXED_TO_DOUBLE(endFOV));
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\ttime %i\n", time);
	FS_Write(s.c_str(), s.length(), file);

	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}


void idCameraPosition::write(fileHandle_t file, const char *p) {
	
	idStr s = va("\t\ttime %i\n", time);
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\ttype %i\n", (int)(type));
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\tname %s\n", name.c_str());
	FS_Write(s.c_str(), s.length(), file);

	s = va("\t\tbaseVelocity %f\n", FIXED_TO_DOUBLE(baseVelocity));
	FS_Write(s.c_str(), s.length(), file);

	for (int i = 0; i < velocities.Num(); i++) {
		s = va("\t\tvelocity %i %i %f\n", velocities[i]->startTime, velocities[i]->time, FIXED_TO_DOUBLE(velocities[i]->speed));
		FS_Write(s.c_str(), s.length(), file);
	}

}

void idFixedPosition::write(fileHandle_t file, const char *p) {
	idStr s = va("\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	idCameraPosition::write(file, p);
	s = va("\t\tpos ( %f %f %f )\n", FIXED_TO_DOUBLE(pos.x), FIXED_TO_DOUBLE(pos.y), FIXED_TO_DOUBLE(pos.z));
	FS_Write(s.c_str(), s.length(), file);
	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}

void idInterpolatedPosition::write(fileHandle_t file, const char *p) {
	idStr s = va("\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	idCameraPosition::write(file, p);
	s = va("\t\tstartPos ( %f %f %f )\n", FIXED_TO_DOUBLE(startPos.x), FIXED_TO_DOUBLE(startPos.y), FIXED_TO_DOUBLE(startPos.z));
	FS_Write(s.c_str(), s.length(), file);
	s = va("\t\tendPos ( %f %f %f )\n", FIXED_TO_DOUBLE(endPos.x), FIXED_TO_DOUBLE(endPos.y), FIXED_TO_DOUBLE(endPos.z));
	FS_Write(s.c_str(), s.length(), file);
	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}

void idSplinePosition::write(fileHandle_t file, const char *p) {
	idStr s = va("\t%s {\n", p);
	FS_Write(s.c_str(), s.length(), file);
	idCameraPosition::write(file, p);
	target.write(file, "target");
	s = "\t}\n";
	FS_Write(s.c_str(), s.length(), file);
}

void idCameraDef::addTarget(const char *name, idCameraPosition::positionType type) {
	//const char *text = (name == NULL) ? va("target0%d", numTargets()+1) : name; // TTimo: unused
	idCameraPosition *pos = newFromType(type);
	if (pos) {
		pos->setName(name);
		targetPositions.Append(pos);
		activeTarget = numTargets()-1;
		if (activeTarget == 0) {
			// first one
			addEvent(idCameraEvent::EVENT_TARGET, name, 0);
		}
	}
}



idCameraDef camera;

extern "C" {
qboolean loadCamera(const char *name) {
  camera.clear();
  return (qboolean)(camera.load(name));
}

qboolean getCameraInfo(int time, gfixed *origin, gfixed*angles) {
	idVec3_t dir, org;
	org[0] = origin[0];
	org[1] = origin[1];
	org[2] = origin[2];
	gfixed fov = GFIXED(90,0);
	if (camera.getCameraInfo(time, org, dir, &fov)) {
		origin[0] = org[0];
		origin[1] = org[1];
		origin[2] = org[2];
		angles[1] = FIXED_ATAN2(dir[1], dir[0])*GFIXED(180,0)/GFIXED(3,14159);
		angles[0] = FIXED_ASIN(dir[2])*GFIXED(180,0)/GFIXED_PI;
		return qtrue;
	}
	return qfalse;
}

void startCamera(int time) {
	camera.startCamera(time);
}

}


