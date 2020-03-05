#ifndef __ANALYZER__
#define __ANALYZER__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Operator.h"
#include "llvm/Analysis/AliasAnalysis.h"

#include <forward_list>

#include "omp.h"

using namespace llvm;
#endif
