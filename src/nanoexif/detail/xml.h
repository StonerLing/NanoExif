// SPDX-License-Identifier: MIT
//
// Minimal XML parser -- a recursive-descent parser that handles a useful
// subset of XML: elements, attributes, text content, CDATA sections,
// character entities, comments, and processing instructions.
//
// This is not a general-purpose XML library; it is tailored to the XMP
// subset commonly found in image metadata.

#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include "nanoexif/errors.h"

namespace nne::detail {

// Represents a single XML attribute (name="value").
struct XmlAttribute {
  std::string name;
  std::string value;
};

// Represents an XML element with its name, text content, attributes, and
// child elements.  Provides convenience lookups for XMP parsing.
struct XmlElement {
  std::string name;
  std::string text;
  std::vector<XmlAttribute> attributes;
  std::vector<XmlElement> children;

  // Finds the first direct child whose name matches exactly.
  [[nodiscard]] const XmlElement* FindChild(const std::string& name) const {
    for (const auto& child : children) {
      if (child.name == name) {
        return &child;
      }
    }
    return nullptr;
  }

  // Returns the value of an attribute by name, or nullptr if not found.
  [[nodiscard]] const std::string* FindAttribute(
      const std::string& name) const {
    for (const auto& attr : attributes) {
      if (attr.name == name) {
        return &attr.value;
      }
    }
    return nullptr;
  }

  // Finds a child by qualified name (e.g. FindChildByQualified("rdf","RDF")
  // matches an element named "rdf:RDF").
  [[nodiscard]] const XmlElement* FindChildByQualified(
      const std::string& ns, const std::string& local) const {
    const std::string qname = ns + ":" + local;
    for (const auto& child : children) {
      if (child.name == qname) {
        return &child;
      }
    }
    return nullptr;
  }
};

// Recursive-descent XML parser.  Not strictly compliant but handles the XML
// subset found in XMP metadata packets (RDF/XML).
class XmlParser {
 public:
  // NOLINTNEXTLINE(modernize-use-default-member-init)
  explicit XmlParser(std::string_view input) : input_(input), pos_(0) {}

  // Parses the entire input, returning a tree of XmlElements rooted at a
  // synthetic "__root__" node.
  [[nodiscard]] Result<XmlElement> Parse() {
    if (!SkipProlog()) {
      return make_result<XmlElement>(error_);
    }

    XmlElement root;
    root.name = "__root__";
    while (pos_ < input_.size()) {
      SkipWhitespace();
      if (pos_ >= input_.size()) {
        break;
      }
      if (input_[pos_] == '<') {
        if (IsAt("<!--")) {
          AdvanceN(4);
          if (!SkipComment()) {
            return make_result<XmlElement>(error_);
          }
          continue;
        }
        if (IsAt("<?")) {
          AdvanceN(2);
          if (!SkipProcessingInstruction()) {
            return make_result<XmlElement>(error_);
          }
          continue;
        }
        XmlElement child;
        if (!ParseElement(child)) {
          return make_result<XmlElement>(error_);
        }
        root.children.push_back(std::move(child));
      } else {
        break;
      }
    }
    return make_result<XmlElement>(std::move(root));
  }

 private:
  std::string_view input_;
  std::size_t pos_ = 0;
  std::size_t line_ = 1;
  std::size_t column_ = 1;
  bool has_error_ = false;
  ErrorCode error_ = {};

  bool Fail(ErrorCode ec) {
    if (!has_error_) {
      has_error_ = true;
      error_ = ec;
    }
    return false;
  }

  bool Ensure(bool cond, ErrorCode ec) {
    if (!cond) {
      return Fail(ec);
    }
    return true;
  }

  [[nodiscard]] char Peek() const { return input_[pos_]; }

  char Advance() {
    const char c = input_[pos_++];
    if (c == '\n') {
      ++line_;
      column_ = 1;
    } else {
      ++column_;
    }
    return c;
  }

  void AdvanceN(std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
      Advance();
    }
  }

  [[nodiscard]] bool IsAt(const std::string& seq) const {
    return pos_ + seq.size() <= input_.size() &&
           input_.substr(pos_, seq.size()) == seq;
  }

