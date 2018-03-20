
// Generated from SQL.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime.h"
#include "SQLParser.h"


namespace openrasp {

/**
 * This interface defines an abstract listener for a parse tree produced by SQLParser.
 */
class  SQLListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterInit(SQLParser::InitContext *ctx) = 0;
  virtual void exitInit(SQLParser::InitContext *ctx) = 0;


};

}  // namespace openrasp
