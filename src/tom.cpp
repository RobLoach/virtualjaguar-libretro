//
// TOM Processing
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups and endian wrongness amelioration by James L. Hammons
// Note: Endian wrongness probably stems from the MAME origins of this emu and
//       the braindead way in which MAME handles memory. :-)
//
// Note: TOM has only a 16K memory space
//
//	------------------------------------------------------------
//	TOM REGISTERS (Mapped by Aaron Giles)
//	------------------------------------------------------------
//	F00000-F0FFFF   R/W   xxxxxxxx xxxxxxxx   Internal Registers
//	F00000          R/W   -x-xx--- xxxxxxxx   MEMCON1 - memory config reg 1
//	                      -x------ --------      (CPU32 - is the CPU 32bits?)
//	                      ---xx--- --------      (IOSPEED - external I/O clock cycles)
//	                      -------- x-------      (FASTROM - reduces ROM clock cycles)
//	                      -------- -xx-----      (DRAMSPEED - sets RAM clock cycles)
//	                      -------- ---xx---      (ROMSPEED - sets ROM clock cycles)
//	                      -------- -----xx-      (ROMWIDTH - sets width of ROM: 8,16,32,64 bits)
//	                      -------- -------x      (ROMHI - controls ROM mapping)
//	F00002          R/W   --xxxxxx xxxxxxxx   MEMCON2 - memory config reg 2
//	                      --x----- --------      (HILO - image display bit order)
//	                      ---x---- --------      (BIGEND - big endian addressing?)
//	                      ----xxxx --------      (REFRATE - DRAM refresh rate)
//	                      -------- xx------      (DWIDTH1 - DRAM1 width: 8,16,32,64 bits)
//	                      -------- --xx----      (COLS1 - DRAM1 columns: 256,512,1024,2048)
//	                      -------- ----xx--      (DWIDTH0 - DRAM0 width: 8,16,32,64 bits)
//	                      -------- ------xx      (COLS0 - DRAM0 columns: 256,512,1024,2048)
//	F00004          R/W   -----xxx xxxxxxxx   HC - horizontal count
//	                      -----x-- --------      (which half of the display)
//	                      ------xx xxxxxxxx      (10-bit counter)
//	F00006          R/W   ----xxxx xxxxxxxx   VC - vertical count
//	                      ----x--- --------      (which field is being generated)
//	                      -----xxx xxxxxxxx      (11-bit counter)
//	F00008          R     -----xxx xxxxxxxx   LPH - light pen horizontal position
//	F0000A          R     -----xxx xxxxxxxx   LPV - light pen vertical position
//	F00010-F00017   R     xxxxxxxx xxxxxxxx   OB - current object code from the graphics processor
//	F00020-F00023     W   xxxxxxxx xxxxxxxx   OLP - start of the object list
//	F00026            W   -------- -------x   OBF - object processor flag
//	F00028            W   ----xxxx xxxxxxxx   VMODE - video mode
//	                  W   ----xxx- --------      (PWIDTH1-8 - width of pixel in video clock cycles)
//	                  W   -------x --------      (VARMOD - enable variable color resolution)
//	                  W   -------- x-------      (BGEN - clear line buffer to BG color)
//	                  W   -------- -x------      (CSYNC - enable composite sync on VSYNC)
//	                  W   -------- --x-----      (BINC - local border color if INCEN)
//	                  W   -------- ---x----      (INCEN - encrustation enable)
//	                  W   -------- ----x---      (GENLOCK - enable genlock)
//	                  W   -------- -----xx-      (MODE - CRY16,RGB24,DIRECT16,RGB16)
//	                  W   -------- -------x      (VIDEN - enables video)
//	F0002A            W   xxxxxxxx xxxxxxxx   BORD1 - border color (red/green)
//	F0002C            W   -------- xxxxxxxx   BORD2 - border color (blue)
//	F0002E            W   ------xx xxxxxxxx   HP - horizontal period
//	F00030            W   -----xxx xxxxxxxx   HBB - horizontal blanking begin
//	F00032            W   -----xxx xxxxxxxx   HBE - horizontal blanking end
//	F00034            W   -----xxx xxxxxxxx   HSYNC - horizontal sync
//	F00036            W   ------xx xxxxxxxx   HVS - horizontal vertical sync
//	F00038            W   -----xxx xxxxxxxx   HDB1 - horizontal display begin 1
//	F0003A            W   -----xxx xxxxxxxx   HDB2 - horizontal display begin 2
//	F0003C            W   -----xxx xxxxxxxx   HDE - horizontal display end
//	F0003E            W   -----xxx xxxxxxxx   VP - vertical period
//	F00040            W   -----xxx xxxxxxxx   VBB - vertical blanking begin
//	F00042            W   -----xxx xxxxxxxx   VBE - vertical blanking end
//	F00044            W   -----xxx xxxxxxxx   VS - vertical sync
//	F00046            W   -----xxx xxxxxxxx   VDB - vertical display begin
//	F00048            W   -----xxx xxxxxxxx   VDE - vertical display end
//	F0004A            W   -----xxx xxxxxxxx   VEB - vertical equalization begin
//	F0004C            W   -----xxx xxxxxxxx   VEE - vertical equalization end
//	F0004E            W   -----xxx xxxxxxxx   VI - vertical interrupt
//	F00050            W   xxxxxxxx xxxxxxxx   PIT0 - programmable interrupt timer 0
//	F00052            W   xxxxxxxx xxxxxxxx   PIT1 - programmable interrupt timer 1
//	F00054            W   ------xx xxxxxxxx   HEQ - horizontal equalization end
//	F00058            W   xxxxxxxx xxxxxxxx   BG - background color
//	F000E0          R/W   ---xxxxx ---xxxxx   INT1 - CPU interrupt control register
//	                      ---x---- --------      (C_JERCLR - clear pending Jerry ints)
//	                      ----x--- --------      (C_PITCLR - clear pending PIT ints)
//	                      -----x-- --------      (C_OPCLR - clear pending object processor ints)
//	                      ------x- --------      (C_GPUCLR - clear pending graphics processor ints)
//	                      -------x --------      (C_VIDCLR - clear pending video timebase ints)
//	                      -------- ---x----      (C_JERENA - enable Jerry ints)
//	                      -------- ----x---      (C_PITENA - enable PIT ints)
//	                      -------- -----x--      (C_OPENA - enable object processor ints)
//	                      -------- ------x-      (C_GPUENA - enable graphics processor ints)
//	                      -------- -------x      (C_VIDENA - enable video timebase ints)
//	F000E2            W   -------- --------   INT2 - CPU interrupt resume register
//	F00400-F005FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table A
//	F00600-F007FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table B
//	F00800-F00D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer A
//	F01000-F0159F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer B
//	F01800-F01D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer currently selected
//	------------------------------------------------------------
//	F02000-F021FF   R/W   xxxxxxxx xxxxxxxx   GPU control registers
//	F02100          R/W   xxxxxxxx xxxxxxxx   G_FLAGS - GPU flags register
//	                R/W   x------- --------      (DMAEN - DMA enable)
//	                R/W   -x------ --------      (REGPAGE - register page)
//	                  W   --x----- --------      (G_BLITCLR - clear blitter interrupt)
//	                  W   ---x---- --------      (G_OPCLR - clear object processor int)
//	                  W   ----x--- --------      (G_PITCLR - clear PIT interrupt)
//	                  W   -----x-- --------      (G_JERCLR - clear Jerry interrupt)
//	                  W   ------x- --------      (G_CPUCLR - clear CPU interrupt)
//	                R/W   -------x --------      (G_BLITENA - enable blitter interrupt)
//	                R/W   -------- x-------      (G_OPENA - enable object processor int)
//	                R/W   -------- -x------      (G_PITENA - enable PIT interrupt)
//	                R/W   -------- --x-----      (G_JERENA - enable Jerry interrupt)
//	                R/W   -------- ---x----      (G_CPUENA - enable CPU interrupt)
//	                R/W   -------- ----x---      (IMASK - interrupt mask)
//	                R/W   -------- -----x--      (NEGA_FLAG - ALU negative)
//	                R/W   -------- ------x-      (CARRY_FLAG - ALU carry)
//	                R/W   -------- -------x      (ZERO_FLAG - ALU zero)
//	F02104            W   -------- ----xxxx   G_MTXC - matrix control register
//	                  W   -------- ----x---      (MATCOL - column/row major)
//	                  W   -------- -----xxx      (MATRIX3-15 - matrix width)
//	F02108            W   ----xxxx xxxxxx--   G_MTXA - matrix address register
//	F0210C            W   -------- -----xxx   G_END - data organization register
//	                  W   -------- -----x--      (BIG_INST - big endian instruction fetch)
//	                  W   -------- ------x-      (BIG_PIX - big endian pixels)
//	                  W   -------- -------x      (BIG_IO - big endian I/O)
//	F02110          R/W   xxxxxxxx xxxxxxxx   G_PC - GPU program counter
//	F02114          R/W   xxxxxxxx xx-xxxxx   G_CTRL - GPU control/status register
//	                R     xxxx---- --------      (VERSION - GPU version code)
//	                R/W   ----x--- --------      (BUS_HOG - hog the bus!)
//	                R/W   -----x-- --------      (G_BLITLAT - blitter interrupt latch)
//	                R/W   ------x- --------      (G_OPLAT - object processor int latch)
//	                R/W   -------x --------      (G_PITLAT - PIT interrupt latch)
//	                R/W   -------- x-------      (G_JERLAT - Jerry interrupt latch)
//	                R/W   -------- -x------      (G_CPULAT - CPU interrupt latch)
//	                R/W   -------- ---x----      (SINGLE_GO - single step one instruction)
//	                R/W   -------- ----x---      (SINGLE_STEP - single step mode)
//	                R/W   -------- -----x--      (FORCEINT0 - cause interrupt 0 on GPU)
//	                R/W   -------- ------x-      (CPUINT - send GPU interrupt to CPU)
//	                R/W   -------- -------x      (GPUGO - enable GPU execution)
//	F02118-F0211B   R/W   xxxxxxxx xxxxxxxx   G_HIDATA - high data register
//	F0211C-F0211F   R     xxxxxxxx xxxxxxxx   G_REMAIN - divide unit remainder
//	F0211C            W   -------- -------x   G_DIVCTRL - divide unit control
//	                  W   -------- -------x      (DIV_OFFSET - 1=16.16 divide, 0=32-bit divide)
//	------------------------------------------------------------
//	BLITTER REGISTERS
//	------------------------------------------------------------
//	F02200-F022FF   R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Blitter registers
//	F02200            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_BASE - A1 base register
//	F02204            W   -------- ---xxxxx -xxxxxxx xxxxx-xx   A1_FLAGS - A1 flags register
//	                  W   -------- ---x---- -------- --------      (YSIGNSUB - invert sign of Y delta)
//	                  W   -------- ----x--- -------- --------      (XSIGNSUB - invert sign of X delta)
//	                  W   -------- -----x-- -------- --------      (Y add control)
//	                  W   -------- ------xx -------- --------      (X add control)
//	                  W   -------- -------- -xxxxxx- --------      (width in 6-bit floating point)
//	                  W   -------- -------- -------x xx------      (ZOFFS1-6 - Z data offset)
//	                  W   -------- -------- -------- --xxx---      (PIXEL - pixel size)
//	                  W   -------- -------- -------- ------xx      (PITCH1-4 - data phrase pitch)
//	F02208            W   -xxxxxxx xxxxxxxx -xxxxxxx xxxxxxxx   A1_CLIP - A1 clipping size
//	                  W   -xxxxxxx xxxxxxxx -------- --------      (height)
//	                  W   -------- -------- -xxxxxxx xxxxxxxx      (width)
//	F0220C          R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_PIXEL - A1 pixel pointer
//	                R/W   xxxxxxxx xxxxxxxx -------- --------      (Y pixel value)
//	                R/W   -------- -------- xxxxxxxx xxxxxxxx      (X pixel value)
//	F02210            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_STEP - A1 step value
//	                  W   xxxxxxxx xxxxxxxx -------- --------      (Y step value)
//	                  W   -------- -------- xxxxxxxx xxxxxxxx      (X step value)
//	F02214            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_FSTEP - A1 step fraction value
//	                  W   xxxxxxxx xxxxxxxx -------- --------      (Y step fraction value)
//	                  W   -------- -------- xxxxxxxx xxxxxxxx      (X step fraction value)
//	F02218          R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_FPIXEL - A1 pixel pointer fraction
//	                R/W   xxxxxxxx xxxxxxxx -------- --------      (Y pixel fraction value)
//	                R/W   -------- -------- xxxxxxxx xxxxxxxx      (X pixel fraction value)
//	F0221C            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_INC - A1 increment
//	                  W   xxxxxxxx xxxxxxxx -------- --------      (Y increment)
//	                  W   -------- -------- xxxxxxxx xxxxxxxx      (X increment)
//	F02220            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A1_FINC - A1 increment fraction
//	                  W   xxxxxxxx xxxxxxxx -------- --------      (Y increment fraction)
//	                  W   -------- -------- xxxxxxxx xxxxxxxx      (X increment fraction)
//	F02224            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A2_BASE - A2 base register
//	F02228            W   -------- ---xxxxx -xxxxxxx xxxxx-xx   A2_FLAGS - A2 flags register
//	                  W   -------- ---x---- -------- --------      (YSIGNSUB - invert sign of Y delta)
//	                  W   -------- ----x--- -------- --------      (XSIGNSUB - invert sign of X delta)
//	                  W   -------- -----x-- -------- --------      (Y add control)
//	                  W   -------- ------xx -------- --------      (X add control)
//	                  W   -------- -------- -xxxxxx- --------      (width in 6-bit floating point)
//	                  W   -------- -------- -------x xx------      (ZOFFS1-6 - Z data offset)
//	                  W   -------- -------- -------- --xxx---      (PIXEL - pixel size)
//	                  W   -------- -------- -------- ------xx      (PITCH1-4 - data phrase pitch)
//	F0222C            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A2_MASK - A2 window mask
//	F02230          R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A2_PIXEL - A2 pixel pointer
//	                R/W   xxxxxxxx xxxxxxxx -------- --------      (Y pixel value)
//	                R/W   -------- -------- xxxxxxxx xxxxxxxx      (X pixel value)
//	F02234            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   A2_STEP - A2 step value
//	                  W   xxxxxxxx xxxxxxxx -------- --------      (Y step value)
//	                  W   -------- -------- xxxxxxxx xxxxxxxx      (X step value)
//	F02238            W   -xxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_CMD - command register
//	                  W   -x------ -------- -------- --------      (SRCSHADE - modify source intensity)
//	                  W   --x----- -------- -------- --------      (BUSHI - hi priority bus)
//	                  W   ---x---- -------- -------- --------      (BKGWREN - writeback destination)
//	                  W   ----x--- -------- -------- --------      (DCOMPEN - write inhibit from data comparator)
//	                  W   -----x-- -------- -------- --------      (BCOMPEN - write inhibit from bit coparator)
//	                  W   ------x- -------- -------- --------      (CMPDST - compare dest instead of src)
//	                  W   -------x xxx----- -------- --------      (logical operation)
//	                  W   -------- ---xxx-- -------- --------      (ZMODE - Z comparator mode)
//	                  W   -------- ------x- -------- --------      (ADDDSEL - select sum of src & dst)
//	                  W   -------- -------x -------- --------      (PATDSEL - select pattern data)
//	                  W   -------- -------- x------- --------      (TOPNEN - enable carry into top intensity nibble)
//	                  W   -------- -------- -x------ --------      (TOPBEN - enable carry into top intensity byte)
//	                  W   -------- -------- --x----- --------      (ZBUFF - enable Z updates in inner loop)
//	                  W   -------- -------- ---x---- --------      (GOURD - enable gouraud shading in inner loop)
//	                  W   -------- -------- ----x--- --------      (DSTA2 - reverses A2/A1 roles)
//	                  W   -------- -------- -----x-- --------      (UPDA2 - add A2 step to A2 in outer loop)
//	                  W   -------- -------- ------x- --------      (UPDA1 - add A1 step to A1 in outer loop)
//	                  W   -------- -------- -------x --------      (UPDA1F - add A1 fraction step to A1 in outer loop)
//	                  W   -------- -------- -------- x-------      (diagnostic use)
//	                  W   -------- -------- -------- -x------      (CLIP_A1 - clip A1 to window)
//	                  W   -------- -------- -------- --x-----      (DSTWRZ - enable dest Z write in inner loop)
//	                  W   -------- -------- -------- ---x----      (DSTENZ - enable dest Z read in inner loop)
//	                  W   -------- -------- -------- ----x---      (DSTEN - enables dest data read in inner loop)
//	                  W   -------- -------- -------- -----x--      (SRCENX - enable extra src read at start of inner)
//	                  W   -------- -------- -------- ------x-      (SRCENZ - enables source Z read in inner loop)
//	                  W   -------- -------- -------- -------x      (SRCEN - enables source data read in inner loop)
//	F02238          R     xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_CMD - status register
//	                R     xxxxxxxx xxxxxxxx -------- --------      (inner count)
//	                R     -------- -------- xxxxxxxx xxxxxx--      (diagnostics)
//	                R     -------- -------- -------- ------x-      (STOPPED - when stopped in collision detect)
//	                R     -------- -------- -------- -------x      (IDLE - when idle)
//	F0223C            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_COUNT - counters register
//	                  W   xxxxxxxx xxxxxxxx -------- --------      (outer loop count)
//	                  W   -------- -------- xxxxxxxx xxxxxxxx      (inner loop count)
//	F02240-F02247     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_SRCD - source data register
//	F02248-F0224F     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_DSTD - destination data register
//	F02250-F02257     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_DSTZ - destination Z register
//	F02258-F0225F     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_SRCZ1 - source Z register 1
//	F02260-F02267     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_SRCZ2 - source Z register 2
//	F02268-F0226F     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_PATD - pattern data register
//	F02270            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_IINC - intensity increment
//	F02274            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_ZINC - Z increment
//	F02278            W   -------- -------- -------- -----xxx   B_STOP - collision control
//	                  W   -------- -------- -------- -----x--      (STOPEN - enable blitter collision stops)
//	                  W   -------- -------- -------- ------x-      (ABORT - abort after stop)
//	                  W   -------- -------- -------- -------x      (RESUME - resume after stop)
//	F0227C            W   -------- xxxxxxxx xxxxxxxx xxxxxxxx   B_I3 - intensity 3
//	F02280            W   -------- xxxxxxxx xxxxxxxx xxxxxxxx   B_I2 - intensity 2
//	F02284            W   -------- xxxxxxxx xxxxxxxx xxxxxxxx   B_I1 - intensity 1
//	F02288            W   -------- xxxxxxxx xxxxxxxx xxxxxxxx   B_I0 - intensity 0
//	F0228C            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_Z3 - Z3
//	F02290            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_Z2 - Z2
//	F02294            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_Z1 - Z1
//	F02298            W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   B_Z0 - Z0
//	------------------------------------------------------------

