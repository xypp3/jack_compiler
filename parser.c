#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

typedef enum { false, true } Boolean;

typedef struct {
  Boolean wasConsumed;
  ParserInfo info;
} ParserWrapper;

typedef struct {
  unsigned int length;
  TokenType *set; // array
} TokenTypeSet;

// all return an EMPTY TOKEN upon success
// top level grammer
char *emtpyStart[] = {"\0"};
ParserWrapper classDeclar();
ParserWrapper memberDeclar();
ParserWrapper classVarDeclar();
char *classVarDeclarStart[] = {"static", "field", "\0"};
ParserWrapper type();
char *typeStart[] = {"int", "char", "boolean", "\0"};
ParserWrapper subroutineDeclar();
char *subroutineDeclarStart[] = {"constructor", "function", "method", "\0"};
ParserWrapper paramList();
ParserWrapper subroutineBody();

// statements
ParserWrapper stmt();
char *stmtStart[] = {"var", "let", "if", "while", "do", "return", "\0"};
ParserWrapper varStmt();
ParserWrapper letStmt();
ParserWrapper ifStmt();
ParserWrapper whileStmt();
ParserWrapper doStmt();
ParserWrapper subroutineCall();
ParserWrapper exprList();
ParserWrapper returnStmt();
// expressions
ParserWrapper expr();
ParserWrapper relationalExpr();
ParserWrapper arithmeticExpr();
ParserWrapper term();
ParserWrapper factor();
ParserWrapper operand();
// function stubs above

/**********************************************************************
 **********************************************************************
 **********************************************************************
 ************************* Helper functions ***************************
 **********************************************************************
 **********************************************************************
 **********************************************************************
 */

Boolean strcmpList(char *word, char **acceptCases) {
  int pos = 0;

  // if empty acceptCases return true (for accepting ID tokens)
  if (0 == strcmp(acceptCases[0], "\0"))
    return true;

  // else test acceptCases
  while (strcmp(acceptCases[pos], "\0") != 0) {
    if (0 == strncmp(word, acceptCases[pos], 128))
      return true;

    pos++;
  }

  return false;
}

/**********************************************************************
 **********************************************************************
 **********************************************************************
 ************************* Grammer functions **************************
 **********************************************************************
 **********************************************************************
 **********************************************************************
 */

ParserWrapper classDeclar() {
  ParserWrapper info;
  Token token;

  // class
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (SYMBOL == token.tp && strcmpList(token.lx, classVarDeclarStart)) {
  } else {
    return (ParserWrapper){false, (ParserInfo){classExpected, token}};
  }

  // ID
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (ID == token.tp) {
  } else {
    return (ParserWrapper){false, (ParserInfo){idExpected, token}};
  }

  // '{'
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"{", "\0"})) {
  } else {
    return (ParserWrapper){false, (ParserInfo){openBraceExpected, token}};
  }

  // { memberDeclar }
  while (true) {

    token = PeekNextToken();
    if (ERR == token.tp)
      return (ParserWrapper){false, (ParserInfo){lexerErr, token}};

    // break case
    if (RESWORD != token.tp && !strcmpList(token.lx, classVarDeclarStart) &&
        !strcmpList(token.lx, subroutineDeclarStart))
      break;

    // class var declar
    if (strcmpList(token.lx, classVarDeclarStart)) {
      info = classVarDeclar();

      if (info.info.er != none)
        return info;
    }

    // subroutineDeclar
    if (strcmpList(token.lx, subroutineDeclarStart)) {
      info = subroutineDeclar();

      if (info.info.er != none)
        return info;
    }
  }

  // '}'
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"}", "\0"})) {
  } else {
    return (ParserWrapper){false, (ParserInfo){closeBraceExpected, token}};
  }

  return (ParserWrapper){true, (ParserInfo){none, token}};
}

