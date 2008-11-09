/******************************************************************************

 @File         PVRShell.h

 @Title        

 @Copyright    Copyright (C) 2005 - 2008 by Imagination Technologies Limited.

 @Platform     s, PVRShell takes a set of command-line arguments which allow things like the position and size of the demo

 @Description  Makes programming for 3D APIs easier by wrapping surface
               initialization, Texture allocation and other functions for use by a demo.

******************************************************************************/
#ifndef __PVRSHELL_H_
#define __PVRSHELL_H_

// Uncomment to enable the -fps command-line option
// #define PVRSHELL_FPS_OUTPUT

/*****************************************************************************
** Includes
*****************************************************************************/
#include <stdlib.h>

#define EXIT_NOERR_CODE 0
#define EXIT_ERR_CODE (!EXIT_NOERR_CODE)

/*!***********************************************************************
 *	Keyboard mapping.
 ************************************************************************/
enum PVRShellKeyName
{
	PVRShellKeyNameNull,
	PVRShellKeyNameQUIT,
	PVRShellKeyNameSELECT,
	PVRShellKeyNameACTION1,
	PVRShellKeyNameACTION2,
	PVRShellKeyNameUP,
	PVRShellKeyNameDOWN,
	PVRShellKeyNameLEFT,
	PVRShellKeyNameRIGHT,
	PVRShellKeyNameScreenshot
};
/*!***********************************************************************
 * @Enum prefNameBoolEnum
 * @Brief Boolean Shell preferences.
 ************************************************************************/
enum prefNameBoolEnum
{
	prefFullScreen,				/*!< Set to: 1 for full-screen rendering; 0 for windowed */
	prefIsRotated,				/*!< Query this to learn whether screen is rotated */
	prefPBufferContext,			/*!< 1 if you need pbuffer support (default is pbuffer not needed) */
	prefPixmapContext,			/*!< 1 to use a pixmap as a render-target (default off) */
	prefPixmapDisableCopy,		/*!< 1 to disable the copy if pixmaps are used */
	prefZbufferContext,			/*!< 1 if you wish to have zbuffer support (default to on) */
	prefLockableBackBuffer,		/*!< DX9 only: true to use D3DPRESENTFLAG_LOCKABLE_BACKBUFFER (default: false) */
	prefSoftwareRendering,		/*!< 1 to select software rendering (default: off, i.e. use hardware) */
	prefStencilBufferContext,	/*!< 1 if you wish to have stencil support (default: off) */
	prefOpenVGContext,			/*!< EGL only: 1 to initialize OpenVG instead of OpenGL ES (default: off) */
	prefAlphaFormatPre,			/*!< EGL only: 1 to create the EGL surface with EGL_ALPHA_FORMAT_PRE (default: 0) */
	prefPowerSaving,			/*!< If true then the app will go into powersaving mode (if available) when not in use. */
#ifdef PVRSHELL_FPS_OUTPUT
    prefOutputFPS,				/*!< If true then the FPS are output using PVRShellOutputdebug */
#endif
	prefOutputInfo				/*!< If true then the app will output helpful information such as colour buffer format via PVRShellOutputDebug. */
};

/*!***********************************************************************
 * @Enum prefNameFloatEnum
 * @Brief Float Shell preferences.
 ************************************************************************/
enum prefNameFloatEnum
{
	prefQuitAfterTime			/*!< Shell will quit after this number of seconds (-1 to disable) */
};

/*!***********************************************************************
 * @Enum prefNameIntEnum
 * @Brief Integer Shell preferences.
 ************************************************************************/
enum prefNameIntEnum
{
	prefEGLMajorVersion,	/*!< EGL: returns the major version as returned by eglInitialize() */
	prefEGLMinorVersion,	/*!< EGL: returns the minor version as returned by eglInitialize() */
	prefWidth,				/*!< Width of render target */
	prefHeight,				/*!< Height of render target */
	prefPositionX,			/*!< X position of the window */
	prefPositionY,			/*!< Y position of the window */
	prefQuitAfterFrame,		/*!< Shell will quit after this number of frames (-1 to disable) */
	prefSwapInterval,		/*!< 0 to preventing waiting for monitor vertical syncs */
	prefInitRepeats,		/*!< Number of times to reinitialise (if >0 when app returns false from RenderScene(), shell will ReleaseView(), InitView() then re-enter RenderScene() loop). Decrements on each initialisation. */
	prefFSAAMode,			/*!< Set to: 0 to disable full-screen anti-aliasing; 1 for 2x; 2 for 4x. */
	prefCommandLineOptNum	/*!< Returns the length of the array returned by prefCommandLineOpts */
};

