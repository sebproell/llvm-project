//===--- UnnecessarySmartPointerCheck.h - clang-tidy ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_UNNECESSARYSMARTPOINTERCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_UNNECESSARYSMARTPOINTERCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::misc {

/// FIXME: Write a short description.
///
/// TODO: double const when replacing const ref
/// TODO: check for default arguments that would require a ptr
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/misc/unnecessary-smart-pointer.html
class UnnecessarySmartPointerCheck : public ClangTidyCheck {
public:
  UnnecessarySmartPointerCheck(StringRef Name, ClangTidyContext *Context);

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override {
    return LangOpts.CPlusPlus;
  }
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;

private:
  void checkFirstPass(const ast_matchers::MatchFinder::MatchResult &Result);
  void checkSecondPass(const ast_matchers::MatchFinder::MatchResult &Result);

  void checkVarDecl(const ast_matchers::MatchFinder::MatchResult &Result);
  void checkFunctionDecl(const ast_matchers::MatchFinder::MatchResult &Result);

  const std::vector<StringRef> SmartPointerTypes;
  const std::vector<StringRef> SmartPointerFactories;
  const StringRef DumpDirectory;
  const StringRef RefFile;
  std::optional<llvm::raw_fd_stream> Stream;
  std::unordered_map<std::string, std::vector<int>> FunctionWithArityToIndex;
};

} // namespace clang::tidy::misc

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_UNNECESSARYSMARTPOINTERCHECK_H
