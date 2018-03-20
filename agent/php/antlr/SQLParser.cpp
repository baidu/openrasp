
// Generated from SQL.g4 by ANTLR 4.7.1


#include "SQLListener.h"

#include "SQLParser.h"


using namespace antlrcpp;
using namespace openrasp;
using namespace antlr4;

SQLParser::SQLParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

SQLParser::~SQLParser() {
  delete _interpreter;
}

std::string SQLParser::getGrammarFileName() const {
  return "SQL.g4";
}

const std::vector<std::string>& SQLParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& SQLParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- InitContext ------------------------------------------------------------------

SQLParser::InitContext::InitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SQLParser::InitContext::getRuleIndex() const {
  return SQLParser::RuleInit;
}

void SQLParser::InitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SQLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInit(this);
}

void SQLParser::InitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SQLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInit(this);
}

SQLParser::InitContext* SQLParser::init() {
  InitContext *_localctx = _tracker.createInstance<InitContext>(_ctx, getState());
  enterRule(_localctx, 0, SQLParser::RuleInit);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);

   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

// Static vars and initialization.
std::vector<dfa::DFA> SQLParser::_decisionToDFA;
atn::PredictionContextCache SQLParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN SQLParser::_atn;
std::vector<uint16_t> SQLParser::_serializedATN;

std::vector<std::string> SQLParser::_ruleNames = {
  "init"
};

std::vector<std::string> SQLParser::_literalNames = {
  "", "", "", "'\"'", "'''", "", "", "", "'*/'", "", "", "','", "'.'", "'||'"
};

std::vector<std::string> SQLParser::_symbolicNames = {
  "", "Identifier", "Number", "DOUBLE_QUOTE", "SINGLE_QUOTE", "StringLiteral", 
  "EscapeSequence", "BLOCK_COMMENT_START", "BLOCK_COMMENT_END", "BLOCK_COMMENT", 
  "POUND_COMMENT", "COMMA", "DOT", "OR", "SYMBOL", "WS"
};

dfa::Vocabulary SQLParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> SQLParser::_tokenNames;

SQLParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0x11, 0x7, 0x4, 0x2, 0x9, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x2, 
    0x2, 0x3, 0x2, 0x2, 0x2, 0x2, 0x5, 0x2, 0x4, 0x3, 0x2, 0x2, 0x2, 0x4, 
    0x5, 0x3, 0x2, 0x2, 0x2, 0x5, 0x3, 0x3, 0x2, 0x2, 0x2, 0x2, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

SQLParser::Initializer SQLParser::_init;
