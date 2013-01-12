#define UNICODE
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <string>//dla wstring
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "SourceStream.h"
#include "Token.h"

using std::wstring;


class WinConsoleOutputStream {

    HANDLE console;
    DWORD dummy;
    WCHAR endline[2];

public:
    WinConsoleOutputStream() {
        console = GetStdHandle(STD_OUTPUT_HANDLE);//The standard output device. Initially, this is the active console screen buffer, CONOUT$.
        endline[0] = 0x000A; //Line feed
        endline[1] = 0x000D; //carriage return
    }

    WinConsoleOutputStream& out(wchar_t wideChar) {
        WriteConsole(console, &wideChar, 1, &dummy, NULL);
        return *this;
    }

    WinConsoleOutputStream& out(const std::wstring& wideString) {
        WriteConsole(console, wideString.c_str(), wideString.size(), &dummy, NULL);
  	//c_str() Generates a null-terminated sequence of characters (c-string) with the same content as the string object and returns it as a pointer to an array of characters
		//A terminating null character is automatically appended
        return *this;
		
    }

    WinConsoleOutputStream& out(char narrowChar) {
        WCHAR wideChar = (WCHAR) narrowChar;
        WriteConsole(console, &wideChar, 1, &dummy, NULL);
        return *this;
    }

    WinConsoleOutputStream& out(const std::string& narrowString) {
        int utf16chars = MultiByteToWideChar(CP_UTF8, 0, narrowString.c_str(), -1, 0, 0);
        WCHAR* utf16buffer = (WCHAR*) malloc(utf16chars*sizeof(WCHAR));
        MultiByteToWideChar(CP_UTF8, 0, narrowString.c_str(), -1, utf16buffer, utf16chars);
        WriteConsole(console, utf16buffer, utf16chars, &dummy, NULL);
        free(utf16buffer);
        return *this;
    }

    WinConsoleOutputStream& endl() {
        WriteConsole(console, endline, 2, &dummy, NULL);
        return *this;
    }

    WinConsoleOutputStream& out(int value) {
        std::stringstream stream;
        stream << value; // int na string 
        out(stream.str());
        return *this;
    }

    WinConsoleOutputStream& color(WORD colorCode) {
        SetConsoleTextAttribute(console, colorCode);
        return *this;
    }

    WinConsoleOutputStream& red() {
        return color(FOREGROUND_RED | FOREGROUND_INTENSITY);
    }

    WinConsoleOutputStream& green() {
        return color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }

    WinConsoleOutputStream& white() {
        return color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    }

    WinConsoleOutputStream& gray() {
        return color(FOREGROUND_INTENSITY);
    }

} winout; //stworzony zostaje obietk klasy WinConsoleOutputStream, i odwoluje sie do tej klasy piszac "wincout"




enum TokenizerState                //wklacze go jako plik naglowkowy
{
    DATA = 0, TAG_OPEN, TAG_NAME, END_TAG_OPEN, BEFORE_ATTRIBUTE_NAME, ATTRIBUTE_NAME, BEFORE_ATTRIBUTE_VALUE,
	ATTRIBUTE_VALUE_DOUBLE_QUOTED, AFTER_ATTRIBUTE_VALUE_QUOTED, MARKUP_DECLARATION_OPEN, COMMENT_START, COMMENT_START_DASH,
	COMMENT_STATE, COMMENT_END_DASH, COMMENT_END, DOCTYPE_STATE, BEFORE_DOCTYPE_NAME, DOCTYPE_NAME, AFTER_DOCTYPE_NAME,
	AFTER_DOCTYPE_PUBLIC_KEYWORD, AFTER_DOCTYPE_SYSTEM_KEYWORD, BEFORE_DOCTYPE_PUBLIC_IDENTIFIER, DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED,
	DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED, AFTER_DOCTYPE_PUBLIC_IDENTIFIER, BETWEEN_DOCTYPE_PUBLIC_AND_SYSTEM_IDENTIFERS,
	DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED, DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED, BEFORE_DOCTYPE_SYSTEM_IDENTIFIER,
	AFTER_DOCTYPE_SYSTEM_IDENTIFIER, BOGUS_DOCTYPE,ATTRIBUTE_VALUE_UNQUOTED,BOGUS_COMMENT,SELF_CLOSING_START_TAG,AFTER_ATTRIBUTE_NAME,
	ATTRIBUTE_VALUE_SINGLE_QUOTED
};


class Tokenizer
{
      SourceStream* html_stream;           //zapamietac w skladowej jakiego strumienia obiektu klasy SourceStream uzywam.
                                           //dostane go jako wskaznik z zewnatz

      enum TokenizerState currentState;        // zmienna typu wyliczeniowego  ( c++ traktuje enum jako zwykly int )

      bool tokenReady; //czy token sie juz uzbieral

      Token* currentToken; //wskaznik do obiketu klasy Token . bedzie zwracany przez getNextToken()

