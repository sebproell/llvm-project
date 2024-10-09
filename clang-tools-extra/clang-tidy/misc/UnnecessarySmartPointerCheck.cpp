//===--- UnnecessarySmartPointerCheck.cpp - clang-tidy --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UnnecessarySmartPointerCheck.h"
#include "../utils/Matchers.h"
#include "../utils/OptionsUtils.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <fstream>
#include <random>

using namespace clang::ast_matchers;

namespace clang::tidy::misc {

namespace {
bool isReferencedOutsideOfCallExpr(const FunctionDecl &Function,
                                   ASTContext &Context) {
  auto Matches = match(declRefExpr(to(functionDecl(equalsNode(&Function))),
                                   unless(hasAncestor(callExpr()))),
                       Context);
  return !Matches.empty();
}

std::string generateRandomFileName(const std::string &extension = ".json") {
  const char charset[] =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::random_device Rng;
  std::uniform_int_distribution<> Dist(0, sizeof(charset) -
                                              2); // -1 to avoid null terminator

  std::string RandomString;
  for (size_t i = 0; i < 16; ++i) {
    RandomString += charset[Dist(Rng)];
  }

  return RandomString + extension; // Append extension
}

auto makeMatcher(std::string FunctionName,
                 const llvm::SmallVector<llvm::StringRef> &Params,
                 const llvm::SmallVector<int> &Indices) {

  const auto CheckParmDecl = [&](int index) {
    return hasParameter(index, hasType(asString(Params[index].str())));
  };

  const auto MaybeBindArg = [&](int index) {
    // If we want to modify element at index, we bind here
    if (llvm::find(Indices, index) != Indices.end()) {
      return hasArgument(index, expr().bind("arg" + std::to_string(index)));
    }
    // Otherwise we return a non-binding matcher
    return hasArgument(index, expr());
  };

  switch (Params.size()) {
  case 1:
    return callExpr(
               callee(functionDecl(hasName(FunctionName), CheckParmDecl(0))),
               MaybeBindArg(0))
        .bind("call");
  case 2:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1))),
                    MaybeBindArg(0), MaybeBindArg(1))
        .bind("call");
  case 3:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1), CheckParmDecl(2))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2))
        .bind("call");
  case 4:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1), CheckParmDecl(2),
                                        CheckParmDecl(3))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3))
        .bind("call");
  case 5:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1), CheckParmDecl(2),
                                        CheckParmDecl(3), CheckParmDecl(4))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4))
        .bind("call");
  case 6:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1), CheckParmDecl(2),
                                        CheckParmDecl(3), CheckParmDecl(4),
                                        CheckParmDecl(5))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5))
        .bind("call");
  case 7:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1), CheckParmDecl(2),
                                        CheckParmDecl(3), CheckParmDecl(4),
                                        CheckParmDecl(5), CheckParmDecl(6))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5),
                    MaybeBindArg(6))
        .bind("call");
  case 8:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1), CheckParmDecl(2),
                                        CheckParmDecl(3), CheckParmDecl(4),
                                        CheckParmDecl(5), CheckParmDecl(6),
                                        CheckParmDecl(7))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5),
                    MaybeBindArg(6), MaybeBindArg(7))
        .bind("call");
  case 9:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1), CheckParmDecl(2),
                                        CheckParmDecl(3), CheckParmDecl(4),
                                        CheckParmDecl(5), CheckParmDecl(6),
                                        CheckParmDecl(7), CheckParmDecl(8))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5),
                    MaybeBindArg(6), MaybeBindArg(7), MaybeBindArg(8))
        .bind("call");
  case 10:
    return callExpr(callee(functionDecl(
                        hasName(FunctionName), CheckParmDecl(0),
                        CheckParmDecl(1), CheckParmDecl(2), CheckParmDecl(3),
                        CheckParmDecl(4), CheckParmDecl(5), CheckParmDecl(6),
                        CheckParmDecl(7), CheckParmDecl(8), CheckParmDecl(9))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5),
                    MaybeBindArg(6), MaybeBindArg(7), MaybeBindArg(8),
                    MaybeBindArg(9))
        .bind("call");
  case 11:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1), CheckParmDecl(2),
                                        CheckParmDecl(3), CheckParmDecl(4),
                                        CheckParmDecl(5), CheckParmDecl(6),
                                        CheckParmDecl(7), CheckParmDecl(8),
                                        CheckParmDecl(9), CheckParmDecl(10))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5),
                    MaybeBindArg(6), MaybeBindArg(7), MaybeBindArg(8),
                    MaybeBindArg(9), MaybeBindArg(10))
        .bind("call");
  case 12:
    return callExpr(callee(functionDecl(
                        hasName(FunctionName), CheckParmDecl(0),
                        CheckParmDecl(1), CheckParmDecl(2), CheckParmDecl(3),
                        CheckParmDecl(4), CheckParmDecl(5), CheckParmDecl(6),
                        CheckParmDecl(7), CheckParmDecl(8), CheckParmDecl(9),
                        CheckParmDecl(10), CheckParmDecl(11))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5),
                    MaybeBindArg(6), MaybeBindArg(7), MaybeBindArg(8),
                    MaybeBindArg(9), MaybeBindArg(10), MaybeBindArg(11))
        .bind("call");
  case 13:
    return callExpr(callee(functionDecl(hasName(FunctionName), CheckParmDecl(0),
                                        CheckParmDecl(1), CheckParmDecl(2),
                                        CheckParmDecl(3), CheckParmDecl(4),
                                        CheckParmDecl(5), CheckParmDecl(6),
                                        CheckParmDecl(7), CheckParmDecl(8),
                                        CheckParmDecl(9), CheckParmDecl(10),
                                        CheckParmDecl(11), CheckParmDecl(12))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5),
                    MaybeBindArg(6), MaybeBindArg(7), MaybeBindArg(8),
                    MaybeBindArg(9), MaybeBindArg(10), MaybeBindArg(11),
                    MaybeBindArg(12))
        .bind("call");
  case 14:
    return callExpr(
               callee(functionDecl(
                   hasName(FunctionName), CheckParmDecl(0), CheckParmDecl(1),
                   CheckParmDecl(2), CheckParmDecl(3), CheckParmDecl(4),
                   CheckParmDecl(5), CheckParmDecl(6), CheckParmDecl(7),
                   CheckParmDecl(8), CheckParmDecl(9), CheckParmDecl(10),
                   CheckParmDecl(11), CheckParmDecl(12), CheckParmDecl(13))),
               MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
               MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5),
               MaybeBindArg(6), MaybeBindArg(7), MaybeBindArg(8),
               MaybeBindArg(9), MaybeBindArg(10), MaybeBindArg(11),
               MaybeBindArg(12), MaybeBindArg(13))
        .bind("call");
  case 15:
    return callExpr(callee(functionDecl(
                        hasName(FunctionName), CheckParmDecl(0),
                        CheckParmDecl(1), CheckParmDecl(2), CheckParmDecl(3),
                        CheckParmDecl(4), CheckParmDecl(5), CheckParmDecl(6),
                        CheckParmDecl(7), CheckParmDecl(8), CheckParmDecl(9),
                        CheckParmDecl(10), CheckParmDecl(11), CheckParmDecl(12),
                        CheckParmDecl(13), CheckParmDecl(14))),
                    MaybeBindArg(0), MaybeBindArg(1), MaybeBindArg(2),
                    MaybeBindArg(3), MaybeBindArg(4), MaybeBindArg(5),
                    MaybeBindArg(6), MaybeBindArg(7), MaybeBindArg(8),
                    MaybeBindArg(9), MaybeBindArg(10), MaybeBindArg(11),
                    MaybeBindArg(12), MaybeBindArg(13), MaybeBindArg(14))
        .bind("call");
  default:
    llvm::errs() << "Unsupported number of parameters: " << Params.size()
                 << "\n";
    return callExpr(callee(functionDecl(hasName(FunctionName))),
                    argumentCountIs(Params.size()))
        .bind("call");
  }
}
} // namespace