#include <SDL.h>
//#include "SDLptc.h"
#include "tom.h"
#include "gpu.h"
#include "objectp.h"
#include "cry2rgb.h"

// TOM registers (offset from $F00000)

#define MEMCON1		0x00
#define MEMCON2		0x02
#define HC			0x04
#define VC			0x06
#define VMODE		0x28
#define   MODE		0x0006		// Line buffer to video generator mode
#define   BGEN		0x0080		// Background enable (CRY & RGB16 only)
#define   VARMOD	0x0100		// Mixed CRY/RGB16 mode
#define   PWIDTH	0x0E00		// Pixel width in video clock cycles
#define HP			0x2E		// Values range from 1 - 1024 (value written + 1)
#define HBB			0x30
#define HBE			0x32
#define HDB1		0x38
#define HDB2		0x3A
#define HDE			0x3C
#define VP			0x3E		// Value ranges from 1 - 2048 (value written + 1)
#define VBB			0x40
#define VBE			0x42
#define VS			0x44
#define VDB			0x46
#define VDE			0x48
#define VI			0x4E
#define BG			0x58

//This can be defined in the makefile as well...
//(It's easier to do it here, though...)
//#define TOM_DEBUG

extern uint32 jaguar_mainRom_crc32;
//extern Console console;
//extern Surface * surface;
extern uint8 objectp_running;

