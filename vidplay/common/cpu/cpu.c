/*****************************************************************************
 *
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: cpu.c 153 2004-12-20 16:46:49Z picard $
 *
 * The Core Pocket Media Player
 * Copyright (c) 2004-2005 Gabor Kovacs
 *
 ****************************************************************************/

#include "../common.h"

extern int64_t STDCALL CPUSpeedClk(int);

#ifdef ARM
extern int STDCALL CheckARM5E();
extern int STDCALL CheckARMXScale();
#endif

int CPUSpeed()
{
	int Speed = 0;

#if defined(MIPS) || defined(ARM) || defined(_M_IX86) || defined(SH3)
	int Old = ThreadPriority(NULL,-100);
	int TimeFreq = GetTimeFreq();
	if (TimeFreq>0)
	{

#ifdef TARGET_PALMOS
		int Iter = 1;
#else
		int Iter = 4;
#endif

		int64_t Clk,Best = 0;
		int BestCount = 0;
		int n,Count;
		int Sub=1,SubTime=8;
		int c0[2],c1[2],c2[2];
		int t;
		int SubTimeLimit = TimeFreq/250;
		if (SubTimeLimit <= 1)
			SubTimeLimit = 2;
		Count = 800000 / TimeFreq;

		for (n=0;n<4*Iter;++n)
		{
			GetTimeCycle(c0);
			GetTimeCycle(c1);
			GetTimeCycle(c2);

			c2[0] -= c1[0];
			c1[0] -= c0[0];

			if (c2[1]<c1[1])
			{
				c1[0] = c2[0];
				c1[1] = c2[1];
			}

			if (c1[1]*SubTime > c1[0]*Sub)
			{
				Sub = c1[1];
				SubTime = c1[0];
			}

			if (n>6 && SubTime >= SubTimeLimit)
				break;
		}

		if (SubTime < SubTimeLimit)
		{
			// Sub = approx number of possbile GetTime() calls per SubTime ms

			for (n=0;n<16*Iter;++n)
			{
				CPUSpeedClk(1); // load instruction cache
				GetTimeCycle(c0);
				Clk = CPUSpeedClk(Count);
				GetTimeCycle(c1);

				t = c1[0]-c0[0];
#if defined(TARGET_WINCE)
				if (QueryPlatform(PLATFORM_TYPENO) == TYPE_SMARTPHONE)
					Clk += t*2000;
#elif defined(TARGET_WIN32)
				// we are assuming a 1ms timer interrupt with 
				// minimum 6500 cycles interrupt handler
				Clk += t*6500;
#endif
				t *= 100000;
				t -=(c1[1] * 100000 * SubTime)/Sub; // adjust with subtime

				if (t < 25000) // quater tick -> double count
					Count *= 2;
				else
				if (t < 50000) // we need at least half tick
					Count += Count/8;
				else
				{
#ifdef ARM
					Clk = (Clk *  99600 * TimeFreq) / t;
#else
					Clk = (Clk * 100000 * TimeFreq) / t;
#endif
					if (Best < Clk)
					{
						Best = Clk;
						BestCount = 1;
					}
					else
					if ((Best * 127)/128 < Clk && ++BestCount>4*Iter)
						break;
				}
			}
		}

		Speed = (int)((Best + 500000)/1000000);
	}

	//rounding
	if ((Speed % 104)==1 || Speed==207) --Speed;
	else if ((Speed % 104)==103 || Speed==205) ++Speed;
	else if ((Speed % 10)==1) --Speed;
	else if ((Speed % 10)==9) ++Speed;
	else if (((Speed % 100)%33)==1) --Speed;
	else if (((Speed % 100)%33)==32) ++Speed;

	ThreadPriority(NULL,Old);

#endif
	return Speed;
}

#ifndef SH3
extern void STDCALL GetCpuId(int,uint32_t*);

void SafeGetCpuId(int Id, uint32_t* p)
{
	memset(p,0,4*sizeof(uint32_t));
	TRY_BEGIN
	{
		bool_t Mode = KernelMode(1);
		GetCpuId(Id,p); 
		KernelMode(Mode);
	}
	TRY_END
}
#endif

