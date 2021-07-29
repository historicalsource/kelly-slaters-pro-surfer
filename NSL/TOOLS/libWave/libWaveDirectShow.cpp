#if 0
#include <dshow.h>
#include <qedit.h>
#include <objbase.h>
#include <wxdebug.h>
#include <combase.h>
#include "libWave.h"
#pragma comment(lib,"strmiids")

#ifdef	_DEBUG
#pragma comment(lib,"strmbase")
#endif
#ifdef	NDEBUG
#pragma comment(lib,"strmbasd")
#endif

// directshow wave/bitmap loader
// code: malkia@treyarch.com
// Treyarch Coproration

#define	GRAPHEDIT_DEBUG	0 // 0-disabled, 1-enabled

#if	GRAPHEDIT_DEBUG
static DWORD dwROTReg = 0xfedcba98;
static HRESULT AddToROT(IUnknown *pUnkGraph)
{
    IMoniker * pmk;
    IRunningObjectTable *pirot;
    if (FAILED(GetRunningObjectTable(0, &pirot))) {
        return S_FALSE;
    }
    WCHAR wsz[256];
    wsprintfW(wsz, L"FilterGraph %08x  pid %08x", (DWORD_PTR) 0, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pmk);
    if (SUCCEEDED(hr)) {
        hr = pirot->Register(0, pUnkGraph, pmk, &dwROTReg);
        pmk->Release();
    }
    pirot->Release();
    return hr;
}
static void RemoveFromROT(void)
{
    IRunningObjectTable *pirot;
    if (SUCCEEDED(GetRunningObjectTable(0, &pirot))) {
        pirot->Revoke(dwROTReg);
        pirot->Release();
    }
}
#endif

static int isComActive = 0;
static void ComDone( void )
{
	if( isComActive )
	{
		CoUninitialize();
		isComActive--;
	}
}
static int ComInit( void )
{
	if( !isComActive )
	{
		if( CoInitialize(NULL) != S_OK )
			return S_FALSE;
		atexit(ComDone);
		isComActive++;
	}
	return S_OK;
}

#define	GETREF(x)	((x)->AddRef(),(x)->Release())

static int GetRenderer( IGraphBuilder* pGraph, IBaseFilter** _filter, IPin** _pin )
{
	IPin *pin;
    IEnumFilters *pEnum;
    IBaseFilter *pFilter;
	IEnumPins *enumPins;
	PIN_INFO pinInfo;
	ULONG fetched;
	int n_inputPins;
	int n_outputPins;

	int ref;

    if(pGraph->EnumFilters(&pEnum)==S_OK)
	{
		while(pEnum->Next(1,&pFilter,NULL)==S_OK)
		{
			n_inputPins = 0;
			n_outputPins = 0;

			ref = GETREF(pFilter);
			if(pFilter->EnumPins(&enumPins)==S_OK)
			{
				ref = GETREF(pFilter);
				ref = GETREF(pFilter);
				ref = GETREF(enumPins);
				while(enumPins->Next(1,&pin,&fetched)==S_OK)
				{
					if( pin->QueryPinInfo(&pinInfo) == S_OK )
						pinInfo.dir == PINDIR_INPUT ? n_inputPins++ : n_outputPins++;
					pin->Release();
				}
				ref = GETREF(enumPins);
				enumPins->Release();
			}
			ref = GETREF(pFilter);

			if( n_inputPins == 1 && n_outputPins == 0 )
			{
				if(pFilter->EnumPins(&enumPins)==S_OK)
				{
					while(enumPins->Next(1,&pin,&fetched)== S_OK )
					{
						if( pin->QueryPinInfo(&pinInfo) == S_OK && pinInfo.dir == PINDIR_INPUT )
						{
							*_pin = pin;
							*_filter = pFilter;
							enumPins->Release();
							pEnum->Release();
							return S_OK;
						}
						pin->Release();
					}
					enumPins->Release();
				}
			}	 
			pFilter->Release();
		}
		pEnum->Release();
	}
	return S_FALSE;
}

