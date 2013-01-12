#ifndef Token_h
#define Token_h

#include <string>

using std::wstring;

enum Token_Type ///there is 6 types of tokens
{
  GENERIC = 0, CHARACTER, START_TAG, END_TAG, COMMENT, DOCTYPE, DOCTYPE_PUBLIC, DOCTYPE_SYSTEM


};



class Token //this is base class for all tokens
{
  public:
      virtual Token_Type getType()
{
    return GENERIC;

}



};



class CharacterToken : public Token //
{
  public:
      wchar_t character; //skladowa moze byc publiczna, bo niez zalezy zeby Token byl
     //chermetycznym typem danych. bedzie przenosil dane z Tokenizer do Parser
	  CharacterToken() : character(0)
	  {}


      virtual Token_Type getType()
	 {

		  return CHARACTER;
     }
};

class StartTagToken : public Token
{
public:
     wstring tagName; //wstring poniewaz te znaki beda stopniowo dodawane

	 virtual Token_Type getType()
	 {
	  return START_TAG;
	 }


};

class EndTagToken : public Token
{
public:
     wstring tagName; //wstring poniewaz te znaki beda stopniowo dodawane

	 virtual Token_Type getType()
	 {
	  return END_TAG;
	 }


};

class CommentToken : public Token
{
public:
	wstring comment;



	 virtual Token_Type getType()
	 {
	  return COMMENT;
	 }


};

class DoctypeToken : public Token
{
public:
	wstring doctype;

	 virtual Token_Type getType()
	 {
	  return DOCTYPE;
	 }


};

class DoctypePublicToken : public Token
{
public:
	wstring doctypePublic;

	 virtual Token_Type getType()
	 {
	  return DOCTYPE_PUBLIC;
	 }


};

class DoctypeSystemToken : public Token
{
public:
	wstring doctypeSystem;

	 virtual Token_Type getType()
	 {
	  return DOCTYPE_SYSTEM;
	 }

};











#endif // TOKEN_H_INCLUDED
