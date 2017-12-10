#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <afxwin.h>
#include <atlstr.h>
#include <atlcomtime.h>

#include "BuildVer.rh"

#undef ERROR
#define ERROR(x, y) { _tprintf(x, y); return(false); }

//Some globals
bool g_bShowHelp = false;
CString g_strVerFile;
CString g_strMajorVer, g_strMinorVer, g_strPatchVer, g_strBuildVer;
CString g_strHeaderFile, g_strDotNetFile;
CString g_strConfigFile;
CString g_strLabel;

CMapStringToString g_Dictionary;

//Builds the output file substituting the dictionary values on the way
bool BuildFile ( void )
{
   bool bBuildNet = !g_strDotNetFile.IsEmpty();
   bool bBuildVC = !g_strHeaderFile.IsEmpty();
   
   //Open the output file
   CStdioFile OutFile, OutNetFile;
   if (bBuildVC && !OutFile.Open(g_strHeaderFile, CFile::modeWrite | CFile::modeCreate))
      ERROR(TEXT("Failed to open C++ output file for writing\n"), NULL);
   if (bBuildNet && !OutNetFile.Open(g_strDotNetFile, CFile::modeWrite | CFile::modeCreate))
      ERROR(TEXT("Failed to open .NET output file for writing\n"), NULL);
      	
   //Load the template resource  
   HRSRC hNetRsc = NULL, hRsc = NULL;
   if (bBuildNet) 
      hNetRsc = FindResource(NULL, MAKEINTRESOURCE(IDR_TEMPLATEFILENET), TEXT("TEMPLATE"));   
   if (bBuildVC)
      hRsc = FindResource(NULL, MAKEINTRESOURCE(IDR_TEMPLATEFILE), TEXT("TEMPLATE"));      
   HGLOBAL hTempl = bBuildVC ? LoadResource(NULL, hRsc) : NULL;
   HGLOBAL hNetTempl = bBuildNet ? LoadResource(NULL, hNetRsc) : NULL;
   if (bBuildVC && (NULL == hTempl))
      ERROR(TEXT("Failed to load C++ template file\n"), NULL);
   if (bBuildNet && (NULL == hNetTempl))
      ERROR(TEXT("Failed to load .NET template file\n"), NULL);
      
   //Map the resource over to a string
   CString strTempl = bBuildVC ? CA2CT(reinterpret_cast<LPCSTR>(LockResource(hTempl))) : "";
   CString strNetTempl = bBuildNet ? CA2CT(reinterpret_cast<LPCTSTR>(LockResource(hNetTempl))) : "";
     
   //Walk the dictionary
   CString strKey, strValue;
   POSITION Pos = g_Dictionary.GetStartPosition();
   while (Pos)
   {
      //Replace all occurrences in the template file
      g_Dictionary.GetNextAssoc(Pos, strKey, strValue);
      
      if (bBuildVC)
		 strTempl.Replace(CString(TEXT("%")) + strKey + CString(TEXT("%")), strValue);
	  if (bBuildNet)
	     strNetTempl.Replace(CString(TEXT("%")) + strKey + CString(TEXT("%")), strValue);
   };

   //MODIFIED : MLT - 12/18/02 Replace all CRLF to NL otherwise each line has
   //2 LFs following it.
   if (bBuildVC)
   {
      strTempl.Replace(TEXT("\x0d\x0a"), TEXT("\n"));
      OutFile.WriteString(strTempl);
   };
   if (bBuildNet)
   {
      strNetTempl.Replace(TEXT("\x0d\x0a"), TEXT("\n"));   
      OutNetFile.WriteString(strNetTempl);
   };     
   
   return true;
}

