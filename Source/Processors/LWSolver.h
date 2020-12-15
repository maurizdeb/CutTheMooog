/*
  ==============================================================================

    LWSolver.h
    Created: 21 Nov 2020 5:16:11pm
    Author:  Maurizio de Bari

  ==============================================================================
*/

#pragma once

class LWSolver{
    
public:
    
    static float omega3(float x);
    static float omega4(float x);
    static float logf_approx(float x);
    
private:
    
    LWSolver(){};
    static float log2f_approx(float x);
    static float pow2f_approx(float x);
    static float expf_approx(float x);
    
    
};