static int GetInputOutput( IBaseFilter *pFilter, IPin** _input, IPin** _output )
{
	if( pFilter )
	{
		int n_inputPins;
		int n_outputPins;
		IEnumPins *enumPins;
		PIN_INFO pinInfo;
		IPin *pin;
		ULONG fetched;

		n_inputPins = 0;
		n_outputPins = 0;
		if(pFilter->EnumPins(&enumPins)==S_OK)
		{
			while(enumPins->Next(1, &pin, &fetched) == S_OK )
			{
				if( pin->QueryPinInfo(&pinInfo) == S_OK )
					pinInfo.dir == PINDIR_INPUT ? n_inputPins++ : n_outputPins++;
				pin->Release();
			}
			enumPins->Release();
		}

		if( n_inputPins == 1 && n_outputPins == 1 )
		{
			if(pFilter->EnumPins(&enumPins)==S_OK)
			{
				while(enumPins->Next(1, &pin, &fetched)==S_OK)
				{
					if(pin->QueryPinInfo(&pinInfo)==S_OK)
					{
						if( pin->QueryPinInfo(&pinInfo) == S_OK )
							pinInfo.dir == PINDIR_INPUT ? *_input = pin : *_output = pin;
						else
							pin->Release();
					}
				}
				enumPins->Release();
				return S_OK;
			}
		}
	}
	return S_FALSE;
}

static int GetInput( IBaseFilter *pFilter, IPin** _input )
{
	if( pFilter )
	{
		int n_inputPins;
		int n_outputPins;
		IEnumPins *enumPins;
		PIN_INFO pinInfo;
		IPin *pin;
		unsigned long fetched;

		n_inputPins = 0;
		n_outputPins = 0;
		if(pFilter->EnumPins(&enumPins)==S_OK)
		{
			while(enumPins->Next(1, &pin, &fetched) == S_OK )
			{
				if( pin->QueryPinInfo(&pinInfo) == S_OK )
					pinInfo.dir == PINDIR_INPUT ? n_inputPins++ : n_outputPins++;
				pin->Release();
			}
			enumPins->Release();
		}

		if( n_inputPins == 1 && n_outputPins == 0 )
		{
			if(pFilter->EnumPins(&enumPins)==S_OK)
			{
				while(enumPins->Next(1, &pin, &fetched)==S_OK)
				{
					if(pin->QueryPinInfo(&pinInfo)==S_OK && pinInfo.dir == PINDIR_INPUT )
					{
						*_input = pin;
						enumPins->Release();
						return S_OK;
					}
					else
					{
						pin->Release();
					}
				}
				enumPins->Release();
			}
		}
	}
	return 0;
}

static void	SetNoSyncSource( IGraphBuilder* pGraph )
{
	ULONG fetched;
	IEnumFilters* pEnum;
	IBaseFilter* pFilter;

	if(pGraph->EnumFilters(&pEnum)==S_OK)
	{
		while(pEnum->Next(1,&pFilter,&fetched)==S_OK)
		{
			IReferenceClock* pRefClock;
			if(pFilter->GetSyncSource(&pRefClock)==S_OK && pRefClock)
			{
				pRefClock->Release();
			}
			pFilter->SetSyncSource(NULL);
			pFilter->Release();
		}
		pEnum->Release();
	}
}

// Boooo! - what a lazy coder I am - this one taken directly from samples
class CGrabCB: public CUnknown, public ISampleGrabberCB
{
public:
    DECLARE_IUNKNOWN;

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
    {
        if( riid == IID_ISampleGrabberCB )
        {
            return GetInterface((ISampleGrabberCB*)this, ppv);
        }
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }

    // ISampleGrabberCB methods
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
    {
		BYTE *p;
		p = (BYTE *)WAVE_MemoryRealloc( waveData, waveSize + BufferLen );
		if( p )
		{
			waveData = p;
			memcpy( waveData+waveSize, pBuffer, BufferLen );
			waveSize += BufferLen;
			waveData = p;
			return S_OK;
		}
        return E_NOTIMPL;
    }
    