UnnecessarySmartPointerCheck::UnnecessarySmartPointerCheck(
    StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      SmartPointerTypes(utils::options::parseStringList(
          Options.get("SmartPointerTypes", "shared_ptr;unique_ptr"))),
      DumpDirectory(Options.get("DumpDirectory", "")),
      RefFile(Options.get("RefFile", "")) {
  if (!DumpDirectory.empty()) {
    std::string file_name =
        (DumpDirectory + "/" + generateRandomFileName(".dump")).str();
    std::error_code errc;
    Stream.emplace((StringRef)file_name, errc);
  }
}

void UnnecessarySmartPointerCheck::registerMatchers(MatchFinder *Finder) {

  if (!RefFile.empty()) {
    std::ifstream File(RefFile.str());
    std::string Line;
    while (std::getline(File, Line)) {
      llvm::SmallVector<StringRef> Parts;
      llvm::StringRef(Line).split(Parts, "@", -1, false);
      if (Parts.size() != 2) {
        llvm::errs() << "Invalid line in reference file: " << Line << "\n";
        continue;
      }

      llvm::SmallVector<StringRef> FunctionArgs;
      llvm::StringRef(Parts[1]).split(FunctionArgs, "|", -1, false);

      const auto FunctionName = FunctionArgs[0].str();
      llvm::SmallVector<llvm::StringRef> Params(FunctionArgs.begin() + 1,
                                                FunctionArgs.end());

      llvm::SmallVector<int> Indices;
      {
        llvm::SmallVector<StringRef> IndicesTemp;
        Parts[0].split(IndicesTemp, ",", -1, false);
        for (const auto &Index : IndicesTemp) {
          Indices.push_back(std::stoi(Index.str()));
        }
      }

      Finder->addMatcher(makeMatcher(FunctionName, Params, Indices), this);
    }
  } else {
    // Check if smart pointer or reference to smart pointer
    auto SmartPointerParmVarDecl =
        parmVarDecl(anyOf(hasType(references(
                              recordDecl(hasAnyName(SmartPointerTypes)))),
                          hasType(recordDecl(hasAnyName(SmartPointerTypes)))))
            .bind("x");

    Finder->addMatcher(
        functionDecl(hasBody(stmt()), isDefinition(), unless(isImplicit()),
                     unless(cxxMethodDecl(anyOf(isOverride(), isFinal()))),
                     unless(cxxConstructorDecl()),
                     unless(cxxMethodDecl(ofClass(isLambda()))),
                     has(typeLoc(forEach(SmartPointerParmVarDecl))),
                     decl().bind("functionDecl")),
        this);
  }
}

void UnnecessarySmartPointerCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (RefFile.empty()) {
    checkFirstPass(Result);
  } else {
    checkSecondPass(Result);
  }
}