	  void processDataState(wchar_t character);  //metody musza byc prywatne
      void processTagOpenState(wchar_t character);
	  void processTagNameState(wchar_t character);
	  void endTagOpenState(wchar_t character);
	  void beforeAttributeNameState(wchar_t character);
	  void attributeNameState(wchar_t character);
	  void befoteAttributeValueState(wchar_t character);
	  void attributeValueDoubleQuotedState(wchar_t character);
	  void afterAttributeValueQuoted(wchar_t character);
	  void markupDeclarationOpenState(wchar_t character);
	  void commentStartState(wchar_t character);
	  void commentState(wchar_t character);
	  void commentStartDashState(wchar_t character);
	  void commentEndDashState(wchar_t character);
	  void commentEndState(wchar_t character);
	  void doctypeState(wchar_t character);
	  void beforeDoctypeNameState(wchar_t character);
	  void doctypeNameState(wchar_t character);
	  void afterDoctypeNameState(wchar_t character);
	  void afterDoctypePublicKeyword(wchar_t character);
	  void afterDoctypeSystemKeyword(wchar_t character);
	  void beforeDoctypePublicIdentifierState(wchar_t character);
	  void doctypePublicIdentifierDoubleQuotedState(wchar_t character);
	  void afterDoctypePublicIdentifierState(wchar_t character);
	  void doctypePublicIdentifierSingleQuotedState(wchar_t character);
	  void betweenDoctypePublicAndSystemIdentifiersState(wchar_t character);
	  void beforeDoctypeSystemIdentifierState(wchar_t character);
	  void doctypeSystemIdentifierDoubleQuotedState(wchar_t character);
	  void doctypeSystemIdentifierSingleQuotedState(wchar_t character);
	  void afterDoctypeSystemIdentifierState(wchar_t character);
	  void bogusDoctypeState(wchar_t character);
	  void attributValueUnquotedState(wchar_t character);
	  void bogusCommentState(wchar_t character);
	  void selfClosingStartTagState(wchar_t character);
	  void afterAttributeNameState(wchar_t character);
	  void attributeValueSingleQuoted(wchar_t character);



 public:
      Tokenizer(SourceStream* wskStream); //konstruktor w ktorym dostane wskaznik do strumienia na jakim obiekt klasy Tokenizer ma pracowac

	  Token* getNextToken();


};

Tokenizer::Tokenizer(SourceStream* wskStream)
{
            html_stream = wskStream;
            currentState = DATA; //unezpieczam sie zeby tworzac obiekt rozpoczynal sie od stanu "DATA"
            tokenReady = false; //wszystkie skladwe jakie sa zawarte w klasie zawsze trzeba w konstruktorze inicjalizowac !!!
            currentToken = 0; //inicjuje "NULL`em" bo to wskaznik
}



void Tokenizer::processDataState(wchar_t character)
{
	wchar_t less_than_sign = '<';


	    if(character == less_than_sign ) //Switch to the tag open state
			//symbole jak '<' moge jako stale w klasie;
		 {



			 currentState = TAG_OPEN;
		     return;
		 }

		if(character == 0)
		{
			 CharacterToken* token = new CharacterToken; //wskaznik na obietk klasy CharacterToken
		     token->character = character; //argument funkcji(character) to skaldowa w obieckie
		     currentToken = token; //podstawiam go do currentToken,
             tokenReady = true;
			 return;



		}

		else//anything else Emit the current input character as a character token.
		{

	    CharacterToken* token = new CharacterToken; //wskaznik na obietk klasy CharacterToken
		token->character = character; //argument funkcji(character) to skaldowa w obieckie
		currentToken = token; //podstawiam go do currentToken,
        tokenReady = true;

		return;


		}
}


void Tokenizer::processTagOpenState(wchar_t character)
{



    wchar_t a = 'a';
	wchar_t z = 'z';
	wchar_t exclamation_mark = '!';
	wchar_t solidus = 0x002F;
	wchar_t question = 0x003F;

	      if(character == exclamation_mark)
		  {
			  currentState = MARKUP_DECLARATION_OPEN;
			  return;

		  }

		  if(character == solidus)
		  {

             currentState = END_TAG_OPEN;
			 return;

		  }

		   if(character >= 'A' && character <= 'Z' )
		  {
		    StartTagToken* token = new StartTagToken; //wskaznik do obiektu typu StartTagToken
		    token->tagName = tolower(character);
			currentToken = token;
            currentState = TAG_NAME;
			//nie ustawaiam flagi tokenReady bo moze jescze cos do tego tokena byc dodane
			return;

		  }

		    if(character == question)
		  {
			  currentState = BOGUS_COMMENT;
		      return;

		  }

	      if(character >= a && character <= z )
		  {
		    StartTagToken* token = new StartTagToken; //wskaznik do obiektu typu StartTagToken
		    token->tagName = character;
			currentToken = token;
            currentState = TAG_NAME;
			//nie ustawaiam flagi tokenReady bo moze jescze cos do tego tokena byc dodane
			return;

		  }

		  else
		  {
			CharacterToken* token = new CharacterToken; //wskaznik na obietk klasy CharacterToken
		    token->character = '<';
			currentToken = token;
			currentState = DATA;
            tokenReady = true;
			return;


		  }
}

void Tokenizer::endTagOpenState(wchar_t character)
{



	wchar_t a = 'a';
	wchar_t z = 'z';

		if(character >= 'A' && character <= 'Z' )
	{
		EndTagToken* token = new EndTagToken;
		token->tagName = tolower(character);
		currentToken = token;;
		currentState = TAG_NAME;

		return;

	}

   if(character >= a && character <= z )
	 {
		    EndTagToken* token = new EndTagToken;
		    token->tagName = character;
			currentToken = token;
            currentState = TAG_NAME;
			return;

	}

   if(character == '>')
   {
	   currentState = DATA;
	   return;
   }

   else
   {
	   currentState = BOGUS_COMMENT;
	   return;
   }

}


void Tokenizer::bogusCommentState(wchar_t character)
{
	CommentToken* token = new CommentToken;

	token->comment = character;
	currentToken = token;


    if(character == '>')
	{
	tokenReady = true;
	currentState = DATA;
	return;
	}

	return;


}





void Tokenizer::processTagNameState(wchar_t character)
{



	wchar_t greater_than_sign = '>';
	wchar_t space = ' ';
	wchar_t spacehex = 0x0020;
	wchar_t solidus = 0x002F;


	     if(character == greater_than_sign ) //Switch to the data state. Emit the current tag token
		 {
		    currentState = DATA;
			tokenReady = true; //token jest "gotowy" i pentla moze while moze byc skonczona
			return;
		 }

		 if(character == solidus)
		 {
			 currentState = SELF_CLOSING_START_TAG;
			 return;

		 }

		 if(character == space) //Switch to the before attribute name state
		 {

			 currentState = BEFORE_ATTRIBUTE_NAME;
			 return;
		 }

		 if(character >= 'a' && character <= 'z' )
		  {

			  ((StartTagToken*)currentToken)->tagName += character; //mozna teraz dopisac kolejna litere do lancucha,on sie wydluzy
			  return;
		  }
		 else
		 {
			 ((StartTagToken*)currentToken)->tagName += character;
			 return;
		 }

}

