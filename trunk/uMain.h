//--------------------------------------------------------------------------------------------------
#ifndef uMainH
#define uMainH
//--------------------------------------------------------------------------------------------------

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <mmsystem.h>
#include <Dialogs.hpp>
#include <ExtDlgs.hpp>
#include <ComCtrls.hpp>
#include <vector>
#include <fstream.h>
#include <vcl.h>

//--------------------------------------------------------------------------------------------------

struct TVecPoint {
	double x;
	double y;
};

//--------------------------------------------------------------------------------------------------

#pragma pack(push, 1)

struct TWaveHeader {
	char chunkId[4];
	unsigned int chunkSize;
	char format[4];
	char subChunk1Id[4];
	unsigned int subChunk1Size;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;
	char subChunk2Id[4];
	unsigned int subChunk2Size;
};

#pragma pack(pop)

//--------------------------------------------------------------------------------------------------

using namespace std;

//--------------------------------------------------------------------------------------------------

class TfrmMain : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlDraw;
	TPanel *pnlAxisX;
	TPanel *Panel3;
	TSpeedButton *SpeedButton1;
	TImage *imgMain;
	TImage *imgY;
	TPanel *pnlAxisY;
	TImage *imgX;
	TOpenDialog *openDialog;
	TProgressBar *progress;
	TEdit *ebDuration;
	TEdit *ebCycles;
	TLabel *lblDone;
	TSpeedButton *SpeedButton2;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label1;
	TLabel *Label4;
	TButton *Button1;
	void __fastcall SpeedButton1Click(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall SpeedButton2Click(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
	bool __fastcall LoadFile(AnsiString fileName);
	void __fastcall DecodeLine(AnsiString line, bool relative);
	void __fastcall DrawPoints(void);
	void __fastcall NormalizePoints();
	void __fastcall BreakAxis();
	void __fastcall WriteWaveData();
	void __fastcall WriteWaveFile(AnsiString fileName);
	void __fastcall ProcessFile(AnsiString file);
	vector <TVecPoint> points;
	vector <double> axis_x;
	vector <double> axis_y;
	float sampleRate;
	double duration;
	//int cycles;
    int fileLength;
	//bool calculating;
public:		// User declarations
	__fastcall TfrmMain(TComponent* Owner);
};

//--------------------------------------------------------------------------------------------------

extern PACKAGE TfrmMain *frmMain;

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------
