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

/// Alpha-quality check for unnecessary smart pointer usage. Not ready to
/// be merged to LLVM yet, since
///
/// - hard-coded for the 4C code base 4C-multiphysics/4C (trivial fix, replace
///   the check for 4C in the file name with some parameter
/// - requires two passes of clang-tidy (with different settings!) to collect
///   all references first and generate the fixit in the second pass. (hard,
///   does not fit into the logic of clang-tidy yet)
///
/// Run this check as:
///   run-clang-tidy -p . -checks='-*,misc-unnecessary-smart-pointer' -config="{CheckOptions: {misc-unnecessary-smart-pointer.DumpDirectory: '$DUMP_DIR'}}"'
/// Then merge the dumped files:
///   python merge_dump_files.py /tmp/4C-dump/
/// Run clang-tidy again with the merged dump file:
///   run-clang-tidy -p . -checks='-*,misc-unnecessary-smart-pointer' -config="{CheckOptions: {misc-unnecessary-smart-pointer.RefFile: '$DUMP_DIR/merged.dump'}}"'
/// 
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
