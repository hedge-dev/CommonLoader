//===-- SparcMCTargetDesc.cpp - Sparc Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Sparc specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "SparcMCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "SparcMCAsmInfo.h"
#include "SparcTargetStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "SparcGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SparcGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SparcGenRegisterInfo.inc"

static MCAsmInfo *createSparcMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT) {
  MCAsmInfo *MAI = new SparcELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(SP::O6, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, Reg, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCAsmInfo *createSparcV9MCAsmInfo(const MCRegisterInfo &MRI,
                                         const Triple &TT) {
  MCAsmInfo *MAI = new SparcELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(SP::O6, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, Reg, 2047);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstrInfo *createSparcMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitSparcMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createSparcMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitSparcMCRegisterInfo(X, SP::O7);
  return X;
}

static MCSubtargetInfo *
createSparcMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  if (CPU.empty())
    CPU = (TT.getArch() == Triple::sparcv9) ? "v9" : "v8";
  return createSparcMCSubtargetInfoImpl(TT, CPU, FS);
}

// Code models. Some only make sense for 64-bit code.
//
// SunCC  Reloc   CodeModel  Constraints
// abs32  Static  Small      text+data+bss linked below 2^32 bytes
// abs44  Static  Medium     text+data+bss linked below 2^44 bytes
// abs64  Static  Large      text smaller than 2^31 bytes
// pic13  PIC_    Small      GOT < 2^13 bytes
// pic32  PIC_    Medium     GOT < 2^32 bytes
//
// All code models require that the text segment is smaller than 2GB.

extern "C" void LLVMInitializeSparcTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(TheSparcTarget, createSparcMCAsmInfo);
  RegisterMCAsmInfoFn Y(TheSparcV9Target, createSparcV9MCAsmInfo);
  RegisterMCAsmInfoFn Z(TheSparcelTarget, createSparcMCAsmInfo);

  for (Target *T : {&TheSparcTarget, &TheSparcV9Target, &TheSparcelTarget}) {
    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createSparcMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createSparcMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createSparcMCSubtargetInfo);

    // Register the MC Code Emitter.
    TargetRegistry::RegisterMCCodeEmitter(*T, createSparcMCCodeEmitter);

    // Register the asm backend.
    TargetRegistry::RegisterMCAsmBackend(*T, createSparcAsmBackend);
  }
}
