
// Generated from SQL.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime.h"


namespace openrasp {


class  SQLLexer : public antlr4::Lexer {
public:
  enum {
    Identifier = 1, Number = 2, DOUBLE_QUOTE = 3, SINGLE_QUOTE = 4, StringLiteral = 5, 
    EscapeSequence = 6, BLOCK_COMMENT_START = 7, BLOCK_COMMENT_END = 8, 
    BLOCK_COMMENT = 9, POUND_COMMENT = 10, COMMA = 11, DOT = 12, OR = 13, 
    SYMBOL = 14, WS = 15
  };

  SQLLexer(antlr4::CharStream *input);
  ~SQLLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

}  // namespace openrasp