void Tokenizer::selfClosingStartTagState(wchar_t character)
{
	wchar_t greaterThan = 0x003E;

	if(character == greaterThan)
	{
		tokenReady = true;
		currentState = DATA;
		return;


	}


	else
	{
		currentState = BEFORE_ATTRIBUTE_NAME;
		return;

	}




}




void Tokenizer::beforeAttributeNameState(wchar_t character)
{

	wchar_t solidus = 0x002F;
	wchar_t greater_than_sign = '>';

	if(character == solidus)
		 {
			 currentState = SELF_CLOSING_START_TAG;
			 return;

		 }


	if(character == greater_than_sign )
		 {
		    currentState = DATA;
			tokenReady = true;
			return;
		 }


	if(character >= 'A' && character <= 'Z' )
	{
		((StartTagToken*)currentToken)->tagName += tolower(character);
		currentState = ATTRIBUTE_NAME;

		return;

	}

	else
	{

	    StartTagToken* token = new StartTagToken;
		token->tagName = character;
	    currentToken = token;
        currentState = ATTRIBUTE_NAME;
	    return;
	}

}

void Tokenizer::attributeNameState(wchar_t character)
{

	wchar_t solidus = 0x002F;
	wchar_t equals = 0x003D;
	wchar_t space = 0x0020;
	wchar_t greaterThan = 0x003E;

	if(character == greaterThan)
	{
		tokenReady = true;
		currentState = DATA;
		return;


	}

	if(character == space)
	{
		currentState = AFTER_ATTRIBUTE_NAME;
		return;


	}


	if(character == equals)
	{
		currentState = BEFORE_ATTRIBUTE_VALUE;
		return;
    }


	if(character == solidus)
		 {
			 currentState = SELF_CLOSING_START_TAG;
			 return;

		 }


	if(character >= 'A' && character <= 'Z' )
	{
		((StartTagToken*)currentToken)->tagName += tolower(character);
		return;

	}

	else
	{
		((StartTagToken*)currentToken)->tagName += character;
		return;
	}

}



void Tokenizer::afterAttributeNameState(wchar_t character)
{
	wchar_t solidus = 0x002F;
	wchar_t equals = '=';
	wchar_t space = 0x0020;
	wchar_t greaterThan = 0x003E;

	 if(character == solidus)
	{
	   currentState = SELF_CLOSING_START_TAG;
	   return;
	}

	 if(character == equals)
	{
		currentState = BEFORE_ATTRIBUTE_VALUE;
		return;
    }

	 if(character == greaterThan)
	 {
		 tokenReady = true;
		 currentState = DATA;
		 return;


	 }

	 if(character >= 'A' && character <= 'Z' )
	{
		StartTagToken* token = new StartTagToken;
		token->tagName = character;
	    currentToken = token;
        currentState = ATTRIBUTE_NAME;
	    return;
	 }

	 else
	 {

		StartTagToken* token = new StartTagToken;
		token->tagName = character;
	    currentToken = token;
        currentState = ATTRIBUTE_NAME;
	    return;
	 }
}



void Tokenizer::befoteAttributeValueState(wchar_t character)
{


	wchar_t apostrophe = 0x0027;
	wchar_t quotation_mark = 0x0022;
	wchar_t ampersand = 0x0026;

	if(character == quotation_mark)
	{
		currentState = ATTRIBUTE_VALUE_DOUBLE_QUOTED;
		return;

	}

	if(character == apostrophe)
	{

		currentState = ATTRIBUTE_VALUE_SINGLE_QUOTED;
		return;
	}

	if(character== NULL)
	{
		((StartTagToken*)currentToken)->tagName += 0xFFFD;
		return;
	}

	if(character == ampersand)
	{
		currentState = ATTRIBUTE_VALUE_SINGLE_QUOTED;
		return;

	}


	else
	{
		((StartTagToken*)currentToken)->tagName += character;
		currentState = ATTRIBUTE_VALUE_UNQUOTED;
		return;
	}
}


void Tokenizer::attributeValueSingleQuoted(wchar_t character)
{
		wchar_t apostrophe = 0x0027;

		if(character == apostrophe)
	{

		currentState = AFTER_ATTRIBUTE_VALUE_QUOTED;
		return;
	}

		else
		{

			((StartTagToken*)currentToken)->tagName += character;
		     return;

		}




}


void Tokenizer::attributValueUnquotedState(wchar_t character)
{
	wchar_t space = '_';
	wchar_t greater_than_sign = '>';

	 if(character == space)
		 {

			 currentState = BEFORE_ATTRIBUTE_NAME;
			 return;
		 }

	 if(character == greater_than_sign )
		 {
		    currentState = DATA;
			tokenReady = true;
			return;
		 }
	 else
	 {
		((StartTagToken*)currentToken)->tagName += character;

		return;
	 }


}


void Tokenizer::attributeValueDoubleQuotedState(wchar_t character)
{


	wchar_t quotation_mark = 0x0022;

	if(character == quotation_mark )
	{
		currentState = AFTER_ATTRIBUTE_VALUE_QUOTED;
        return;

	}

	else

	{

		((StartTagToken*)currentToken)->tagName += character;
		return;

    }

}

