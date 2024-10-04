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

void UnnecessarySmartPointerCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      varDecl(hasType(cxxRecordDecl(
                  matchers::matchesAnyListedName(SmartPointerTypes))))
          .bind("x"),
      this);
}

void UnnecessarySmartPointerCheck::check(
    const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  const auto *MatchedDecl = Result.Nodes.getNodeAs<VarDecl>("x");

  // Get the parent scope if available.
  // Traverse upwards until we find a CompoundStmt.
  const auto *ParentScope = MatchedDecl->getParentFunctionOrMethod();
  if (!ParentScope)
    return;

  diag(MatchedDecl->getBeginLoc(), "looking at this parent_scope");

  // cast to a Decl
  const auto *ParentScopeDecl = dyn_cast<Decl>(ParentScope);
  if (!ParentScopeDecl)
    return;

  diag(ParentScopeDecl->getBeginLoc(), "looking at this parent_scope",
       DiagnosticIDs::Note);

  const auto *Body = ParentScopeDecl->getBody();
  if (!Body)
    return;

  diag(Body->getBeginLoc(), "looking at this body", DiagnosticIDs::Note);

  for (const auto &stmt : Body->children()) {
    if (stmt == (const Stmt *)MatchedDecl)
      continue;
    diag(stmt->getBeginLoc(), "looking at this stmt", DiagnosticIDs::Note);
    const auto &usages =
        match(findAll(declRefExpr(to(equalsNode(MatchedDecl))).bind("usage")),
              *stmt, *Result.Context);
    for (const auto &usage : usages) {
      diag(usage.getNodeAs<DeclRefExpr>("usage")->getLocation(), "used here");
    }
  }
}

void UnnecessarySmartPointerCheck::storeOptions(
    ClangTidyOptions::OptionMap &Opts) {
  // TODO load checks
}

} // namespace clang::tidy::misc
