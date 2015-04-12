//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2014-03-31 Chongyu Zhu <i@lembacon.com>
//
#include <stdint.h> // for uint32_t
#include <utf8.h>
#include <rime/candidate.h>
#include <rime/common.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/dict/vocabulary.h>
#include <rime/gear/charset_filter.h>

namespace rime {

bool is_extended_cjk(uint32_t ch)
{
  if ((ch >= 0x3400 && ch <= 0x4DBF) ||    // CJK Unified Ideographs Extension A
      (ch >= 0x20000 && ch <= 0x2A6DF) ||  // CJK Unified Ideographs Extension B
      (ch >= 0x2A700 && ch <= 0x2B73F) ||  // CJK Unified Ideographs Extension C
      (ch >= 0x2B740 && ch <= 0x2B81F) ||  // CJK Unified Ideographs Extension D
      (ch >= 0x2B820 && ch <= 0x2CEAF) ||  // CJK Unified Ideographs Extension E
      (ch >= 0x2F800 && ch <= 0x2FA1F))    // CJK Compatibility Ideographs Supplement
    return true;

  return false;
}

bool contains_extended_cjk(const std::string& text)
{
  const char *p = text.c_str();
  uint32_t ch;

  while ((ch = utf8::unchecked::next(p)) != 0) {
    if (is_extended_cjk(ch)) {
      return true;
    }
  }

  return false;
}

// CharsetFilterTranslation

CharsetFilterTranslation::CharsetFilterTranslation(
    shared_ptr<Translation> translation)
    : translation_(translation) {
  LocateNextCandidate();
}

bool CharsetFilterTranslation::Next() {
  if (exhausted())
    return false;
  if (!translation_->Next()) {
    set_exhausted(true);
    return false;
  }
  return LocateNextCandidate();
}

shared_ptr<Candidate> CharsetFilterTranslation::Peek() {
  return translation_->Peek();
}

bool CharsetFilterTranslation::LocateNextCandidate() {
  while (!translation_->exhausted()) {
    auto cand = translation_->Peek();
    if (cand && CharsetFilter::FilterText(cand->text()))
      return true;
    translation_->Next();
  }
  set_exhausted(true);
  return false;
}

// CharsetFilter

bool CharsetFilter::FilterText(const std::string& text) {
  return !contains_extended_cjk(text);
}

bool CharsetFilter::FilterDictEntry(shared_ptr<DictEntry> entry) {
  return entry && FilterText(entry->text);
}

CharsetFilter::CharsetFilter(const Ticket& ticket)
    : Filter(ticket), TagMatching(ticket) {
}

shared_ptr<Translation> CharsetFilter::Apply(
    shared_ptr<Translation> translation, CandidateList* candidates) {
  if (engine_->context()->get_option("extended_charset")) {
    return translation;
  }
  return New<CharsetFilterTranslation>(translation);
}

}  // namespace rime
