#include "Eightyfier.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

Eightyfier::Eightyfier(const InstanceInfo& info)
  : Plugin(info, MakeConfig(kNumParams, kNumPrograms))
{
  
#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  tapeDelay1 = new TapeDelay(44100);
  stereoDelay = new StereoDelay(44100);
  butter = new CFilterButterworth24db();
  reverb = new WDL_ReverbAllpass();
  reverb->setsize(2000);
  reverb->setfeedback(0.1);
  

  //Initial mixing levels (will be reset by preset loading anyway).
  dry1 = dry2 = 0.0f;
  wet1 = wet2 = 0.0f;

  //Used for tempo syncing.

  tempoUnit = 1.0 / 5.0;

  bottomDelay_smooth = 0.9995;
  bottomDelay_lfoRate = bottomDelay_lfoIntensity = 0.0f;
  bottomDelay_fltIntensity = 0.0;
  bottomDelay_fltRate = 0.0f;
  bottomDelay_fine = 0.2;
  bottomDelay_fudgedFine = 0.0;
  currentTempoUnit = 0.0;
  bottomDelay_tempoNote = 0.5;
  bottomDelay_lpCut = 0.0f;
  bottomDelay_hpCut = 1.0f;
  bottomDelay_mix = 0.5f;
  bottomDelay_feedback = 0.5;
  rackSwitch = 1.0f;
  beatsPerMinute = 0.0f;

  topDelay_tempoNote = 0.0f;
  topDelay_wetDry = 0.0f;
  topDelay_lp1Cutoff = topDelay_lp2Cutoff = 0.0f;
  topDelay_hp1Cutoff = topDelay_hp2Cutoff = 0.0f;
  topDelay_pan1 = topDelay_pan2 = 0.0f;
  topDelay_delay1 = topDelay_delay2 = 0.0f;
  topDelay_fb1 = topDelay_fb2 = 0.0f;
  topDelay_level1 = topDelay_level2 = 0.0f;
  onOffPong = 0.0;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(lowpass)->InitDouble("Low pass", 0, 0, 0.9, 0.01, "lowpass");
  GetParam(lowcut)->InitDouble("Low cut", 0.0, 0.0, 1, 0.01, "%");
  GetParam(k2hicut)->InitDouble("2hicut", 80.0, 0., 100.0, 0.01, "%");
  GetParam(k2lowcut)->InitDouble("2lowcut", 21., 0., 100.0, 0.01, "%");
  GetParam(k2msDelay1)->InitDouble("2msDelay1", 50., 0., 100.0, 0.01, "%");
  GetParam(k2feedback)->InitDouble("2feedback", 50., 0., 100.0, 0.01, "%");
  GetParam(k2pong)->InitDouble("2pong", 50., 0., 100.0, 0.01, "%");
  GetParam(k2note)->InitDouble("2note", 65., 0., 100.0, 0.01, "%");
  GetParam(k2fine)->InitDouble("2fine", 40., 0., 100.0, 0.01, "%");
  GetParam(k2LFOintensity)->InitDouble("2LFO intensity", 8., 0., 100.0, 0.01, "%");
  GetParam(k2LFOrate)->InitDouble("2LFO rate", 80., 0., 100.0, 0.01, "%");
  GetParam(k2FltInt)->InitDouble("2Flt Int", 20., 0., 100.0, 0.01, "%");
  GetParam(k2FltRate)->InitDouble("2Flt Rate", 40., 0., 100.0, 0.01, "%");
  GetParam(k2Smooth)->InitDouble("2Smooth", 20., 0., 100.0, 0.01, "%");
  GetParam(k2mix)->InitDouble("2mix", 0., 0., 60, 0.01, "%");
  GetParam(kswitch)->InitDouble("switch", 100., 0., 100.0, 0.01, "%");
  GetParam(kunits)->InitDouble("topDelay_tempoNote", 24., 0., 100.0, 0.01, "%");
  //GetParam(kwetdry1)->InitDouble("topDelay_wetDry", 50., 0., 100.0, 0.01, "%");
  //GetParam(kLP1)->InitDouble("LP1", 50., 0., 100.0, 0.01, "%");
  //GetParam(kHP1)->InitDouble("HP1", 50., 0., 100.0, 0.01, "%");
  //GetParam(kPan1)->InitDouble("Pan1", 50., 0., 100.0, 0.01, "%");
  //GetParam(kDelay1)->InitDouble("Delay1", 50., 0., 100.0, 0.01, "%");
  //GetParam(kFeedback1)->InitDouble("Feedback1", 50., 0., 100.0, 0.01, "%");
  //GetParam(kLevel1)->InitDouble("Level1", 50., 0., 100.0, 0.01, "%");
  GetParam(kLP2)->InitDouble("LP2", 100., 0., 100.0, 0.01, "%");
  GetParam(kHP2)->InitDouble("HP2", 38., 0., 100.0, 0.01, "%");
  GetParam(kPan2)->InitDouble("Pan2", 50., 0., 100.0, 0.01, "%");
  GetParam(kDelay2)->InitDouble("Delay2", 27., 0., 100.0, 0.01, "%");
  GetParam(kFeedback2)->InitDouble("Feedback2", 20., 0., 70.0, 0.01, "%");
  GetParam(kLevel2)->InitDouble("Level2", 47., 0., 100.0, 0.01, "%");
  GetParam(reverbs)->InitDouble("Reverb", 0., 0., 100.0, 0.01, "%");
  //GetParam(moogFilt)->InitDouble("Moog", 0.0, 0.0, 100.0, 0.01, "%");
  GetParam(noise)->InitDouble("Noise", 0.0, 0.0, 100.0, 0.01, "%");
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);

    pGraphics->AttachBackground(BACKGROUND_FN);
    //IBitmap knob = pGraphics->LoadBitmap(KNOB_FN, 101);
    IBitmap smallknob = pGraphics->LoadBitmap(SMALLKNOB_FN, 127);
    IBitmap tinyknob = pGraphics->LoadBitmap(TINYKNOB_FN, 101);
    IBitmap slider = pGraphics->LoadBitmap(SLIDER_FN, 31);
    IRECT b = pGraphics->GetBounds().GetAltered(150, 224, 23, 29);
    const int nRows = 5;
    const int nCols = 8;
    int cellIdx = -1;
    auto nextCell = [&]() {
      return b.GetGridCell(++cellIdx, nRows, nCols).GetPadded(-5.);
    };
    
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    //const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new IBKnobControl(tapeX, tapeY, smallknob, k2mix));
    pGraphics->AttachControl(new IBKnobControl(fltrX, fltrY, smallknob, k2FltInt));
    pGraphics->AttachControl(new IBKnobControl(lfoX, lfoY, smallknob, k2LFOrate));
    pGraphics->AttachControl(new IBKnobControl(feedbackX, feedbackY, smallknob, kFeedback2));
    pGraphics->AttachControl(new IBKnobControl(lowpassX, lowpassY, smallknob, lowpass));
    pGraphics->AttachControl(new IBKnobControl(lowcutX, lowcutY, smallknob, lowcut));
    pGraphics->AttachControl(new IBKnobControl(noiseX, noiseY, tinyknob, noise));
    pGraphics->AttachControl(new IBKnobControl(reverbX, reverbY, tinyknob, reverbs));
    //pGraphics->AttachControl(new IVSliderControl(nextCell().GetCentredInside(110.), noise, "-", DEFAULT_STYLE, true, EDirection::Horizontal), kCtrlTagVectorSlider, "vcontrols");
    
    /*
    MakePresetFromNamedParams((char*)"Bizzare Delay", 27,
      kunits, 24.29,
      kwetdry1, 47.0,
      kLP1, 100.0,
      kHP1, 38.61,
      kPan1, 40.0,
      kDelay1, 16.36,
      kFeedback1, 75.0,
      kLevel1, 47.0,
      kLP2, 100.0,
      kHP2, 38.61,
      kPan2, 50.0,
      kDelay2, 27.27,
      kFeedback2, 75.0,
      kLevel2, 47.0,
      k2fine, 40.13,
      k2feedback, 15.0,
      k2note, 65.66,
      k2mix, 50.0,
      k2hicut, 100.0,
      k2lowcut, 21.68,
      k2FltRate, 40.0,
      k2FltInt, 80.0,
      k2LFOintensity, 30.0,
      k2LFOrate, 80.0,
      kswitch, 100.0,
      k2Smooth, 39.47,
      k2pong, 100.0);

    MakePresetFromNamedParams((char*)"Madcap", 27,
      kunits, 24.29,
      kwetdry1, 47.0,
      kLP1, 100.0,
      kHP1, 38.61,
      kPan1, 50.0,
      kDelay1, 16.36,
      kFeedback1, 100.0,
      kLevel1, 47.0,
      kLP2, 100.0,
      kHP2, 38.61,
      kPan2, 50.0,
      kDelay2, 27.27,
      kFeedback2, 100.0,
      kLevel2, 47.0,
      k2fine, 40.13,
      k2feedback, 69.0,
      k2note, 65.66,
      k2mix, 50.0,
      k2hicut, 100.0,
      k2lowcut, 21.68,
      k2FltRate, 0.0,
      k2FltInt, 0.0,
      k2LFOintensity, 0.0,
      k2LFOrate, 0.0,
      kswitch, 0.0,
      k2Smooth, 39.47,
      k2pong, 100.0);

    MakePresetFromNamedParams((char*)"Dissipate", 27,
      kunits, 100.0,
      kwetdry1, 47.0,
      kLP1, 81.0,
      kHP1, 38.61,
      kPan1, 0.0,
      kDelay1, 20.0,
      kFeedback1, 100.0,
      kLevel1, 75.0,
      kLP2, 81.0,
      kHP2, 38.61,
      kPan2, 50.0,
      kDelay2, 20.0,
      kFeedback2, 100.0,
      kLevel2, 75.0,
      k2fine, 40.13,
      k2feedback, 67.0,
      k2note, 100.0,
      k2mix, 50.0,
      k2hicut, 100.0,
      k2lowcut, 30.0,
      k2FltRate, 0.0,
      k2FltInt, 0.0,
      k2LFOintensity, 100.0,
      k2LFOrate, 16.5,
      kswitch, 0.0,
      k2Smooth, 39.47,
      k2pong, 100.0);
    
    RestorePreset(1);
    tempo = 1.0;
    */
  };