//Create a file containing the version number and label
bool CreateVerFile ( void )
{
   if (g_strVerFile.IsEmpty())
      return true;

   //VB only needs major, minor and build
   CString strVer;
   strVer.Format(TEXT("%s.%s.%s.%s"), 
                 g_Dictionary[TEXT("MAJOR_VER")], g_Dictionary[TEXT("MINOR_VER")], 
                 g_Dictionary[TEXT("PATCH_VER")], g_Dictionary[TEXT("BUILD_VER")]);
   if (!g_strLabel.IsEmpty())
      strVer.AppendFormat(TEXT(" (%s)"), g_strLabel);

   CStdioFile OutFile;
   if (!OutFile.Open(g_strVerFile, CFile::modeWrite | CFile::modeCreate))
      ERROR(TEXT("Failed to create version file - '%s'\n"), g_strVerFile);

   OutFile.WriteString(strVer);
   return true;
}

//Display the program syntax
void DisplayHelp ( void )
{
   _tprintf(TEXT("BuildVer [-n file] [-v build] [-b file] [-l label] [-c configfile] [headerfile]\n"));
   _tprintf(TEXT("\twhere headerfile is the name of the file to generate\n"));
   _tprintf(TEXT("\t-n\tGenerate a .NET version file as well\n"));
   _tprintf(TEXT("\t-c\tConfig file containing template variables - it is updated with the new build number\n"));
   _tprintf(TEXT("\t-v\tUse the given version number instead of incrementing the existing number in the config file (a.b.c.d)\n"));
   _tprintf(TEXT("\t-b\tGenerate version file containing version and label\n"));
   _tprintf(TEXT("\t-l\tAdd the given label to the version string\n"));
   _tprintf(TEXT("\t-h\tShow this screen\n"));
}

//Generate the various forms of the build date and time
void GenerateBuildDateTime ( void )
{
   //Get the current date and time
   COleDateTime Current = COleDateTime::GetCurrentTime();

   //Build the various formats for the date and time
   g_Dictionary["BUILD_DATE"] = Current.Format(TEXT("%B %d, %Y"));
   g_Dictionary["BUILD_DATE_SHORT"] = Current.Format(TEXT("%m/%d/%Y"));
   g_Dictionary["BUILD_DATE_INTL"] = Current.Format(TEXT("%d %B %Y"));
   g_Dictionary["BUILD_TIME"] = Current.Format(TEXT("%I:%M %p"));
   g_Dictionary["BUILD_TIME_INTL"] = Current.Format(TEXT("%H:%M"));   
}

//Parse a version string
bool ParseVersion ( CString const& strVer )
{
   //Parse out the various versions
   int nStart = 0;
   g_strMajorVer = strVer.Tokenize(TEXT("."), nStart);
   if (g_strMajorVer.IsEmpty())
      return false;
   g_strMinorVer = strVer.Tokenize(TEXT("."), nStart);
   if (g_strMinorVer.IsEmpty())
      return false;

   g_strPatchVer = strVer.Tokenize(TEXT("."), nStart);
   if (g_strPatchVer.IsEmpty())
      return false;

   g_strBuildVer = strVer.Tokenize(TEXT("."), nStart);
   if (g_strBuildVer.IsEmpty())
      return false;

   //Make sure each one represents a number   
   LPTSTR lpszEnd = NULL;
   ULONG nValue;
   nValue = strtoul(g_strMajorVer, &lpszEnd, 10);
   if (*lpszEnd)
      return false;
   nValue = strtoul(g_strMinorVer, &lpszEnd, 10);
   if (*lpszEnd)
      return false;
   nValue = strtoul(g_strPatchVer, &lpszEnd, 10);
   if (*lpszEnd)
      return false;
   nValue = strtoul(g_strBuildVer, &lpszEnd, 10);
   if (*lpszEnd)
      return false;

   return true;
}

