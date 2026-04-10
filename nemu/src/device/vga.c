#include "common.h"

#ifdef HAS_IOE

#include "device/mmio.h"
#include "device/port-io.h"
#include <SDL2/SDL.h>

#define VMEM 0x40000
#define SYNC_PORT 0x100

#define SCREEN_H 300
#define SCREEN_W 400

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static uint32_t (*vmem) [SCREEN_W];
static uint32_t *sync_reg;
void update_screen();

static void vga_sync_io_handler(ioaddr_t addr, int len, bool is_write) {
  if (is_write && addr == SYNC_PORT && sync_reg[0] != 0) {
    update_screen();
    sync_reg[0] = 0;
  }
}

void vga_vmem_io_handler(paddr_t addr, int len, bool is_write) {
}

void update_screen() {
  SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(vmem[0][0]));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void init_vga() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(SCREEN_W * 2, SCREEN_H * 2, 0, &window, &renderer);
  SDL_SetWindowTitle(window, "NEMU");
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888,
      SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);

  vmem = add_mmio_map(VMEM, 0x80000, vga_vmem_io_handler);
  sync_reg = add_pio_map(SYNC_PORT, 4, vga_sync_io_handler);
}
#endif	/* HAS_IOE */