ParserWrapper classVarDeclar() {
  ParserWrapper info;
  Token token;

  // 'static' | 'field'
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (RESWORD == token.tp && strcmpList(token.lx, classVarDeclarStart)) {
  } else {
    return (ParserWrapper){false, (ParserInfo){classVarErr, token}};
  }

  // type()
  token = PeekNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (ID == token.tp ||
      (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
    type();
    // should i check even though is already checked above??
  } else {
    return (ParserWrapper){false, (ParserInfo){illegalType, token}};
  }

  // identifier
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (ID == token.tp) {
  } else {
    return (ParserWrapper){false, (ParserInfo){idExpected, token}};
  }

  // {, identifier}
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      return (ParserWrapper){false, (ParserInfo){lexerErr, token}};

    // break case
    if (SYMBOL != token.tp && !strcmpList(token.lx, (char *[]){",", "\0"})) {
      break;
    }

    token = GetNextToken(); // get ','
    token = GetNextToken(); // get ID, hopefully
    if (ERR == token.tp)
      return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
    if (ID == token.tp) {
    } else {
      return (ParserWrapper){false, (ParserInfo){idExpected, token}};
    }

    // (to stretch whitespace in formatter)
  }

  // ';'
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){";", "\0"})) {
  } else {
    return (ParserWrapper){false, (ParserInfo){semicolonExpected, token}};
  }

  return (ParserWrapper){true, (ParserInfo){none, token}};
}

ParserWrapper type() {
  Token token;

  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (ID == token.tp ||
      (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
  } else {
    return (ParserWrapper){false, (ParserInfo){illegalType, token}};
  }

  return (ParserWrapper){true, (ParserInfo){none, token}};
}

ParserWrapper subroutineDeclar() {
  Token token;

  // 'constructor' | 'function' | 'method'
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (RESWORD == token.tp && strcmpList(token.lx, subroutineDeclarStart)) {
  } else {
    return (ParserWrapper){false, (ParserInfo){subroutineDeclarErr, token}};
  }

  // type() | 'void'
  token = PeekNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (ID == token.tp ||
      (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
    type();
  } else {
    // 'void'
    token = GetNextToken(); // already checked if type == err, above
    if (RESWORD == token.tp && strcmpList(token.lx, (char *[]){"void", "\0"})) {
    } else {
      return (ParserWrapper){false, (ParserInfo){syntaxError, token}};
    }
  }

  // identifier
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (ID == token.tp) {
  } else {
    return (ParserWrapper){false, (ParserInfo){idExpected, token}};
  }

  // '('
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"(", "\0"})) {
  } else {
    return (ParserWrapper){false, (ParserInfo){openParenExpected, token}};
  }

  // paramList()
  token = PeekNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};

  // if paramList() is NOT empty
  if (SYMBOL != token.tp && !strcmpList(token.lx, (char *[]){")", "\0"})) {

    if (ID == token.tp ||
        (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {

      ParserWrapper info = paramList();

      if (!info.wasConsumed && info.info.er != none)
        return info;

    } else {
      return (ParserWrapper){false, (ParserInfo){syntaxError, token}};
    }
  }

  // ')'
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){")", "\0"})) {
  } else {
    return (ParserWrapper){false, (ParserInfo){closeParenExpected, token}};
  }

  // subroutineBody()
  token = PeekNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"{", "\0"})) {

    ParserWrapper info = subroutineBody();

    if (!info.wasConsumed && info.info.er != none)
      return info;

  } else {
    return (ParserWrapper){false, (ParserInfo){openBraceExpected, token}};
  }

  return (ParserWrapper){true, (ParserInfo){none, token}};
}

