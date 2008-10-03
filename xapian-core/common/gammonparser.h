#ifndef PARSER_H
#define PARSER_H

#ifdef WIN32
  
// disable warnings about long names
  #pragma warning( disable : 4786)

#endif

#include <string>
#include <map>
#include <stdexcept>
#include <sstream>

#include <cmath>
#include <ctime>

class Parser
  {
  
  public:
    
  typedef enum TokenType
    {
    NONE,
    NAME,
    NUMBER,
    END,
    PLUS='+',
    MINUS='-',
    MULTIPLY='*',
    DIVIDE='/',
    ASSIGN='=',
    LHPAREN='(',
    RHPAREN=')',
    COMMA=',',
    NOT='!',
    
    // comparisons
    LT='<',
    GT='>',
    LE,     // <=
    GE,     // >=
    EQ,     // ==
    NE,     // !=
    AND,    // &&
    OR,      // ||
    
    // special assignments
    
    ASSIGN_ADD,  //  +=
    ASSIGN_SUB,  //  +-
    ASSIGN_MUL,  //  +*
    ASSIGN_DIV   //  +/
    
    };

  private:
    
  std::string program_;
  
  const char * pWord_;
  const char * pWordStart_;
  // last token parsed
  TokenType type_;
  std::string word_;
  double value_;
     
  public:
 
  // ctor
  Parser (const std::string & program) 
    : program_ (program), pWord_ (program_.c_str ()), type_ (NONE)
      {  
      // insert pre-defined names:
      symbols_ ["pi"] = 3.1415926535897932385;
      symbols_ ["e"]  = 2.7182818284590452354;      
      }
  
  const double Evaluate ();  // get result
  const double Evaluate (const std::string & program);  // get result

  // access symbols with operator []
  double & operator[] (const std::string & key) { return symbols_ [key]; }

  // symbol table - can be accessed directly (eg. to copy a batch in)
  std::map<std::string, double> symbols_;

  private:

  const TokenType GetToken (const bool ignoreSign = false);  
  const double CommaList (const bool get);
  const double Expression (const bool get);
  const double Comparison (const bool get);
  const double AddSubtract (const bool get);
  const double Term (const bool get);      // multiply and divide
  const double Primary (const bool get);   // primary (base) tokens

  inline void CheckToken (const TokenType wanted)
    {
    if (type_ != wanted)
      {
      std::ostringstream s;
      s << "'" << static_cast <char> (wanted) << "' expected.";
      throw std::runtime_error (s.str ());    
      }
    }
  };  // end of Parser


#endif // PARSER_H

