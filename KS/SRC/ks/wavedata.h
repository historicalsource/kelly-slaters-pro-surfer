/*  THIS FILE IS AUTOMATICALLY GENERATED BY EXPORT FROM THE EXCEL FILE:  WAVEDATA.XLS
    ALL CHANGES SHOULD BE MADE IN EXCEL AND RE_EXPORTED.  DO NOT HAND EDIT.
    David Cook, Treyarch, 6/25/01
*/

#ifndef WAVEDATA_H
#define WAVEDATA_H

enum {
    WAVE_ANTARCTICA,
    WAVE_BELLS,
    WAVE_CORTESBANK,
    WAVE_GLAND,
    WAVE_JAWS,
    WAVE_JEFFERSONBAY,
    WAVE_MAVERICKS,
    WAVE_MUNDAKA,
    WAVE_PIPELINE,
    WAVE_SEBASTIAN,
    WAVE_TEAHUPOO,
    WAVE_TRESTLES,
    WAVE_INDOOR,
    WAVE_COSMOS,
    WAVE_CURRENSPOINT,
    WAVE_BELLSNIGHT,
    WAVE_test_big_wave,
    WAVE_test_buoy,
    WAVE_test_icon,
    WAVE_test_moving_wave,
    WAVE_test_no_tube,
    WAVE_test_small_wave,
    WAVE_GLAND2,
    WAVE_JAWSBIG,
    WAVE_KIRRA,
    WAVE_KIRRA2,
    WAVE_MAVBIG,
    WAVE_MUNDAKA2,
    WAVE_TEANIGHT,
    WAVE_TEAEVE,
    WAVE_OPENSEA,
    WAVE_LAST
};

