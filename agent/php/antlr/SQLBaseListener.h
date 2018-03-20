
// Generated from SQL.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime.h"
#include "SQLListener.h"


namespace openrasp {

/**
 * This class provides an empty implementation of SQLListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  SQLBaseListener : public SQLListener {
public:

  virtual void enterInit(SQLParser::InitContext * /*ctx*/) override { }
  virtual void exitInit(SQLParser::InitContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

}  // namespace openrasp
