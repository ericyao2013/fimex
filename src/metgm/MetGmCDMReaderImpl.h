#ifndef METGM_CDM_READERIMPL_H
#define METGM_CDM_READERIMPL_H

// implementation
//
#include "MetGmCDMVariableProfile.h"
#include "MetGmConfigurationMappings.h"

// fimex
//
#include "fimex/CDMReader.h"
#include "fimex/CDMDimension.h"
#include "fimex/XMLInput.h"

// boost
//
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>

// standard
//
#include <cstdio>
#include <string>
#include <vector>

namespace MetNoFimex {

    class XMLDoc;
    class MetGmVersion;
    class MetGmHandlePtr;
    class MetGmGroup3Ptr;
    class MetGmFileHandlePtr;

    class MetGmCDMReaderImpl : public CDMReader
    {
    public:

        explicit MetGmCDMReaderImpl(const std::string& metgmsource, const XMLInput& configXML, const boost::shared_ptr<CDM>& cdm);
        ~MetGmCDMReaderImpl();

        void init(const XMLInput& configXML);
        bool deinit();

        int getPidForMetgmName(const std::string& metgm_name);
        int getPidForCdmName(const std::string& cdm_name);

        virtual DataPtr getDataSlice(const std::string& varName, size_t unLimDimPos);
        virtual DataPtr getDataSlice(const std::string& varName, const SliceBuilder& sb);

    protected:

        explicit MetGmCDMReaderImpl(const boost::shared_ptr<CDM>& cdm);

        virtual void configure(const boost::shared_ptr<XMLDoc>& doc);
        virtual void parseMgmFile(const std::string& mgmFileName);
        virtual void addVariables();

        void addGlobalCDMAttributes();
        void addTimeDimension();
        void addHorizontalDimensions();
        void addVerticalDimensions();
        void sanityCheck();

        std::string spaceToUnderscore(const std::string& text);

        std::string                            sourceFileName_;
        std::string                            configId_;
        boost::shared_ptr<MetGmHandlePtr>      pHandle_;
        boost::shared_ptr<MetGmGroup1Ptr>      pGroup1_;
        boost::shared_ptr<MetGmGroup2Ptr>      pGroup2_;

        xml_configuration xmlConfiguration_;
        cdm_configuration cdmConfiguration_;

        /**
          * cache widely dimensions
          */
        CDMDimension xDim_;
        CDMDimension yDim_;
        CDMDimension tDim_;
    };

} // end namespace

#endif // METGM_CDM_READERIMPL_H