  bool Match(char c) {
    if (pos_ < input_.size() && input_[pos_] == c) {
      Advance();
      return true;
    }
    return false;
  }

  bool MatchSequence(const std::string& seq) {
    if (IsAt(seq)) {
      AdvanceN(seq.size());
      return true;
    }
    return false;
  }

  void SkipWhitespace() {
    while (pos_ < input_.size()) {
      const char c = input_[pos_];
      if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
        break;
      }
      Advance();
    }
  }

  bool SkipProlog() {
    SkipWhitespace();
    // Skip UTF-8 BOM (0xEF 0xBB 0xBF)
    if (pos_ + 3 <= input_.size() &&
        static_cast<unsigned char>(input_[pos_]) == 0xEF &&
        static_cast<unsigned char>(input_[pos_ + 1]) == 0xBB &&
        static_cast<unsigned char>(input_[pos_ + 2]) == 0xBF) {
      AdvanceN(3);
    }
    SkipWhitespace();
    while (pos_ < input_.size()) {
      if (IsAt("<?xml")) {
        AdvanceN(5);
        if (!SkipProcessingInstruction()) {
          return false;
        }
        SkipWhitespace();
      } else if (IsAt("<!--")) {
        AdvanceN(4);
        if (!SkipComment()) {
          return false;
        }
        SkipWhitespace();
      } else {
        break;
      }
    }
    return true;
  }

  bool SkipComment() {
    while (pos_ < input_.size()) {
      if (MatchSequence("-->")) {
        return true;
      }
      Advance();
    }
    return Fail(ErrorCode::XML_PARSE_ERROR);
  }

  bool SkipProcessingInstruction() {
    while (pos_ < input_.size()) {
      if (MatchSequence("?>")) {
        return true;
      }
      Advance();
    }
    return Fail(ErrorCode::XML_PARSE_ERROR);
  }

  bool SkipCdata() {
    while (pos_ < input_.size()) {
      if (MatchSequence("]]>")) {
        return true;
      }
      Advance();
    }
    return Fail(ErrorCode::XML_PARSE_ERROR);
  }

  // XML 1.0 NameStartChar and NameChar rules (simplified).
  static bool IsNameStart(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
           c == ':';
  }

  static bool IsNameChar(char c) {
    return IsNameStart(c) || (c >= '0' && c <= '9') || c == '-' || c == '.';
  }

  bool ParseName(std::string& out) {
    if (pos_ >= input_.size() || !IsNameStart(Peek())) {
      return Fail(ErrorCode::XML_PARSE_ERROR);
    }
    out.clear();
    out += Advance();
    while (pos_ < input_.size() && IsNameChar(Peek())) {
      out += Advance();
    }
    return true;
  }

  bool ParseAttributeValue(std::string& out) {
    const char quote = Peek();
    if (quote != '"' && quote != '\'') {
      return Fail(ErrorCode::XML_PARSE_ERROR);
    }
    Advance();
    out.clear();
    while (pos_ < input_.size()) {
      if (input_[pos_] == quote) {
        Advance();
        return true;
      }
      if (input_[pos_] == '&') {
        std::string entity;
        if (!ParseEntity(entity)) {
          return false;
        }
        out += entity;
      } else {
        out += Advance();
      }
    }
    return Fail(ErrorCode::XML_PARSE_ERROR);
  }

  bool ParseEntity(std::string& out) {
    if (!Match('&')) {
      return Fail(ErrorCode::XML_PARSE_ERROR);
    }
    std::string entity;
    while (pos_ < input_.size() && input_[pos_] != ';') {
      entity += Advance();
    }
    if (!Match(';')) {
      return Fail(ErrorCode::XML_PARSE_ERROR);
    }
    if (entity == "amp") {
      out = "&";
    } else if (entity == "lt") {
      out = "<";
    } else if (entity == "gt") {
      out = ">";
    } else if (entity == "quot") {
      out = "\"";
    } else if (entity == "apos") {
      out = "'";
    } else if (!entity.empty() && entity[0] == '#') {
      if (entity.size() > 1 && entity[1] == 'x') {
        char* end = nullptr;
        const long code = std::strtol(entity.c_str() + 2, &end, 16);
        if (end != nullptr && *end == '\0') {
          out = std::string(1, static_cast<char>(code));
        } else {
          out = "&" + entity + ";";
        }
      } else {
        char* end = nullptr;
        const long code = std::strtol(entity.c_str() + 1, &end, 10);
        if (end != nullptr && *end == '\0') {
          out = std::string(1, static_cast<char>(code));
        } else {
          out = "&" + entity + ";";
        }
      }
    } else {
      out = "&" + entity + ";";
    }
    return true;
  }

  bool ParseAttribute(XmlAttribute& out) {
    if (!ParseName(out.name)) {
      return false;
    }
    SkipWhitespace();
    if (!Match('=')) {
      return Fail(ErrorCode::XML_PARSE_ERROR);
    }
    SkipWhitespace();
    if (!ParseAttributeValue(out.value)) {
      return false;
    }
    return true;
  }

  bool ParseAttributes(std::vector<XmlAttribute>& attrs) {
    SkipWhitespace();
    while (pos_ < input_.size() && IsNameStart(Peek())) {
      XmlAttribute attr;
      if (!ParseAttribute(attr)) {
        return false;
      }
      attrs.push_back(std::move(attr));
      SkipWhitespace();
    }
    return true;
  }

  bool ParseTextContent(std::string& out) {
    out.clear();
    while (pos_ < input_.size() && input_[pos_] != '<') {
      if (input_[pos_] == '&') {
        std::string entity;
        if (!ParseEntity(entity)) {
          return false;
        }
        out += entity;
      } else {
        out += Advance();
      }
    }
    return true;
  }

  // Core parsing routine: handles <name ...>content</name> and
  // <name .../> self-closing forms.  Recursively parses children.
  bool ParseElement(XmlElement& out) {
    if (!Match('<')) {
      return Fail(ErrorCode::XML_PARSE_ERROR);
    }
    if (!ParseName(out.name)) {
      return false;
    }
    out.attributes.clear();
    out.children.clear();
    out.text.clear();

    if (!ParseAttributes(out.attributes)) {
      return false;
    }

    if (Match('/')) {
      if (!Match('>')) {
        return Fail(ErrorCode::XML_PARSE_ERROR);
      }
      return true;
    }

    if (!Match('>')) {
      return Fail(ErrorCode::XML_PARSE_ERROR);
    }

    while (pos_ < input_.size()) {
      if (input_[pos_] == '<') {
        if (IsAt("</")) {
          AdvanceN(2);
          std::string end_name;
          if (!ParseName(end_name)) {
            return false;
          }
          SkipWhitespace();
          if (!Match('>')) {
            return Fail(ErrorCode::XML_PARSE_ERROR);
          }
          if (end_name != out.name) {
            return Fail(ErrorCode::XML_PARSE_ERROR);
          }
          return true;
        }
        if (IsAt("<!--")) {
          AdvanceN(4);
          if (!SkipComment()) {
            return false;
          }
          continue;
        }
        if (IsAt("<?")) {
          AdvanceN(2);
          if (!SkipProcessingInstruction()) {
            return false;
          }
          continue;
        }
        if (IsAt("<![CDATA[")) {
          AdvanceN(9);
          const std::size_t start = pos_;
          if (!SkipCdata()) {
            return false;
          }
          out.text += std::string(input_.substr(start, pos_ - start - 3));
          continue;
        }
        XmlElement child;
        if (!ParseElement(child)) {
          return false;
        }
        out.children.push_back(std::move(child));
      } else {
        std::string text;
        if (!ParseTextContent(text)) {
          return false;
        }
        out.text += text;
      }
    }

    return Fail(ErrorCode::XML_PARSE_ERROR);
  }
};

// Convenience entry point for the XML parser.
inline Result<XmlElement> ParseXML(std::string_view input) {
  XmlParser parser(input);
  return parser.Parse();
}

}  // namespace nne::detail
