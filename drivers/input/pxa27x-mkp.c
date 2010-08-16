/*
 * PXA27x matrix keypad controller driver
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 * Copyright (C) 2012 Vasily Khoruzhick <anarsoul@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <stdio_dev.h>
#include <watchdog.h>
#include <asm/arch/regs-keypad.h>
#include <asm/io.h>

#define	DEVNAME		"pxa27x-mkp"

struct {
	char	row;
	char	col;
	char	key;
	char	shift;
	char	alt;
	char	ctrl;
} keymap[] = {
	CONFIG_PXA27X_MKP_KEYMAP,
};

static unsigned char queue[64] = {0};
static int queue_len;
static struct pxa_keypad_regs *regs = (struct pxa_keypad_regs *)KEYPAD_BASE;

/* autorepeat stuff */
static int last_key_row = 0xff, last_key_col = 0xff;
static char key_counter;

/* number of key scans before autorepeat kicks in */
#ifndef CONFIG_PXA27X_KEY_REPEAT_FIRST
#define	CONFIG_PXA27X_KEY_REPEAT_FIRST	12
#endif
#ifndef CONFIG_PXA27X_KEY_REPEAT_NEXT
#define	CONFIG_PXA27X_KEY_REPEAT_NEXT	2
#endif

/* Number of cycles to wait */
#define PXA_KEYPAD_TIMEOUT	100000

enum {
	MOD_NONE,
	MOD_SHIFT,
	MOD_ALT,
	MOD_CTRL,
};

static int kbd_get_mdf(int row, int col)
{
	char mod_shift[2] = CONFIG_PXA27X_MKP_MOD_SHIFT;
	char mod_alt[2] = CONFIG_PXA27X_MKP_MOD_ALT;
	char mod_ctrl[2] = CONFIG_PXA27X_MKP_MOD_CTRL;

	if (mod_shift[0] == row && mod_shift[1] == col)
		return MOD_SHIFT;
	if (mod_alt[0] == row && mod_alt[1] == col)
		return MOD_ALT;
	if (mod_ctrl[0] == row && mod_ctrl[1] == col)
		return MOD_CTRL;
	return MOD_NONE;
}

static int kbd_lookup(int row, int col, int mod)
{
	int i = 0;
	char key = 0xff;

	while (!(keymap[i].col == 0xff && keymap[i].row == 0xff)) {
		if (keymap[i].row != row || keymap[i].col != col) {
			i++;
			continue;
		}
		switch (mod) {
		case MOD_NONE:
			key = keymap[i].key;
			break;
		case MOD_SHIFT:
			key = keymap[i].shift;
			break;
		case MOD_ALT:
			key = keymap[i].alt;
			break;
		case MOD_CTRL:
			key = keymap[i].ctrl;
			break;
		}
		if (key == 0xff) {
			i++;
			continue;
		}

		if (row != last_key_row || col != last_key_col) {
			queue[queue_len++] = key;
			last_key_row = row;
			last_key_col = col;
			key_counter = 0;
		} else /* same key as before */
			if (key_counter < CONFIG_PXA27X_KEY_REPEAT_FIRST) {
				/* ignore key press */
				key_counter++;
			} else {
				/* ok, autorepeat */
				queue[queue_len++] = key;
				key_counter = CONFIG_PXA27X_KEY_REPEAT_FIRST
					- CONFIG_PXA27X_KEY_REPEAT_NEXT;
			}
		i++;
	}
	return key;
}

static int scan_keys(int scan_modif, uint32_t kpasmkp[4])
{
	uint32_t reg = 0;
	int col, row;
	static int mod = MOD_NONE;
	int key;
	for (col = 0; col < 8; col += 1) {
		reg = kpasmkp[col >> 1];
		reg >>= 16 * (col % 2);
		for (row = 0; row < 8; row++) {
			if (!(reg & (1 << row)))
				continue;

			if (scan_modif) {
				mod = kbd_get_mdf(row, col);
				if (mod != MOD_NONE)
					return mod;
			} else {
				key = kbd_lookup(row, col, mod);
				if (key != 0xff)
					return key;
			}
		}
	}

	if (scan_modif)
		return MOD_NONE;
	else
		return 0xff;
}