//Parse the parameters and set the appropriate variables
bool ParseParams ( int argc, TCHAR* argv[] )
{
   CString strArg;

   enum State { Option, Param } CurrState = Option;
   TCHAR PrevOption = 0;

   for (int nCount = 1;
        nCount < argc;
        ++nCount)
   {
      strArg = argv[nCount];
      switch(CurrState)
      {
         //Get the option 
         case Option :
         {
			//MODIFIED : MLT - 12/20/02 Don't lower case until we know we are dealing with an option
	        strArg.MakeLower();

            //If this is an option
            if ((TEXT('/') == strArg[0]) || (TEXT('-') == strArg[0]))
            {
               //Can't be too long
               if (strArg.GetLength() > 2)
                  ERROR(TEXT("Invalid parameter - '%s'\n"), strArg);

               switch(strArg[1])
               {
                  //Help
                  case TEXT('h') :
                  {
                     g_bShowHelp = true;
                     return true;
                  };

                  //.NET
                  case TEXT('n') :  //Fall through

                  //Verfile option
                  case TEXT('b') :  //Fall through

                  //Label
                  case TEXT('l') : //Fall through

                  //Build
                  case TEXT('v') : //Fall through

                  //Config file
                  case TEXT('c') :
                  {
                     //Move to the next state to get the parameter
                     PrevOption = strArg[1];
                     CurrState = Param;
                     break;
                  };

                  default :
                  {
                     ERROR(TEXT("Invalid option - '%s'\n"), strArg);
                     break;
                  };
               };
            } else   //Must be the filename
            {
               if (!g_strHeaderFile.IsEmpty())
                  ERROR(TEXT("Header file already set - '%s'\n"), strArg);
               g_strHeaderFile = strArg;
            };
            break;
         };

         //Get the parameter to an option
         case Param :
         {
			//Catch the case where the user left an option off
			if ((strArg[0] == '-') || (strArg[1] == '/'))
				ERROR(TEXT("Missing parameter for option '%c'\n"), PrevOption);
			
            switch(PrevOption)
            {
               //Build
               case TEXT('v') :
               {
                  //Validate it
                  if (!ParseVersion(strArg))
                     ERROR(TEXT("Invalid version number - '%s'\n"), strArg);
                  break;
               };

               //Config file
               case TEXT('c') :
               {
                  g_strConfigFile = strArg;
                  break;
               };

               //Label
               case TEXT('l') :
               {
                  g_strLabel = strArg;
                  break;
               };

               //VB
               case TEXT('b') :
               {
                  g_strVerFile = strArg;
                  break;
               };

			   //.NET
			   case TEXT('n') : 
			   {
			      g_strDotNetFile = strArg;
			      break;
			   };
			   
               default :
               {
                  ERROR(TEXT("Invalid parameter - '%s'\n"), strArg);
                  break;
               };
            };

            PrevOption = TEXT(' ');
            CurrState = Option;
            break;
         };
      };  //Switch
   }; //For

   //If in param state then error
   if (CurrState != Option)
      ERROR(TEXT("Missing parameter - '%c'\n"), PrevOption);

   //If no header file is given then error
   if (g_strHeaderFile.IsEmpty() && g_strDotNetFile.IsEmpty())
      ERROR(TEXT("No output filename given\n"), NULL);

   return true;
}