    // Constructor
    CGrabCB( ) : CUnknown("SGCB", NULL)
    { waveData = NULL, waveSize = 0; }

public:
	BYTE *waveData;
	int waveSize;
};

#ifdef	FAILED
#undef	FAILED
#endif
#ifdef	RELEASE
#undef	RELEASE
#endif

#define	FAILED(dummy,x)	((x)!=S_OK)
#define	RELEASE(x)		if(x) (x)->Release();

// This function is rather slow - because it always rebuilds the media graph
// Use it in case where your main routine fails, and you want to try this one

static HRESULT DSHOW_Load( const char *filename, AM_MEDIA_TYPE *mt, void **data, int *size )
{
	HRESULT			hr;
	CGrabCB			cb;
	long			evCode;
	WCHAR			wPath[MAX_PATH]			= L"";
	WAVEFORMATEX*	wfx						= NULL;
    IGraphBuilder*	pGraph					= NULL;
	IBaseFilter*	pRenderer				= NULL;
	ISampleGrabber*	pSampleGrabber			= NULL;
	IMediaPosition*	pMediaPosition			= NULL;
	IMediaControl*	pMediaControl			= NULL;
	IMediaEvent*	pMediaEvent				= NULL;
	IBaseFilter*	pSampleGrabberFilter	= NULL;
	IBaseFilter*	pNullRenderer			= NULL;
	IPin*			pSampleGrabberInput		= NULL;
	IPin*			pSampleGrabberOutput	= NULL;
	IPin*			pRendererInput			= NULL;
	IPin*			pNullRendererInput		= NULL;
	IPin*			pSourceOutput			= NULL;

	MultiByteToWideChar(CP_ACP, 0, filename, -1, wPath, MAX_PATH);

	//	1.	Call CoInitialize (once per application start, we have atexit() to close it at exit) (may be speedup plus?!??!)
	//	2.	Create empty Graph capable of doing automatic graph connections (interface - IGraphBuilder)
	//	3.	Build automatically graph - read more here - how it's done - 
	if(	FAILED(1,hr=ComInit())
	||	FAILED(2,hr=CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(LPVOID *)&pGraph))
	||	FAILED(3,hr=pGraph->RenderFile(wPath,NULL))
	)	goto DSHOW_waveLoad_FAILED;

	//	If you have GRAPHEDIT running - you can capture your current GRAPH state (go to Connect in MainMenu)
	//	GraphEdit won't be capable of showing anything (will hang) if you currently have stopped using Debugger
	//	You can put some getch() after AddToROT(pGraph) and then call it, This happens because GRAPHEDIT actually
	//	Calls pGraph object which is LOCALLY loaded (INPROC_SERVER), but if debugger have stopped the application then
	//	graph is also stopped - so GraphEdit can't communicate
	#if	GRAPHEDIT_DEBUG
	AddToROT(pGraph);
	//getch();
	#endif

	//	1.	Get Audio Renderer in pRenderer (probably DirectSound device) and get it's input -> pRendererInput
	//	2.	Get where Renderer Input Pin is connected (probably to WAVE PARSER or ACM convertor DEVICE) -> pSourceOutput
	//	3.	Get what media type is the Source Output (must be AUDIO/PCM valid Khz, stereo/mono, 8/16 bit)
	//	4.	Disconnect the Source Output from Renderer Input (DSound device)
	//	5.	Disconnect the Renderer Input from Source Output (probably not needed - but for any case)
	//	6.	Remove Renderer (DSound device) from the graph, later we'll replace it with SampleGrabber and NullRenderer
	//	7.	Create the Sample Grabber filter which will be used to GRAB all data
	//	8	GetInputOutput() checks if filter has exactly ONE INPUT and ONE OUTPUT and if it's ok - returns them
	//	9.	Get ISampleGrabber interface for SampleGrabber filter, with it we can setup it.
	//	10.	Create the Null Renderer device (it's used to TAP the graph - otherwise Graph will not run)
	//	11.	GetInput() check if filter has exactly ONE INPUT and if it's ok returns it
	//	12.	Setups mediatype, found in the SourceOutput for SampleGrabber
	//	13.	Here i want BufferCB callback to be called not SampleCB (that's why there is 1 (not 0) as last argument)
	//	14.	Setup callback interface/class for SampleGrabber
	//	15.	Add the SampleGrabberFilter into graph
	//	16.	Add	the NullRendererFilter into graph
	//	17.	Connect	SourceOutput to SampleGrabberInput
	//	18.	Connect	SampleGrabberOutput	to NullRenderer to tap it
	//	19	Get MediaControl interface to be able to RUN the filter graph
	//	20.	Get MediaEvent interface to be able to know when filter graph is finished his work

	if(	FAILED( 1,hr=GetRenderer(pGraph,&pRenderer,&pRendererInput))
	||	FAILED( 2,hr=pRendererInput->ConnectedTo(&pSourceOutput))
	||	FAILED( 3,hr=pSourceOutput->ConnectionMediaType(mt))
	||	FAILED( 4,hr=pSourceOutput->Disconnect())
	||	FAILED( 5,hr=pRendererInput->Disconnect())
	||	FAILED( 6,hr=pGraph->RemoveFilter(pRenderer))
	||	FAILED( 7,hr=CoCreateInstance(CLSID_SampleGrabber,NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(LPVOID *)&pSampleGrabberFilter))
	||	FAILED( 8,hr=GetInputOutput(pSampleGrabberFilter,&pSampleGrabberInput,&pSampleGrabberOutput))
	||	FAILED( 9,hr=pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber,(void**)&pSampleGrabber))
	||	FAILED(10,hr=CoCreateInstance(CLSID_NullRenderer, NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(LPVOID *)&pNullRenderer))
	||	FAILED(11,hr=GetInput(pNullRenderer,&pNullRendererInput))
	||	FAILED(12,hr=pSampleGrabber->SetMediaType(mt))
	||	FAILED(13,hr=pSampleGrabber->SetBufferSamples(TRUE))
	||	FAILED(14,hr=pSampleGrabber->SetCallback(&cb,1))
	||	FAILED(15,hr=pGraph->AddFilter(pSampleGrabberFilter,L"DSHOW_Load:SampleGrabber"))
	||	FAILED(16,hr=pGraph->AddFilter(pNullRenderer,L"DSHOW_Load:NullRenderer"))
		// Eventually here I'll add the 3rd party Directx plugin support
	||	FAILED(17,hr=pSourceOutput->Connect(pSampleGrabberInput,mt))
	||	FAILED(18,hr=pSampleGrabberOutput->Connect(pNullRendererInput,mt))
	||	FAILED(19,hr=pGraph->QueryInterface(IID_IMediaControl,(void**)&pMediaControl))
	||	FAILED(20,hr=pGraph->QueryInterface(IID_IMediaEvent,(void**)&pMediaEvent))
	)	goto DSHOW_waveLoad_FAILED;

	//	Get MediaPosition interface (if have one) 
	pGraph->QueryInterface(IID_IMediaPosition,(void**)&pMediaPosition);

	//	1.	If we have IMediaPosition(Seeking) interface then we want to get samples one by one
	//		Otherwise we want samples at once. This is kind of solution for STILL (jpeg) IMAGES
	//		If we want to get all samples at once for JPEG images then we don't know when to stop!
	if(FAILED(1,hr=pSampleGrabber->SetOneShot(pMediaPosition?TRUE:FALSE)))
		goto DSHOW_waveLoad_FAILED;