void CPUDetect(tchar_t* Name,int* PCaps,int* PICache,int* PDCache)
{
	int Caps = 0;
	int ICache = 0;
	int DCache = 0;

#ifdef ARM
	uint32_t CpuId[4];
	tcscpy(Name,T("ARM"));
	SafeGetCpuId(0,CpuId);

	if (CpuId[0])
	{
		ICache = 512 << ((CpuId[1] >> 6) & 7);
		DCache = 512 << ((CpuId[1] >> 18) & 7);
	}
	else
	{

#if !defined(TARGET_PALMOS)
		// when need to detect cpu features somehow
		// (only works if we can catch cpu exceptions)
		TRY_BEGIN
		{	
			if (CheckARM5E())
			{
				int XScale;
				Caps |= CAPS_ARM_5E;

				XScale = CheckARMXScale();
				if (XScale)
				{
					ICache = DCache = 32768;
					Caps |= CAPS_ARM_XSCALE;
					if (XScale > 1)
						Caps |= CAPS_ARM_WMMX;
				}
			}
		}
		TRY_END
#endif
	}

	if ((CpuId[0] & 0xFF000000) == 0x54000000) //TI
	{
		tcscat(Name,T(" TI"));
		switch ((CpuId[0] >> 4) & 0xFFF)
		{
		case 0x915: tcscat(Name,T("915T")); break;
		case 0x925: tcscat(Name,T("925T")); break;
		case 0x926: tcscat(Name,T("926T")); Caps |= CAPS_ARM_5E; break;
		}
	}
	else
	if ((CpuId[0] & 0xFF000000) == 0x41000000) //arm
	{
		switch ((CpuId[0] >> 4) & 0xFFF)
		{
		case 0x920: tcscat(Name,T(" 920T")); break;
		case 0x922: tcscat(Name,T(" 922T")); break;
		case 0x926: tcscat(Name,T(" 926E")); Caps |= CAPS_ARM_5E; break;
		case 0x940: tcscat(Name,T(" 940T")); break;
		case 0x946: tcscat(Name,T(" 946E")); Caps |= CAPS_ARM_5E; break;
		case 0xA22: tcscat(Name,T(" 1020E")); Caps |= CAPS_ARM_5E; break;
		}
	}
	else
	if ((CpuId[0] & 0xFF000000) == 0x69000000) //intel
	{
		tcscat(Name,T(" Intel"));

		if ((CpuId[0] & 0xFF0000) == 0x050000) //intel arm5e
			Caps |= CAPS_ARM_5E|CAPS_ARM_XSCALE;

		if (((CpuId[0] >> 4) & 0xFFF) == 0xB11)
		{
			tcscat(Name,T(" SA1110"));
		}
		else
		{
			switch ((CpuId[0] >> 13) & 7)
			{
			case 0x2: Caps |= CAPS_ARM_WMMX; break;
			}

			switch ((CpuId[0] >> 4) & 31)
			{
			case 0x10: 
				tcscat(Name,T(" PXA25x/26x")); break;
				break;
			case 0x11: 
				tcscat(Name,T(" PXA27x")); break;
				break;
			case 0x12:
				tcscat(Name,T(" PXA210"));
				break;
			}
		}
	}

#elif defined(MIPS)
	uint32_t CpuId[4];
	SafeGetCpuId(0,CpuId);
	tcscpy(Name,T("MIPS"));

	if (((CpuId[0] >> 8) & 255) == 0x0c)
	{
		if ((CpuId[0] & 0xF0) == 0x50)
		{
			Caps |= CAPS_MIPS_VR4110;
			tcscat(Name,T(" VR411X"));
		}
		else
		{
			Caps |= CAPS_MIPS_VR4120;
			if ((CpuId[0] & 0xF0) == 0x80)
				tcscat(Name,T(" VR413X"));
			else
				tcscat(Name,T(" VR412X"));
		}
	}

#elif defined(SH3)
	tcscpy(Name,T("SH3"));

#elif defined(_M_IX86)
	uint32_t CpuId[4];
	tcscpy(Name,T("x86"));
	SafeGetCpuId(0,CpuId);

    if (CpuId[1] == 0x756e6547 &&
        CpuId[3] == 0x49656e69 &&
        CpuId[2] == 0x6c65746e) 
	{
		tcscat(Name,T(" Intel"));

Intel:
		SafeGetCpuId(1,CpuId);
        if (CpuId[3] & 0x00800000)
		{
			Caps |= CAPS_X86_MMX;
			if (CpuId[3] & 0x02000000) 
				Caps |= CAPS_X86_MMX2 | CAPS_X86_SSE;
			if (CpuId[3] & 0x04000000) 
				Caps |= CAPS_X86_SSE2;
		}
    } 
	else if (CpuId[1] == 0x68747541 &&
             CpuId[3] == 0x69746e65 &&
             CpuId[2] == 0x444d4163) 
	{
		tcscat(Name,T(" AMD"));

		SafeGetCpuId(0x80000000,CpuId);
        if (CpuId[0] < 0x80000001)
            goto Intel;

		SafeGetCpuId(0x80000001,CpuId);
        if (CpuId[3] & 0x00800000)
		{
			Caps |= CAPS_X86_MMX;
			if (CpuId[3] & 0x80000000)
				Caps |= CAPS_X86_3DNOW;
			if (CpuId[3] & 0x00400000)
				Caps |= CAPS_X86_MMX2;
		}
    } 
	else if (CpuId[1] == 0x746e6543 &&
             CpuId[3] == 0x48727561 &&
             CpuId[2] == 0x736c7561) 
	{
		tcscat(Name,T(" VIA C3"));

		SafeGetCpuId(0x80000000,CpuId);
        if (CpuId[0] < 0x80000001)
            goto Intel;

		SafeGetCpuId(0x80000001,CpuId);
		if (CpuId[3] & (1<<31))
			Caps |= CAPS_X86_3DNOW;
		if (CpuId[3] & (1<<23))
			Caps |= CAPS_X86_MMX;
		if (CpuId[3] & (1<<24))
			Caps |= CAPS_X86_MMX2;
	}
	else if (CpuId[1] == 0x69727943 &&
             CpuId[3] == 0x736e4978 &&
             CpuId[2] == 0x64616574) 
	{
		tcscat(Name,T(" Cyrix"));

        if (CpuId[0] != 2) 
            goto Intel;

		SafeGetCpuId(0x80000001,CpuId);
        if (CpuId[3] & 0x00800000)
		{
			Caps |= CAPS_X86_MMX;
			if (CpuId[3] & 0x01000000)
				Caps |= CAPS_X86_MMX2;
		}
    }

#endif

	*PCaps = Caps;
	*PDCache = DCache;
	*PICache = ICache;
}

