/******************************************************************************
 * Copyright (c) 2021 Hui Peng.
 * All rights reserved. This program and the accompanying materials are made
 * available under the private copyright of Hui Peng. It is only for studying
 * purpose, usage for other purposes is not allowed.
 *
 * Author:
 *     Hui Peng <peng124@purdue.edu>
 * Date:
 *     2021-02-25
 *****************************************************************************/


#ifndef MY_INST_VISITOR_H
#define MY_INST_VISITOR_H

#include "Util/BasicTypes.h"

class MyInstVisitor:
  public llvm::InstVisitor<MyInstVisitor> {

  private:
    std::set<const llvm::Value *> &values;
  public:
    MyInstVisitor(std::set<const llvm::Value *> &values):
      values(values) {
    }

    void addValue(const llvm::Value *v) {
      values.insert(v);
    }

    std::set<const llvm::Value *> &
    getResults() {
      return values;
    }
};

#endif /* MY_INST_VISITOR_H */
