//--------------------------------------------------------------------------------------------------


#pragma hdrstop

#include "uMain.h"

//--------------------------------------------------------------------------------------------------

#pragma resource "*.dfm"
TfrmMain *frmMain;

//--------------------------------------------------------------------------------------------------

__fastcall TfrmMain::TfrmMain(TComponent* Owner)
	: TForm(Owner)
{
}

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::SpeedButton1Click(TObject *Sender)
{
	if (openDialog->Execute()) {
		ProcessFile(openDialog->FileName);
	}
}

//--------------------------------------------------------------------------------------------------

bool __fastcall TfrmMain::LoadFile(AnsiString fileName)
{
	TStringList * file = new TStringList;

	if (FileExists(fileName) == false) {
		ShowMessage("Erro ao carregar o arquivi:\n" + fileName);
		return false;
	}

	openDialog->FileName = fileName;

	file->LoadFromFile(fileName);

	if (file->Count < 1) {
		return false;
	}

	AnsiString line;
	do {
		line = file->Strings[0];
		file->Delete(0);
	} while ((file->Count > 0) && (line.Pos("<path") < 1));

	if (file->Count < 1) {
		return false;
	}

	line = file->Strings[0];
	line = line.LowerCase();
	bool relative = true;
	int p = line.Pos("d=\"m ");
	if (p == 0) {
		p = line.Pos("d=\"M ");
		relative = false;
	}
	while (p == 0)  {
		file->Delete(0);
		line = file->Strings[0];
		p = line.Pos("d=\"m ");
		relative = true;
		if (p == 0) {
			p = line.Pos("d=\"M ");
			relative = false;
		}
	}

	if (p > 0) {
		line = line.Delete(1, p + 4);
	}

	if (line.Pos("\"") == 0) {
		return false;
	}

	line = line.Delete(line.Pos("\""), line.Length());

	DecodeLine(line, relative);

	NormalizePoints();

	BreakAxis();

	DrawPoints();

	delete file;

	Application->ProcessMessages();

	return true;
}

//--------------------------------------------------------------------------------------------------