ParserWrapper paramList() {
  Token token;

  // type()
  token = PeekNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (ID == token.tp ||
      (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
    type();
    // should i check even though is already checked above??
  } else {
    return (ParserWrapper){false, (ParserInfo){illegalType, token}};
  }

  // identifier
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (ID == token.tp) {
  } else {
    return (ParserWrapper){false, (ParserInfo){idExpected, token}};
  }

  // {',' type() identifier}
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      return (ParserWrapper){false, (ParserInfo){lexerErr, token}};

    // break case
    if (SYMBOL != token.tp && !strcmpList(token.lx, (char *[]){",", "\0"})) {
      break;
    }

    token = GetNextToken(); // get ','
    // type()
    token = PeekNextToken();
    if (ERR == token.tp)
      return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
    if (ID == token.tp ||
        (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
      type();
      // should i check even though is already checked above??
    } else {
      return (ParserWrapper){false, (ParserInfo){illegalType, token}};
    }

    // identifier
    token = GetNextToken();
    if (ERR == token.tp)
      return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
    if (ID == token.tp) {
    } else {
      return (ParserWrapper){false, (ParserInfo){idExpected, token}};
    }

    // (to stretch whitespace in formatter)
  }

  return (ParserWrapper){true, (ParserInfo){none, token}};
}

ParserWrapper subroutineBody() {
  Token token;

  // '{'
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"{", "\0"})) {
  } else {
    return (ParserWrapper){false, (ParserInfo){openBraceExpected, token}};
  }

  // stmt()
  token = PeekNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (RESWORD == token.tp && strcmpList(token.lx, stmtStart)) {
    ParserWrapper info = stmt();

    // todo: should I check if hasRun???????
    if (info.info.er != none)
      return info;
  } else {
    return (ParserWrapper){false, (ParserInfo){syntaxError, token}};
  }

  // '}'
  token = GetNextToken();
  if (ERR == token.tp)
    return (ParserWrapper){false, (ParserInfo){lexerErr, token}};
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"}", "\0"})) {
  } else {
    return (ParserWrapper){false, (ParserInfo){closeBraceExpected, token}};
  }

  return (ParserWrapper){true, (ParserInfo){none, token}};
}

ParserWrapper stmt() {
  Token token;
  ParserWrapper info;

  token = PeekNextToken();

  if (0 == strncmp(token.lx, "var", 128)) {
    info = varStmt();
  } else if (0 == strncmp(token.lx, "let", 128)) {
    info = letStmt();
  } else if (0 == strncmp(token.lx, "if", 128)) {
    info = ifStmt();
  } else if (0 == strncmp(token.lx, "while", 128)) {
    info = whileStmt();
  } else if (0 == strncmp(token.lx, "do", 128)) {
    info = doStmt();
  } else if (0 == strncmp(token.lx, "return", 128)) {
    info = returnStmt();
  } else {
    return (ParserWrapper){false, (ParserInfo){syntaxError, token}};
  }

  // todo: should I check if hasRun???????
  if (info.info.er != none)
    return info;

  return (ParserWrapper){true, (ParserInfo){none, token}};
}

/**********************************************************************
 **********************************************************************
 **********************************************************************
 ************************* Parser functions ***************************
 **********************************************************************
 **********************************************************************
 **********************************************************************
 */

int InitParser(char *file_name) {
  if (false == InitLexer(file_name))
    return false;

  return true;
}

ParserInfo Parse() {
  ParserInfo pi;

  // implement the function

  pi.er = none;
  return pi;
}

int StopParser() {
  if (false == StopLexer())
    return false;

  return true;
}

#ifndef TEST_PARSER
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./parser filename.jack");
    return 1;
  }

  InitParser(argv[1]);
  // Parse();

  // ParserWrapper info =
  //     consumeTerminal(RESWORD, (char *[]){"class", "\0"}, classExpected);
  //
  // printf("Has run: %d, data: %d, %s\n", info.hasRun, info.info.er,
  //        info.info.tk.lx);
  //
  // info = consumeTerminal(ID, (char *[]){"\0"}, idExpected);
  //
  // printf("Has run: %d, data: %d, %s\n", info.hasRun, info.info.er,
  //        info.info.tk.lx);

  StopParser();

  return 1;
}
#endif
