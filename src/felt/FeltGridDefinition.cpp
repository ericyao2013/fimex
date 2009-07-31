/*
 wdb

 Copyright (C) 2007 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 E-mail: wdb@met.no

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA  02110-1301, USA
 */

#include "felt/FeltGridDefinition.h"
#include "felt/FeltFile.h"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cmath>

namespace felt {

// first scaling method
void scaleGridInfoFirst_(boost::array<float, 6>& gridPar, int parsUsed, float scale, const std::vector<short int> & extraData)
{
    assert(parsUsed <= 6);
    for (int i = 0; i < parsUsed; ++i) {
        int i2 = 2*i;
        int iGrid1 = extraData.at(i2);
        int iGrid2 = extraData.at(i2 + 1);

        gridPar[i] = (iGrid1 * 10000. + iGrid2) / scale;
    }
}

// second scaling method, internal scaling factor, int scale not used
void scaleGridInfoSecond_(boost::array<float, 6>& gridPar, int parsUsed, float scale, const std::vector<short int> & extraData)
{
    assert(parsUsed <= 6);
    assert(extraData.at(0) == parsUsed);
    assert(extraData.at(1) == 3);
    for (int i = 0; i < parsUsed; ++i) {
        int i3 = i*3 + 2; // 2 offset
        scale = std::pow(10., extraData.at(i3));
        int iGrid1 = extraData.at(i3 + 1);
        int iGrid2 = extraData.at(i3 + 2);

        gridPar[i] = (iGrid1 * 10000. + iGrid2) / scale;
    }
}


boost::array<float, 6> gridParametersPolarStereo_(int gridTypeFlag, int xNum, int yNum, int ixp, int iyp, int idist, int iphi, const std::vector<short int> & extraData)
{
    const int parsUsed = 5; // gridPar 6 not used
    boost::array<float, 6> gridPar;
    for (int i = 0; i < 6; ++i) {
        gridPar[i] = 0.;
    }
    // default at 60 degree
    gridPar[4] = 60.;
    if (idist > 0) {
        // standard
        gridPar[0] = ixp * .01;
        gridPar[1] = iyp * .01;
        gridPar[2] = idist * .1;
        gridPar[3] = iphi;
    } else {
        // old extension
        gridPar[0] = ixp;
        gridPar[1] = iyp;
        gridPar[2] = idist * -.1;
        gridPar[3] = iphi;
    }

    // corrections from extraData
    switch (gridTypeFlag) {
        case (2*parsUsed): scaleGridInfoFirst_(gridPar, parsUsed, 100, extraData); break;
        case (2+3*parsUsed): scaleGridInfoSecond_(gridPar, parsUsed, 100, extraData); break;
        default: throw std::runtime_error("unknown gridTypeFlag");
    }

    if (gridPar[2] != 0) gridPar[2] = 79.*150./gridPar[2];
    else throw std::runtime_error("polar-stereographic: cells between equator and pole cannot be 0");

    if (gridPar[4] == 0 || gridPar[4] < -90. || gridPar[4] > 90.)
        throw std::runtime_error("polar-stereographic: undefined angle phi");

    return gridPar;
}

boost::array<float, 6> gridParameters(int gridType, int xNum, int yNum, int a, int b, int c, int d, const std::vector<short int> & extraData)
{
    int gridTypeFlag = 0;
    if (gridType >= 1000) {
        gridTypeFlag = gridType % 1000;
        gridType /= 1000;
    }

    switch (gridType/1000) {
        case 1:
        case 4: return gridParametersPolarStereo_(gridTypeFlag, xNum, yNum, a, b, c, d, extraData); break;
    }
}


std::string FeltGridDefinition::getProjDefinition_(int gridType, const float * gs) const
{
    std::ostringstream projStr;
    switch (gridType) {
    case 1:
    	projStr << "+proj=stere +lat_0=90 +lon_0=" << (gs[3]) << " +lat_ts=" << (gs[4]) << " +a=6371000 +units=m +no_defs";
    	break;
    // TODO: Check the earth diameter ... again
    case 2:
        projStr << "+proj=longlat +a=6367470.0 +no_defs";
        break;
    case 3:
        projStr << "+proj=ob_tran +o_proj=longlat +lon_0=" << (gs[4]) << " +o_lat_p=" << (90 - gs[5]) << " +a=6367470.0 +no_defs";
        break;
    }
    return projStr.str();
}

FeltGridDefinition::Orientation FeltGridDefinition::getScanMode_(float * gs, int jNum) const
{
	float & jIncrement = gs[3];
	float & startLatitude = gs[1];
	if(jIncrement < 0)
    {
		jIncrement = jIncrement * -1;
		startLatitude = startLatitude - (jNum * jIncrement) + jIncrement;
        return LeftUpperHorizontal;
    }
    return LeftLowerHorizontal;
}

FeltGridDefinition::FeltGridDefinition( int gridType,
										int xNum, int yNum, int a, int b, int c, int d,
										const std::vector<short int> & extraData) :
										    xNum_(xNum), yNum_(yNum)
{
    switch (gridType){
    case 0:
        throw std::invalid_argument("Unspecified grid is not supported");
    case 1:
        // a, b - scaled up by 100 if c (grid distance) is positive
        // c - convert km to m, scaled up by 10
    	if (c >= 0) {
		    FeltFile::log("FeltGridDefinition: Low granularity grids");
    		polarStereographicProj( gridType, a/100.0, b/100.0, (c * 1000.0) / 10.0, d, extraData );
    	}
    	else {
			FeltFile::log("FeltGridDefinition: High granularity grids");
   			polarStereographicProj( gridType, a, b, (c * -1000.0) / 10.0, d, extraData );
    	}
        break;
    case 2:
    case 3:
      	geographicProj( gridType, b/100.0, a/100.0, d/100.0, c/100.0, extraData );
        break;
    case 4:
		throw std::invalid_argument("Non-standard polar stereographic grid is not supported");
    case 5:
        throw std::invalid_argument("Mercator grid is not supported");
    default:
        throw std::invalid_argument("Unknown grid specification");
    }
}

FeltGridDefinition::~FeltGridDefinition()
{
}

void
FeltGridDefinition::polarStereographicProj( int gridType,
									        float poleX, float poleY,
									        float gridD, float rot,
									        const std::vector<short int> & extraData)
{
    FeltFile::log("FeltGridDefinition: Polar stereographic projection identified in FELT file");
    const int gsSize = 6;
    float scale = 10000.0;
    float gs[gsSize];
    if ( extraData.empty() )
    {
        double pi = 3.14159265358979323844;
        gs[0] = poleX;
        gs[1] = poleY;
        gs[3] = rot;
        gs[4] = 60.0;
        // grid-distance due to scale-factor and old 150km grid correction
        // make sure to set gs[4], in particular with gridType 5
        gs[2] = gridD * (6371.* (1+std::sin(pi/180.*gs[4]))) / (79.*150.);
        gs[5] = 0.0;
    }
	else
		throw std::runtime_error("The encoded polar stereographic grid specification in the FELT file is not supported");

    if (FeltFile::isLogging()) {
        std::ostringstream buffer;
        buffer << "FeltGridDefinition: " << std::endl;
        buffer << "Size of Grid: " << xNum_ << " x " << yNum_ << std::endl;
        buffer << "Grid Specification: " << gs[0] << " | " << gs[1] << " | " << gs[2] << " | " << gs[3] << " | " << gs[4] << " | " << gs[5] << std::endl;
        buffer << "Grid Type: " << gridType << std::endl;
        FeltFile::log(buffer.str());
    }
    projDef_ = getProjDefinition_(gridType, gs);
    FeltFile::log("FeltGridDefinition: Proj Specification: " + projDef_);

    orientation_ = LeftLowerHorizontal; // Default
    startX_ = ( 1 - gs[0] ) * gs[2];
    startY_ = ( 1 - gs[1] ) * gs[2];
    incrementX_ = gs[2];
    incrementY_ = gs[2];
}

void
FeltGridDefinition::geographicProj( int gridType,
									float startLongitude, float startLatitude,
									float iInc, float jInc,
									const std::vector<short int> & extraData)
{
    FeltFile::log("FeltGridDefinition: Geographic projection identified in FELT file");
    const int gsSize = 6;
    float scale = 10000.0;
    float gs[gsSize];
    if ( extraData.empty() )
    {
        gs[0] = startLongitude;
        gs[1] = startLatitude;
        gs[2] = iInc;
        gs[3] = jInc;
        gs[4] = 0.0;
        gs[5] = 0.0;
    }
    else if ( extraData.size() == gsSize * 2 )
    {
		for(int i = 0;i < gsSize;i++)
			gs[i] = static_cast<float>((extraData[i * 2] * scale) + extraData[(i * 2) + 1]) / scale;
	}
	else if ( extraData.size() == 2 + (gsSize * 3) )
	{
		if(extraData[0] != gsSize)
			throw std::runtime_error("First word in the grid encoding does not correspond to the gridspec size");

		if(extraData[1] != 3)
			throw std::runtime_error("Second word in the grid encoding does not correspond to 3 (rotated grid)");

		for(int i = 0;i < gsSize;i++)
			gs[i] = static_cast<float>((extraData[(i * 3) + 3] * scale) + extraData[(i * 3) + 4]) / (extraData[(i * 3) + 2] * 10.0);
	}
	else
		throw std::runtime_error("The encoded grid specification in the FELT file is not supported");


    if (FeltFile::isLogging()) {
        std::ostringstream buffer;
        buffer << "FeltGridDefinition: " << std::endl;
        buffer << "Grid Specification: " << gs[0] << " | " << gs[1] << " | " << gs[2] << " | " << gs[3] << " | " << gs[4] << " | " << gs[5] << std::endl;
        buffer << "Grid Type: " << gridType << std::endl;
        FeltFile::log(buffer.str());
    }
    projDef_ = getProjDefinition_(gridType, gs);
    FeltFile::log("FeltGridDefinition: Proj Specification: " + projDef_);

    orientation_ = getScanMode_(gs, yNum_);
    incrementX_ = gs[2];
    incrementY_ = gs[3];
    startX_ = gs[0];
    startY_= gs[1];
}

std::string FeltGridDefinition::projDefinition() const
{
	return projDef_;
};

int
FeltGridDefinition::getXNumber() const
{
    return xNum_;
};

int
FeltGridDefinition::getYNumber() const
{
    return yNum_;
};

float
FeltGridDefinition::getXIncrement() const
{
	return incrementX_;
};

float
FeltGridDefinition::getYIncrement() const
{
	return incrementY_;
};

float
FeltGridDefinition::startLatitude() const
{
	return startY_;
};

float
FeltGridDefinition::startLongitude() const
{
	return startX_;
};

float
FeltGridDefinition::startX() const
{
	return startX_;
};

float
FeltGridDefinition::startY() const
{
	return startY_;
};

FeltGridDefinition::Orientation FeltGridDefinition::getScanMode() const
{
	return orientation_;
}

std::ostream & contentSummary(std::ostream & out, const FeltGridDefinition & grid)
{
	return out << "FeltGridDefinition( " << grid.getXIncrement() << ", "<<grid.getYIncrement() << ", "
		<< grid.startLongitude() << ", " << grid.startLatitude() << " )" << std::endl;
}

}