/*!***********************************************************************
 * @Enum prefNamePtrEnum
 * @Brief Pointers/Handlers Shell preferences.
 ************************************************************************/
enum prefNamePtrEnum
{
	prefD3DDevice,			/*!< D3D: returns the device pointer */
	prefEGLDisplay,			/*!< EGL: returns the EGLDisplay */
	prefEGLSurface,			/*|< EGL: returns the EGLSurface */
	prefHINSTANCE,			/*!< Windows: returns the application instance handle */
	prefNativeWindowType,	/*!< Returns the window handle */
	prefGameSystemServices, /*!< NGAGE: returns the game system services pointer */
	prefAccelerometer		/*!< Accelerometer values */
};

/*!***********************************************************************
 * @Enum prefNameConstPtrEnum
 * @Brief Constant pointers Shell preferences.
 ************************************************************************/
enum prefNameConstPtrEnum
{
	prefAppName,			/*!< ptrValue is char* */
	prefReadPath,			/*!< ptrValue is char*; will include a trailing slash */
	prefWritePath,			/*!< ptrValue is char*; will include a trailing slash */
	prefCommandLine,		/*!< used to retrieve the entire application command line */
	prefCommandLineOpts,	/*!< ptrValue is SCmdLineOpt*; retrieves an array of arg/value pairs (parsed from the command line) */
	prefExitMessage,		/*!< ptrValue is char*; gives the shell a message to show on exit, typically an error */
	prefVersion				/*!< ptrValue is char* */
};

/****************************************************************************
 PVRShell implementation Prototypes and definitions
*****************************************************************************/

struct PVRShellData;

/*!***************************************************************************
 * @Class PVRShellInit
 *****************************************************************************/
class PVRShellInit;

/*!***********************************************************************
 *	@Struct SCmdLineOpt
 *	@Brief Stores a variable name/value pair for an individual command-line option.
 ************************************************************************/
struct SCmdLineOpt
{
	const char *pArg, *pVal;
};


class PVREventHandler
{
public:
	virtual void KeyUp(int keycode,int time)=0;
	virtual void KeyDown(int keycode,int time)=0;
	virtual void KeyChar(int character, int time)=0;
	virtual void MouseMove(int x, int y, int time)=0;
	virtual void MouseDown(int button, int time)=0;
	virtual void MouseUp(int button, int time)=0;
};



/*!***************************************************************************
 * @Class PVRShell
 * @Brief Inherited by the application; responsible for abstracting the OS and API.
 * @Description
 *  PVRShell is the main Shell class that an application uses. An
 *  application should supply a class which inherits PVRShell and supplies
 *  implementations of the virtual functions of PVRShell (InitApplication(),
 *  QuitApplication(), InitView(), ReleaseView(), RenderScene()). Default stub
 *  functions are supplied; this means that an application is not
 *  required to supply a particular function if it does not need to do anything
 *  in it.
 *  The other, non-virtual, functions of PVRShell are utility functions that the
 *  application may call.
 *****************************************************************************/
class PVRShell
{
protected:
	friend class PVRShellInitOS;
	friend class PVRShellInit;

	PVRShellData	*m_pShellData;
	PVRShellInit	*m_pShellInit;

	bool setSwapInterval(const int i32Value);

public:
	/*!***********************************************************************
	@Function			PVRShell
	@Description		Constructor
	*************************************************************************/
	PVRShell();

	/*!***********************************************************************
	@Function			~PVRShell
	@Description		Destructor
	*************************************************************************/
	virtual ~PVRShell();

	/*
		PVRShell functions that the application should implement.
	*/

	/*!***********************************************************************
	 @Function		InitApplication
	 @Return		true for success, false to exit the application
	 @Description	This function can be overloaded by the application. It
	 				will be called by PVRShell once only at the beginning of
	 				the PVRShell WinMain()/main() function. This function
	 				enables the user to perform any initialisation before the
	 				render API is initialised. From this function the user can
	 				call PVRShellSet() to change default values, e.g.
	 				requesting a particular resolution or device setting.
	*************************************************************************/
	virtual bool InitApplication() { return true; };

	/*!***********************************************************************
	 @Function		QuitApplication
	 @Return		true for success, false to exit the application
	 @Description	This function can be overloaded by the application. It
	 				will be called by PVRShell just before finishing the
	 				program. It enables the application to release any
	 				memory/resources acquired in InitApplication().
	*************************************************************************/
	virtual bool QuitApplication() { return true; };

