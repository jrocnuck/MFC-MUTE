#ifndef _MFC_MUTE_UTILITIES_
#define _MFC_MUTE_UTILITIES_

// JROC -- disclaimer --> all of this code is ripped from the emule source code.
// I think EMULE is one of the coolest and richest applications out there and
// hope that it's creators will appreciate my paying "homage" to them by applying
// their code to this project and giving them credit for it.


#define _WINVER_NT4_	0x0004
#define _WINVER_95_		0x0004
#define _WINVER_98_		0x0A04
#define _WINVER_ME_		0x5A04
#define _WINVER_2K_		0x0005
#define _WINVER_XP_		0x0105


#define MAKEDLLVERULL(major, minor, build, qfe) \
        (((ULONGLONG)(major) << 48) | \
         ((ULONGLONG)(minor) << 32) | \
         ((ULONGLONG)(build) << 16) | \
         ((ULONGLONG)(  qfe) <<  0))


#define MP_MESSAGE				10102
#define MP_DETAIL				10103
#define MP_ADDFRIEND			10104
#define MP_REMOVEFRIEND			10105
#define MP_SHOWLIST				10106
#define MP_FRIENDSLOT			10107
#define MP_CANCEL				10201
#define MP_STOP					10202
#define MP_RESUME				10204
#define MP_PAUSE				10203
#define	MP_CLEARCOMPLETED		10205
#define	MP_OPEN					10206
#define	MP_PREVIEW				10207

#define MP_PRIOVERYLOW			10300
#define MP_PRIOLOW				10301
#define MP_PRIONORMAL			10302
#define MP_PRIOHIGH				10303
#define MP_PRIOVERYHIGH			10304
#define MP_PRIOAUTO				10317
#define MP_GETED2KLINK			10305
#define MP_GETHTMLED2KLINK		10306
#define	MP_GETSOURCEED2KLINK	10299
#define MP_METINFO				10307
#define MP_PERMALL				10308
#define MP_PERMFRIENDS			10309
#define MP_PERMNONE				10310
#define MP_CONNECTTO			10311
#define MP_REMOVE				10312
#define MP_REMOVEALL			10313
#define MP_REMOVESELECTED		10314
#define MP_UNBAN				10315
#define MP_ADDTOSTATIC			10316
#define MP_CLCOMMAND			10317
#define MP_REMOVEFROMSTATIC		10318
#define MP_VIEWFILECOMMENTS		10319
#define MP_VERSIONCHECK			10320
#define MP_CAT_ADD				10321
#define MP_CAT_EDIT				10322
#define MP_CAT_REMOVE			10323

#define MPG_DELETE				10325
#define	MP_COPYSELECTED			10326
#define	MP_SELECTALL			10327
#define	MP_AUTOSCROLL			10328
#define MP_RESUMENEXT			10329

#define MP_WEBURL				10600

#define MLC_BLEND(A, B, X) ((A + B * (X-1) + ((X+1)/2)) / X)

#define MLC_RGBBLEND(A, B, X) (                   \
	RGB(MLC_BLEND(GetRValue(A), GetRValue(B), X), \
	MLC_BLEND(GetGValue(A), GetGValue(B), X),     \
	MLC_BLEND(GetBValue(A), GetBValue(B), X))     \
)

#define MLC_DT_TEXT (DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS)

#define MLC_IDC_MENU	4875
#define MLC_IDC_UPDATE	(MLC_IDC_MENU - 1)

//a value that's not a multiple of 4 and uncommon
#define MLC_MAGIC 0xFEEBDEEF

//used for very slow assertions
//#define MLC_ASSERT(f)	ASSERT(f)
#define MLC_ASSERT(f)	((void)0)

#ifndef HDM_SETBITMAPMARGIN
#define HDM_SETBITMAPMARGIN	(HDM_FIRST + 20)
#define HDM_GETBITMAPMARGIN	(HDM_FIRST + 21)
#endif



#define	RESSTRIDTYPE		UINT
#define	IDS2RESIDTYPE(id)	id
CString GetResString(RESSTRIDTYPE StringID);


WORD			DetectWinVersion();
int				GetAppImageListColorFlag();
CHAR *			stristr(const CHAR *str1, const CHAR *str2);


//////////////////////////////////////////////////////////////////////////////
// CTempIconLoader

class CTempIconLoader
{
public:
	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	CTempIconLoader(LPCTSTR pszResourceID, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR);
	~CTempIconLoader();

	operator HICON() const{
		return this == NULL ? NULL : m_hIcon;
	}

protected:
	HICON m_hIcon;
};

#endif // _MFC_MUTE_UTILITIES_