/*void __fastcall TfrmMain::BreakAxis()
{
	double totalx = 0;
	axis_x.clear();
	axis_y.clear();

	int psize = points.size()-1;

	for (int i = 0; i < psize; i++) {
		totalx += abs((long)(points.at(i).x - points.at(i+1).x));
	}

	double lowerDeltaT = 10000000;
	double last_x = 0;

	for (int i = 0; i < psize; i++) {
		double x1, x2, y1, y2;
		x1 = points.at(i).x;
		x2 = points.at(i+1).x;
		y1 = points.at(i).y;
		y2 = points.at(i+1).y;

		double dnp;
		if (x2>x1) dnp = x2-x1; else dnp = x1-x2;

		//int np = (dnp/totalx + 1) * double(sampleRate) / totalx+1;
		int np = (double(duration) * dnp / totalx) + 1;// * double(sampleRate) + 1;
		for (int j = 1; j <= np; j++) {
			double r = double(j) / double(np);
			double y = - (y1 + (y2 - y1) * r);
			double x = (x1 + (x2 - x1) * r);  // x vira negativo qdo vira y, pois o eixo y eh invertido
			axis_x.push_back(x);
			axis_y.push_back(y);

			if ((x - last_x) < lowerDeltaT) {
				lowerDeltaT = x - last_x;
			}

			last_x = x;
		}
	}

	double d1 = axis_y.size();
	double d2 = sampleRate;
	double d3 = cycles;
	int t = (d1 / d2);
	lblPointCount->Caption = IntToStr(t);
	lblSmallerDeltaX->Caption = FloatToStr(lowerDeltaT * 1000) + " ms";
}  */

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::BreakAxis()
{
	double totalx = 0;
	axis_x.clear();
	axis_y.clear();

	int psize = points.size()-1;

	// totalx holds the full length (sum of all "x" segments)
	for (int i = 0; i < psize; i++) {
		double delta;
		if (points.at(i).x > points.at(i+1).x) {
			delta = points.at(i).x - points.at(i+1).x;
		} else {
			delta = points.at(i+1).x - points.at(i).x;
		}
		totalx += delta;
	}

	// "times" hold all "x" values converted to real-time
	vector <double> times;
	times.push_back(0.0);
	for (int i = 0; i < psize; i++) {
		double delta;
		if (points.at(i).x > points.at(i+1).x) {
			delta = points.at(i).x - points.at(i+1).x;
		} else {
			delta = points.at(i+1).x - points.at(i).x;
		}
		double time = (delta/totalx) * duration + times.at(i);
		times.push_back(time);
	}

	unsigned int numberOfPoints = sampleRate * duration;
	int idx = 0;
	double timeStep = 1.0 / sampleRate;
	double time = 0.0;
	double grad = 0.0;
	double x1 = points.at(idx).x;
	double x2 = points.at(idx+1).x ;
	double y1 = points.at(idx).y;
	double y2 = points.at(idx+1).y;
	double lim_inf = times.at(idx);
	double lim_sup = times.at(idx+1);
	double x, y;
	int k = times.size();
	for (unsigned int i = 0; i < numberOfPoints; i++) {
		if (abs(lim_sup - lim_inf) < 0.00001) {
			grad = 0;
		} else {
			grad = ((time - lim_inf) / (lim_sup - lim_inf));
		}
		double delta = y2-y1;
		if (abs(delta) > 0.00001) {
			y = y1 + grad * delta;
		} else {
			y = y1;
		}
		delta = x2-x1;
		if (abs(delta) > 0.00001) {
			x = x1 + grad * delta;
		} else {
			x = x1;
		}
		//double x = points.at(idx + 1).x + grad * (points.at(idx + 1).x - points.at(i).x);

		axis_x.push_back(x);
		//axis_y.push_back(-(y));
		axis_y.push_back(y);

		time = time + timeStep;
		if (time > lim_sup) {
			idx++;
			if (idx >= psize) {
				if (i < numberOfPoints - 10) {
					ShowMessage("Ops! Deu pau na interpolacao!");
				}
				break;
			}
			x1 = points.at(idx).x;
			x2 = points.at(idx+1).x ;
			y1 = points.at(idx).y;
			y2 = points.at(idx+1).y;
			lim_inf = times.at(idx);
			lim_sup = times.at(idx+1);
		}
	}

	/*double d1 = axis_y.size();
	double d2 = sampleRate;
	double d3 = cycles;
	int t = (d1 / d2);
	lblPointCount->Caption = IntToStr(t);
	lblSmallerDeltaX->Caption = FloatToStrF(timeStep * 1000, ffFixed, 5, 3) + " ms";*/
}

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::NormalizePoints()
{
	double minx = 10000000;
	double maxx = -10000000;
	double miny = 10000000;
	double maxy = -10000000;

	for (unsigned int i = 0; i < points.size(); i++) {
		if (points.at(i).x > maxx) {
			maxx = points.at(i).x;
		}
		if (points.at(i).y > maxy) {
			maxy = points.at(i).y;
		}
		if (points.at(i).x < minx) {
			minx = points.at(i).x;
		}
		if (points.at(i).y < miny) {
			miny = points.at(i).y;
		}
	}

	double w = maxx - minx;
	double h = maxy - miny;
	double zerox = w / 2;
	double zeroy = h / 2;

	for (unsigned int i = 0; i < points.size(); i++) {
		double x = points.at(i).x;
		double y = points.at(i).y;
		x = (x - minx) * 80 / w - 50 + 10;
		y = (y - miny) * 80 / h - 50 + 10;
		points.at(i).x = x;
		points.at(i).y = y;
	}
}

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::DecodeLine(AnsiString line, bool relative)
{
	points.clear();
	int p = line.Pos(" ");
	bool first = true;
	TVecPoint last = {0, 0};
	while (line.Length() > 0) {
		AnsiString mline = line.SubString(1, p-1);
		line = line.Delete(1, mline.Length() + 1);

		int pv = mline.Pos(",");

		if (pv > 0) {
			AnsiString xs = mline.SubString(0, pv-1);
			AnsiString ys = mline.SubString(pv+1, mline.Length());

			TVecPoint point;
			try {
				point.x = xs.ToDouble();
				point.y = ys.ToDouble();
			} catch (...) {
				point.x = 0;
				point.y = 0;
			}

			if (relative) {
				point.x += last.x;
				point.y += last.y;
			}
			memcpy(&last, &point, sizeof(TVecPoint));

			points.push_back(point);
		} else {
			if (mline == "z") {
				points.push_back(points.at(0));
			}

			if (mline == "l") {
				relative = true;
			}

			if (mline.UpperCase() == mline) {
				relative = false;
			}
		}

		p = line.Pos(" ");
		if (p == 0) {
			p = line.Length();
		}
		first = false;
	}
}

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::DrawPoints(void)
{

	double w = pnlDraw->Width;
	double h = pnlDraw->Height;
	TCanvas * can = imgMain->Picture->Bitmap->Canvas;
	imgMain->Picture->Bitmap->Width = w;
	imgMain->Picture->Bitmap->Height = h;
	can->Pen->Color = clBlack;
	can->Brush->Color = clBlack;
	can->FillRect(TRect(0, 0, w, h));

	can->Pen->Color = clGreen;
	can->MoveTo(0, h/2);
	can->LineTo(w, h/2);
	can->MoveTo(w/2, 0);
	can->LineTo(w/2, h);
	can->Pen->Color = clLime;
	double x, y;
	if (points.size() > 0) {
		x = points.at(0).x;
		x = x * w/100 + w/2;
		y = points.at(0).y;
		y = y * h/100 + h/2;
		can->MoveTo(x, y);
		for (unsigned int i = 1; i < points.size(); i++) {
			x = points.at(i).x;
			x = x * w/100 + w/2;
			y = points.at(i).y;
			y = y * h/100 + h/2;
			can->LineTo(x, y);
		}
	}

	// axis y
	w = pnlAxisY->Width;
	h = pnlAxisY->Height;
	can = imgY->Picture->Bitmap->Canvas;
	imgY->Picture->Bitmap->Width = w;
	imgY->Picture->Bitmap->Height = h;
	can->Pen->Color = clBlack;
	can->Brush->Color = clBlack;
	can->FillRect(TRect(0, 0, w, h));

	can->Pen->Color = clGreen;
	can->MoveTo(0, h/2);
	can->LineTo(w, h/2);
	can->MoveTo(w/2, 0);
	can->LineTo(w/2, h);
	can->Pen->Color = clLime;
	if (axis_y.size() > 0) {
		y = axis_y.at(0);
		y = y * h/200 + h/2;
		x = 0;
		can->MoveTo(x, y);
		for (unsigned int i = 1; i < axis_y.size(); i++) {
			y = axis_y.at(i);
			y = /*h - */(y * h/200 + h/2);
			x = i * w / axis_y.size();
			can->LineTo(x, y);
		}
	}

	// axis x
	w = pnlAxisX->Width;
	h = pnlAxisX->Height;
	can = imgX->Picture->Bitmap->Canvas;
	imgX->Picture->Bitmap->Width = w;
	imgX->Picture->Bitmap->Height = h;
	can->Pen->Color = clBlack;
	can->Brush->Color = clBlack;
	can->FillRect(TRect(0, 0, w, h));

	can->Pen->Color = clGreen;
	can->MoveTo(0, h/2);
	can->LineTo(w, h/2);
	can->MoveTo(w/2, 0);
	can->LineTo(w/2, h);
	can->Pen->Color = clLime;
	if (axis_x.size() > 0) {
		y = axis_x.at(0);
		y = y * h/200 + h/2;
		x = 0;
		can->MoveTo(x, y);
		for (unsigned int i = 1; i < axis_x.size(); i++) {
			y = axis_x.at(i);
			y = h - (y * h/200 + h/2);
			x = i * w / axis_x.size();
			can->LineTo(x, y);
		}
	}
}

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::FormCreate(TObject *Sender)
{
	DecimalSeparator = '.';
	sampleRate = 48000;//StrToInt(ebSampleRate->Text);
	duration = StrToFloat(ebDuration->Text);
	fileLength = /*cycles = */StrToInt(ebCycles->Text);

}

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::WriteWaveFile(AnsiString fileName)
{
	unsigned int numSamples = 0;
	unsigned int dataSize = 0;
	TWaveHeader header;

	memcpy(header.chunkId, "RIFF", 4);
	memcpy(header.format, "WAVE", 4);

	memcpy(header.subChunk1Id, "fmt ", 4);
	header.subChunk1Size = 16;
	header.audioFormat = 1;
	header.numChannels = 2;
	header.sampleRate = sampleRate;
	header.bitsPerSample = 16;
	header.byteRate  = header.sampleRate * header.numChannels * (header.bitsPerSample/8);
	header.blockAlign = header.numChannels * (header.bitsPerSample/8);

	memcpy(header.subChunk2Id, "data", 4);

	char buffer[100];
	ofstream file (fileName.c_str(), ios::out | ios::binary);

	int sz = sizeof(TWaveHeader);
	file.write ((char *)&header, sz);
	int total = axis_y.size();
	int cycles = fileLength / duration;
	int pTotal = cycles * total;
	int prog = 0;
	for (int j = 0; j < cycles; j++) {
		for (int i = 0; i < total; i++) {
			double vy = axis_y.at(i);
			double vx = axis_x.at(i);
			short sample[2];
			sample[0] = (short) (vy / 100 * 32767);
			sample[1] = (short) (vx / 100 * 32767);
			file.write((char *)sample, 4);
			numSamples ++;
			dataSize += 4;

			prog++;
			int p = prog * 100 / pTotal;
			if (progress->Position != p) {
				progress->Position = p;
			}
		}
	}
	file.seekp(0);
	header.subChunk2Size = numSamples * header.numChannels * header.bitsPerSample/8;
	header.chunkSize = 28 + 8 +  dataSize;
	file.write ((char *)&header, sizeof(TWaveHeader));
	file.close();
	progress->Position = 0;
}

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::FormResize(TObject *Sender)
{
	DrawPoints();
	Application->ProcessMessages();
}

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::FormActivate(TObject *Sender)
{
	if (FileExists(ParamStr(1)))  {
		ProcessFile(ParamStr(1));
	}
}