void Tokenizer::afterAttributeValueQuoted(wchar_t character)
{

	wchar_t solidus = 0x002F;



	wchar_t space = '_';
	wchar_t greater_than_sign = '>';

	 if(character == space)
		 {

			 currentState = BEFORE_ATTRIBUTE_NAME;
			 return;
		 }

	 if(character == greater_than_sign )
		 {
		    currentState = DATA;
			tokenReady = true;
			return;
		 }

	  if(character == solidus)
	{
	   currentState = SELF_CLOSING_START_TAG;
	   return;
	}

	 else
	 {
		StartTagToken* token = new StartTagToken;
		token->tagName = character;
	    currentToken = token;
        currentState = ATTRIBUTE_NAME;
	    return;
	 }



}

int i = 0;
wstring temp ;
void Tokenizer::markupDeclarationOpenState(wchar_t character)
{


	wchar_t hyphen_minus = 0x002D;




	if(character == hyphen_minus )
	{
		if(i == 1)
		{


			CommentToken* token = new CommentToken; //wskaznik do obiektu klsay CommentToken

			currentToken = token;
		    currentState = COMMENT_START;
			return;
		}

		else
		{

			i = 1;
			return;
		}

	}

	else
	{

		wstring doctype ( L"DOCTYPE" );
		temp += toupper(character);

		 if (doctype == temp)
		 {
			 currentState = DOCTYPE_STATE;
			 return;
		 }
		 else
		 {
			 currentState = BOGUS_COMMENT;
			 return;
		 }


	}


}


void Tokenizer::commentStartState(wchar_t character)
{

	wchar_t hyphen_minus = 0x002D;

	if(character == hyphen_minus)
	{
		currentState = COMMENT_START_DASH;
	    return;
	}

	else
	{
		((CommentToken*)currentToken)->comment += character;
		currentState = COMMENT_STATE;
		return;



	}
}


void Tokenizer::commentState(wchar_t character)
{
	wchar_t hyphen_minus = 0x002D;


	if(character == hyphen_minus )
	{
		currentState = COMMENT_END_DASH;
		return;
	}

	else
	{
		((CommentToken*)currentToken)->comment += character;
		return;
	}



}

void Tokenizer::commentStartDashState(wchar_t character)
{

	wchar_t hyphen_minus = 0x002D;

	if (character == hyphen_minus)
	{
		currentState = COMMENT_END;
		return;
	}

	else
	{
		((CommentToken*)currentToken)->comment += hyphen_minus;
		((CommentToken*)currentToken)->comment += character;
		currentState = COMMENT_STATE;
		return;

	}



}

void Tokenizer::commentEndDashState(wchar_t character)
{
	wchar_t hyphen_minus = 0x002D;


	if(character == hyphen_minus)
	{
		currentState = COMMENT_END;
		return;
	}

	else
	{
		((CommentToken*)currentToken)->comment += hyphen_minus;
		((CommentToken*)currentToken)->comment += character;
		currentState = COMMENT_STATE;
		return;

	}

}

void Tokenizer::commentEndState(wchar_t character)
{
	wchar_t hyphen_minus = 0x002D;

	wchar_t greater_than_sign = '>';

	if(character == greater_than_sign)
	{
		currentState = DATA;
		tokenReady = true;
		return;
	}

	else
	{
	   ((CommentToken*)currentToken)->comment += hyphen_minus;
	   ((CommentToken*)currentToken)->comment += hyphen_minus;
       currentState = COMMENT_STATE;
	   return;
	}

}


void Tokenizer::doctypeState(wchar_t character)
{

	wchar_t space = 0x0020;

	if (character == space)
	{
		currentState = BEFORE_DOCTYPE_NAME;
		return;
	}

	else
	{
		currentState = DOCTYPE_NAME;
		return;
	}


}

void Tokenizer:: beforeDoctypeNameState(wchar_t character)
{

	wchar_t space = 0x0020;

	if(character >= 'A' && character <= 'Z' )
	{
		DoctypeToken* token = new DoctypeToken; //wskaznik do obiektu klsay DoctypeToken

		currentToken = token;
		((DoctypeToken*)currentToken)->doctype += space; //(add 0x0020 to the character's code point)
		((DoctypeToken*)currentToken)->doctype += tolower(character);
		currentState = DOCTYPE_NAME;
	    return;
	}

	else
	{
		DoctypeToken* token = new DoctypeToken; //wskaznik do obiektu klsay DoctypeToken

		currentToken = token;
		((DoctypeToken*)currentToken)->doctype += character;
		currentState = DOCTYPE_NAME;
	    return;
	}





}

void Tokenizer::doctypeNameState(wchar_t character)
{

	wchar_t greater_than_sign = '>';
	wchar_t space = 0x0020;

	if(character == space)
	{
      currentState = AFTER_DOCTYPE_NAME;
	   return;
	}

	if(character == greater_than_sign)
	{
		currentState = DATA;
		tokenReady = true;
		return;
	}



	if(character >= 'A' && character <= 'Z' )
	{

		((DoctypeToken*)currentToken)->doctype += tolower(character);
		//	((DoctypeToken*)currentToken)->doctype += space;
		return;
	}


    else
	{

		((DoctypeToken*)currentToken)->doctype += character;
	    return;
	}



}
wstring temp2;
void Tokenizer::afterDoctypeNameState(wchar_t character)
{

	wchar_t greater_than_sign = '>';

	if(character == greater_than_sign)
	{
		currentState = DATA;
		tokenReady = true;
		return;
	}


	else
	{

		wstring doc_public ( L"PUBLIC" );
		wstring doc_system ( L"SYSTEM" );

		temp2 += toupper(character);

		 if (doc_public == temp2)
		 {

			 currentState = AFTER_DOCTYPE_PUBLIC_KEYWORD;
			 return;
		 }

		 if(doc_system == temp2)
		 {

			 currentState = AFTER_DOCTYPE_SYSTEM_KEYWORD;
			 return;

		 }
		 else
		 {
			 currentState = BOGUS_DOCTYPE;
			 return;
		 }


	}

}