typedef struct WaveDataStruct {
    char name[32];                               // Name
    float tileu;                                 // Tiling                U
    float tilev;                                 //                       V
    float tileu_xbox;                            //                       U tiling for xbox
    float tilev_xbox;                            //                       V tiling for xbox
    float speedx;                                // Wave perturbations    ShiftSpeedPeakX
    float speedz;                                //                       ShiftSpeedPeakZ
    float speedx_xbox;                           //                       ShiftSpeedPeakX
    float speedz_xbox;                           //                       ShiftSpeedPeakZ
    float boostu;                                //                       ShiftSpeedBoostX
    float boostv;                                //                       ShiftSpeedBoostZ
    float d_amp;                                 //                       DepthPerturbAmp
    float d_freq;                                //                       DepthPerturbFreq
    float h_amp;                                 //                       HeightPerturbAmp
    float h_freq;                                //                       HeightPerturbFreq
    float emitz;                                 // Particles             Unused
    float powerz;                                //                       Unused
    float emitcrashx;                            //                       EmitterStartCrashX
    float emitlipx;                              //                       EmitterStartLipX
    float emitcrestx;                            //                       EmitterStartCrestX
    float emitendx;                              //                       EmitterEndX
    float wave_dec;                              //                       Wave_Dec
    float wave_inc;                              //                       Wave_Inc
    float p_c1_mm;                               //                       ModMax
    float p_c1_mis;                              //                       MinSize
    float p_c1_ms;                               //                       Modsize
    float p_c1_mc;                               //                       ModConst
    float p_c1_ma;                               //                       ModAlpha
    float p_c1_ss;                               //                       Ssize
    float p_c1_rs;                               //                       Rsize
    float p_c1_es;                               //                       Esize
    float p_c1_ar;                               //                       Aspect
    int p_c1_sc;                                 //                       Scol
    int p_c1_rc;                                 //                       Rcol
    int p_c1_ec;                                 //                       Ecol
    float p_c1_l;                                //                       Life
    float p_c1_rl;                               //                       Rlife
    float p_c1_n;                                //                       Num
    float p_c1_sh;                               //                       Svel_Hangle
    float p_c1_sv;                               //                       Svel_Vangle
    float p_c1_sm;                               //                       Svel_Mag
    float p_c1_r1h;                              //                       Rvel1_Hangle
    float p_c1_r1v;                              //                       Rvel1_Vangle
    float p_c1_r1m;                              //                       Rvel1_Mag
    float p_c1_r2h;                              //                       Rvel2_Hangle
    float p_c1_r2v;                              //                       Rvel2_Vangle
    float p_c1_r2m;                              //                       Rvel2_Mag
    float p_c1_r3h;                              //                       Rvel3_Hangle
    float p_c1_r3v;                              //                       Rvel3_Vangle
    float p_c1_r3m;                              //                       Rvel3_Mag
    float p_c1_fh;                               //                       Force_Hangle
    float p_c1_fv;                               //                       Force_Vangle
    float p_c1_fm;                               //                       Force_Mag
    float p_c3_mm;                               //                       ModMax
    float p_c3_mis;                              //                       MinSize
    float p_c3_ms;                               //                       Modsize
    float p_c3_mc;                               //                       ModConst
    float p_c3_ma;                               //                       ModAlpha
    float p_c3_ss;                               //                       Ssize
    float p_c3_rs;                               //                       Rsize
    float p_c3_es;                               //                       Esize
    float p_c3_ar;                               //                       Aspect
    int p_c3_sc;                                 //                       Scol
    int p_c3_rc;                                 //                       Rcol
    int p_c3_ec;                                 //                       Ecol
    float p_c3_l;                                //                       Life
    float p_c3_rl;                               //                       Rlife
    float p_c3_n;                                //                       Num
    float p_c3_sh;                               //                       Svel_Hangle
    float p_c3_sv;                               //                       Svel_Vangle
    float p_c3_sm;                               //                       Svel_Mag
    float p_c3_r1h;                              //                       Rvel1_Hangle
    float p_c3_r1v;                              //                       Rvel1_Vangle
    float p_c3_r1m;                              //                       Rvel1_Mag
    float p_c3_r2h;                              //                       Rvel2_Hangle
    float p_c3_r2v;                              //                       Rvel2_Vangle
    float p_c3_r2m;                              //                       Rvel2_Mag
    float p_c3_r3h;                              //                       Rvel3_Hangle
    float p_c3_r3v;                              //                       Rvel3_Vangle
    float p_c3_r3m;                              //                       Rvel3_Mag
    float p_c3_fh;                               //                       Force_Hangle
    float p_c3_fv;                               //                       Force_Vangle
    float p_c3_fm;                               //                       Force_Mag
    float p_c4_mm;                               //                       ModMax
    float p_c4_mis;                              //                       MidSize
    float p_c4_ms;                               //                       Modsize
    float p_c4_mc;                               //                       ModConst
    float p_c4_ma;                               //                       ModAlpha
    float p_c4_ss;                               //                       Ssize
    float p_c4_rs;                               //                       Rsize
    float p_c4_es;                               //                       Esize
    float p_c4_ar;                               //                       Aspect
    int p_c4_sc;                                 //                       Scol
    int p_c4_rc;                                 //                       Rcol
    int p_c4_ec;                                 //                       Ecol
    float p_c4_l;                                //                       Life
    float p_c4_rl;                               //                       Rlife
    float p_c4_n;                                //                       Num
    float p_c4_sh;                               //                       Svel_Hangle
    float p_c4_sv;                               //                       Svel_Vangle
    float p_c4_sm;                               //                       Svel_Mag
    float p_c4_r1h;                              //                       Rvel1_Hangle
    float p_c4_r1v;                              //                       Rvel1_Vangle
    float p_c4_r1m;                              //                       Rvel1_Mag
    float p_c4_r2h;                              //                       Rvel2_Hangle
    float p_c4_r2v;                              //                       Rvel2_Vangle
    float p_c4_r2m;                              //                       Rvel2_Mag
    float p_c4_r3h;                              //                       Rvel3_Hangle
    float p_c4_r3v;                              //                       Rvel3_Vangle
    float p_c4_r3m;                              //                       Rvel3_Mag
    float p_c4_fh;                               //                       Force_Hangle
    float p_c4_fv;                               //                       Force_Vangle
    float p_c4_fm;                               //                       Force_Mag
    float crash_displacement;                    //                       Move crash particles
    float tube_spit_scale;                       //                       tube spit scale
    float tubecenstart_x;                        //                       Where the tube camera goes
    float tubecenstart_y;                        //                       Where the tube camera goes
    float tubecenstart_z;                        //                       Where the tube camera goes
    float tubecenstop_x;                         //                       Where the tube camera goes
    float tubecenstop_y;                         //                       Where the tube camera goes
    float tubecenstop_z;                         //                       Where the tube camera goes
    float tube_cam_dist;                         //                       How far behind the surfer the camera is.
    float firsttubethresh;                       //                       Gap in middle of tube
    float secondtubethresh;                      //                       Gap in back of tube
    float knockdowntubethresh;                   // 
    int look_in_tube_adj;                        //                       some tubes need to look deeper in than others in follow cam
    float faceVolume;                            // Wave Sound Params     Wave Volumes
    float tubeVolume;                            //                       Wave Volumes
    float foamVolume;                            //                       Wave Volumes
    float waveCrashVolume;                       //                       Wave Volumes
    float tubeMin;                               //                       WaveSound Range
    float tubeMax;                               //                       WaveSound Range
    float foamMin;                               //                       WaveSound Range
    float foamMax;                               //                       WaveSound Range
    float faceMin;                               //                       WaveSound Range
    float faceMax;                               //                       WaveSound Range
    float waveCrashMin;                          //                       WaveSound Range
    float waveCrashMax;                          //                       WaveSound Range
    float wave_height_scale;                     // 
} WaveData, *WaveDataPtr;

extern WaveData WaveDataArray[];
void WAVEDATA_Load(void);

#endif /* #ifndef WAVEDATA_H */
