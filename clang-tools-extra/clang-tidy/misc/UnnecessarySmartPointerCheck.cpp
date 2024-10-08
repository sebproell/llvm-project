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
} // namespace

UnnecessarySmartPointerCheck::UnnecessarySmartPointerCheck(
    StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      SmartPointerTypes(utils::options::parseStringList(
          Options.get("SmartPointerTypes", "shared_ptr;unique_ptr"))) {}

void UnnecessarySmartPointerCheck::registerMatchers(MatchFinder *Finder) {
  const auto SmartPointerParmVarDecl =
      parmVarDecl(hasType(cxxRecordDecl(
                      matchers::matchesAnyListedName(SmartPointerTypes))))
          .bind("x");
  Finder->addMatcher(
      functionDecl(hasBody(stmt()), isDefinition(), unless(isImplicit()),
                   unless(cxxMethodDecl(anyOf(isOverride(), isFinal()))),
                   unless(cxxConstructorDecl()),
                   has(typeLoc(forEach(SmartPointerParmVarDecl))),
                   decl().bind("functionDecl")),
      this);
}

void UnnecessarySmartPointerCheck::check(
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

  // If we get here, there is no usage of the smart pointer that is not a
  // dereference, so we can suggest replacing the smart pointer with a value.
  auto Diag = diag(Param.getBeginLoc(), "this smart pointer is unnecessary",
                   DiagnosticIDs::Warning);

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
        SourceRange(CurrentParam.getTypeSpecStartLoc(),
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

void UnnecessarySmartPointerCheck::storeOptions(
    ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "SmartPointerTypes",
                utils::options::serializeStringList(SmartPointerTypes));
}

} // namespace clang::tidy::misc