static uint8 * tom_ram_8;
uint32 tom_width, tom_height, tom_real_internal_width;
static uint32 tom_timer_prescaler;
static uint32 tom_timer_divider;
static int32 tom_timer_counter;
uint32 tom_scanline;
uint32 hblankWidthInPixels = 0;
uint16 tom_puck_int_pending;
uint16 tom_timer_int_pending;
uint16 tom_object_int_pending;
uint16 tom_gpu_int_pending;
uint16 tom_video_int_pending;
uint16 * tom_cry_rgb_mix_lut;

static char * videoMode_to_str[8] =
	{ "16 bpp CRY", "24 bpp RGB", "16 bpp DIRECT", "16 bpp RGB",
	  "Mixed mode", "24 bpp RGB", "16 bpp DIRECT", "16 bpp RGB" };

typedef void (render_xxx_scanline_fn)(int16 *);

// Private function prototypes

void tom_render_16bpp_cry_scanline(int16 * backbuffer);
void tom_render_24bpp_scanline(int16 * backbuffer);
void tom_render_16bpp_direct_scanline(int16 * backbuffer);
void tom_render_16bpp_rgb_scanline(int16 * backbuffer);
void tom_render_16bpp_cry_rgb_mix_scanline(int16 * backbuffer);

void tom_render_16bpp_cry_stretch_scanline(int16 * backbuffer);
void tom_render_24bpp_stretch_scanline(int16 * backbuffer);
void tom_render_16bpp_direct_stretch_scanline(int16 * backbuffer);
void tom_render_16bpp_rgb_stretch_scanline(int16 * backbuffer);
void tom_render_16bpp_cry_rgb_mix_stretch_scanline(int16 * backbuffer);