	/*!***********************************************************************
	 @Function		InitView
	 @Return		true for success, false to exit the application
	 @Description	This function can be overloaded by the application. It
	 				will be called by PVRShell after the OS and rendering API
	 				are initialised, before entering the RenderScene() loop.
	 				It is called any time the rendering API is initialised,
	 				i.e. once at the beginning, and possibly again if the
	 				resolution changes, or a power management even occurs, or
	 				if the app requests a reinialisation.
	 				The application should check here the configuration of
	 				the rendering API; it is possible that requests made in
	 				InitApplication() were not successful.
					Since everything is initialised when this function is
					called, you can load textures and perform rendering API
					functions.
	*************************************************************************/
	virtual bool InitView() { return true; };

	/*!***********************************************************************
	 @Function		ReleaseView
	 @Return		true for success, false to exit the application
	 @Description	This function can be overloaded by the application. It
	 				will be called after the RenderScene() loop, before
	 				shutting down the render API. It enables the application
	 				to release any memory/resources acquired in InitView().
	*************************************************************************/
	virtual bool ReleaseView() {  return true; };

	/*!***********************************************************************
	 @Function		RenderScene
	 @Return		true for success, false to exit the application
	 @Description	This function can be overloaded by the application.
					It is main application function in which you have to do your own rendering.  Will be
					called repeatedly until the application exits.
	*************************************************************************/
	virtual bool RenderScene() { return true; };

	/*
		PVRShell functions available for the application to use.
	*/

	/*!***********************************************************************
	 @Function		PVRShellSet
	 @Input			prefName				Name of preference to set to value
	 @Input			value					Value
	 @Return		true for success
	 @Description	This function is used to pass preferences to the PVRShell.
					If used, it must be called from InitApplication().
	*************************************************************************/
	bool PVRShellSet(const prefNameBoolEnum prefName, const bool value);

	/*!***********************************************************************
	 @Function		PVRShellSet
	 @Input			prefName				Name of preference to set to value
	 @Input			value					Value
	 @Return		true for success
	 @Description	This function is used to pass preferences to the PVRShell.
					If used, it must be called from InitApplication().
	*************************************************************************/
	bool PVRShellSet(const prefNameFloatEnum prefName, const float value);

	/*!***********************************************************************
	 @Function		PVRShellSet
	 @Input			prefName				Name of preference to set to value
	 @Input			value					Value
	 @Return		true for success
	 @Description	This function is used to pass preferences to the PVRShell.
					If used, it must be called from InitApplication().
	*************************************************************************/
	bool PVRShellSet(const prefNameIntEnum prefName, const int value);

	/*!***********************************************************************
	 @Function		PVRShellSet
	 @Input			prefName				Name of preference to set to value
	 @Input			ptrValue				Value
	 @Return		true for success
	 @Description	This function is used to pass preferences to the PVRShell.
					If used, it must be called from InitApplication().
	*************************************************************************/
	bool PVRShellSet(const prefNamePtrEnum prefName, const void * const ptrValue);

	/*!***********************************************************************
	 @Function		PVRShellSet
	 @Input			prefName				Name of preference to set to value
	 @Input			ptrValue				Value
	 @Return		true for success
	 @Description	This function is used to pass preferences to the PVRShell.
					If used, it must be called from InitApplication().
	*************************************************************************/
	bool PVRShellSet(const prefNameConstPtrEnum prefName, const void * const ptrValue);

	/*!***********************************************************************
	 @Function		PVRShellGet
	 @Input			prefName				Name of preference to set to value
	 @Return		Value asked for.
	 @Description	This function is used to get parameters from the PVRShell
					It can be called from any where in the program.
	*************************************************************************/
	bool PVRShellGet(const prefNameBoolEnum prefName) const;

	/*!***********************************************************************
	@Function		PVRShellGet
	@Input			prefName				Name of preference to set to value
	@Return			Value asked for.
	@Description	This function is used to get parameters from the PVRShell
					It can be called from any where in the program.
	*************************************************************************/
	float PVRShellGet(const prefNameFloatEnum prefName) const;

	/*!***********************************************************************
	@Function		PVRShellGet
	@Input			prefName				Name of preference to set to value
	@Return			Value asked for.
	@Description	This function is used to get parameters from the PVRShell
					It can be called from any where in the program.
	*************************************************************************/
	int PVRShellGet(const prefNameIntEnum prefName) const;

