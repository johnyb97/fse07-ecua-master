/*
 * FSE.07 Accumulator Management System
 *
 * Written by Martin Cejp. Portions by Jan Sixta.
 *
 * Copyright (c) 2015, 2018 eForce FEE Prague Formula
 */

#ifndef ecua_ecua_hpp
#define ecua_ecua_hpp

#include "ecuacan.hpp"
#include "ecua_options.hpp"

#include <fse04.h>
#include <library/pin.hpp>
#include <library/timer.hpp>

extern Pin 	SDCend,			// SDC END present
			airMCon_n,		// AIR- 0=closed 1=open
			airPCon_n,		// AIR+ 0=closed 1=open
			acpOK,			// AMS status 0=fail 1=OK
			precharge,		// Precharge enable
			airPEn,			// AIR+ control 0=close 1=open
			airMEn,			// AIR- control 0=close 1=open
			hvInterlock,	// HV_Interlock present
			imdOk,			// IMD 0=error 1=OK
			bmsWkup;

#endif