void Tokenizer::bogusDoctypeState(wchar_t character)
{
	if(character == '>')
	{
		currentState = DATA;
		tokenReady = true;
		return;

	}

	else
	{
		return;
	}





}



void Tokenizer::afterDoctypePublicKeyword(wchar_t character)
{

    wchar_t space = 0x0020;
	wchar_t quotation_mark = 0x0022;
	wchar_t apostrophe = 0x0027;

	if(character == space)
	{
		currentState = BEFORE_DOCTYPE_PUBLIC_IDENTIFIER;
		return;
	}

	if(character == quotation_mark)
	{
		DoctypePublicToken* token = new DoctypePublicToken;

		currentToken = token;
		currentState = DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED;
	    return;

	}

	if(character == apostrophe)
	{
		DoctypePublicToken* token = new DoctypePublicToken;

		currentToken = token;
		currentState = DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED;
	    return;


	}

	else
	{
		currentState = BOGUS_DOCTYPE;
		return;
	}

}

void Tokenizer::beforeDoctypePublicIdentifierState(wchar_t character)
{


	wchar_t quotation_mark = 0x0022;
	wchar_t apostrophe = 0x0027;

	if(character == quotation_mark)
	{
		DoctypePublicToken* token = new DoctypePublicToken;

		currentToken = token;
		currentState = DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED;
	    return;

	}

	if(character == apostrophe)
	{
		DoctypePublicToken* token = new DoctypePublicToken;

		currentToken = token;
		currentState = DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED;
	    return;


	}

    else
	{
		currentState = BOGUS_DOCTYPE;
		return;
	}
}

void Tokenizer:: doctypePublicIdentifierDoubleQuotedState(wchar_t character)
{


	wchar_t quotation_mark = 0x0022;
	if (character == quotation_mark)
	{
		currentState = AFTER_DOCTYPE_PUBLIC_IDENTIFIER;
		return;
	}

	else
	{
		((DoctypePublicToken*)currentToken)->doctypePublic += character;
		return;
	}



}


void Tokenizer::doctypePublicIdentifierSingleQuotedState(wchar_t character)
{


	wchar_t apostrophe = 0x0027;
	if (character == apostrophe)
	{
		currentState = AFTER_DOCTYPE_PUBLIC_IDENTIFIER;
		return;
	}

	else
	{
		((DoctypePublicToken*)currentToken)->doctypePublic += character;
		return;
	}



}


void Tokenizer::afterDoctypePublicIdentifierState(wchar_t character)
{
;

	wchar_t space = 0x0020;
	wchar_t greater_than_sign = '>';

	if(character == space)
    {
		currentState = BETWEEN_DOCTYPE_PUBLIC_AND_SYSTEM_IDENTIFERS;
		return;

	}


	if(character == greater_than_sign)
	{
		currentState = DATA;
		tokenReady = true;
		return;
	}

	else
	{
		currentState = BOGUS_DOCTYPE;
		return;
	}

}


void Tokenizer::betweenDoctypePublicAndSystemIdentifiersState(wchar_t character)
{

	wchar_t quotation_mark = 0x0022;
	wchar_t apostrophe = 0x0027;
	wchar_t greater_than_sign = '>';


	if(character == greater_than_sign)
	{
		currentState = DATA;
		tokenReady = true;
		return;
	}

	if(character == quotation_mark)
	{
		DoctypeSystemToken* token = new DoctypeSystemToken;

		currentToken = token;
		currentState = DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED;
	    return;

	}

	if(character == apostrophe)
	{
		DoctypeSystemToken* token = new DoctypeSystemToken;


		currentToken = token;
		currentState = DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED;
	    return;


	}

	else
	{
		currentState = BOGUS_DOCTYPE;
		return;
	}



}

void Tokenizer::beforeDoctypeSystemIdentifierState(wchar_t character)
{

	wchar_t quotation_mark = 0x0022;
	wchar_t apostrophe = 0x0027;

	if(character == quotation_mark)
	{
		DoctypeSystemToken* token = new DoctypeSystemToken;

		currentToken = token;
		currentState = DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED;
	    return;

	}

	if(character == apostrophe)
	{
		DoctypeSystemToken* token = new DoctypeSystemToken;


		currentToken = token;
		currentState = DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED;
	    return;


	}

	else
	{
		currentState = BOGUS_DOCTYPE;
		return;
	}



}



void Tokenizer::afterDoctypeSystemKeyword(wchar_t character)
{


	wchar_t quotation_mark = 0x0022;
	wchar_t apostrophe = 0x0027;
	wchar_t space = 0x0020;

	if(character == space)
	{
		currentState = BEFORE_DOCTYPE_SYSTEM_IDENTIFIER;
		return;

	}

	if(character == quotation_mark)
	{
		DoctypeSystemToken* token = new DoctypeSystemToken;

		currentToken = token;
		currentState = DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED;
	    return;

	}

	if(character == apostrophe)
	{
		DoctypeSystemToken* token = new DoctypeSystemToken;


		currentToken = token;
		currentState = DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED;
	    return;


	}

	else
	{
		currentState = BOGUS_DOCTYPE;
		return;
	}


}

void Tokenizer::doctypeSystemIdentifierDoubleQuotedState(wchar_t character)
{



	wchar_t quotation_mark = 0x0022;
	if (character == quotation_mark)
	{
		currentState = AFTER_DOCTYPE_SYSTEM_IDENTIFIER;
		return;
	}

	else
	{
		((DoctypeSystemToken*)currentToken)->doctypeSystem += character;
		return;
	}



}


