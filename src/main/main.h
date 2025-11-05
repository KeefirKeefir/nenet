#ifndef MAIN_H
#define MAIN_H

#include "../tests/tests.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../raylib/raylib.h"

#include "../neuron/neuron.h"

#ifdef IMPL_

int main(void) {

#ifdef DEBUG

  run_tests();

#endif

  neuron_t *neurons = new_neuron_array();

  const int screen_x = 128 * 7;
  const int screen_y = 128 * 7;

  InitWindow(screen_x, screen_y, "nenet");

  SetTargetFPS(INT_MAX);

  RenderTexture2D target = LoadRenderTexture(128, 128);

  while (!WindowShouldClose()) {

    BeginTextureMode(target);
    {

      ClearBackground(RAYWHITE);

      neuron_tick(neurons);
    }
    EndTextureMode();

    BeginDrawing();
    {

      ClearBackground(RAYWHITE);

      DrawTexturePro(target.texture,
                     (Rectangle){0.0f, 0.0f, (float)target.texture.width,
                                 (float)-target.texture.height},
                     (Rectangle){0.0f, 0.0f, (float)screen_x, (float)screen_y},
                     (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    }
    EndDrawing();
  }

  UnloadRenderTexture(target);
  CloseWindow();

  return 0;
}

#endif
#endif