void UnnecessarySmartPointerCheck::checkFirstPass(
    const MatchFinder::MatchResult &Result) {
  const auto &Param = *Result.Nodes.getNodeAs<ParmVarDecl>("x");

  const auto *ParentScope = Param.getParentFunctionOrMethod();
  if (!ParentScope)
    return;

  // cast to a Decl
  const auto *ParentScopeDecl = dyn_cast<Decl>(ParentScope);
  if (!ParentScopeDecl)
    return;

  const auto *Body = ParentScopeDecl->getBody();
  if (!Body)
    return;

  auto RefersToVarDecl = (declRefExpr(to(equalsNode(&Param))).bind("usage"));

  auto OverloadedOperator = [&](StringRef Name) {
    // operator-> is not obtainable via hasOperatorName("->") so we check for
    // the method name
    return cxxOperatorCallExpr(
        has(declRefExpr(to(cxxMethodDecl(hasName(Name)))).bind(Name)),
        has(RefersToVarDecl));
  };

  std::vector<BoundNodes> FixableUsages;

  for (const auto &Stmt : Body->children()) {
    const auto &UsagesWithDereference =
        match(traverse(TK_IgnoreUnlessSpelledInSource,
                       findAll(expr(anyOf(OverloadedOperator("operator*"),
                                          OverloadedOperator("operator->"))))),
              *Stmt, *Result.Context);

    const auto &Usages =
        match(findAll(RefersToVarDecl), *Stmt, *Result.Context);

    if (Usages.size() > UsagesWithDereference.size()) {
      // There are some usages that do not dereference, so this match is not
      // relevant

      return;
    }
    if (Usages.size() == UsagesWithDereference.size()) {
      // All usages are dereferences, so we can suggest replacing the smart
      // pointer with a value.
      FixableUsages.insert(FixableUsages.end(), UsagesWithDereference.begin(),
                           UsagesWithDereference.end());
    }
  }

  const auto &Function = *Result.Nodes.getNodeAs<FunctionDecl>("functionDecl");

  const size_t Index =
      llvm::find(Function.parameters(), &Param) - Function.parameters().begin();

  // Do not propose fixes when:
  // 1. the ParmVarDecl is in a macro, since we cannot place them correctly
  // 2. the function is virtual as it might break overrides
  // 3. the function is referenced outside of a call expression within the
  //    compilation unit as the signature change could introduce build errors.
  // 4. the function is an explicit template/ specialization.
  const auto *Method = llvm::dyn_cast<CXXMethodDecl>(&Function);
  if (Param.getBeginLoc().isMacroID() || (Method && Method->isVirtual()) ||
      isReferencedOutsideOfCallExpr(Function, *Result.Context) ||
      Function.getTemplateSpecializationKind() == TSK_ExplicitSpecialization)
    return;

  // Ignore functions that are not defined in a source file that contains 4C
  // in its path
  if (!Function.getLocation().isValid() ||
      !Result.SourceManager->getFilename(Function.getLocation()).contains("4C"))
    return;

  // If we get here, there is no usage of the smart pointer that is not a
  // dereference, so we can suggest replacing the smart pointer with a value.
  auto Diag = diag(Param.getBeginLoc(), "this smart pointer is unnecessary",
                   DiagnosticIDs::Warning);

  // Dump the function signature to a file if a dump directory is provided
  if (Stream) {
    // First index. Then write the fully qualified function name to the dump
    // file including all parameters with their fully qualified type.
    *Stream << Index << "@";
    *Stream << Function.getQualifiedNameAsString() << "|";
    for (const auto *FParam : Function.parameters()) {
      *Stream << FParam->getType().getAsString() << "|";
    }
    *Stream << "\n";
  }

  // Replace parameter in all declarations
  for (const auto *FunctionDecl = &Function; FunctionDecl != nullptr;
       FunctionDecl = FunctionDecl->getPreviousDecl()) {
    const auto &CurrentParam = *FunctionDecl->getParamDecl(Index);
    // Strip the outermost template argument to get the type of the smart
    // pointer (e.g. shared_ptr<int> -> int)
    auto SmartPointerType = CurrentParam.getType().getAsString();
    auto InnerType = SmartPointerType.substr(SmartPointerType.find('<') + 1);
    InnerType = InnerType.substr(0, InnerType.rfind('>')) + "&";
    Diag << FixItHint::CreateReplacement(
        SourceRange(CurrentParam.getSourceRange().getBegin(),
                    CurrentParam.getTypeSpecEndLoc()),
        InnerType);
  }

  // Replace all dereferences
  const Expr *Node = nullptr;
  for (const auto &Deref : FixableUsages) {
    if ((Node = Deref.getNodeAs<DeclRefExpr>("operator*"))) {
      diag(Node->getBeginLoc(), "dereferenced here", DiagnosticIDs::Note);
      Diag << FixItHint::CreateReplacement(Node->getSourceRange(), "");
    } else if ((Node = Deref.getNodeAs<DeclRefExpr>("operator->"))) {
      diag(Node->getBeginLoc(), "dereferenced here", DiagnosticIDs::Note);
      Diag << FixItHint::CreateReplacement(Node->getSourceRange(), ".");
    }
  }
}

void UnnecessarySmartPointerCheck::checkSecondPass(
    const MatchFinder::MatchResult &Result) {
  const auto &Call = *Result.Nodes.getNodeAs<CallExpr>("call");
  auto Diag = diag(Call.getBeginLoc(), "Found function match on second pass");

  for (unsigned i = 0; i < Call.getNumArgs(); i++) {
    if (auto *ArgExpr =
            Result.Nodes.getNodeAs<Expr>("arg" + std::to_string(i))) {
      Diag << FixItHint::CreateInsertion(ArgExpr->getBeginLoc(), "*");
    }
  }
}

void UnnecessarySmartPointerCheck::storeOptions(
    ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "SmartPointerTypes",
                utils::options::serializeStringList(SmartPointerTypes));
}

} // namespace clang::tidy::misc
