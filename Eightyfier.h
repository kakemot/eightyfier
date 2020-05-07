#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "DSP_TapeDelay.h"
#include "DSP_StereoDelay.h"
#include "FilterButterworth24db.h"
#include "utilities.h"
#include "verbengine.h"
#include "besselfilter.h"

const int kNumPrograms = 1;

enum EParams
{
  k2hicut = 0,
  k2lowcut = 1,
  k2msDelay1 = 2,
  k2feedback = 3,
  k2pong = 4,
  k2note = 5,
  k2fine = 6,
  k2LFOintensity = 7,
  k2LFOrate = 8,
  k2FltInt = 9,
  k2FltRate = 10,
  k2Smooth = 11,
  k2mix = 12,
  kswitch = 13,
  kunits = 14,
  kwetdry1 = 15,
  kLP1 = 16,
  kHP1 = 17,
  kPan1 = 18,
  kDelay1 = 19,
  kFeedback1 = 20,
  kLevel1 = 21,
  kLP2 = 22,
  kHP2 = 23,
  kPan2 = 24,
  kDelay2 = 25,
  kFeedback2 = 26,
  kLevel2 = 27,
  lowpass = 28,
  moogFilt = 29,
  noise = 30,
  lowcut = 31,
  reverbs = 32,
  kNumParams = 33
};

enum ECtrlTags
{
  kCtrlTagDialogResult = 0,
  kCtrlTagVectorButton,
  kCtrlTagVectorSlider,
  kCtrlTagTabSwitch,
  kCtrlTagRadioButton,
  kCtrlTagScope,
  kCtrlTagMeter,
  kCtrlTags
};

enum ELayout
{
  saturationX = 29,
  saturationY = 50,
  tapeX = 175,
  tapeY = 130,
  fltrX = 35,
  fltrY = 40,
  lfoX = 35,
  lfoY = 130,
  feedbackX = 175,
  feedbackY = 40,
  lowpassX = 315,
  lowpassY = 40,
  lowcutX = 315,
  lowcutY = 130,
  noiseX = 91,
  noiseY = 215,
  reverbX = 200,
  reverbY = 216,
  kKnobFrames = 101,
  sKnobFrames = 127
};

using namespace iplug;
using namespace igraphics;
class Eightyfier : public Plugin
{
public:
  Eightyfier(const InstanceInfo& info);
#if IPLUG_DSP // All DSP methods and member variables should be within an IPLUG_DSP guard, should you want distributed UI
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnParamChange(int paramIdx);
#endif
private:
  TapeDelay* tapeDelay1;
  StereoDelay* stereoDelay;
  CFilterButterworth24db* butter;
  WDL_ReverbAllpass* reverb;

  //the plug-in parameter values
  float tempoUnit;
  float* outl;
  float inl, inr;
  //count samples until nag cutout (No longer used because the plug-in is free).
  int nagCut;
  //cutout multipler
  //can be 0 or 1 and we multiply the output by it
  float cutout;
  /**************************************************
  *********THE ANALOG DELAY**************************/


  float tempo;

  float bottomDelay_lpCut;
  float bottomDelay_hpCut;

  float bottomDelay_feedback;
  float bottomDelay_mix;
  float onOffPong;

  float bottomDelay_tempoNote;
  float currentTempoUnit;

  float bottomDelay_fine;
  float bottomDelay_fudgedFine;

  float bottomDelay_lfoRate;
  float bottomDelay_lfoIntensity;

  float bottomDelay_fltRate;
  float bottomDelay_fltIntensity;

  float bottomDelay_smooth;

  float rackSwitch;

  /**************************************************
  *********THE STEREO DELAY**************************/

  float beatsPerMinute;
  float topDelay_tempoNote;
  float topDelay_wetDry;
  float topDelay_lp1Cutoff, topDelay_lp2Cutoff;
  float topDelay_hp1Cutoff, topDelay_hp2Cutoff;
  float topDelay_pan1, topDelay_pan2;
  float topDelay_delay1, topDelay_delay2;
  float topDelay_fb1, topDelay_fb2;
  float topDelay_level1, topDelay_level2;

  float wet1, dry1;
  float wet2, dry2;

  double hout1 = 0;
  double hout2 = 0;
  double hout3 = 0;
  double hout4 = 0;

  double hin1 = 0;
  double hin2 = 0;
  double hin3 = 0;
  double hin4 = 0;

  double Eightyfier::moog(double input, double fc, double res)
  {
    double f = fc * 1.16;
    double fb = res * (1.0 - 0.15 * f * f);

    input -= hout4 * fb;
    input *= 0.35013 * (f * f) * (f * f);
    hout1 = input + 0.3 * hin1 + (1 - f) * hout1; // Pole 1
    hin1 = input;
    hout2 = hout1 + 0.3 * hin2 + (1 - f) * hout2;  // Pole 2
    hin2 = hout1;
    hout3 = hout2 + 0.3 * hin3 + (1 - f) * hout3;  // Pole 3
    hin3 = hout2;
    hout4 = hout3 + 0.3 * hin4 + (1 - f) * hout4;  // Pole 4
    hin4 = hout3;
    return hout4;
  }

};