//--------------------------------------------------------------------------------------------------

void __fastcall TfrmMain::ProcessFile(AnsiString file)
{
	lblDone->Caption = "Processing...";
	Application->ProcessMessages();

	//sampleRate = StrToInt(ebSampleRate->Text);
	duration = StrToFloat(ebDuration->Text);
	fileLength = StrToInt(ebCycles->Text);

	//ebSampleRate->Text = IntToStr((int)sampleRate);
	ebDuration->Text = FloatToStr(duration);
	ebCycles->Text = IntToStr(fileLength);

	if (LoadFile(file)) {
		AnsiString wave = file;
		wave = wave.Delete(wave.Length() - 3, 4) + ".wav";
		WriteWaveFile(wave);
	} else {
		ShowMessage("Deu pau");
	}

	lblDone->Caption = "Done";
	Application->ProcessMessages();
}

//--------------------------------------------------------------------------------------------------







void __fastcall TfrmMain::SpeedButton2Click(TObject *Sender)
{
	ProcessFile(openDialog->FileName);
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::Button1Click(TObject *Sender)
{
	AnsiString cmd = "http://dalpix.com/rabiscoscopio";

	ShellExecuteA(Handle, "open", cmd.c_str(), NULL, NULL, SW_SHOWNORMAL);
//	WinExec(cmd.c_str(), SW_SHOW);
}
//---------------------------------------------------------------------------