	/*!***********************************************************************
	@Function		PVRShellGet
	@Input			prefName				Name of preference to set to value
	@Return			Value asked for.
	@Description	This function is used to get parameters from the PVRShell
					It can be called from any where in the program.
	*************************************************************************/
	void *PVRShellGet(const prefNamePtrEnum prefName) const;

	/*!***********************************************************************
	@Function		PVRShellGet
	@Input			prefName				Name of preference to set to value
	@Return			Value asked for.
	@Description	This function is used to get parameters from the PVRShell
					It can be called from any where in the program.
	*************************************************************************/
	const void *PVRShellGet(const prefNameConstPtrEnum prefName) const;

	/*!***********************************************************************
	 @Function		PVRShellScreenCaptureBuffer
	 @Input			Width			size of image to capture (relative to 0,0)
	 @Input			Height			size of image to capture (relative to 0,0)
	 @Modified		pLines			receives a pointer to an area of memory containing the screen buffer.
	 @Return		true for success
	 @Description	It will be stored as 24-bit per pixel, 8-bit per chanel RGB. The
	 				memory should be freed with free() when no longer needed.
	*************************************************************************/
	bool PVRShellScreenCaptureBuffer(const int Width, const int Height, unsigned char **pLines);

	/*!***********************************************************************
	 @Function		PVRShellScreenSave
	 @Input			fname			base of file to save screen to
	 @Output		ofname			If non-NULL, receives the filename actually used
	 @Modified		pLines			image data to write out (24bpp, 8-bit per channel RGB)
	 @Return		true for success
	 @Description	Writes out the image data to a BMP file with basename
	 				fname. The file written will be fname suffixed with a
	 				number to make the file unique.
	 				For example, if fname is "abc", this function will attempt
	 				to save to "abc0000.bmp"; if that file already exists, it
	 				will try "abc0001.bmp", repeating until a new filename is
	 				found. The final filename used is returned in ofname.
	*************************************************************************/
	int PVRShellScreenSave(
		const char			* const fname,
		const unsigned char	* const pLines,
		char				* const ofname = NULL);

	/*!***********************************************************************
	 @Function		PVRShellWriteBMPFile
	 @Input			pszFilename		file to save screen to
	 @Input			uWidth			the width of the data
	 @Input			uHeight			the height of the data
	 @Input			pImageData		image data to write out (24bpp, 8-bit per channel RGB)
	 @Return		0 on success
	 @Description	Writes out the image data to a BMP file with name fname.
	*************************************************************************/
	int PVRShellWriteBMPFile(
		const char			* const pszFilename,
		const unsigned long	uWidth,
		const unsigned long	uHeight,
		const void			* const pImageData);

	/*!***********************************************************************
	@Function		PVRShellOutputDebug
	@Input			format			printf style format followed by arguments it requires
	@Description	Writes the resultant string to the debug output (e.g. using
	printf(), OutputDebugString(), ...). Check the SDK release notes for
	details on how the string is output.
	*************************************************************************/
	void PVRShellOutputDebug(char const * const format, ...) const;

	/*!***********************************************************************
	 @Function		PVRShellGetTime
	 @Returns		A value which increments once per millisecond.
	 @Description	The number itself should be considered meaningless; an
	 				application should use this function to determine how much
	 				time has passed between two points (e.g. between each
	 				frame).
	*************************************************************************/
	unsigned long PVRShellGetTime();

	/*!***********************************************************************
	 @Function		PVRShellIsKeyPressed
	 @Input			key		Code of the key to test
	 @Return		true if key was pressed
	 @Description	Check if a key was pressed. The keys on various devices
	 are mapped to the PVRShell-supported keys (listed in @a PVRShellKeyName) in
	 a platform-dependent manner, since most platforms have different input
	 devices. Check the SDK release notes for details on how the enum values
	 map to your device's input device.
	*************************************************************************/
	bool PVRShellIsKeyPressed(const PVRShellKeyName key);

	void SetEventHandler(PVREventHandler *pkh);
	PVREventHandler *GetEventHandler();

};

/****************************************************************************
** Declarations for functions that the scene file must supply
****************************************************************************/

/*!***************************************************************************
 @Function		NewDemo
 @Return		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application
*****************************************************************************/
PVRShell* NewDemo();

#endif /* __PVRSHELL_H_ */

/*****************************************************************************
 End of file (PVRShell.h)
*****************************************************************************/
