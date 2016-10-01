////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	etxscope.cpp
//
// Project:	XuLA2-LX25 SoC based upon the ZipCPU
//
// Purpose:	This file decodes the debug bits produced by the enetpackets.v
//		Verilog module, and stored in a Wishbone Scope.  It is useful
//	for determining if the packet transmitter works at all or not.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015-2016, Gisselquist Technology, LLC
//
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of  the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory, run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include "port.h"
#include "regdefs.h"
#include "scopecls.h"

#define	WBSCOPE		R_NETSCOPE
#define	WBSCOPEDATA	R_NETSCOPED

FPGA	*m_fpga;
void	closeup(int v) {
	m_fpga->kill();
	exit(0);
}

class	CFGSCOPE : public SCOPE {
public:
	CFGSCOPE(FPGA *fpga, unsigned addr, bool vecread)
		: SCOPE(fpga, addr, false, false) {};
	~CFGSCOPE(void) {}
	virtual	void	decode(DEVBUS::BUSW val) const {
		int	trigger, addr, cancel, cmd, complete, busy, en, txd;
		int	lrxclk, ltxclk, txstb;

		trigger = (val>>31)&1;
		ltxclk  = (val>>30)&1;
		lrxclk  = (val>>29)&1;
		addr    = (val>>10)&0x0ffff;
		txstb   = (val>> 9)&1;
		cancel  = (val>> 8)&1;
		cmd     = (val>> 7)&1;
		complete= (val>> 6)&1;
		busy    = (val>> 5)&1;
		en      = (val>> 4)&1;
		txd     = (val    )&15;

		printf("%s %s %s ",
			(lrxclk)?"LRX":"   ",
			(ltxclk)?"LTX":"   ",
			(txstb)?"TXSTB":"     ");
		printf("%s %04x %s%s%s%s %s/%x",
			(trigger)?"TR":"  ",
			(addr),
			(cancel)?"X":"   ",
			(cmd)?" CMD":"    ",
			(complete)?"DON":"   ",
			(busy)?"BSY":"   ",
			(en)?"EN":"  ", txd);
	}
};

int main(int argc, char **argv) {
	FPGAOPEN(m_fpga);

	signal(SIGSTOP, closeup);
	signal(SIGHUP, closeup);

	CFGSCOPE *scope = new CFGSCOPE(m_fpga, WBSCOPE, false);
	if (!scope->ready()) {
		printf("Scope is not yet ready:\n");
		scope->decode_control();
	} else
		scope->read();
	delete	m_fpga;
}