render_xxx_scanline_fn * scanline_render_normal[]=
{
	tom_render_16bpp_cry_scanline,
	tom_render_24bpp_scanline,
	tom_render_16bpp_direct_scanline,
	tom_render_16bpp_rgb_scanline,
	tom_render_16bpp_cry_rgb_mix_scanline,
	tom_render_24bpp_scanline,
	tom_render_16bpp_direct_scanline,
	tom_render_16bpp_rgb_scanline,
};

render_xxx_scanline_fn * scanline_render_stretch[]=
{
	tom_render_16bpp_cry_stretch_scanline,
	tom_render_24bpp_stretch_scanline,
	tom_render_16bpp_direct_stretch_scanline,
	tom_render_16bpp_rgb_stretch_scanline,
	tom_render_16bpp_cry_rgb_mix_stretch_scanline,
	tom_render_24bpp_stretch_scanline,
	tom_render_16bpp_direct_stretch_scanline,
	tom_render_16bpp_rgb_stretch_scanline,
};

render_xxx_scanline_fn * scanline_render[8];


void tom_calc_cry_rgb_mix_lut(void)
{
	memory_malloc_secure((void **)&tom_cry_rgb_mix_lut, 2 * 0x10000, "CRY/RGB mixed mode LUT");

	for (uint32 i=0; i<0x10000; i++)
	{
		uint16 color = i;

		if (color & 0x01)
		{
			color >>= 1;
			color = (color & 0x007C00) | ((color & 0x00003E0) >> 5) | ((color & 0x0000001F) << 5);
		}
		else
		{
			uint32 chrm = (color & 0xF000) >> 12,
				chrl = (color & 0x0F00) >> 8,
				y = color & 0x00FF;
			uint16 red = (((uint32)redcv[chrm][chrl]) * y) >> 11,
				green = (((uint32)greencv[chrm][chrl]) * y) >> 11,
				blue = (((uint32)bluecv[chrm][chrl]) * y) >> 11;
			color = (red << 10) | (green << 5) | blue;
		}
		tom_cry_rgb_mix_lut[i] = color;
	}
}

void tom_set_pending_puck_int(void)
{
	tom_puck_int_pending = 1;
}

void tom_set_pending_timer_int(void)
{
	tom_timer_int_pending = 1;
}

void tom_set_pending_object_int(void)
{
	tom_object_int_pending = 1;
}

void tom_set_pending_gpu_int(void)
{
	tom_gpu_int_pending = 1;
}

void tom_set_pending_video_int(void)
{
	tom_video_int_pending = 1;
}

uint8 * tom_get_ram_pointer(void)
{
	return tom_ram_8;
}

uint8 tom_getVideoMode(void)
{
	uint16 vmode = GET16(tom_ram_8, VMODE);
	return ((vmode & VARMOD) >> 6) | ((vmode & MODE) >> 1);
}

uint16 tom_get_scanline(void)
{
	return tom_scanline;
}

/*uint16 tom_get_hdb(void)
{
	return GET16(tom_ram_8, HDB);
}*/

uint16 tom_get_vdb(void)
{
	// This in NOT VDB!!!
//	return GET16(tom_ram_8, VBE);
	return GET16(tom_ram_8, VDB);
}

//
// 16 BPP CRY/RGB mixed mode rendering
//
void tom_render_16bpp_cry_rgb_mix_scanline(int16 * backbuffer)
{
	uint16 width = tom_width;
	uint8 * current_line_buffer = (uint8 *)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color = (*current_line_buffer++) << 8;
		color |= *current_line_buffer++;
		*backbuffer++ = tom_cry_rgb_mix_lut[color];
		width--;
	}
}

