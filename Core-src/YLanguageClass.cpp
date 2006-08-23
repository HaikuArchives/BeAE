/*******************************************************
*   YLanguageClass©
*
*   Quick and dirty way of loading up language files
*   for apps. Verry simple and powerful :)
*
*   @author  YNOP(ynop@acm.org) 
*   @version 1.0.1
*   @date    May 4 2000
*******************************************************/
/******************************************************
*   The original LanguageClass was writen by Dirk Olbertz
*   back in the day and was last seen lurking on BeWare
*   on 12/98.  I don't know if he still uses/updates it but
*   his email is something like beinformed@beoscentral.com
*
*   From that base, which I actually found first in 
*   the Photon source code, I have changed ... uh ..
*   everthing.  We now use the STL::map (red-black tree)
*   as our storage unit. Here is a good URL to stl::map
*   http://www.sgi.com/Technology/STL/Map.html.
*   This will provide significant speed increases for 
*   large Language files. It also is much simpler
*   to work with. I also redesigned a couple of things
*   If you call 'get' and no matching key is found you
*   will get the key back. This was a good way of seeing
*   if your keys got entered corectly or not.  I also
*   redid the layout of the wordfile makeing it !much!
*   easyer to work with.
*   So basicly I changed everthing :)
*   
*   Some other improvments are:
*      impiment the wordfile as XML. not realy important
*      clean up the file reading code.
*      stop useing B* classes to make it portable
*
*   NOTES:
*
*   Although we support multi-bit charaters - The user
*   must set the font. Mabey we should add a way for the
*   Lang file to spesify or suggest a font to use.
*
*   Also the chars '\n' '\t' '\\' '\"' are now properly
*   converted into there respecive charicters, letting
*   you write C style strings in the Lang files.
*
*   Lang file syntax:
*
*      KEY_NAME = "Blah blah\n\tblah blah blah" //comment
*      ANOTHER_KEY = "this that and the other"
*      // boo
*      QUOATED = "This \"IS\" quoated"
*      ERROR = "Error %i: %s" // use with a int and string param
*      LASTKEY = ""
*   
*   notice that there is a space ( ) after and befor the
*   equal (=) sign, this is !reqired!. The quoats (") around
*   the string are also required as they let you add
*   spaces to your text.  Comments can be added any place
*   however there can be NO blank lines untill the end
*   of the file (I should change this soon) The Key should
*   be a single word, no spaces starting at the verry 
*   begining of a line, all capps is a good idea for style
*   points. For exaples see the English file
*
*   10-23-00: Fixxed bug with the \? code. It was inserting
*   a extra space befor each charicter. Fixed now
*******************************************************/
#include <File.h>
#include <Path.h>
#include <Roster.h>
#include <StorageKit.h>
#include <String.h>
#include <map>

#include <stdio.h>

#include "YLanguageClass.h"

// This is our Gloable def of lang. Its nice to have some gloables right?
YLanguageClass Language;

/*******************************************************
*   Constructor
*******************************************************/
YLanguageClass::YLanguageClass(){
   LName.SetTo("UN_SET"); // Defalut to something
   our_status = B_NO_INIT; 
   userDefinedPath = false;
}

/*******************************************************
*   Constructor
*******************************************************/
YLanguageClass::YLanguageClass(const char *path){
   LName.SetTo("UN_SET"); // Defalut to something
   our_status = B_NO_INIT; 
   LanguagePath.SetTo(path);
   userDefinedPath = true;
}

/*******************************************************
*   Destructor
*******************************************************/
YLanguageClass::~YLanguageClass(){
}


/*******************************************************
*   Returns the current name of the lang we are useing
*******************************************************/
status_t YLanguageClass::InitCheck(){
   return our_status;
}

/*******************************************************
*   Returns the current name of the lang we are useing
*******************************************************/
const char* YLanguageClass::Name(){
   return LName.String();
}

/*******************************************************
*   Sets and loads a new language
*******************************************************/
void YLanguageClass::SetName(const char* name){
   LName.SetTo(name);
   LoadLang();
}

/*******************************************************
*   Get a string via key lookup
*******************************************************/
const char* YLanguageClass::get(const char* key){
   const char* data = NULL;
   data = LangList[key];
   if(data == NULL){
      //printf("UNMATCHED KEY - \"%s\"\n",key);
      //debugger("key");
      our_status = B_ERROR;
      return key;
   }
   our_status = B_OK;
   return data;
}

/*******************************************************
*   Add a pair of key/word to the list.
*******************************************************/
void YLanguageClass::set(const char *key, const char *string){
  LangList[key] = string;
}

