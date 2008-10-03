/*

 Parser - an expression parser

 Author:  Nick Gammon 
          http://www.gammon.com.au/ 

(C) Copyright Nick Gammon 2004. Permission to copy, use, modify, sell and
distribute this software is granted provided this copyright notice appears
in all copies. This software is provided "as is" without express or implied
warranty, and with no claim as to its suitability for any purpose.
 
Modified 24 October 2005 by Nick Gammon.

  1. Changed use of "abs" to "fabs"
  2. Changed inclues from math.h and time.h to fmath and ftime
  3. Rewrote DoMin and DoMax to inline the computation because of some problems with some libraries.
  4. Removed "using namespace std;" and put "std::" in front of std namespace names where appropriate
  5. Removed MAKE_STRING macro and inlined the functionality where required.
  6. Changed Evaluate function to take its argument by reference.

Thanks to various posters on my forum for suggestions. The relevant post is currently at:

  http://www.gammon.com.au/forum/bbshowpost.php?bbsubject_id=4649

*/

#include "gammonparser.h"


const int getrandom (const int x)
{
	long n;

	if (x <= 0)
		return 0;
  n = rand ();
	return (n % x);
}   // end of getrandom

const int roll (const int howmany, const int die)
{
	int count;
	int total = 0;

	for (count = 0; count < howmany; ++count)
		total += getrandom (die) + 1;

	return total;

} // end of roll


// returns true of a x% probability exists
// eg. percent (80) will be true 80% of the time
const bool percent (const int prob)
  {
  if (prob <= 0)
    return false;
  if (prob >= 100)
    return true;

  return getrandom (100) > (100 - prob);

  }

static int initRandom ()
  {
  srand   (time (NULL));
#ifndef WIN32
  srand48 (time (NULL));
#endif
  return 0;
  }

// initialise random number generator
static int someNumber = initRandom ();

/*

Expression-evaluator
--------------------

Author: Nick Gammon
-------------------


Example usage:

    Parser p ("2 + 2 * (3 * 5) + nick");
    
    p.symbols_ ["nick"] = 42;
    
    double v = p.Evaluate ();

    double v1 = p.Evaluate ("5 + 6");   // supply new expression and evaluate it
    
Syntax:

  You can use normal algebraic syntax. 
  
  Multiply and divide has higher precedence than add and subtract.
  
  You can use parentheses (eg. (2 + 3) * 5 )
  
  Variables can be assigned, and tested. eg. a=24+a*2
  
  Variables can be preloaded:
  
    p.symbols_ ["abc"] = 42;
    p.symbols_ ["def"] = 42;
    
  Afterwards they can be retrieved:
  
    x = p.symbols_ ["abc"];

  There are 2 predefined symbols, "pi" and "e".
  
  You can use the comma operator to load variables and then use them, eg.
  
    a=42, b=a+6
    
  You can use predefined functions, see below for examples of writing your own.
  
    42 + sqrt (64)
    
  
  Comparisons
  -----------
  
  Comparisons work by returning 1.0 if true, 0.0 if false.
  
  Thus, 2 > 3 would return 0.0
        3 > 2 would return 1.0
        
  Similarly, tests for truth (eg. a && b) test whether the values are 0.0 or not.
  
  If test
  -------
  
  There is a ternary function: if (truth-test, true-value, false-value)
  
  eg.  if (1 < 2, 22, 33)  returns 22
  
  
  Precedence
  ----------
  
  ( )  =   - nested brackets, including function calls like sqrt (x), and assignment
  * /      - multiply, divide
  + -      - add and subtract
  < <= > >= == !=  - comparisons
  && ||    - AND and OR
  ,        - comma operator
    
    Credits:
    
    Based in part on a simple calculator described in "The C++ Programming Language"
    by Bjarne Stroustrup, however with considerable enhancements by me, and also based
    on my earlier experience in writing Pascal compilers, which had a similar structure.
    
*/

// functions we can call from an expression

double DoInt (double arg)
  {
  return (int) arg;   // drop fractional part
  }

double DoRandom (double arg)
  {
  return getrandom (static_cast <int> (arg));  // random number in range 0 to arg
  }

double DoPercent (double arg)
  {
  if (percent (static_cast <int> (arg)))  // true x% of the time
    return 1.0;
  else
    return 0.0;
  }

const double DoMin (const double arg1, const double arg2)
  {
  return (arg1 < arg2 ? arg1 : arg2);
  }

const double DoMax (const double arg1, const double arg2)
  {
  return (arg1 > arg2 ? arg1 : arg2);
  }

const double DoFmod (const double arg1, const double arg2)
  {
  if (arg2 == 0.0)
    throw std::runtime_error ("Divide by zero in mod");
  
  return fmod (arg1, arg2);
  }

