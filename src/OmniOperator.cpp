/*
 * OmniOperator.cpp
 *
 *  Created on: Jul 18, 2019
 *      Author: mfukunaga
 */

#include "OmniOperator.hpp"
#include <math.h>
#include <stdio.h>
#include <Arduino.h>

void OmniOperator::calc_translation(float x, float y){
	if(x==0 && y==0){
		top_motor_temp=0;
		left_motor_temp=0;
		right_motor_temp=0;
		return;
	}

	float rad = x!=0?atan2f(y,x):y>0?PI/2:-PI/2;
	float motor_power = sqrt(powf(x,2)+powf(y,2));

	top_motor_temp = (int32_t)(cosf(rad-TOP_MOTOR_RAD)*motor_power * calc_max_count);
	left_motor_temp = (int32_t)(cosf(rad-LEFT_MOTOR_RAD)*motor_power * calc_max_count);
	right_motor_temp = (int32_t)(cosf(rad-RIGHT_MOTOR_RAD)*motor_power * calc_max_count);
}

void OmniOperator::calc_rotation(float r){
	if(r==0.f){
		return;//return to reduce calculation.
	}
	float abs_r = r>0?r:-r;

	top_motor_temp = (int32_t)(top_motor_temp+r*(calc_max_count - top_motor_temp)) ;
	left_motor_temp =(int32_t)(left_motor_temp+r*(calc_max_count - left_motor_temp)) ;
	right_motor_temp =(int32_t)(right_motor_temp+r*(calc_max_count - right_motor_temp)) ;
}

void OmniOperator::get_targetValue(){
	
	//TODO:今回のMDに合わせた処理を追加．
	//この時点で必要なモータ回転数の割合が出ているので，それをUARTで渡す形になる
}

OmniOperator::OmniOperator(){
}

void OmniOperator::init(int range){

    max_count=range/2;
    calc_max_count = max_count;
    middle_count = range/2;
}

void OmniOperator::set_limit(int percent) {
    calc_max_count = max_count *percent/100;
}


// void OmniOperator::move(float x,float y,float r){

// 	calc_translation(x,y);
// 	calc_rotation(r);
// 	set_motor_count();
// }