/*******************************************************
*   Load the lang from a file into our list
*******************************************************/
void YLanguageClass::LoadLang(){
   BPath path;
   BFile *langfile;
   BString line;
   int i;
   BString Key;
   BString Text;
   BString tmp;

   //sscanf(lLine, "%s = %s", &lItem, &lValue);
   
   // Clear the list of any old Lang keys
   LangList.clear();
   
   if(!userDefinedPath){
      // Grab the current path 
      // WARNING must have a be_app !
      app_info ai;
      be_app->GetAppInfo(&ai);
      BEntry entry(&ai.ref);
      entry.GetPath(&LanguagePath);
      LanguagePath.GetParent(&LanguagePath);
      LanguagePath.Append("Languages");    // HARD CODE 
   }
   
   path.SetTo(LanguagePath.Path());
   path.Append(LName.String());

   // Creat a file amd make shur its there   
   langfile = new BFile(path.Path(),B_READ_ONLY);
   if((our_status = langfile->InitCheck()) != B_OK){
      // file error
      return;
   }
   // Read each line untill a blank line
   // this realy needs to be something better
   // like until EOF or something .. 
   line = ReadLine(langfile);
   while(line != ""){
      i = line.FindFirst(" = ");
      if(i >= 0){
         line.CopyInto(Key,0,i);
         // make shure to use Length and not CharCount as this is the key
         // for multi-bit fonts and stuff. Like Japanese
         line.CopyInto(tmp,i+3,line.Length()-i+3);
         if(tmp[0] != '\"'){
            // malforemd
            // no open quote
            our_status = B_ERROR;
         }else{
            //i = tmp.FindFirst("\"",1);
            i = tmp.FindLast("\"");
            if(i >= 0){
               tmp.CopyInto(Text,1,i-1);
               // This is a perfect line lets add it in to our
               // little list
               
               // Fix for the \n char
               i = Text.FindFirst("\\n");
               while(i >= 0){
                  //Text[i] = ' ';
                  //Text[i+1] = '\n';
                  Text.Remove(i,1);
                  i = Text.FindFirst("\\n");
               }
               // Fix for the \t char
               i = Text.FindFirst("\\t");
               while(i >= 0){
                  //Text[i] = ' ';
                  //Text[i+1] = '\t';
                  Text.Remove(i,1);
                  i = Text.FindFirst("\\t");
               }
               // Fix for the \\ char
               i = Text.FindFirst("\\\\");
               while(i >= 0){
                  //Text[i] = ' ';
                  //Text[i+1] = '\\';
                  Text.Remove(i,1);
                  i = Text.FindFirst("\\\\");
               }
               // Fix for the \" char
               i = Text.FindFirst("\\\"");
               while(i >= 0){
                  //Text[i] = ' ';
                  //Text[i+1] = '"';
                  Text.Remove(i,1);
                  i = Text.FindFirst("\\\"");
               }
               
               // Add the key to the table. We need to 
               // allocate memory for the final resting 
               // place of our key,data pair.
               char* thekey = new char[Key.Length()+1];
               strcpy(thekey,Key.String());
               char* thedata = new char[Text.Length()+1];
               strcpy(thedata,Text.String());
               // The almighty add
               set(thekey,thedata);
            }else{
               //this is malformed
               // no close quote
               our_status = B_ERROR;
            }
         }
      }else{
         //invalide line .. is a comment or mabey malformed line 
         // no ' = '
         if((line[0] == '/') && (line[1] == '/')){
            // This line is a comment ... hehe
         }else{
            our_status = B_ERROR;
         }
         
      }
      line = ReadLine(langfile);
   }
}

/*******************************************************
*   Read one line of the file.
*******************************************************/
BString YLanguageClass::ReadLine(BFile *file){
   BString line;
   char a;
   line.SetTo("");
   while(file->Read((void*)&a,1) && (a != '\n')){
      line << a;
   } 
   return line;
}

/*******************************************************
*   
*******************************************************/
status_t YLanguageClass::Install(const char *key,const char *text,const char *comment){
   BPath path(LanguagePath.Path());
   path.Append(LName.String());
   
   if(LName.Compare("UN_SET") == B_OK){
      return B_ERROR;
   }
   
   //We need to escape the text befor we do anything with it...
   BString Text(text);
   // \n \t \\ \"
   int32 i = -1;
   
   i = Text.FindFirst("\\");
   while(i >= 0){
      Text.Remove(i,1);
      Text.Insert("\\\\",i);
      i = Text.FindFirst("\\",i+4);
   }
   
   i = Text.FindFirst("\n");
   while(i >= 0){
      Text.Remove(i,1);
      Text.Insert("\\n",i);
      i = Text.FindFirst("\n",i+3);
   }
   
   i = Text.FindFirst("\t");
   while(i >= 0){
      Text.Remove(i,1);
      Text.Insert("\\t",i);
      i = Text.FindFirst("\t",i+3);
   }
   
   i = Text.FindFirst("\"");
   while(i >= 0){
      Text.Remove(i,1);
      Text.Insert("\\\"",i);
      i = Text.FindFirst("\"",i+4);
   }
   
   const char *value = NULL;
   value = get(key);
   
   if(InitCheck() != B_OK){
      // does not exsist in file
      BFile langfile(path.Path() ,B_READ_WRITE|B_CREATE_FILE|B_OPEN_AT_END);
      
      BString str;
      str.Append(key);
      str.Append(" = ");
      str.Append("\"");
      str.Append(Text.String());
      str.Append("\"");
      if(comment){
         str.Append(" // ");
         str.Append(comment);
      }
      str.Append("\n");
      langfile.Write(str.String(),str.Length());
   }else{
      // Its alreay installed I gess you want me to change it?
   }
   
   
   return B_OK;
}

/*******************************************************
*   
*******************************************************/
status_t YLanguageClass::InstallFromFile(const char *path){
   return B_ERROR;
}