const double DoPow (const double arg1, const double arg2)
  {
  return pow (arg1, arg2);
  }

const double DoRoll (const double arg1, const double arg2)
  {
  return roll (static_cast <int> (arg1), static_cast <int> (arg2));
  }

const double DoIf (const double arg1, const double arg2, const double arg3)
  {
  if (arg1 != 0.0)
    return arg2;
  else
    return arg3;
  }

typedef double (*OneArgFunction)  (double arg);
typedef const double (*TwoArgFunction)  (const double arg1, const double arg2);
typedef const double (*ThreeArgFunction)  (const double arg1, const double arg2, const double arg3);

// maps of function names to functions
static std::map<std::string, OneArgFunction>    OneArgumentFunctions;
static std::map<std::string, TwoArgFunction>    TwoArgumentFunctions;
static std::map<std::string, ThreeArgFunction>  ThreeArgumentFunctions;

// for standard library functions
#define STD_FUNCTION(arg) OneArgumentFunctions [#arg] = arg

static int LoadOneArgumentFunctions ()
  {
  OneArgumentFunctions ["abs"] = fabs;
  STD_FUNCTION (acos);
  STD_FUNCTION (asin);
  STD_FUNCTION (atan);
#ifndef WIN32   // doesn't seem to exist under Visual C++ 6
  STD_FUNCTION (atanh);
#endif
  STD_FUNCTION (ceil);
  STD_FUNCTION (cos);
  STD_FUNCTION (cosh);
  STD_FUNCTION (exp);
  STD_FUNCTION (exp);
  STD_FUNCTION (floor);
  STD_FUNCTION (log);
  STD_FUNCTION (log10);
  STD_FUNCTION (sin);
  STD_FUNCTION (sinh);
  STD_FUNCTION (sqrt);
  STD_FUNCTION (tan);
  STD_FUNCTION (tanh);
  
  OneArgumentFunctions ["int"] = DoInt;
  OneArgumentFunctions ["rand"] = DoRandom;
  OneArgumentFunctions ["rand"] = DoRandom;
  OneArgumentFunctions ["percent"] = DoPercent;
  return 0;
  } // end of LoadOneArgumentFunctions

static int LoadTwoArgumentFunctions ()
  {
  TwoArgumentFunctions ["min"]  = DoMin;
  TwoArgumentFunctions ["max"]  = DoMax;
  TwoArgumentFunctions ["mod"]  = DoFmod;
  TwoArgumentFunctions ["pow"]  = DoPow;     //   x to the power y
  TwoArgumentFunctions ["roll"] = DoRoll;   // dice roll
  return 0;
  } // end of LoadTwoArgumentFunctions

static int LoadThreeArgumentFunctions ()
  {
  ThreeArgumentFunctions ["if"]  = DoIf;
  return 0;
  } // end of LoadThreeArgumentFunctions

