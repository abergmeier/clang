//===--- Spir.cpp - Spir ToolChain Implementation -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Spir.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

spir::Linker::Linker(const ToolChain &TC)
  : GnuTool("spir::Linker", "lld", TC) {}

bool spir::Linker::isLinkJob() const {
  return true;
}

bool spir::Linker::hasIntegratedCPP() const {
  return false;
}

void spir::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                const InputInfo &Output,
                                const InputInfoList &Inputs,
                                const ArgList &Args,
                                const char *LinkingOutput) const {

  const ToolChain &ToolChain = getToolChain();
  const char *Linker = Args.MakeArgString(ToolChain.GetLinkerPath());
  ArgStringList CmdArgs;
  CmdArgs.push_back("-flavor");
  CmdArgs.push_back("spir");

  if (Args.hasArg(options::OPT_s))
    CmdArgs.push_back("--strip-all");

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  ToolChain.AddFilePathLibArgs(Args, CmdArgs);

  AddLinkerInputs(ToolChain, Inputs, Args, CmdArgs, JA);

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs)) {
    if (ToolChain.ShouldLinkCXXStdlib(Args))
      ToolChain.AddCXXStdlibLibArgs(Args, CmdArgs);

    // TODO: remove pthread usage
  }

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  C.addCommand(llvm::make_unique<Command>(JA, *this, Linker, CmdArgs, Inputs));
}

Spir::Spir(const Driver &D, const llvm::Triple &Triple,
                         const llvm::opt::ArgList &Args)
  : ToolChain(D, Triple, Args) {

  assert(Triple.isArch32Bit() != Triple.isArch64Bit());

  getProgramPaths().push_back(getDriver().getInstalledDir());

  getFilePaths().push_back(getDriver().SysRoot + "/lib");
}

bool Spir::IsMathErrnoDefault() const { return false; }

bool Spir::IsObjCNonFragileABIDefault() const { return true; }

bool Spir::UseObjCMixedDispatch() const { return true; }

bool Spir::isPICDefault() const { return false; }

bool Spir::isPIEDefault() const { return false; }

bool Spir::isPICDefaultForced() const { return false; }

bool Spir::IsIntegratedAssemblerDefault() const { return true; }

bool Spir::SupportsObjCGC() const { return false; }

bool Spir::hasBlocksRuntime() const { return false; }

// TODO: Support profiling.
bool Spir::SupportsProfiling() const { return false; }

bool Spir::HasNativeLLVMSupport() const { return true; }

void Spir::addClangTargetOptions(const ArgList &DriverArgs,
                                        ArgStringList &CC1Args,
                                        Action::OffloadKind) const {
}

ToolChain::RuntimeLibType Spir::GetDefaultRuntimeLibType() const {
  return ToolChain::RLT_CompilerRT;
}

ToolChain::CXXStdlibType Spir::GetCXXStdlibType(const ArgList &Args) const {
  return ToolChain::CST_Libcxx;
}

void Spir::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                            ArgStringList &CC1Args) const {
  if (!DriverArgs.hasArg(options::OPT_nostdinc))
    addSystemInclude(DriverArgs, CC1Args, getDriver().SysRoot + "/include");
}

void Spir::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                               ArgStringList &CC1Args) const {
  if (!DriverArgs.hasArg(options::OPT_nostdlibinc) &&
      !DriverArgs.hasArg(options::OPT_nostdincxx))
    addSystemInclude(DriverArgs, CC1Args,
                     getDriver().SysRoot + "/include/c++/v1");
}

Tool *Spir::buildLinker() const {
  return new tools::spir::Linker(*this);
}
