#include "cross_p.h"

#include "MacMain.h"

LaunchParamBlockRec launchThis;
	
OSErr RunNetscape();

void LaunchNetscape(const char *url)
	{
	if (RunNetscape())
		return;
		
	AppleEvent event;
	OSErr err;
	
	AEDesc theAddress;
	AEDesc desc;
	
	err = AECreateDesc(typeProcessSerialNumber, (Ptr)&launchThis.launchProcessSN, 
     	sizeof(ProcessSerialNumber), &theAddress);
       
	if (err)
		return;
	
	err = ::AECreateAppleEvent('GURL', 'GURL',
									&theAddress,
									kAutoGenerateReturnID,
									kAnyTransactionID,
									&event);
	if (err)
		return;
	
	err = AECreateDesc(typeChar, url, strlen(url), &desc);
	if (err)
			return;
		
	err = ::AEPutParamDesc(&event,keyDirectObject,&desc);
	if (err)
		return;
		
	err = ::AESend(&event,nil,
		kAECanSwitchLayer + kAEAlwaysInteract +kAENoReply,
		kAENormalPriority,500,nil, nil);
	
	AEDisposeDesc(&theAddress);
	AEDisposeDesc(&desc);
	}

OSErr RunNetscape()
	{
	DTPBRec paramblk;
	
	Str255 applname;
	
	memset(&paramblk, 0, sizeof(paramblk));
	
	OSErr myError;
	
	myError = PBDTGetPath(&paramblk);
	if (myError)
		return myError;
	
	paramblk.ioNamePtr = applname;
	paramblk.ioFileCreator = 'MOSS';
		
	myError = PBDTGetAPPLSync(&paramblk);
	if (myError)
		return myError;	
	
	FSSpec applfspec;
	
	FSMakeFSSpec(0,paramblk.ioAPPLParID, applname, &applfspec);
	
	memset(&launchThis,0,sizeof(launchThis));

 	launchThis.launchBlockID = extendedBlock;
    launchThis.launchEPBLength = extendedBlockLen;
    launchThis.launchFileFlags = nil;
    launchThis.launchControlFlags = launchContinue + launchNoFileFlags;
    launchThis.launchAppSpec = &applfspec;
   	
   	myError = LaunchApplication(&launchThis);
  	return myError;
	}