//
// 16 BPP CRY mode rendering
//
void tom_render_16bpp_cry_scanline(int16 * backbuffer)
{
	uint16 width = tom_width;
	uint8 * current_line_buffer = (uint8 *)&tom_ram_8[0x1800];

	while (width)
	{
		uint16 color = (*current_line_buffer++) << 8;
		color |= *current_line_buffer++;
		
		uint32 chrm = (color & 0xF000) >> 12,
			chrl = (color & 0x0F00) >> 8,
			y = (color & 0x00FF);
				
		uint16 red   = (((uint32)redcv[chrm][chrl]) * y) >> 11,
			green = (((uint32)greencv[chrm][chrl]) * y) >> 11,
			blue  = (((uint32)bluecv[chrm][chrl]) * y) >> 11;
		
		*backbuffer++ = (red << 10) | (green << 5) | blue;
		width--;
	}
}

//
// 24 BPP mode rendering
//
void tom_render_24bpp_scanline(int16 * backbuffer)
{
	uint16 width = tom_width;
	uint8 * current_line_buffer = (uint8 *)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 green = (*current_line_buffer++) >> 3;
		uint16 red = (*current_line_buffer++) >> 3;
		current_line_buffer++;
		uint16 blue = (*current_line_buffer++) >> 3;
		*backbuffer++ = (red << 10) | (green << 5) | blue;
		width--;
	}
}

//
// 16 BPP direct mode rendering
//
void tom_render_16bpp_direct_scanline(int16 * backbuffer)
{
	uint16 width = tom_width;
	uint8 * current_line_buffer = (uint8 *)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color = (*current_line_buffer++) << 8;
		color |= *current_line_buffer++;
		*backbuffer++ = color >> 1;
		width--;
	}
}

//
// 16 BPP RGB mode rendering
//
void tom_render_16bpp_rgb_scanline(int16 * backbuffer)
{
	uint16 width = tom_width;
	uint8 * current_line_buffer = (uint8 *)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color = (*current_line_buffer++) << 8;
		color = (color | *current_line_buffer++) >> 1;
		color = (color&0x7C00) | ((color&0x03E0) >> 5) | ((color&0x001F) << 5);
		*backbuffer++ = color;
		width--;
	}
}

// This stuff may just go away by itself, especially if we do some
// good old OpenGL goodness...

void tom_render_16bpp_cry_rgb_mix_stretch_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color;
		color=*current_line_buffer++;
		color<<=8;
		color|=*current_line_buffer++;
		*backbuffer++=tom_cry_rgb_mix_lut[color];
		current_line_buffer+=2;
		width--;
	}
}

void tom_render_16bpp_cry_stretch_scanline(int16 *backbuffer)
{
	uint32 chrm, chrl, y;

	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color;
		color=*current_line_buffer++;
		color<<=8;
		color|=*current_line_buffer++;
		
		chrm = (color & 0xF000) >> 12;    
		chrl = (color & 0x0F00) >> 8;
		y    = (color & 0x00FF);
				
		uint16 red   =	((((uint32)redcv[chrm][chrl])*y)>>11);
		uint16 green =	((((uint32)greencv[chrm][chrl])*y)>>11);
		uint16 blue  =	((((uint32)bluecv[chrm][chrl])*y)>>11);
		
		uint16 color2;
		color2=*current_line_buffer++;
		color2<<=8;
		color2|=*current_line_buffer++;
		
		chrm = (color2 & 0xF000) >> 12;    
		chrl = (color2 & 0x0F00) >> 8;
		y    = (color2 & 0x00FF);
				
		uint16 red2   =	((((uint32)redcv[chrm][chrl])*y)>>11);
		uint16 green2 =	((((uint32)greencv[chrm][chrl])*y)>>11);
		uint16 blue2  =	((((uint32)bluecv[chrm][chrl])*y)>>11);
		
		red=(red+red2)>>1;
		green=(green+green2)>>1;
		blue=(blue+blue2)>>1;

		*backbuffer++=(red<<10)|(green<<5)|blue;
		width--;
	}
}

void tom_render_24bpp_stretch_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 green=*current_line_buffer++;
		uint16 red=*current_line_buffer++;
		/*uint16 nc=*/current_line_buffer++;
		uint16 blue=*current_line_buffer++;
		red>>=3;
		green>>=3;
		blue>>=3;
		*backbuffer++=(red<<10)|(green<<5)|blue;
		current_line_buffer+=4;
		width--;
	}
}

void tom_render_16bpp_direct_stretch_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color=*current_line_buffer++;
		color<<=8;
		color|=*current_line_buffer++;
		color>>=1;
		*backbuffer++=color;
		current_line_buffer+=2;
		width--;
	}
}

void tom_render_16bpp_rgb_stretch_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color1=*current_line_buffer++;
		color1<<=8;
		color1|=*current_line_buffer++;
		color1>>=1;
		uint16 color2=*current_line_buffer++;
		color2<<=8;
		color2|=*current_line_buffer++;
		color2>>=1;
		uint16 red=(((color1&0x7c00)>>10)+((color2&0x7c00)>>10))>>1;
		uint16 green=(((color1&0x00003e0)>>5)+((color2&0x00003e0)>>5))>>1;
		uint16 blue=(((color1&0x0000001f))+((color2&0x0000001f)))>>1;

		color1=(red<<10)|(blue<<5)|green;
		*backbuffer++=color1;
		width--;
	}
}

//
// Process a single scanline
//
void tom_exec_scanline(int16 * backbuffer, int32 scanline, bool render)
{
	tom_scanline = scanline;

	// Increment the horizontal count (why? RNG?)
//	tom_word_write(0xF00004, tom_word_read(0xF00004) + 1);

	if (render)
	{
		uint8 * current_line_buffer = (uint8 *)&tom_ram_8[0x1800];
		uint8 bgHI = tom_ram_8[BG], bgLO = tom_ram_8[BG+1];

		// Clear line buffer with BG
		if (GET16(tom_ram_8, VMODE) & BGEN) // && (CRY or RGB16)...
			for(uint32 i=0; i<720; i++)
				*current_line_buffer++ = bgHI, *current_line_buffer++ = bgLO;

//		op_process_list(backbuffer, scanline, render);
		OPProcessList(scanline, render);
		
		scanline_render[tom_getVideoMode()](backbuffer);
	}
}

uint32 TOMGetSDLScreenPitch(void)
{
	extern SDL_Surface * surface;

	return surface->pitch;
}

//
// TOM initialization
//
void tom_init(void)
{
	op_init();
	blitter_init();
//This should be done by JERRY!	pcm_init();
	memory_malloc_secure((void **)&tom_ram_8, 0x4000, "TOM RAM");
	tom_reset();
	// Setup the non-stretchy scanline rendering...
	memcpy(scanline_render, scanline_render_normal, sizeof(scanline_render));
	tom_calc_cry_rgb_mix_lut();
}

