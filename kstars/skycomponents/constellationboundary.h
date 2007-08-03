/***************************************************************************
               constellationboundary.h  -  K Desktop Planetarium
                             -------------------
    begin                : 25 Oct. 2005
    copyright            : (C) 2005 by Jason Harris
    email                : kstars@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONSTELLATIONBOUNDARY_H
#define CONSTELLATIONBOUNDARY_H

#include "linelistindex.h"

#include <QHash>
#include <QPolygonF>

class ConstellationBoundaryPoly;

/**
	*@class ConstellationBoundary
	*Collection of lines comprising the borders between constellations

	*@author Jason Harris
	*@version 0.1
	*/

class ConstellationBoundary : public LineListIndex 
{
	public:
	/**
		*@short Constructor
		*Simply adds all of the coordinate grid circles 
		*(meridians and parallels)
		*@p parent Pointer to the parent SkyComponent object
		*/
		ConstellationBoundary( SkyComponent *parent );

	/**
		*@short Initialize the Constellation boundary 
		*Reads the constellation boundary data from cbounds.dat.  
		*The boundary data is defined by a series of RA,Dec coordinate pairs 
		*defining the "nodes" of the boundaries.  The nodes are organized into 
		*"segments", such that each segment represents a continuous series 
		*of boundary-line intervals that divide two particular constellations.
		*
		*@param data Pointer to the KStarsData object
		*/
		void init( KStarsData *data );

        bool selected();

        void preDraw( KStars *ks, QPainter &psky );
        
        void JITupdate( KStarsData *data, LineList* lineList );

	    ConstellationBoundaryPoly* boundaries() { return m_Boundary; }

    private:
        ConstellationBoundaryPoly* m_Boundary;
};


#endif