//	SetNoSyncSource( pGraph );

	//	Loop must run at least once if we don't have IMediaPosition - then we'll get all samples at once
	//	If we have IMediaPosition we'll get samples step-by-step and we'll check MediaPosition for timing
	//	This handles JPEG images.
	do
	{
		// Get sample by sample from the graph, wait for all filters
		while( (hr=pMediaControl->Run()) == S_FALSE )
		{
			// Here we can go, if graph was just started - but some of its filters are still not ready
			int waiting=1;
			while( waiting )
			{
				OAFilterState pfs;
				// wait each time 1 millisecond while the filters get ready,
				switch( pMediaControl->GetState(1,&pfs) )
				{
					case S_OK:
						waiting=0;
						break;

					case VFW_S_STATE_INTERMEDIATE:
						break;

					default:
						goto DSHOW_waveLoad_FAILED;
				}
			}
		}
		// If last Run() failed other than S_FALSE then we fail too
		if( hr!=S_OK )
			goto DSHOW_waveLoad_FAILED;

		//	1. Wait graph to complete rendering current sample
		if( FAILED(1,pMediaEvent->WaitForCompletion(INFINITE,&evCode)) )
			goto DSHOW_waveLoad_FAILED;

		if( evCode != EC_COMPLETE )
		{
			hr = S_FALSE;
			goto DSHOW_waveLoad_FAILED;
		}

		if( pMediaPosition )
		{
			REFTIME durTime, curTime, stopTime, prerollTime;
			double rate;

			pMediaPosition->get_Duration( &durTime );
			pMediaPosition->get_CurrentPosition( &curTime );
			pMediaPosition->get_StopTime( &stopTime );
			pMediaPosition->get_PrerollTime( &prerollTime );
			pMediaPosition->get_Rate( &rate );
			if( curTime >= stopTime )
				break;
		}
	} while( pMediaPosition );

	pMediaControl->StopWhenReady();

	// At the end - return WAVEFORMATEX to the user, I wonder if JPEG was loaded
	// what will happen :) :) :) (proably DIBINFO????)
	if( data ) *data = cb.waveData;
	if( size ) *size = cb.waveSize;