void tom_done(void)
{
	op_done();
//This should be done by JERRY!	pcm_done();
	blitter_done();
	WriteLog("TOM: Resolution %i x %i %s\n", tom_getVideoModeWidth(), tom_getVideoModeHeight(),
		videoMode_to_str[tom_getVideoMode()]);
//	WriteLog("\ntom: object processor:\n");
//	WriteLog("tom: pointer to object list: 0x%.8x\n",op_get_list_pointer());
//	WriteLog("tom: INT1=0x%.2x%.2x\n",tom_byte_read(0xf000e0),tom_byte_read(0xf000e1));
	gpu_done();
	dsp_done();
	memory_free(tom_ram_8);
}

uint32 tom_getHBlankWidthInPixels(void)
{
	return hblankWidthInPixels;
}

uint32 tom_getVideoModeWidth(void)
{
	uint16 vmode = GET16(tom_ram_8, VMODE);
	uint16 hdb1 = GET16(tom_ram_8, HDB1);
//	uint16 hde = GET16(tom_ram_8, HDE);
//	uint16 hbb = GET16(tom_ram_8, HBB);
//	uint16 hbe = GET16(tom_ram_8, HBE);

	// NOTE: PWIDTH is value + 1...!
	int pwidth = ((vmode & PWIDTH) >> 9) + 1;
	// Also note that the JTRM says that PWIDTH of 4 gives pixels that are "about" square--
	// this implies that the other modes have pixels that are *not* square!

	uint32 width = 640;
	switch (pwidth)
	{
/*	case 1: width = 640; break;
	case 2: width = 640; break;
	case 3: width = 448; break;
	case 4: width = 320; break;
	case 5: width = 256; break;
	case 6: width = 256; break;
	case 7: width = 256; break;
	case 8: width = 320; break;//*/
	case 1: width = 1330; break;		// 0.25:1 pixels (X:Y ratio)
	case 2: width = 665; break;			// 0.50:1 pixels
	case 3: width = 443; break;			// 0.75:1 pixels
	case 4: width = 332; break;			// 1.00:1 pixels
	case 5: width = 266; break;			// 1.25:1 pixels
	case 6: width = 222; break;			// 1.50:1 pixels
	case 7: width = 190; break;			// 1.75:1 pixels
	case 8: width = 166; break;			// 2.00:1 pixels
//Temporary, for testing Doom...
//	case 8: width = 332; break;			// 2.00:1 pixels
//*/
	}
	
	if (hdb1 == 123)
		hblankWidthInPixels = 16;
	else
		hblankWidthInPixels = 0;

//	WriteLog("TOM: HDB1=%i HBE=%i\n", hdb1, hbe);
	return width;
}

// *** SPECULATION ***
// It might work better to virtualize the height settings, i.e., set the vertical
// height at 240 lines and clip using the VDB and VDE/VP registers...
// Same with the width...

uint32 tom_getVideoModeHeight(void)
{
//	uint16 vmode = GET16(tom_ram_8, VMODE);
	uint16 vbe = GET16(tom_ram_8, VBE);
	uint16 vbb = GET16(tom_ram_8, VBB);
//	uint16 vdb = GET16(tom_ram_8, VDB);
//	uint16 vde = GET16(tom_ram_8, VDE);
//	uint16 vp = GET16(tom_ram_8, VP);
	
/*	if (vde == 0xFFFF)
		vde = vbb;//*/

//	return 227;//WAS:(vde/*-vdb*/) >> 1;
	// The video mode height probably works this way:
	// VC counts from 0 to VP. VDB starts the OP. Either when
	// VDE is reached or VP, the OP is stopped. Let's try it...
	// Also note that we're conveniently ignoring interlaced display modes...!
//	return ((vde > vp ? vp : vde) - vdb) >> 1;
//	return ((vde > vbb ? vbb : vde) - vdb) >> 1;
//Let's try from the Vertical Blank interval...
	return (vbb - vbe) >> 1;
}

//
// TOM reset code
// NOTE: Should set up PAL values here when in PAL mode (use BIOS to find default values)
//       for when user starts with -nobios -pal flags... [DONE]
//
void tom_reset(void)
{
	extern bool hardwareTypeNTSC;

	op_reset();
	blitter_reset();
//This should be done by JERRY!		pcm_reset();

	memset(tom_ram_8, 0x00, 0x4000);

	if (hardwareTypeNTSC)
	{
		SET16(tom_ram_8, MEMCON1, 0x1861);
		SET16(tom_ram_8, MEMCON2, 0x35CC);
		SET16(tom_ram_8, HP, 844);					// Horizontal Period
		SET16(tom_ram_8, HBB, 1713);				// Horizontal Blank Begin
		SET16(tom_ram_8, HBE, 125);					// Horizontal Blank End
		SET16(tom_ram_8, HDE, 1665);				// Horizontal Display End
		SET16(tom_ram_8, HDB1, 203);				// Horizontal Display Begin 1
		SET16(tom_ram_8, VP, 523);					// Vertical Period (1-based; in this case VP = 524)
		SET16(tom_ram_8, VBE, 24);					// Vertical Blank End
		SET16(tom_ram_8, VDB, 38);					// Vertical Display Begin
		SET16(tom_ram_8, VDE, 518);					// Vertical Display End
		SET16(tom_ram_8, VBB, 500);					// Vertical Blank Begin
		SET16(tom_ram_8, VS, 517);					// Vertical Sync
		SET16(tom_ram_8, VMODE, 0x06C1);
	}
	else	// PAL Jaguar
	{
		SET16(tom_ram_8, MEMCON1, 0x1861);
		SET16(tom_ram_8, MEMCON2, 0x35CC);
		SET16(tom_ram_8, HP, 850);					// Horizontal Period
		SET16(tom_ram_8, HBB, 1711);				// Horizontal Blank Begin
		SET16(tom_ram_8, HBE, 158);					// Horizontal Blank End
		SET16(tom_ram_8, HDE, 1665);				// Horizontal Display End
		SET16(tom_ram_8, HDB1, 203);				// Horizontal Display Begin 1
		SET16(tom_ram_8, VP, 623);					// Vertical Period (1-based; in this case VP = 624)
		SET16(tom_ram_8, VBE, 34);					// Vertical Blank End
		SET16(tom_ram_8, VDB, 38);					// Vertical Display Begin
		SET16(tom_ram_8, VDE, 518);					// Vertical Display End
		SET16(tom_ram_8, VBB, 600);					// Vertical Blank Begin
		SET16(tom_ram_8, VS, 618);					// Vertical Sync
		SET16(tom_ram_8, VMODE, 0x06C1);
	}

	tom_width = tom_real_internal_width = 0;
	tom_height = 0;
	tom_scanline = 0;

	hblankWidthInPixels = GET16(tom_ram_8, HDB1) >> 1;

	tom_puck_int_pending = 0;
	tom_timer_int_pending = 0;
	tom_object_int_pending = 0;
	tom_gpu_int_pending = 0;
	tom_video_int_pending = 0;

	tom_timer_prescaler = 0;
	tom_timer_divider = 0;
	tom_timer_counter = 0;
	memcpy(scanline_render, scanline_render_normal, sizeof(scanline_render));
} 

