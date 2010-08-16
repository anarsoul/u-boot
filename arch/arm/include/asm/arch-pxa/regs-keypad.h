/*
 * Copyright (C) 2012 Vasily Khoruzhick <anarsoul@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __REGS_KEYPAD_H__
#define __REGS_KEYPAD_H__

#define KEYPAD_BASE	0x41500000

struct kpasmkp_regs {
	uint32_t	kpasmkpx;
	uint32_t	reserved;
};

struct pxa_keypad_regs {
	uint32_t		kpc;
	uint32_t		reserved_1;
	uint32_t		kpdk;
	uint32_t		reserved_2;
	uint32_t		kprec;
	uint32_t		reserved_3;
	uint32_t		kpmk;
	uint32_t		reserved_4;
	uint32_t		kpas;
	uint32_t		reserved_5;
	struct kpasmkp_regs	kpasmkp[4];
	uint32_t		kpkdi;
};

#define KPC_AS		(0x1 << 30)	/* Automatic Scan bit */
#define KPC_ASACT	(0x1 << 29)	/* Automatic Scan on Activity */
#define KPC_MI		(0x1 << 22)	/* Matrix interrupt bit */
#define KPC_IMKP	(0x1 << 21)	/* Ignore Multiple Key Press */
#define KPC_MS7		(0x1 << 20)	/* Matrix scan line 7 */
#define KPC_MS6		(0x1 << 19)	/* Matrix scan line 6 */
#define KPC_MS5		(0x1 << 18)	/* Matrix scan line 5 */
#define KPC_MS4		(0x1 << 17)	/* Matrix scan line 4 */
#define KPC_MS3		(0x1 << 16)	/* Matrix scan line 3 */
#define KPC_MS2		(0x1 << 15)	/* Matrix scan line 2 */
#define KPC_MS1		(0x1 << 14)	/* Matrix scan line 1 */
#define KPC_MS0		(0x1 << 13)	/* Matrix scan line 0 */
#define KPC_ME		(0x1 << 12)	/* Matrix Keypad Enable */
#define KPC_MIE		(0x1 << 11)	/* Matrix Interrupt Enable */
#define KPC_DK_DEB_SEL	(0x1 << 9)	/* Direct Key Debounce select */
#define KPC_DI		(0x1 << 5)	/* Direct key interrupt bit */
#define KPC_DEE0	(0x1 << 2)	/* Rotary Encoder 0 Enable */
#define KPC_DE		(0x1 << 1)	/* Direct Keypad Enable */
#define KPC_DIE		(0x1 << 0)	/* Direct Keypad interrupt Enable */

#define KPDK_DKP	(0x1 << 31)
#define KPDK_DK7	(0x1 << 7)
#define KPDK_DK6	(0x1 << 6)
#define KPDK_DK5	(0x1 << 5)
#define KPDK_DK4	(0x1 << 4)
#define KPDK_DK3	(0x1 << 3)
#define KPDK_DK2	(0x1 << 2)
#define KPDK_DK1	(0x1 << 1)
#define KPDK_DK0	(0x1 << 0)

#define KPREC_OF1	(0x1 << 31)
#define kPREC_UF1	(0x1 << 30)
#define KPREC_OF0	(0x1 << 15)
#define KPREC_UF0	(0x1 << 14)

#define KPMK_MKP	(0x1 << 31)
#define KPAS_SO		(0x1 << 31)
#define KPASMKPx_SO	(0x1 << 31)

#endif