static void kbd_read(void)
{
	uint32_t reg;
	int col, row, i, have_new_key = 0;
	int numkeys;
	int mod = MOD_NONE;
	unsigned int timeout = PXA_KEYPAD_TIMEOUT;
	static uint32_t kpasmkp_old[4];
	uint32_t kpasmkp[4], kpasmkp_diff[4];

	/* start one automatic scan */
	writel(readl(&regs->kpc) | KPC_AS, &regs->kpc);

	/* wait for scan to finish */
	while (timeout--) {
		if (!(readl(&regs->kpc) & KPC_AS))
			break;
		WATCHDOG_RESET();
	}

	if (!timeout)
		return;

	numkeys = (readl(&regs->kpas) >> 26) & 0x1f;
	switch (numkeys) {
	case 0:
		/* no key pressed, clear autorepeat counter */
		key_counter = 0;
		last_key_row = last_key_col = 0xff;
		for (i = 0; i < ARRAY_SIZE(kpasmkp); i++)
			kpasmkp[i] = 0;
		break;
	case 1:
		reg = readl(&regs->kpas) & 0xff;
		col = reg & 0x0f;
		row = reg >> 4;
		for (i = 0; i < ARRAY_SIZE(kpasmkp); i++)
			kpasmkp[i] = 0;
		kpasmkp[col >> 1] |= (1 << (row + 16 * (col % 2)));

		break;
	default:
		for (i = 0; i < ARRAY_SIZE(kpasmkp); i++) {
			timeout = PXA_KEYPAD_TIMEOUT;
			while (timeout--) {
				kpasmkp[i] = readl(&regs->kpasmkp[i].kpasmkpx);
				if (!(kpasmkp[i] & KPASMKPx_SO))
					break;
				WATCHDOG_RESET();
			}
			if (!timeout)
				kpasmkp[i] = 0;
		}
		break;
	}

	/* Find new keypress */
	for (i = 0; i < ARRAY_SIZE(kpasmkp); i++) {
		kpasmkp_diff[i] = (kpasmkp_old[i] ^ kpasmkp[i]) &
			kpasmkp[i];
		if (kpasmkp_diff[i])
			have_new_key = 1;
		kpasmkp_old[i] = kpasmkp[i];
	}

	if (!numkeys)
		return;

	/* Scan for modifiers */
	mod = scan_keys(1, kpasmkp);
	if (!have_new_key) {
		/* Check if old key is still pressed */
		if (kpasmkp[last_key_col >> 1] &
		   (1 << (last_key_row + 16 * (last_key_col % 2))))
			kbd_lookup(last_key_row, last_key_col, mod);

	} else {
		key_counter = 0;
		last_key_row = last_key_col = 0xff;
		scan_keys(0, kpasmkp_diff);
	}
}

static int kbd_getc(struct stdio_dev *dev)
{
	if (!queue_len) {
		kbd_read();
		udelay(CONFIG_PXA27X_MKP_DELAY);
	}

	if (queue_len)
		return queue[--queue_len];
	else
		return 0;
}

static int kbd_testc(struct stdio_dev *dev)
{
	if (!queue_len)
		kbd_read();
	return queue_len;
}

int drv_keyboard_init(void)
{
	int error = 0;
	struct stdio_dev kbddev;

	writel((CONFIG_PXA27X_MKP_MKP_ROWS << 26) |
		(CONFIG_PXA27X_MKP_MKP_COLS << 23) |
		(0xff << 13) | KPC_ME, &regs->kpc);
	writel(CONFIG_PXA27X_MKP_DEBOUNCE, &regs->kpkdi);

	memset(&kbddev, 0, sizeof(kbddev));
	strcpy(kbddev.name, DEVNAME);
	kbddev.flags =  DEV_FLAGS_INPUT;
	kbddev.putc = NULL;
	kbddev.puts = NULL;
	kbddev.getc = kbd_getc;
	kbddev.tstc = kbd_testc;

	error = stdio_register(&kbddev);
	return error;
}