void Tokenizer::doctypeSystemIdentifierSingleQuotedState(wchar_t character)
{



	wchar_t apostrophe = 0x0027;
	if (character == apostrophe)
	{
		currentState = AFTER_DOCTYPE_SYSTEM_IDENTIFIER;
		return;
	}

	else
	{
		((DoctypeSystemToken*)currentToken)->doctypeSystem += character;
		return;
	}

}


void Tokenizer::afterDoctypeSystemIdentifierState(wchar_t character)
{


	wchar_t greater_than_sign = '>';


	if(character == greater_than_sign)
	{
		currentState = DATA;
		tokenReady = true;
		return;
	}
	else
	{
		currentState = BOGUS_DOCTYPE;
		return;
	}


}

Token* Tokenizer::getNextToken()
{
       tokenReady = false;


       currentToken = 0;  //zanim rozpoczene ten proces musze wyzerowac

       do
         {

/*			switch(currentState)
			{
                   case DATA:
                        wcout << "DATA STATE ";
                        break;
				   case TAG_OPEN:
                        wcout << "TAG_OPEN STATE ";
                        break;
				   case TAG_NAME:
                        wcout << "TAG_NAME STATE ";
                        break;
                   case END_TAG_OPEN:
                        wcout << "END_TAG_OPEN STATE ";
                        break;
				   case BEFORE_ATTRIBUTE_NAME:
                        wcout << "BEFORE_ATTRIBUTE_NAME STATE ";
                        break;
				   case ATTRIBUTE_NAME:
                        wcout << "ATTRIBUTE_NAME STATE ";
                        break;
				   case BEFORE_ATTRIBUTE_VALUE:
                        wcout << "BEFORE_ATTRIBUTE_VALUE STATE ";
                        break;
				   case ATTRIBUTE_VALUE_DOUBLE_QUOTED:
                        wcout << "ATTRIBUTE_VALUE_DOUBLE_QUOTED STATE ";
                        break;
				   case AFTER_ATTRIBUTE_VALUE_QUOTED:
                        wcout << "AFTER_ATTRIBUTE_VALUE_QUOTED STATE ";
                        break;
				   case MARKUP_DECLARATION_OPEN:
                        wcout << "MARKUP_DECLARATION_OPEN STATE ";
                        break;
				   case COMMENT_START:
                        wcout << "COMMENT_START STATE ";
                        break;
				   case COMMENT_START_DASH:
                        wcout << "COMMENT_START_DASH STATE ";
                        break;
				   case COMMENT_STATE:
                        wcout << "COMMENT_STATE STATE ";
                        break;
				   case COMMENT_END_DASH:
                        wcout << "COMMENT_END_DASH STATE ";
                        break;
				   case COMMENT_END:
                        wcout << "COMMENT_END STATE ";
                        break;
				   case DOCTYPE_STATE:
                        wcout << "DOCTYPE_STATE STATE ";
                        break;
				   case BEFORE_DOCTYPE_NAME:
                        wcout << "BEFORE_DOCTYPE_NAME STATE ";
                        break;
				   case DOCTYPE_NAME :
                        wcout << "DOCTYPE_NAME STATE ";
                        break;
				   case AFTER_DOCTYPE_NAME:
                        wcout << "AFTER_DOCTYPE_NAME STATE ";
                        break;
				   case AFTER_DOCTYPE_PUBLIC_KEYWORD:
                        wcout << "AFTER_DOCTYPE_PUBLIC_KEYWORD STATE ";
                        break;
				   case AFTER_DOCTYPE_SYSTEM_KEYWORD:
                        wcout << "AFTER_DOCTYPE_SYSTEM_KEYWORD STATE ";
                        break;
				   case BEFORE_DOCTYPE_PUBLIC_IDENTIFIER:
                        wcout << "BEFORE_DOCTYPE_PUBLIC_IDENTIFIER STATE ";
                        break;
				   case DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED:
                        wcout << "DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED STATE ";
                        break;
				   case AFTER_DOCTYPE_PUBLIC_IDENTIFIER:
                        wcout << "AFTER_DOCTYPE_PUBLIC_IDENTIFIER STATE ";
                        break;
				   case DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED:
                        wcout << "DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED STATE ";
                        break;
				   case BETWEEN_DOCTYPE_PUBLIC_AND_SYSTEM_IDENTIFERS :
                        wcout << "BETWEEN_DOCTYPE_PUBLIC_AND_SYSTEM_IDENTIFERS STATE ";
                        break;
				   case BEFORE_DOCTYPE_SYSTEM_IDENTIFIER:
                        wcout << "BEFORE_DOCTYPE_SYSTEM_IDENTIFIER STATE ";
                        break;
				   case  DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED:
                        wcout << "DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED STATE ";
                        break;
				   case DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED:
                        wcout << "DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED STATE ";
                        break;
				   case AFTER_DOCTYPE_SYSTEM_IDENTIFIER:
                        wcout << "AFTER_DOCTYPE_SYSTEM_IDENTIFIER STATE ";
                        break;
				   case BOGUS_DOCTYPE:
                        wcout << "BOGUS_DOCTYPE STATE ";
                        break;
				   case ATTRIBUTE_VALUE_UNQUOTED:
                        wcout << "ATTRIBUTE_VALUE_UNQUOTED STATE ";
                        break;
				   case BOGUS_COMMENT:
					   wcout << "BOGUS_COMMENT_STATE" ;
					   break;
				   case SELF_CLOSING_START_TAG:
					   wcout << "SELF_CLOSING_START_TAG_STATE" ;
					break;
					case AFTER_ATTRIBUTE_NAME:
						wcout << "AFTER_ATTRIBUTE_NAME_STATE";
				    break;
					case  ATTRIBUTE_VALUE_SINGLE_QUOTED :
						wcout << "ATTRIBUTE_VALUE_SINGLE_QUOTED_STATE";
					break;
 				   default:
                    wcout << "NO MATCHING STATE!" << endl;
                    break;
			}
*/
		    wchar_t c = html_stream->getNextChar();  //dynamic cast jesli *html_stream = 0 throw
		    if(c == 0)
		    {
//		        winout.out("getNextChar returned 0 -------------------------------------------").endl();
		        return 0;
		    }
//            winout.out(' ').out(c).out(" [").out((int)c).out("]").endl();


			switch(currentState)
			{
                   case DATA : processDataState(c); //skladowa ktorej przekaze znak zwracany
                   break;      				   //ona bedzie zmieniac stan obiektu nie musi nic zwracac
				   case TAG_OPEN: processTagOpenState(c);
				   break;
				   case TAG_NAME: processTagNameState(c);
                   break;
                   case END_TAG_OPEN: endTagOpenState(c);
				   break;
				   case BEFORE_ATTRIBUTE_NAME: beforeAttributeNameState(c);
				   break;
				   case ATTRIBUTE_NAME: attributeNameState(c);
				   break;
				   case BEFORE_ATTRIBUTE_VALUE: befoteAttributeValueState(c);
                   break;
				   case ATTRIBUTE_VALUE_DOUBLE_QUOTED: attributeValueDoubleQuotedState(c);
                   break;
				   case AFTER_ATTRIBUTE_VALUE_QUOTED: afterAttributeValueQuoted(c);
				   break;
				   case MARKUP_DECLARATION_OPEN: markupDeclarationOpenState(c);
				   break;
				   case COMMENT_START: commentStartState(c);
			       break;
				   case COMMENT_START_DASH: commentStartDashState(c);
				   break;
				   case COMMENT_STATE: commentState(c);
				   break;
				   case COMMENT_END_DASH: commentEndDashState(c);
				   break;
				   case COMMENT_END: commentEndState(c);
				   break;
				   case DOCTYPE_STATE: doctypeState(c);
				   break;
				   case BEFORE_DOCTYPE_NAME: beforeDoctypeNameState(c);
				   break;
				   case DOCTYPE_NAME : doctypeNameState(c);
				   break;
				   case AFTER_DOCTYPE_NAME: afterDoctypeNameState(c);
			       break;
				   case AFTER_DOCTYPE_PUBLIC_KEYWORD: afterDoctypePublicKeyword(c);
				   break;
				   case AFTER_DOCTYPE_SYSTEM_KEYWORD: afterDoctypeSystemKeyword(c);
				   break;
				   case BEFORE_DOCTYPE_PUBLIC_IDENTIFIER: beforeDoctypePublicIdentifierState(c);
				   break;
				   case DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED: doctypePublicIdentifierDoubleQuotedState(c);
				   break;
				   case AFTER_DOCTYPE_PUBLIC_IDENTIFIER: afterDoctypePublicIdentifierState(c);
				   break;
				   case DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED: doctypePublicIdentifierSingleQuotedState(c);
				   break;
				   case BETWEEN_DOCTYPE_PUBLIC_AND_SYSTEM_IDENTIFERS : betweenDoctypePublicAndSystemIdentifiersState(c);
				   break;
				   case BEFORE_DOCTYPE_SYSTEM_IDENTIFIER: beforeDoctypeSystemIdentifierState(c);
				   break;
				   case  DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED: doctypeSystemIdentifierDoubleQuotedState(c);
				   break;
				   case DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED: doctypeSystemIdentifierSingleQuotedState(c);
				   break;
				   case AFTER_DOCTYPE_SYSTEM_IDENTIFIER: afterDoctypeSystemIdentifierState(c);
				   break;
				   case BOGUS_DOCTYPE: bogusDoctypeState(c);
				   break;
				   case ATTRIBUTE_VALUE_UNQUOTED: attributValueUnquotedState(c);
				   break;
				   case BOGUS_COMMENT : bogusCommentState(c);
				   break;
				   case SELF_CLOSING_START_TAG: selfClosingStartTagState(c);
				   break;
				   case AFTER_ATTRIBUTE_NAME: afterAttributeNameState(c);
				   break;
				   case  ATTRIBUTE_VALUE_SINGLE_QUOTED : attributeValueSingleQuoted(c);
				   break;
				   default:
                    winout.out("NO MATCHING STATE!").endl();
                    break;
			}



         }

		while(!tokenReady); //"dopoki nie jest jeszcze gotowy"

        Token* tmp = currentToken;
        currentToken = 0;//zeruje dla tego aby ten sklanik(adres) nie wyciekal na zewnatrz, bo ktos mogl by uzyc ten adres nieswiadomie i go usunac
                        // i zmodyfikowac to na co pokazuje
        return tmp;


}















