DSHOW_waveLoad_FAILED:;
	RELEASE(pMediaPosition);
	RELEASE(pMediaControl);
	RELEASE(pMediaEvent);
	RELEASE(pSourceOutput);
	RELEASE(pSampleGrabber);
	RELEASE(pSampleGrabberInput); 
	RELEASE(pSampleGrabberOutput); 
	RELEASE(pSampleGrabberFilter);
	RELEASE(pNullRendererInput); 
	RELEASE(pNullRenderer); 
	RELEASE(pRendererInput);
	RELEASE(pRenderer);
	RELEASE(pGraph); 
	return hr;
}

WAVEFORMATEX* WIN32DSHOW_WaveLoad( const char *filename, void **data, int *size )
{
	WAVE_ReportHeader( "WIN32DSHOW_WaveLoad(%s)", filename );

	WAVEFORMATEX* wfx = NULL;
	AM_MEDIA_TYPE mt  = {0};

	if( DSHOW_Load( filename, &mt, data, size )			== S_OK 
	&&	IsEqualGUID(mt.formattype, FORMAT_WaveFormatEx)	== TRUE
	&&	(wfx=(WAVEFORMATEX *)WAVE_MemoryCalloc(1,mt.cbFormat))		!= NULL )
		memcpy(wfx, mt.pbFormat, mt.cbFormat);

	return wfx;
}

VIDEOINFOHEADER* WIN32DSHOW_BitmapLoad( char *filename, void **data, int *size )
{
	VIDEOINFOHEADER* vih = NULL;
	AM_MEDIA_TYPE	 mt  = {0};

	if( DSHOW_Load( filename, &mt, data, size )			== S_OK 
	&&	IsEqualGUID(mt.formattype, FORMAT_VideoInfo)	== TRUE
	&&	(vih=(VIDEOINFOHEADER *)WAVE_MemoryCalloc(1,mt.cbFormat))	!= NULL )
		memcpy(vih, mt.pbFormat, mt.cbFormat);

	return vih;
}
#endif