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

  const auto *ParentScope = MatchedDecl->getParentFunctionOrMethod();
  if (!ParentScope)
    return;

  // cast to a Decl
  const auto *ParentScopeDecl = dyn_cast<Decl>(ParentScope);
  if (!ParentScopeDecl)
    return;

  const auto *Body = ParentScopeDecl->getBody();
  if (!Body)
    return;

  std::vector<BoundNodes> fixable_usages;

  for (const auto &stmt : Body->children()) {
    auto OverloadedOperatorDereference =
        cxxOperatorCallExpr(
            hasOverloadedOperatorName("*"),
            has(declRefExpr(to(equalsNode(MatchedDecl))).bind("usage")))
            .bind("operator*");

    auto GetFunction =
        cxxMemberCallExpr(
            callee(cxxMethodDecl(hasName("get"))),
            on(declRefExpr(to(equalsNode(MatchedDecl))).bind("usage")))
            .bind("get");

    const auto &dereference_usages =
        match(findAll(expr(anyOf(OverloadedOperatorDereference, GetFunction))),
              *stmt, *Result.Context);

    const auto &all_usages =
        match(findAll(declRefExpr(to(equalsNode(MatchedDecl))).bind("usage")),
              *stmt, *Result.Context);

    if (all_usages.size() > dereference_usages.size()) {
      // There are some usages that do not dereference, so this match is not
      // relevant
      return;
    } else if (all_usages.size() == dereference_usages.size()) {
      // All usages are dereferences, so we can suggest replacing the smart
      // pointer with a value.
      fixable_usages.insert(fixable_usages.end(), dereference_usages.begin(),
                            dereference_usages.end());

    } else {
      // This is an implementation error.
      continue;
    }
  }
  // If we get here, there is no usage of the smart pointer that is not a
  // dereference, so we can suggest replacing the smart pointer with a value.
  diag(MatchedDecl->getBeginLoc(), "this smart pointer is unnecessary",
       DiagnosticIDs::Warning);

  for (const auto &deref : fixable_usages) {
    if (deref.getNodeAs<CXXOperatorCallExpr>("operator*")) {
      diag(deref.getNodeAs<CXXOperatorCallExpr>("operator*")->getBeginLoc(),
           "dereferenced here", DiagnosticIDs::Note)
          << FixItHint::CreateReplacement(
                 deref.getNodeAs<CXXOperatorCallExpr>("operator*")
                     ->getSourceRange(),
                 "");
    } else if (deref.getNodeAs<CXXMemberCallExpr>("get")) {
      diag(deref.getNodeAs<CXXMemberCallExpr>("get")->getBeginLoc(),
           "used as raw pointer here", DiagnosticIDs::Note)
          << FixItHint::CreateReplacement(
                 deref.getNodeAs<CXXMemberCallExpr>("get")->getSourceRange(),
                 "");
    }
  }
}

void UnnecessarySmartPointerCheck::storeOptions(
    ClangTidyOptions::OptionMap &Opts) {
  // TODO load checks
}

} // namespace clang::tidy::misc
