/*
 fimex

 Copyright (C) 2011 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 E-mail: post@met.no

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

#ifndef CDMNAMETRANSLATOR_H_
#define CDMNAMETRANSLATOR_H_

#include <fimex/CDMException.h>

// standard
//
#include <string>
#include <map>

namespace MetNoFimex
{

class XMLDoc;

namespace wdb
{

/**
 * Translates wdb names to cdm names, and vice-versa.
 */
class CdmNameTranslator
{
public:
	CdmNameTranslator();
	explicit CdmNameTranslator(const std::string& xmlFileName);
	explicit CdmNameTranslator(const XMLDoc& xmlDoc);

	~CdmNameTranslator();

	void addNamePair(const std::string& wdbName, const std::string& cdmName);

    /**
	 * both removeNNN will remove name pair
	 * as we are operating on assumption that
	 * we have bijection 1:1 mapping
	 */
	void removeWdbName(const std::string& wdbName);
	void removeCdmName(const std::string& cdmName);

    bool isEmpty() const;
	size_t size() const;
	void clear();

    bool hasWdbName(const std::string& wdbName) const;
	bool hasCdmName(const std::string& cdmName) const;

	std::string toCdmName(const std::string& wdbName) const;
	std::string toWdbName(const std::string& cdmName) const;


	std::string toString() const;
private:
	void readXML(const XMLDoc& xmlDoc);

	void readXML(const XMLDoc& xmlDoc, const std::string& path);
    /**
	 * TODO:
	 *     consider if mapping is
	 *         a)    1:1
	 *         b)    n:1
	 *
	 *     assumption that
	 *     1:M and N:M
	 *     are invalid
	 */
    std::map<std::string, std::string> mapWdbToCdm;
};

class CdmNameTranslatorException : public CDMException
{
public:
    explicit CdmNameTranslatorException(std::string msg)
	    : CDMException("CdmNameTranslatorException: " + msg) {}
};

}

}

#endif /* CDMNAMETRANSLATOR_H_ */