//
// TOM byte access (read)
//

unsigned tom_byte_read(unsigned int offset)
{
//???Is this needed???
// It seems so. Perhaps it's the +$8000 offset being written to (32-bit interface)?
// However, the 32-bit interface is WRITE ONLY, so that can't be it...
// Also, the 68K CANNOT make use of the 32-bit interface, since its bus width is only 16-bits...
//	offset &= 0xFF3FFF;

#ifdef TOM_DEBUG
	WriteLog("TOM: Reading byte at %06X\n", offset);
#endif

	if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
		return gpu_byte_read(offset);
	else if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
		return gpu_byte_read(offset);
	else if ((offset >= 0xF00010) && (offset < 0xF00028))
		return op_byte_read(offset);
	else if ((offset >= 0xF02200) && (offset < 0xF022A0))
		return blitter_byte_read(offset);
	else if (offset == 0xF00050)
		return tom_timer_prescaler >> 8;
	else if (offset == 0xF00051)
		return tom_timer_prescaler & 0xFF;
	else if (offset == 0xF00052)
		return tom_timer_divider >> 8;
	else if (offset == 0xF00053)
		return tom_timer_divider & 0xFF;

	return tom_ram_8[offset & 0x3FFF];
}

//
// TOM word access (read)
//

unsigned tom_word_read(unsigned int offset)
{
//???Is this needed???
//	offset &= 0xFF3FFF;
#ifdef TOM_DEBUG
	WriteLog("TOM: Reading word at %06X\n", offset);
#endif
if (offset >= 0xF02000 && offset <= 0xF020FF)
	WriteLog("TOM: Read attempted from GPU register file (unimplemented)!\n");

	if (offset == 0xF000E0)
	{
		uint16 data = (tom_puck_int_pending << 4) | (tom_timer_int_pending << 3)
			| (tom_object_int_pending << 2) | (tom_gpu_int_pending << 1)
			| (tom_video_int_pending << 0);
		//WriteLog("tom: interrupt status is 0x%.4x \n",data);
		return data;
	}
//Shoud be handled by the jaguar main loop now...
/*	else if (offset == 0xF00006)	// VC
	// What if we're in interlaced mode?
	// According to docs, in non-interlace mode VC is ALWAYS even...
//		return (tom_scanline << 1);// + 1;
//But it's causing Rayman to be fucked up... Why???
//Because VC is even in NI mode when calling the OP! That's why!
		return (tom_scanline << 1) + 1;//*/
	else if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
		return gpu_word_read(offset);
	else if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
		return gpu_word_read(offset);
	else if ((offset >= 0xF00010) && (offset < 0xF00028))
		return op_word_read(offset);
	else if ((offset >= 0xF02200) && (offset < 0xF022A0))
		return blitter_word_read(offset);
	else if (offset == 0xF00050)
		return tom_timer_prescaler;
	else if (offset == 0xF00052)
		return tom_timer_divider;

	offset &= 0x3FFF;
	return (tom_byte_read(offset) << 8) | tom_byte_read(offset+1);
}

//
// TOM byte access (write)
//

void tom_byte_write(unsigned offset, unsigned data)
{
//???Is this needed???
// Perhaps on the writes--32-bit writes that is! And masked with FF7FFF...
	offset &= 0xFF3FFF;

#ifdef TOM_DEBUG
	WriteLog("TOM: Writing byte %02X at %06X\n", data, offset);
#endif

	if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
	{
		gpu_byte_write(offset, data);
		return;
	}
	else if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
	{
		gpu_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF00010) && (offset < 0xF00028))
	{
		op_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF02200) && (offset < 0xF022A0))
	{
		blitter_byte_write(offset, data);
		return;
	}
	else if (offset == 0xF00050)
	{
		tom_timer_prescaler = (tom_timer_prescaler & 0x00FF) | (data << 8);
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF00051)
	{
		tom_timer_prescaler = (tom_timer_prescaler & 0xFF00) | data;
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF00052)
	{
		tom_timer_divider = (tom_timer_divider & 0x00FF) | (data << 8);
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF00053)
	{
		tom_timer_divider = (tom_timer_divider & 0xFF00) | data;
		tom_reset_timer();
		return;
	}
	else if (offset >= 0xF00400 && offset <= 0xF007FF)	// CLUT (A & B)
	{
		// Writing to one CLUT writes to the other
		offset &= 0x5FF;		// Mask out $F00600 (restrict to $F00400-5FF)
		tom_ram_8[offset] = data, tom_ram_8[offset + 0x200] = data;
	}

	tom_ram_8[offset & 0x3FFF] = data;
}

//
// TOM word access (write)
//

