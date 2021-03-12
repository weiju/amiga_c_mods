/**
 * example_01.c - Simple example using the ptplayer library
 * to play a Protracker 2 MOD file.
 */
#include <hardware/custom.h>
#include <graphics/gfxbase.h>
#include <devices/input.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

#include <stdio.h>

// Download from Aminet
// http://aminet.net/package/dev/c/SDI_headers
// http://aminet.net/package/mus/play/ptplayer
#include <SDI/SDI_compiler.h>
#include "ptplayer/ptplayer.h"

extern struct GfxBase *GfxBase;
extern struct Custom custom;

// To handle input
static struct MsgPort *input_mp;
static struct IOStdReq *input_io;
static struct Interrupt handler_info;
static int should_exit;


static struct InputEvent *my_input_handler(__reg("a0") struct InputEvent *event,
                                           __reg("a1") APTR handler_data)
{
    struct InputEvent *result = event, *prev = NULL;

    Forbid();
    // Intercept all raw mouse events before they reach Intuition, ignore
    // everything else
    if (result->ie_Class == IECLASS_RAWMOUSE) {
        if (result->ie_Code == IECODE_LBUTTON) {
            should_exit = 1;
        }
        return NULL;
    }
    Permit();
    return result;
}

static void cleanup_input_handler(void)
{
    if (input_io) {
        // remove our input handler from the chain
        input_io->io_Command = IND_REMHANDLER;
        input_io->io_Data = (APTR) &handler_info;
        DoIO((struct IORequest *) input_io);

        if (!(CheckIO((struct IORequest *) input_io))) AbortIO((struct IORequest *) input_io);
        WaitIO((struct IORequest *) input_io);
        CloseDevice((struct IORequest *) input_io);
        DeleteExtIO((struct IORequest *) input_io);
    }
    if (input_mp) DeletePort(input_mp);
}

static int setup_input_handler(void)
{
    input_mp = CreatePort(0, 0);
    input_io = (struct IOStdReq *) CreateExtIO(input_mp, sizeof(struct IOStdReq));
    BYTE error = OpenDevice("input.device", 0L, (struct IORequest *) input_io, 0);

    handler_info.is_Code = (void (*)(void)) my_input_handler;
    handler_info.is_Data = NULL;
    handler_info.is_Node.ln_Pri = 100;
    handler_info.is_Node.ln_Name = "ex03";
    input_io->io_Command = IND_ADDHANDLER;
    input_io->io_Data = (APTR) &handler_info;
    DoIO((struct IORequest *) input_io);
    return 1;
}

#define MOD_SIZE (57130)
#define MOD_NAME "youtube.mod"
static UBYTE __chip mod_data[MOD_SIZE];

int main(int argc, char **argv)
{
    if (!setup_input_handler()) {
        puts("Could not initialize input handler");
        return 1;
    }
    FILE *fp = fopen(MOD_NAME, "rb");
    int bytes_read = fread(mod_data, sizeof(UBYTE), MOD_SIZE, fp);
    fclose(fp);
    BOOL is_pal = (((struct GfxBase *) GfxBase)->DisplayFlags & PAL) == PAL;

    // initialize ptplayer
    void *p_samples = NULL;
    UBYTE start_pos = 0;
    mt_install_cia(&custom, NULL, is_pal);
    mt_init(&custom, mod_data, p_samples, start_pos);
    mt_Enable = 1;

    while (!should_exit) {
        WaitTOF();
    }

    // Cleanup
    mt_Enable = 0;
    mt_end(&custom);
    mt_remove_cia(&custom);
    cleanup_input_handler();
    return 0;
}
