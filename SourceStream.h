#ifndef SourceStream_h
#define SourceStream_h

#include <iostream>

struct MemoryStruct {
  char *memory;
  size_t size;
};

using std::wcin;
using std::noskipws;

class SourceStream   //tokeznizator bedzie kozycstal z klasy abstrakcyjnej sourcestream
{
public:
   virtual wchar_t getNextChar() = 0; //jako ze jest to klasa abstrakcyjna ta funkjca bedzie virual
                                      // = 0; - czysto abstrakcyjna
                                       //klasa abstrakcyjna zawierajaca metody wirtualne to interface
                                      //elementy staowniace interfejs musza byc publiczne(zeby uzytkownik mial dostep)





};
//wchar_t jest wbudowany w std c++ - to jest to samo co WCHAR z winapi.


class KeyboardSourceStream : public SourceStream
{
public:
   virtual wchar_t getNextChar()
   {
      wchar_t c;
      wcin >> noskipws >> c; //cin dla wchar_t
      return c;
   }



};

class NetStream : public SourceStream
{
    int offset;


    wchar_t *wskSource;
    int size;
    public:

    NetStream(wchar_t* buffer, int size)
    {
        offset = 0;
        this->size = size;
        wskSource = buffer;
    }

    virtual wchar_t getNextChar()
   {



       if(offset >= size)
       {

           return 0;
       }

//       wcout << offset << ": " << (int)wskSource[offset] << endl;
       return wskSource[offset++];



   }




};

#endif // SOURCESTREAM_H_INCLUDED