void tom_word_write(unsigned offset, unsigned data)
{
//???Is this needed???
	offset &= 0xFF3FFF;

#ifdef TOM_DEBUG
	WriteLog("TOM: Writing word %04X at %06X\n", data, offset);
#endif
if (offset == 0xF00000 + MEMCON1)
	WriteLog("TOM: Memory Configuration 1 written: %04X\n", data);
if (offset == 0xF00000 + MEMCON2)
	WriteLog("TOM: Memory Configuration 2 written: %04X\n", data);
if (offset >= 0xF02000 && offset <= 0xF020FF)
	WriteLog("TOM: Write attempted to GPU register file (unimplemented)!\n");

	if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
	{
		gpu_word_write(offset, data);
		return;
	}
	else if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
	{
		gpu_word_write(offset, data);
		return;
	}
//What's so special about this?
/*	else if ((offset >= 0xF00000) && (offset < 0xF00002))
	{
		tom_byte_write(offset, data >> 8);
		tom_byte_write(offset+1, data & 0xFF);
	}*/
	else if ((offset >= 0xF00010) && (offset < 0xF00028))
	{
		op_word_write(offset, data);
		return;
	}
	else if (offset == 0xF00050)
	{
		tom_timer_prescaler = data;
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF00052)
	{
		tom_timer_divider = data;
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF000E0)
	{
//Check this out...
		if (data & 0x0100)
			tom_video_int_pending = 0;
		if (data & 0x0200)
			tom_gpu_int_pending = 0;
		if (data & 0x0400)
			tom_object_int_pending = 0;
		if (data & 0x0800)
			tom_timer_int_pending = 0;
		if (data & 0x1000)
			tom_puck_int_pending = 0;
	}
	else if ((offset >= 0xF02200) && (offset <= 0xF0229F))
	{
		blitter_word_write(offset, data);
		return;
	}
	else if (offset >= 0xF00400 && offset <= 0xF007FE)	// CLUT (A & B)
	{
		// Writing to one CLUT writes to the other
		offset &= 0x5FF;		// Mask out $F00600 (restrict to $F00400-5FF)
// Watch out for unaligned writes here! (Not fixed yet)
		SET16(tom_ram_8, offset, data), SET16(tom_ram_8, offset + 0x200, data);
	}

	offset &= 0x3FFF;
	if (offset == 0x28)			// VMODE (Why? Why not OBF?)
		objectp_running = 1;

	if (offset >= 0x30 && offset <= 0x4E)
		data &= 0x07FF;			// These are (mostly) 11-bit registers
	if (offset == 0x2E || offset == 0x36 || offset == 0x54)
		data &= 0x03FF;			// These are all 10-bit registers

	tom_byte_write(offset, data >> 8);
	tom_byte_write(offset+1, data & 0xFF);

if (offset == VDB)
	WriteLog("TOM: Vertical Display Begin written: %u\n", data);
if (offset == VDE)
	WriteLog("TOM: Vertical Display End written: %u\n", data);
if (offset == VP)
	WriteLog("TOM: Vertical Period written: %u (%sinterlaced)\n", data, (data & 0x01 ? "non-" : ""));
if (offset == HDB1)
	WriteLog("TOM: Horizontal Display Begin 1 written: %u\n", data);
if (offset == HDE)
	WriteLog("TOM: Horizontal Display End written: %u\n", data);
if (offset == HP)
	WriteLog("TOM: Horizontal Period written: %u\n", data);
if (offset == VBB)
	WriteLog("TOM: Vertical Blank Begin written: %u\n", data);
if (offset == VBE)
	WriteLog("TOM: Vertical Blank End written: %u\n", data);
if (offset == VS)
	WriteLog("TOM: Vertical Sync written: %u\n", data);
if (offset == VI)
	WriteLog("TOM: Vertical Interrupt written: %u\n", data);
if (offset == HBB)
	WriteLog("TOM: Horizontal Blank Begin written: %u\n", data);
if (offset == HBE)
	WriteLog("TOM: Horizontal Blank End written: %u\n", data);
if (offset == VMODE)
	WriteLog("TOM: Video Mode written: %04X (PWIDTH = %u, VC = %u)\n", data, ((data >> 9) & 0x07) + 1, GET16(tom_ram_8, VC));

	// detect screen resolution changes
//This may go away in the future, if we do the virtualized screen thing...
	if ((offset >= 0x28) && (offset <= 0x4F))
	{
		uint32 width = tom_getVideoModeWidth(), height = tom_getVideoModeHeight();
		tom_real_internal_width = width;

//This looks like an attempt to render non-square pixels (though wrong...)
/*		if (width == 640)
		{
			memcpy(scanline_render, scanline_render_stretch, sizeof(scanline_render));
			width = 320;
		}
		else
			memcpy(scanline_render, scanline_render_normal, sizeof(scanline_render));//*/
		
		if ((width != tom_width) || (height != tom_height))
		{
			extern SDL_Surface * surface, * mainSurface;
			extern Uint32 mainSurfaceFlags;
//			ws_audio_done();
		
			static char window_title[256];
//			delete surface;
			
			tom_width = width, tom_height = height;
//			Format format(16, 0x007C00, 0x00003E0, 0x0000001F);
//			surface = new Surface(tom_width, tom_height, format);
			SDL_FreeSurface(surface);
			surface = SDL_CreateRGBSurface(SDL_SWSURFACE, tom_width, tom_height,
				16, 0x7C00, 0x03E0, 0x001F, 0);
			if (surface == NULL)
			{
				WriteLog("TOM: Could not create primary SDL surface: %s", SDL_GetError());
				exit(1);
			}

			sprintf(window_title, "Virtual Jaguar (%i x %i)", (int)tom_width, (int)tom_height);
//			console.close();
//			console.open(window_title, width, tom_height, format);
//???Should we do this???
//	SDL_FreeSurface(mainSurface);
			mainSurface = SDL_SetVideoMode(tom_width, tom_height, 16, mainSurfaceFlags);

			if (mainSurface == NULL)
			{
				WriteLog("Joystick: SDL is unable to set the video mode: %s\n", SDL_GetError());
				exit(1);
			}

			SDL_WM_SetCaption(window_title, window_title);

//			ws_audio_init();
//			ws_audio_reset();
		}
	}
}

int tom_irq_enabled(int irq)
{
	// This is the correct byte in big endian... D'oh!
//	return jaguar_byte_read(0xF000E1) & (1 << irq);
	return tom_ram_8[0xE1] & (1 << irq);
}

//unused
/*void tom_set_irq_latch(int irq, int enabled)
{
	tom_ram_8[0xE0] = (tom_ram_8[0xE0] & (~(1<<irq))) | (enabled ? (1<<irq) : 0);
}*/

//unused
/*uint16 tom_irq_control_reg(void)
{
	return (tom_ram_8[0xE0] << 8) | tom_ram_8[0xE1];
}*/

void tom_reset_timer(void)
{
	if (!tom_timer_prescaler || !tom_timer_divider)
		tom_timer_counter = 0;
	else
		tom_timer_counter = (1 + tom_timer_prescaler) * (1 + tom_timer_divider);
//	WriteLog("tom: reseting timer to 0x%.8x (%i)\n",tom_timer_counter,tom_timer_counter);
}

void tom_pit_exec(uint32 cycles)
{
	if (tom_timer_counter > 0)
	{
		tom_timer_counter -= cycles;

		if (tom_timer_counter <= 0)
		{
			tom_set_pending_timer_int();
			GPUSetIRQLine(2, ASSERT_LINE);
			if ((tom_irq_enabled(IRQ_TIMER)) && (jaguar_interrupt_handler_is_valid(64)))
				m68k_set_irq(7);				// Cause a 68000 NMI...

			tom_reset_timer();
		}
	}
}
