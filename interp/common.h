#include <string>
#include <iostream>
#include <stdlib.h>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
//#include "ap_abstract1.h"
#include "ap_manager.h"
#include "box.h"
#include "ap_global1.h"

#include <set>
#include <queue>

#define DEBUG 1

using namespace llvm;
using namespace std;