static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)//callback kolekcjonuje przychodzace fragmenty userp - to czwarty opcjonalny argument dostarczony przez uzytwkownika(mnie), przez niego przekazywany jest wskaznik do struktury w niej adres i rozmiar
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1); //rzutowanie przez dodanie (char*)
  if (mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    exit(EXIT_FAILURE);
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  //  na samym koncu bufora zawsze dopisuje "0",trzba o to 0 zadbac samemu

  return realsize;
}






























int main(void)
{
  CURL *curl_handle;

  struct MemoryStruct chunk; //tworzye obiekty typu struct

  chunk.memory = (char*)malloc(1); //rzutowanie przez dodanie (char*) /* will be grown as needed by the realloc above */ //alokuje cos zebyc cos na poczatku bylo , ona bedzie i tak rosnac
  chunk.size = 0;    /* no data at this point */ // powinno byc real i aktual size .

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
// curl_easy_setopt(curl_handle, CURLOPT_URL, "http://logan.knp.net.pl/test.html");
  curl_easy_setopt(curl_handle, CURLOPT_URL, "http://nowa.uek.krakow.pl/");

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk); //to chunk przekazuje do callback , dzieku temu arg (chunk) zachowuje ciaglas , trwale slady

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  curl_easy_perform(curl_handle);  // po powrocie z perform mogge uzywac to co sie nagromadzilo w chunk

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  /*
   * Now, our chunk.memory points to a memory block that is chunk.size
   * bytes big and contains the remote file.
   *
   * Do something nice with it!
   *
   * You should be aware of the fact that at this point we might have an
   * allocated data block, and nothing has yet deallocated that data. So when
   * you're done with it, you should free() it as a nice application.
   */

  int utf16chars = MultiByteToWideChar(CP_UTF8, 0, chunk.memory, -1, 0, 0);
  //ta funckja policzyla mi ile znakow utf8 jest na stronie i podala to w utf16
  /*
  1 arg -  CP_UTF8 -Code page to use in performing the conversion.
  2 arg -  for UTF-8  dwFlags must be set to either 0
  3 arg -  Pointer to the character string to convert.
  4 arg - Size, in bytes, of the string indicated by the lpMultiByteStr(chunk_memory) parameter. this parameter can be set to -1 if the string is null-terminated.
  5 arg -Pointer to a buffer that receives the converted string.
  6 arg - Size, in characters, of the buffer indicated by lpWideCharStr(chunk_memory)
          If this value is 0, the function returns the required buffer size, in characters, including any
          terminating null character, and makes no use of the lpWideCharStr buffer.
          0 dla tego ze chce tylko policzyc ile jest znakow na stronie, nie chce konwertowac ich do zadnego bufora
  */
  //wiec teraz w utf16chars mam ilosc znakow na stronie w utf 16



  WCHAR* utf16buffer = (WCHAR*) malloc(utf16chars*sizeof(WCHAR));
  // utf16buffer wskazuje na zalokowanewane miiejsce przez malloc, Malloc zalokwal tyle ile
  // mam w policzone w zmiennej utf16chars
  /*
  WCHAR is a 16-bit Unicode character.
  This type is declared in WinNT.h as follows:
  typedef wchar_t WCHAR;



  */
  MultiByteToWideChar(CP_UTF8, 0, chunk.memory, -1, utf16buffer, utf16chars);
 /*  Maps a character string to a UTF-16 (wide character) string
     same as above but:
     arg 5 - Pointer to a buffer(utf16buffer) that receives the converted string.
     arg 6 -size(utf16chars), in characters, of the buffer indicated by lpWideCharStr(chunk_memory)


 */
  DWORD dummy;
  int step = 2000; //ile znakow mam zaladowac w jednym obiegu pentli
  for (int offset=0; offset < utf16chars; offset +=step) {  // tak dlugo jak offset bedzie mniejszy od ilosci znakow na stronie(utf16chars)
//     WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),
//      utf16buffer +offset,
//        (utf16chars-offset < step) ? (utf16chars-offset) : step,
//     &dummy, NULL );
     /* Writes a character string to a console screen buffer beginning at the current cursor location.
        1 agr - A handle to the console screen buffer
        2 arg - A pointer to a buffer(WCHAR* utf16buffer) that contains characters to be written to the console screen buffer + offset
        3 arg - The number of characters to be written
        if utf16chars-offset(counted chars - offset) < step(2000)
            {utf16chars-offset(counted chars - offset)}
         else {  step (2000)     }
        4 arg -  A pointer to a variable that receives the number of characters actually written.
                 this is not need so its dummy,
        5 arg - Reserved; must be NULL.*/



     }