const Parser::TokenType Parser::GetToken (const bool ignoreSign)
  {
  word_.erase (0, std::string::npos);
  
  // skip spaces
  while (*pWord_ && isspace (*pWord_))
    ++pWord_;

  pWordStart_ = pWord_;   // remember where word_ starts *now*
  
  // look out for unterminated statements and things
  if (*pWord_ == 0 &&  // we have EOF
      type_ == END)  // after already detecting it
    throw std::runtime_error ("Unexpected end of expression.");

  unsigned char cFirstCharacter = *pWord_;        // first character in new word_
  
  if (cFirstCharacter == 0)    // stop at end of file
    {
    word_ = "<end of expression>";
    return type_ = END;
    }
  
  unsigned char cNextCharacter  = *(pWord_ + 1);  // 2nd character in new word_
  
  // look for number
  if ((!ignoreSign && 
        (cFirstCharacter == '+' || cFirstCharacter == '-') && 
        isdigit (cNextCharacter)
      ) 
      || isdigit (cFirstCharacter))
    {
    // skip sign for now
    if ((cFirstCharacter == '+' || cFirstCharacter == '-'))
      pWord_++;
    while (isdigit (*pWord_) || *pWord_ == '.')
      pWord_++;
    
    // allow for 1.53158e+15
    if (*pWord_ == 'e' || *pWord_ == 'E')
      {
      pWord_++; // skip 'e'
      if ((*pWord_  == '+' || *pWord_  == '-'))
        pWord_++; // skip sign after e
      while (isdigit (*pWord_))  // now digits after e
        pWord_++;      
      }
    
    word_ = std::string (pWordStart_, pWord_ - pWordStart_);
    
    std::istringstream is (word_);
    // parse std::string into double value
    is >> value_;
      
    if (is.fail () || !is.eof ())
      throw std::runtime_error ("Bad numeric literal: " + word_);
    return type_ = NUMBER;
    }   // end of number found

  // special test for 2-character sequences: <= >= == !=
  // also +=, -=, /=, *=
  if (cNextCharacter == '=')
    {
    switch (cFirstCharacter)
      {
      // comparisons
      case '=': type_ = EQ;   break;
      case '<': type_ = LE;   break;
      case '>': type_ = GE;   break;
      case '!': type_ = NE;   break;
      // assignments
      case '+': type_ = ASSIGN_ADD;   break;
      case '-': type_ = ASSIGN_SUB;   break;
      case '*': type_ = ASSIGN_MUL;   break;
      case '/': type_ = ASSIGN_DIV;   break;
      // none of the above
      default:  type_ = NONE; break;
      } // end of switch on cFirstCharacter
    
    if (type_ != NONE)
      {
      word_ = std::string (pWordStart_, 2);
      pWord_ += 2;   // skip both characters
      return type_;
      } // end of found one    
    } // end of *=
  
  switch (cFirstCharacter)
    {
    case '&': if (cNextCharacter == '&')    // &&
                {
                word_ = std::string (pWordStart_, 2);
                pWord_ += 2;   // skip both characters
                return type_ = AND;
                }
              break;
   case '|': if (cNextCharacter == '|')   // ||
                {
                word_ = std::string (pWordStart_, 2);
                pWord_ += 2;   // skip both characters
                return type_ = OR;
                }
              break;
    // single-character symboles
    case '=':
    case '<':
    case '>':
    case '+':
    case '-':
    case '/':
    case '*':
    case '(':
    case ')':
    case ',':
    case '!':
      word_ = std::string (pWordStart_, 1);
      ++pWord_;   // skip it
      return type_ = TokenType (cFirstCharacter);
    } // end of switch on cFirstCharacter
  
  if (!isalpha (cFirstCharacter))
    {
    if (cFirstCharacter < ' ')
      {
      std::ostringstream s;
      s << "Unexpected character (decimal " << int (cFirstCharacter) << ")";
      throw std::runtime_error (s.str ());    
      }
    else
      throw std::runtime_error ("Unexpected character: " + std::string (1, cFirstCharacter));
    }
  
  // we have a word (starting with A-Z) - pull it out
  while (isalnum (*pWord_) || *pWord_ == '_')
    ++pWord_;
  
  word_ = std::string (pWordStart_, pWord_ - pWordStart_);
  return type_ = NAME;
  }   // end of Parser::GetToken

// force load of functions at static initialisation time
static int doLoadOneArgumentFunctions = LoadOneArgumentFunctions ();
static int doLoadTwoArgumentFunctions = LoadTwoArgumentFunctions ();
static int doLoadThreeArgumentFunctions = LoadThreeArgumentFunctions ();

const double Parser::Primary (const bool get)   // primary (base) tokens
  {
  
  if (get)
    GetToken ();    // one-token lookahead  
  
  switch (type_)
    {
    case NUMBER:  
      {
      double v = value_; 
      GetToken (true);  // get next one (one-token lookahead)
      return v;
      }
    
    case NAME:
      {
      std::string word = word_;
      GetToken (true); 
      if (type_ == LHPAREN)
        {
        // might be single-argument function (eg. abs (x) )
        std::map<std::string, OneArgFunction>::const_iterator si;
        si = OneArgumentFunctions.find (word);
        if (si != OneArgumentFunctions.end ())
          {
          double v = Expression (true);   // get argument
          CheckToken (RHPAREN);
          GetToken (true);        // get next one (one-token lookahead)
          return si->second (v);  // evaluate function
          }
        
        // might be double-argument function (eg. roll (6, 2) )
        std::map<std::string, TwoArgFunction>::const_iterator di;
        di = TwoArgumentFunctions.find (word);
        if (di != TwoArgumentFunctions.end ())
          {
          double v1 = Expression (true);   // get argument 1 (not commalist)
          CheckToken (COMMA);
          double v2 = Expression (true);   // get argument 2 (not commalist)
          CheckToken (RHPAREN);
          GetToken (true);            // get next one (one-token lookahead)
          return di->second (v1, v2); // evaluate function
          }
  
       // might be double-argument function (eg. roll (6, 2) )
        std::map<std::string, ThreeArgFunction>::const_iterator ti;
        ti = ThreeArgumentFunctions.find (word);
        if (ti != ThreeArgumentFunctions.end ())
          {
          double v1 = Expression (true);   // get argument 1 (not commalist)
          CheckToken (COMMA);
          double v2 = Expression (true);   // get argument 2 (not commalist)
          CheckToken (COMMA);
          double v3 = Expression (true);   // get argument 3 (not commalist)
          CheckToken (RHPAREN);
          GetToken (true);  // get next one (one-token lookahead)
          return ti->second (v1, v2, v3); // evaluate function
          }
        
        throw std::runtime_error ("Function '" + word + "' not implemented.");
        }
      
      // not a function? must be a symbol in the symbol table
      double & v = symbols_ [word];  // get REFERENCE to symbol table entry
      // change table entry with expression? (eg. a = 22, or a = 22)
      switch (type_)
        {
        // maybe check for NaN or Inf here (see: isinf, isnan functions)
        case ASSIGN:     v  = Expression (true); break;   
        case ASSIGN_ADD: v += Expression (true); break;   
        case ASSIGN_SUB: v -= Expression (true); break;   
        case ASSIGN_MUL: v *= Expression (true); break;   
        case ASSIGN_DIV: 
            {
            double d = Expression (true); 
            if (d == 0.0)
              throw std::runtime_error ("Divide by zero");
            v /= d;
            break;   // change table entry with expression
            } // end of ASSIGN_DIV
        default: break;   // do nothing for others
        } // end of switch on type_              
      return v;               // and return new value
      }
    
    case MINUS:               // unary minus
      return - Primary (true);
    
    case NOT:   // unary not
      return (Primary (true) == 0.0) ? 1.0 : 0.0;;
    
    case LHPAREN:
      {
      double v = CommaList (true);    // inside parens, you could have commas
      CheckToken (RHPAREN);
      GetToken ();                    // eat the )
      return v;
      }
    
    default:   
      throw std::runtime_error ("Unexpected token: " + word_);
    
    } // end of switch on type
  
  } // end of Parser::Primary 

