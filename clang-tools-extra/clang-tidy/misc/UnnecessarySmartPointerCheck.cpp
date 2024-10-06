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

UnnecessarySmartPointerCheck::UnnecessarySmartPointerCheck(
    StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      SmartPointerTypes(utils::options::parseStringList(
          Options.get("SmartPointerTypes", "shared_ptr;unique_ptr"))) {}

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

  auto OverloadedOperator = [&MatchedDecl](StringRef Name) {
    auto RefersToVarDecl =
        has(declRefExpr(to(equalsNode(MatchedDecl))).bind("usage"));

    // operator-> is not obtainable via hasOperatorName("->") so we check for
    // the method name
    return cxxOperatorCallExpr(
        has(declRefExpr(to(cxxMethodDecl(hasName(Name)))).bind(Name)),
        RefersToVarDecl);
  };

  std::vector<BoundNodes> FixableUsages;

  for (const auto &Stmt : Body->children()) {
    const auto &UsagesWithDereference =
        match(traverse(TK_IgnoreUnlessSpelledInSource,
                       findAll(expr(anyOf(OverloadedOperator("operator*"),
                                          OverloadedOperator("operator->"))))),
              *Stmt, *Result.Context);

    const auto &Usages = match(
        traverse(
            TK_IgnoreUnlessSpelledInSource,
            findAll(declRefExpr(to(equalsNode(MatchedDecl))).bind("usage"))),
        *Stmt, *Result.Context);

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
  // If we get here, there is no usage of the smart pointer that is not a
  // dereference, so we can suggest replacing the smart pointer with a value.
  diag(MatchedDecl->getBeginLoc(), "this smart pointer is unnecessary",
       DiagnosticIDs::Warning);

  const Expr *Node = nullptr;
  for (const auto &Deref : FixableUsages) {
    if ((Node = Deref.getNodeAs<DeclRefExpr>("operator*"))) {
      diag(Node->getBeginLoc(), "dereferenced here", DiagnosticIDs::Note)
          << FixItHint::CreateReplacement(Node->getSourceRange(), "");
    } else if ((Node = Deref.getNodeAs<DeclRefExpr>("operator->"))) {
      diag(Node->getBeginLoc(), "dereferenced here", DiagnosticIDs::Note)
          << FixItHint::CreateReplacement(Node->getSourceRange(), ".");
    }
  }
}

void UnnecessarySmartPointerCheck::storeOptions(
    ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "SmartPointerTypes",
                utils::options::serializeStringList(SmartPointerTypes));
}

} // namespace clang::tidy::misc