//  printf("%lu bytes retrieved in UTF8\n", (long)chunk.size);
//  printf("%d UTF16 charakters\n", utf16chars);

  //if(chunk.memory)
    //free(chunk.memory);

  /* we're done with libcurl, so clean it up */
  //curl_global_cleanup();

  ///////////////////////////////////////
KeyboardSourceStream stream;
NetStream stream2(utf16buffer, utf16chars);

    Tokenizer tokenizator(&stream2); //tworze obiekt klasy Tokenizer , jako argument
     //przesylam obiekt klasy KeyboardSourceStream


    Token_Type lastType = GENERIC;

     while(1)
     {
     Token* token = tokenizator.getNextToken();//potrzebuje nastepny Token
     if (token == 0 )
	 {
//		 winout.out(" dostalem 0 ").endl();
       break;
	 }

     //dowiedziec sie jakiego typu jest ten character:
     switch(token->getType() )
     {
     case GENERIC:
       winout.endl().gray().out("generic token ");
       break;
     case CHARACTER:
        if (lastType != CHARACTER) {
            winout.endl().white();
        }
          winout.out(((CharacterToken*)token) ->character); //rzutowanie na wskaznik
		  
        break;
    /* case START_TAG:
     winout.endl().green().out("Start_tag token ")
           .out(((StartTagToken*)token)->tagName);
        break;
      case END_TAG:
       winout.endl().red().out("End_tag token ")
           .out(((EndTagToken*)token)->tagName);
		 break;
	 case COMMENT:
	   winout.endl().gray().out("Comment token ")
		   .out(((CommentToken*)token )->comment);
	   break;
	 case DOCTYPE:
		 winout.endl().gray().out("Doctype token ")
			   .out(( (DoctypeToken*)token )->doctype);
        break;
	 case DOCTYPE_PUBLIC:
		winout.endl().gray().out("DoctypePublic token ")
			   .out(( (DoctypePublicToken*)token )->doctypePublic);
        break;
	 case DOCTYPE_SYSTEM:
		 winout.endl().gray().out("DoctypeSystem token ")
			   .out(( (DoctypeSystemToken*)token )->doctypeSystem);
        break;*/
    }
    lastType = token->getType();
     delete token;//usuwam z pamieci, bo tworzylem operatorem new
	 
     }



     //na poczatek pominac dla "&"

	 ///trzeba porobic wszyskie else w stanach tokenizera!!!!!!!!!!!!!!!

return 0;

}


