/******************************************************************************
 * Copyright (c) 2021 Hui Peng.
 * All rights reserved. This program and the accompanying materials are made
 * available under the private copyright of Hui Peng. It is only for studying
 * purpose, usage for other purposes is not allowed.
 *
 * Author:
 *     Hui Peng <peng124@purdue.edu>
 * Date:
 *     2021-02-26
 *****************************************************************************/


#include <string>
#include <bits/stdint-intn.h>


void emit_log(std::string msg) {

}

class WebGLRenderingContextBase {
public:
  WebGLRenderingContextBase();

  virtual bool isContextLost();

  void bufferData(int target, int64_t size, int usage);
  virtual bool validate_target(int target);
  bool validate_usage(int usage);
  bool validate_size(int64_t size);
  void impl(int target, int64_t size, int usage);
};

bool WebGLRenderingContextBase::isContextLost() { return false; }

void WebGLRenderingContextBase::bufferData(int target, int64_t size,
                                           int usage) {
  if (isContextLost()) {
    return;
  }

  impl(target, size, usage);
}

bool WebGLRenderingContextBase::validate_target(int target) {
  int x = 0;
  switch (target) {
  case 1:
    x = 1;
    break;
  case 2:
    break;
  default:
    emit_log("msg1");
    return true;
  }

  if (x == 1) {
    emit_log("msg2");
    return true;
  }

  return false;
}

bool WebGLRenderingContextBase::validate_usage(int usage) {
  switch (usage) {
  case 1:
  case 2:
  case 3:
    return true;
  default:
    emit_log("msg3");
    return false;
  }
}

bool WebGLRenderingContextBase::validate_size(int64_t size) {
  if (size < 0) {
    emit_log("msg4");
    return false;
  }
  return true;
}

void WebGLRenderingContextBase::impl(int target, int64_t size, int usage) {

  if (!validate_target(target))
    return;

  if (!validate_usage(usage))
    return;
  if (!validate_size(size))
    return;
}

class WebGLRenderingContext1 : public WebGLRenderingContextBase {
    public:
        bool validate_target(int target) override;
};

class WebGLRenderingContext2 : public WebGLRenderingContext1 {
    public:
        bool validate_target(int target) override;
};

bool WebGLRenderingContext1::validate_target(int target) {
    return false;
}


bool WebGLRenderingContext2::validate_target(int target) {
    return false;
}
