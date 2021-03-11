#ifndef __SVFPROJECT_H_
#define __SVFPROJECT_H_

#include "SVF-FE/SymbolTableInfo.h"
#include "Util/SVFModule.h"

namespace SVF {

class SVFProject {
  private:
    SVFModule *svfModule;
    SymbolTableInfo *symTblInfo;
};

} // end of namespace SVF

#endif // __SVFPROJECT_H_