const double Parser::Term (const bool get)    // multiply and divide
  {
  double left = Primary (get);
  while (true)
    {
    switch (type_)
      {
      case MULTIPLY:  
        left *= Primary (true); break;
      case DIVIDE: 
          {
          double d = Primary (true);
          if (d == 0.0)
            throw std::runtime_error ("Divide by zero");
          left /= d; 
          break;
          }
      default:    return left;
      } // end of switch on type
    }   // end of loop
  } // end of Parser::Term

const double Parser::AddSubtract (const bool get)  // add and subtract
  {
  double left = Term (get);
  while (true)
    {
    switch (type_)
      {
      case PLUS:  left += Term (true); break;
      case MINUS: left -= Term (true); break;
      default:    return left;
      } // end of switch on type
    }   // end of loop
  } // end of Parser::AddSubtract

const double Parser::Comparison (const bool get)  // LT, GT, LE, EQ etc.
  {
  double left = AddSubtract (get);
  while (true)
    {
    switch (type_)
      {
      case LT:  left = left <  AddSubtract (true) ? 1.0 : 0.0; break;
      case GT:  left = left >  AddSubtract (true) ? 1.0 : 0.0; break;
      case LE:  left = left <= AddSubtract (true) ? 1.0 : 0.0; break;
      case GE:  left = left >= AddSubtract (true) ? 1.0 : 0.0; break;
      case EQ:  left = left == AddSubtract (true) ? 1.0 : 0.0; break;
      case NE:  left = left != AddSubtract (true) ? 1.0 : 0.0; break;
           default:    return left;
      } // end of switch on type
    }   // end of loop
  } // end of Parser::Comparison

const double Parser::Expression (const bool get)  // AND and OR
  {
  double left = Comparison (get);
  while (true)
    {
    switch (type_)
      {
      case AND: 
            {
            double d = Comparison (true);   // don't want short-circuit evaluation
            left = (left != 0.0) && (d != 0.0); 
            }
          break;
      case OR:  
            {
            double d = Comparison (true);   // don't want short-circuit evaluation
            left = (left != 0.0) || (d != 0.0); 
            }
          break;
      default:    return left;
      } // end of switch on type
    }   // end of loop
  } // end of Parser::Expression

const double Parser::CommaList (const bool get)  // expr1, expr2
  {
  double left = Expression (get);
  while (true)
    {
    switch (type_)
      {
      case COMMA:  left = Expression (true); break; // discard previous value
      default:    return left;
      } // end of switch on type
    }   // end of loop
  } // end of Parser::CommaList

const double Parser::Evaluate ()  // get result
  {
  double v = CommaList (true);
  if (type_ != END)
    throw std::runtime_error ("Unexpected text at end of expression: " + std::string (pWordStart_));
  return v;  
  }

// change program and evaluate it
const double Parser::Evaluate (const std::string & program)  // get result
  {
  // do same stuff constructor did
  program_  = program;
  pWord_    = program_.c_str ();
  type_     = NONE;
  return Evaluate ();
  }