//Read the configuration file and populate our dictionary
bool ReadConfigFile ( void )
{
   struct ConfigStruct
   {
      LPCTSTR lpszSection;
      LPCTSTR lpszKey;
      LPCTSTR lpszName;

      ConfigStruct(LPCTSTR S, LPCTSTR K, LPCTSTR N) : lpszSection(S), lpszKey(K),
                   lpszName(N)
      {
      };
   } ConfigData[] = 
   {
      //Company information
      ConfigStruct(TEXT("Company"), TEXT("Name"),      TEXT("COMPANY_NAME")),
      ConfigStruct(TEXT("Company"), TEXT("ShortName"), TEXT("COMPANY_NAME_SHORT")),

      //Product information
      ConfigStruct(TEXT("Product"), TEXT("Name"),      TEXT("PRODUCT_NAME")),
      ConfigStruct(TEXT("Product"), TEXT("ShortName"), TEXT("PRODUCT_NAME_SHORT")),
      ConfigStruct(TEXT("Product"), TEXT("Copyright"), TEXT("COPYRIGHT")),
      ConfigStruct(TEXT("Product"), TEXT("Trademark"), TEXT("TRADEMARK")),

      //Version information
      ConfigStruct(TEXT("Version"), TEXT("Major"), TEXT("MAJOR_VER")),
      ConfigStruct(TEXT("Version"), TEXT("Minor"), TEXT("MINOR_VER")),
      ConfigStruct(TEXT("Version"), TEXT("Patch"), TEXT("PATCH_VER")),
      ConfigStruct(TEXT("Version"), TEXT("Build"), TEXT("BUILD_VER"))
   };
   int nConfigCount = sizeof(ConfigData) / sizeof(ConfigData[0]);

   CString strBuffer;

   //Read the standard information
   if (!g_strConfigFile.IsEmpty())
   {
      CStdioFile InFile;
      if (!InFile.Open(g_strConfigFile, CFile::modeRead))
         ERROR(TEXT("Can't open config file - '%s'\n"), g_strConfigFile)
      InFile.Close();
                              
      for (int nIndex = 0;
           nIndex < nConfigCount;
           ++nIndex)
      {
         GetPrivateProfileString(ConfigData[nIndex].lpszSection, 
                                 ConfigData[nIndex].lpszKey, NULL, 
                                 strBuffer.GetBuffer(1024), 1024, g_strConfigFile);
         strBuffer.ReleaseBuffer();
         g_Dictionary[ConfigData[nIndex].lpszName] = strBuffer;
      };
   };

   //Override the build # if needed
   if (!g_strMajorVer.IsEmpty()) 
   {
      g_Dictionary[TEXT("MAJOR_VER")] = g_strMajorVer;
      g_Dictionary[TEXT("MINOR_VER")] = g_strMinorVer; 
      g_Dictionary[TEXT("PATCH_VER")] = g_strPatchVer;
      g_Dictionary[TEXT("BUILD_VER")] = g_strBuildVer;
   } else
   {
      strBuffer = g_Dictionary[TEXT("BUILD_VER")];
      ULONG nValue = _ttol(strBuffer);
      strBuffer.Format(TEXT("%lu"), ++nValue);
      WritePrivateProfileString(TEXT("Version"), TEXT("Build"), strBuffer, g_strConfigFile);

      //Update the dictionary
      g_Dictionary[TEXT("BUILD_VER")] = strBuffer;
   };

   //Build the version string up now
   strBuffer.Format(TEXT("%s.%s.%s.%s"), g_Dictionary[TEXT("MAJOR_VER")],
             g_Dictionary[TEXT("MINOR_VER")], g_Dictionary[TEXT("PATCH_VER")],
             g_Dictionary[TEXT("BUILD_VER")]);
   if (!g_strLabel.IsEmpty())
      strBuffer.AppendFormat(TEXT(" (%s)"), g_strLabel);

   //MODIFIED :MLT - 7/15/02 Add the label as well - even if it is empty
   g_Dictionary[TEXT("LABEL")] = g_strLabel;

   g_Dictionary[TEXT("PRODUCT_VER_STRING")] = strBuffer;
   g_Dictionary[TEXT("FILE_VER_STRING")] = strBuffer;

   //MODIFIED :MLT - 12/14/02 Add the build date and time
   GenerateBuildDateTime();

   return true;
}

int _tmain ( int argc, TCHAR* argv[] )
{
   //Parse parameters
   if (!ParseParams(argc, argv))
   {
      DisplayHelp();
      return -1;
   };

   if (g_bShowHelp)
   {
      DisplayHelp();
      return 0;
   };

   //Read the config file
   if (!ReadConfigFile())
      return -2;

   //Replace all variables in our template file with the
   //dictionary values
   if (!BuildFile())
      return -3;

   //Create the version file, if needed
   if (!CreateVerFile())
      return -4;

   return 0;
}           