/*===- ir.c - Simple test of C APIs ---------------------------------------===*\
|*                                                                            *|
|* Part of the LLVM Project, under the Apache License v2.0 with LLVM          *|
|* Exceptions.                                                                *|
|* See https://llvm.org/LICENSE.txt for license information.                  *|
|* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception                    *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

/* RUN: circt-capi-ir-test 2>&1 | FileCheck %s
 */

#include "mlir-c/IR.h"
#include "circt-c/Dialect/HW.h"
#include "circt-c/Dialect/Seq.h"
#include "mlir-c/AffineExpr.h"
#include "mlir-c/AffineMap.h"
#include "mlir-c/BuiltinTypes.h"
#include "mlir-c/Diagnostics.h"
#include "mlir-c/Registration.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__CYGWIN__)
/// The C API is currently not supported on Windows
/// (https://github.com/llvm/circt/issues/578)
int main() { return 0; }
#else
int registerOnlyHW() {
  MlirContext ctx = mlirContextCreate();
  // The built-in dialect is always loaded.
  if (mlirContextGetNumLoadedDialects(ctx) != 1)
    return 1;

  // HW dialect tests.
  MlirDialectHandle hwHandle = mlirGetDialectHandle__hw__();

  MlirDialect hw =
      mlirContextGetOrLoadDialect(ctx, mlirDialectHandleGetNamespace(hwHandle));
  if (!mlirDialectIsNull(hw))
    return 2;

  mlirDialectHandleRegisterDialect(hwHandle, ctx);
  if (mlirContextGetNumRegisteredDialects(ctx) != 1)
    return 3;
  if (mlirContextGetNumLoadedDialects(ctx) != 1)
    return 4;

  hw =
      mlirContextGetOrLoadDialect(ctx, mlirDialectHandleGetNamespace(hwHandle));
  if (mlirDialectIsNull(hw))
    return 5;
  if (mlirContextGetNumLoadedDialects(ctx) != 2)
    return 6;

  MlirDialect alsoRtl = mlirDialectHandleLoadDialect(hwHandle, ctx);
  if (!mlirDialectEqual(hw, alsoRtl))
    return 7;

  // Seq dialect tests.
  MlirDialectHandle seqHandle = mlirGetDialectHandle__seq__();
  mlirDialectHandleRegisterDialect(seqHandle, ctx);
  mlirDialectHandleLoadDialect(seqHandle, ctx);

  MlirDialect seq = mlirContextGetOrLoadDialect(
      ctx, mlirDialectHandleGetNamespace(seqHandle));
  if (mlirDialectIsNull(seq))
    return 8;

  MlirDialect alsoSeq = mlirDialectHandleLoadDialect(seqHandle, ctx);
  if (!mlirDialectEqual(seq, alsoSeq))
    return 9;

  registerSeqPasses();

  mlirContextDestroy(ctx);

  return 0;
}

int testHWTypes() {
  MlirContext ctx = mlirContextCreate();
  MlirDialectHandle hwHandle = mlirGetDialectHandle__hw__();
  mlirDialectHandleRegisterDialect(hwHandle, ctx);
  mlirDialectHandleLoadDialect(hwHandle, ctx);

  MlirType i8type = mlirIntegerTypeGet(ctx, 8);
  MlirType io8type = hwInOutTypeGet(i8type);
  if (mlirTypeIsNull(io8type))
    return 1;

  MlirType elementType = hwInOutTypeGetElementType(io8type);
  if (mlirTypeIsNull(elementType))
    return 2;

  if (hwTypeIsAInOut(i8type))
    return 3;

  if (!hwTypeIsAInOut(io8type))
    return 4;

  mlirContextDestroy(ctx);

  return 0;
}

int main() {
  fprintf(stderr, "@registration\n");
  int errcode = registerOnlyHW();
  fprintf(stderr, "%d\n", errcode);

  fprintf(stderr, "@hwtypes\n");
  errcode = testHWTypes();
  fprintf(stderr, "%d\n", errcode);

  // clang-format off
  // CHECK-LABEL: @registration
  // CHECK: 0
  // CHECK-LABEL: @hwtypes
  // CHECK: 0
  // clang-format on

  return 0;
}
#endif
