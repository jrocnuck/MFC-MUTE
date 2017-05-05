/*
 * Modification History
 *
 * 2004-January-18   Jason Rohrer
 * Created.
 * Changed to leak one sizer and properly destroy another sizer.
 */


/**
 * Test app for memory leaks in wxWindows.
 */



// includes and definitions copied from wxWindows sample calendar app

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation "TestLeaks.cpp"
    #pragma interface "TestLeaks.cpp"
#endif


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/sizer.h"
#endif



/**
 * Entry point for the app.
 */
class TestLeaks : public wxApp {

    public:

        // override the default init function
        virtual bool OnInit();

        // override the default exit function
        virtual int OnExit();
    };



// set GuiApp as the app to launch when the program starts
IMPLEMENT_APP( TestLeaks );



bool TestLeaks::OnInit() {

    // construct a sizer which we will NOT destroy
    wxBoxSizer *leakedSizer = new wxBoxSizer( wxVERTICAL );

    // construct a sizer which we WILL destroy
    wxBoxSizer *nonLeakedSizer = new wxBoxSizer( wxVERTICAL );

    
    // destroy one of the sizers
    delete nonLeakedSizer;

    
    // return false to force the app to exit immediately
    return false;
    }



int TestLeaks::OnExit() {

    return 0;
    }


 
