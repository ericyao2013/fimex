/*
 * Fimex
 *
 * (C) Copyright 2011, met.no
 *
 * Project Info:  https://wiki.met.no/fimex/start
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

/**
  * Used as private/implementation class
  */

#ifndef METGM_CDMVARIABLEPROFILE_H
#define METGM_CDMVARIABLEPROFILE_H

// internals
//
#include "MetGmTags.h"
#include "MetGmDimensionsTag.h"

//fimex
//
#include "fimex/CDMVariable.h"

// boost
//
#include <boost/shared_ptr.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

// standard
//
#include <string>


namespace MetNoFimex {

    struct MetGmCDMVariableProfile {

        MetGmCDMVariableProfile(short pid, const std::string& cdmName, boost::shared_ptr<MetGmTags> tags)
            : p_id_(pid), cdmName_(cdmName), pTags_(tags) {}

        bool operator<(const MetGmCDMVariableProfile& profile) const {return cdmName_ < profile.cdmName_;}

        const short hd() const { return pTags_->hd(); }

        short                         p_id_;
        std::string                   cdmName_;
        std::string                   standardName_;
        std::string                   units_;
        std::string                   zDimensionName_;
        std::string                   addOffset_;
        std::string                   scaleFactor_;
        std::string                   fillValue_;
        boost::shared_ptr<MetGmTags>  pTags_;
    };

    struct cdm_pid_index       {};
    struct cdm_name_index      {};
    struct cdm_hd_index        {};

    typedef boost::multi_index::multi_index_container<
      MetGmCDMVariableProfile,
      boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
         boost::multi_index::identity<MetGmCDMVariableProfile>
        >,
        boost::multi_index::ordered_non_unique<
          boost::multi_index::tag<cdm_pid_index>,
          boost::multi_index::member<
            MetGmCDMVariableProfile, short, &MetGmCDMVariableProfile::p_id_
          >
        >,
        boost::multi_index::hashed_unique<
          boost::multi_index::tag<cdm_name_index>,
          boost::multi_index::member<
            MetGmCDMVariableProfile, std::string, &MetGmCDMVariableProfile::cdmName_
          >
        >,
        boost::multi_index::ordered_non_unique<
          boost::multi_index::tag<cdm_hd_index>,
          boost::multi_index::const_mem_fun<
           MetGmCDMVariableProfile, const short, &MetGmCDMVariableProfile::hd
          >,
          std::greater<short>
        >
      >
    > cdm_configuration;

    typedef cdm_configuration::index<cdm_pid_index>::type       cdmPidView;
    typedef cdm_configuration::index<cdm_name_index>::type      cdmNameView;
}

#endif // METGM_CDMVARIABLEPROFILE_H
