//===--- MakeCustomSmartPtrCheck.h - clang-tidy -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MODERNIZE_MAKECUSTOMSMARTPTRCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MODERNIZE_MAKECUSTOMSMARTPTRCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::modernize {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/modernize/make-custom-smart-ptr.html
class MakeCustomSmartPtrCheck : public ClangTidyCheck {
public:
  MakeCustomSmartPtrCheck(StringRef Name, ClangTidyContext *Context);

  void registerMatchers(ast_matchers::MatchFinder *Finder) final;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) final;
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;

protected:
  /// Returns whether the C++ version is compatible with current check.
  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override;

  static const char PointerType[];

private:
  const StringRef MakeSmartPtrFunctionHeader;
  const StringRef MakeSmartPtrFunctionName;
  const bool IgnoreMacros;
  const bool IgnoreDefaultInitialization;

  void checkConstruct(SourceManager &SM, ASTContext *Ctx,
                      const CXXConstructExpr *Construct, const QualType *Type,
                      const CXXNewExpr *New);
  void checkReset(SourceManager &SM, ASTContext *Ctx,
                  const CXXMemberCallExpr *Reset, const CXXNewExpr *New);

  /// Returns true when the fixes for replacing CXXNewExpr are generated.
  bool replaceNew(DiagnosticBuilder &Diag, const CXXNewExpr *New,
                  SourceManager &SM, ASTContext *Ctx);
};

} // namespace clang::tidy::modernize

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MODERNIZE_MAKECUSTOMSMARTPTRCHECK_H
