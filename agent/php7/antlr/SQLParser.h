
// Generated from SQL.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime.h"


namespace openrasp {


class  SQLParser : public antlr4::Parser {
public:
  enum {
    Identifier = 1, Number = 2, DOUBLE_QUOTE = 3, SINGLE_QUOTE = 4, StringLiteral = 5, 
    EscapeSequence = 6, BLOCK_COMMENT_START = 7, BLOCK_COMMENT_END = 8, 
    BLOCK_COMMENT = 9, POUND_COMMENT = 10, COMMA = 11, DOT = 12, OR = 13, 
    SYMBOL = 14, WS = 15
  };

  enum {
    RuleInit = 0
  };

  SQLParser(antlr4::TokenStream *input);
  ~SQLParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class InitContext; 

  class  InitContext : public antlr4::ParserRuleContext {
  public:
    InitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  InitContext* init();


private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

}  // namespace openrasp