#endif
  
}

#if IPLUG_DSP
void Eightyfier::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  double* in1 = inputs[0];
  double* in2 = inputs[1];
  double* out1 = outputs[0];
  double* out2 = outputs[1];

  float* tapenoise;
  double lp1;
  double lp2;
  double hp1;
  double hp2;
  bool set = false;
  float c;
  float r = 0.2;
  double b0 = 0;
  double b1 = 0;
  double b2 = 0;
  double b3 = 0;
  double b4 = 0;
  double b5 = 0;
  double b6 = 0;
  float white;
  float pink;

  int tempo = GetTempo();
  const double lp = GetParam(lowpass)->Value();
  const double lc = GetParam(lowcut)->Value();
  const double noiselevel = GetParam(noise)->Value()/10000;
  const double reverbmix = GetParam(reverbs)->Value() / 100;
  butter->SetSampleRate(GetSampleRate());
 
  /* Why are we doing this next part here? I couldn't find a function that informs the plug-in
  directly about host tempo changing, so here we need to check if the host tempo is changing
  to set the right tempo values.  Maybe there is a better way. */

  /*The strange hack requested by Bizzare that snaps to 1.0 fine delay smoothly*/

  bottomDelay_fudgedFine = 0.66666666667 + bottomDelay_fine * 0.83333333334;
  float intUnits = (int)(topDelay_tempoNote * 15) + 1;
  float intDelay1 = (int)(topDelay_delay1 * 15) + 1;
  float intDelay2 = (int)(topDelay_delay2 * 15) + 1;

  if ((currentTempoUnit > 2 || tempo > 60))
  {
    float snapFine = bottomDelay_fudgedFine;
    if (snapFine >= 0.99 && snapFine <= 1.01)
      snapFine = 1.0;
    tapeDelay1->setDelayMS(((240000.0f * (1.0 / currentTempoUnit)) * snapFine / tempo));
  }

  //If the tempo is smaller than 60BPM don't even bother to set the delay time.
  //It would take up too much memory and is an unlikely usecase.
  if (tempo > 60)
  {
    stereoDelay->setDelayMS1((240000.0f * (1.0f / intUnits / 4.0f)) * intDelay1 / tempo);
    stereoDelay->setDelayMS2((240000.0f * (1.0f / intUnits / 4.0f)) * intDelay2 / tempo);
  }

  //go through the samples and process them.
  for (int i = 0; i < nFrames; ++i, ++in1, ++in2, ++out1, ++out2)
  {
    inl = (*in1);
    inr = (*in2);
    white = ((rand() % 20) - 10) * *in1;
    b0 = 0.99886 * b0 + white * 0.0555179;
    b1 = 0.99332 * b1 + white * 0.0750759;
    b2 = 0.96900 * b2 + white * 0.1538520;
    b3 = 0.86650 * b3 + white * 0.3104856;
    b4 = 0.55000 * b4 + white * 0.5329522;
    b5 = -0.7616 * b5 - white * 0.0168980;
    pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362;
    b6 = white * 0.115926;

    outl = tapeDelay1->process(((inl + inr)/2) + pink * noiselevel);
   
    (*out1) = (outl[0] * wet2 + inl * dry2);
    (*out2) = (outl[0] * wet2 + inr * dry2);
    butter->Set(lp, 0.1);

    lp1 = butter->Run(*out1);
    lp2 = butter->Run(*out2);

    *out1 = (*out1 * (1 - lp)) + lp1 * (lp);
    *out2 = (*out2 * (1 - lp)) + lp2 * (lp);

    hp1 = *out1 - lp1;
    hp2 = *out2 - lp2;

    *out1 = (*out1 * (1 - lc)) + hp1 * (lc);
    *out2 = (*out2 * (1 - lc)) + hp2 * (lc);

    *out1 = (*out1 * (1 - reverbmix)) + reverb->process(*out1) * (reverbmix);
    *out2 = (*out2 * (1 - reverbmix)) + reverb->process(*out2) * (reverbmix);
  }

}
void Eightyfier::OnParamChange(int paramIdx) {
  char* writeToDisplay = new char[30];

  //Lowpass cutoff.
  if (paramIdx == EParams::k2hicut)
  {
    bottomDelay_lpCut = GetParam(paramIdx)->Value() / 100.0;
    tapeDelay1->setLPCutoff(bottomDelay_lpCut);
    sprintf(writeToDisplay, "%4.2f", bottomDelay_lpCut * 20.0);
  }
  else
    if (paramIdx == EParams::k2lowcut) //Highpass Cutoff
    {
      bottomDelay_hpCut = GetParam(paramIdx)->Value() / 100.0;
      tapeDelay1->setHPCutoff(bottomDelay_hpCut);
      sprintf(writeToDisplay, "%4.2f", bottomDelay_hpCut * 20.0);
    }
    else //OBSOLETE PARAMETER
      if (paramIdx == 2)
      {
        //OBSOLETE
      }
      else
        if (paramIdx == EParams::k2feedback) //Delay bottomDelay_feedback (for distorting delay)
        {
          bottomDelay_feedback = GetParam(paramIdx)->Value() / 100.0;
          tapeDelay1->setFeedback(bottomDelay_feedback * 1.6);
          sprintf(writeToDisplay, "%4.2f", bottomDelay_feedback * 100.0);
        }
        else
          if (paramIdx == EParams::k2pong) //Turn on or off the ping-pong function for the distorting delay.
          {
            onOffPong = GetParam(paramIdx)->Value() / 100.0;
            if (onOffPong < 0.5)
              tapeDelay1->p_pong = false;
            else
              tapeDelay1->p_pong = true;
          }
          else
            if (paramIdx == EParams::k2note) //Set the tempo-synced delay time tempoUnit.
            {
              bottomDelay_tempoNote = GetParam(paramIdx)->Value() / 100.0;

              float tempoUnit = 1.0 / 5.0;
              float fin = 0;

              if (bottomDelay_tempoNote < tempoUnit)
              {
                fin = 1;
                currentTempoUnit = 1;
              }

              if (bottomDelay_tempoNote >= tempoUnit && bottomDelay_tempoNote < tempoUnit * 2)
              {
                fin = 2;
                currentTempoUnit = 2;
              }

              if (bottomDelay_tempoNote >= tempoUnit * 2 && bottomDelay_tempoNote < tempoUnit * 3)
              {
                fin = 4;
                currentTempoUnit = 4;
              }

              if (bottomDelay_tempoNote >= tempoUnit * 3 && bottomDelay_tempoNote <= 4)
              {
                fin = 8;
                currentTempoUnit = 8;
              }

              if (bottomDelay_tempoNote >= tempoUnit * 4 && bottomDelay_tempoNote <= 1)
              {
                fin = 16;
                currentTempoUnit = 16;
              }

              sprintf(writeToDisplay, "%4.1f", fin);
            }
            else
              if (paramIdx == EParams::k2fine) //Set the fine delay time value.
              {

                bottomDelay_fine = GetParam(paramIdx)->Value() / 100.0;
                sprintf(writeToDisplay, "%4.2f", (0.66666666667 + bottomDelay_fine * 0.83333333334) / 2.0f);

              }
              else
                if (paramIdx == EParams::k2LFOintensity) //Set LFO intensity
                {

                  bottomDelay_lfoIntensity = GetParam(paramIdx)->Value() / 100.0;;;
                  tapeDelay1->setLfoIntensity(bottomDelay_lfoIntensity);
                  sprintf(writeToDisplay, "%4.2f", bottomDelay_lfoIntensity);
                }
                else
                  if (paramIdx == EParams::k2LFOrate) //Set the LFO rate
                  {
                    bottomDelay_lfoRate = GetParam(paramIdx)->Value() / 100.0;;
                    tapeDelay1->setLfoRate((bottomDelay_lfoRate * 0.8f + 0.2f) * 20.0f);
                    sprintf(writeToDisplay, "%4.2f", bottomDelay_lfoRate);
                  }
                  else
                    if (paramIdx == EParams::k2FltInt) //Set the Flutter intensity.
                    {

                      bottomDelay_fltIntensity = GetParam(paramIdx)->Value() / 100.0;;
                      tapeDelay1->setFltIntensity(bottomDelay_fltIntensity);
                      sprintf(writeToDisplay, "%4.2f", bottomDelay_fltIntensity);
                    }
                    else
                      if (paramIdx == EParams::k2FltRate) //Set the flutter rate.
                      {
                        bottomDelay_fltRate = GetParam(paramIdx)->Value() / 100.0;;
                        tapeDelay1->setFltRate((bottomDelay_fltRate * 0.2 + 0.8) * 50);
                        sprintf(writeToDisplay, "%4.2f", bottomDelay_fltRate);
                      }
                      else
                        if (paramIdx == EParams::k2Smooth) //Set the smoothing amount for the fine delay time knob.
                        {

                          bottomDelay_smooth = GetParam(paramIdx)->Value() / 100.0;;
                          tapeDelay1->setSmooth(bottomDelay_smooth * 0.0095 + 0.99);

                        }
                        else
                          if (paramIdx == EParams::k2mix) //Set wet/dry value.
                          {
                            char* ptr;
                            bottomDelay_mix = GetParam(paramIdx)->Value() / 100.0;;
                            if (bottomDelay_mix < 0.5)
                            {
                              wet2 = bottomDelay_mix * 2.0f;
                              dry2 = 1.0f;
                            }

                            if (bottomDelay_mix >= 0.5)
                            {
                              dry2 = 2.0f - ((bottomDelay_mix - 0.5f) * 2.0f);
                              dry2 = 1.0f - ((bottomDelay_mix - 0.5f) * 2.0f);
                              wet2 = 1.0f;
                            }
                            sprintf(writeToDisplay, "%4.2f", bottomDelay_mix);
                          }
                          else
                            if (paramIdx == EParams::kswitch) //Switches between the two delay topDelay_tempoNote.
                            {
                              rackSwitch = GetParam(paramIdx)->Value() / 100.0;
                            }
                            else

                              //STEREO STARTS HERE
                              //UNITS
                              if (paramIdx == EParams::kunits) //Set the tempo-synced delay topDelay_tempoNote for the stereo delay.
                              {
                                topDelay_tempoNote = GetParam(paramIdx)->Value() / 100.0;
                                sprintf(writeToDisplay, "%4.1f", (int)(topDelay_tempoNote * 15.0f) + 1.0f);
                              }
                              else
                                if (paramIdx == EParams::kwetdry1) //Set wet/dry for the stereo delay.
                                {
                                  char* ptr;

                                  topDelay_wetDry = GetParam(paramIdx)->Value() / 100.0;
                                  if (topDelay_wetDry < 0.5)
                                  {
                                    wet1 = topDelay_wetDry * 2.0f;
                                    dry1 = 1.0f;
                                  }
                                  if (topDelay_wetDry >= 0.5)
                                  {
                                    dry1 = 2.0f - ((topDelay_wetDry - 0.5f) * 2.0f);
                                    dry1 = 1.0f - ((topDelay_wetDry - 0.5f) * 2.0f);
                                    wet1 = 1.0f;
                                  }

                                  sprintf(writeToDisplay, "%4.2f", topDelay_wetDry);

                                }
                                else
                                  /////////////////////////////
                                  if (paramIdx == EParams::kLP1) //Set LP1 cutoff
                                  {

                                    topDelay_lp1Cutoff = GetParam(paramIdx)->Value() / 100.0;;
                                    stereoDelay->setLP1Cutoff(topDelay_lp1Cutoff);
                                    sprintf(writeToDisplay, "%4.0f", topDelay_lp1Cutoff * 20000.0f);

                                  }
                                  else
                                    if (paramIdx == EParams::kHP1) //Set HP1 cutoff
                                    {
                                      topDelay_hp1Cutoff = GetParam(paramIdx)->Value() / 100.0;
                                      stereoDelay->setHP1Cutoff(topDelay_hp1Cutoff);
                                      sprintf(writeToDisplay, "%4.0f", getExpo(topDelay_hp1Cutoff));
                                    }
                                    else
                                      if (paramIdx == EParams::kPan1) //Set first panner.
                                      {
                                        topDelay_pan1 = GetParam(paramIdx)->Value() / 100.0;
                                        stereoDelay->setPan1(topDelay_pan1);
                                        sprintf(writeToDisplay, "%4.1f", topDelay_pan1 * 2.0f - 1.0);
                                      }
                                      else
                                        if (paramIdx == EParams::kDelay1) //Set the first delay time.
                                        {
                                          topDelay_delay1 = GetParam(paramIdx)->Value() / 100.0;
                                          sprintf(writeToDisplay, "%4.1f", (int)(topDelay_delay1 * 15.0f) + 1.0f);
                                        }
                                        else
                                          if (paramIdx == EParams::kFeedback1) //Set the first bottomDelay_feedback time.
                                          {
                                            topDelay_fb1 = GetParam(paramIdx)->Value() / 100.0;
                                            stereoDelay->setFeedback1(topDelay_fb1);
                                            sprintf(writeToDisplay, "%4.2f", topDelay_fb1);

                                          }
                                          else
                                            if (paramIdx == EParams::kLevel1) //Set the first gain value.
                                            {
                                              topDelay_level1 = GetParam(paramIdx)->Value() / 100.0;
                                              stereoDelay->setLevel1(topDelay_level1);
                                              sprintf(writeToDisplay, "%4.2f", topDelay_level1);
                                            }
                                            else
                                              //////////////////////////////
                                              if (paramIdx == EParams::kLP2) //Set the second LP Cutoff.
                                              {
                                                topDelay_lp2Cutoff = GetParam(paramIdx)->Value() / 100.0;
                                                stereoDelay->setLP2Cutoff(topDelay_lp2Cutoff);
                                                sprintf(writeToDisplay, "%4.0f", topDelay_lp2Cutoff * 20000.0f);
                                              }
                                              else
                                                if (paramIdx == EParams::kHP2) //Set the second HP cutoff.
                                                {
                                                  topDelay_hp2Cutoff = GetParam(paramIdx)->Value() / 100.0;
                                                  stereoDelay->setHP2Cutoff(topDelay_hp2Cutoff);
                                                  sprintf(writeToDisplay, "%4.0f", getExpo(topDelay_hp2Cutoff));
                                                }
                                                else
                                                  if (paramIdx == EParams::kPan2) //Set the second panner.
                                                  {
                                                    topDelay_pan2 = GetParam(paramIdx)->Value() / 100.0;
                                                    stereoDelay->setPan2(topDelay_pan2);
                                                    sprintf(writeToDisplay, "%4.1f", topDelay_pan2 * 2.0f - 1.0);
                                                  }
                                                  else
                                                    if (paramIdx == EParams::kDelay2) //Set the second delay time.
                                                    {
                                                      topDelay_delay2 = GetParam(paramIdx)->Value() / 100.0;
                                                      sprintf(writeToDisplay, "%4.1f", (int)(topDelay_delay2 * 15.0f) + 1.0f);
                                                    }
                                                    else
                                                      if (paramIdx == EParams::kFeedback2) //Set the second bottomDelay_feedback.
                                                      {

                                                        topDelay_fb2 = GetParam(paramIdx)->Value() / 100.0;
                                                        tapeDelay1->setFeedback(topDelay_fb2);
                                                        sprintf(writeToDisplay, "%4.2f", topDelay_fb2);
                                                      }
                                                      else
                                                        if (paramIdx == EParams::kLevel2) //Set the second gain.
                                                        {
                                                          topDelay_level2 = GetParam(paramIdx)->Value() / 100.0;
                                                          stereoDelay->setLevel2(topDelay_level2);
                                                          sprintf(writeToDisplay, "%4.2f", topDelay_level2);
                                                        }
}
#endif

