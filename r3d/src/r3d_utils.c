/*
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "r3d.h"

#include "./r3d_state.h"
#include <raylib.h>

void R3D_SetMaterialAlbedo(Material* material, Texture2D* texture, Color color)
{
    if (material == NULL) {
        return;
    }

    MaterialMap* map = &material->maps[MATERIAL_MAP_ALBEDO];

    if (texture != NULL) {
        map->texture = *texture;
    }
    else if (map->texture.id == 0) {
        map->texture = R3D_GetWhiteTexture();
    }

    map->color = color;
}

void R3D_SetMaterialOcclusion(Material* material, Texture2D* texture, float value)
{
    if (material == NULL) {
        return;
    }

    MaterialMap* map = &material->maps[MATERIAL_MAP_OCCLUSION];

    if (texture != NULL) {
        map->texture = *texture;
    }
    else if (map->texture.id == 0) {
        map->texture = R3D_GetWhiteTexture();
    }

    map->value = value;
}

void R3D_SetMaterialRoughness(Material* material, Texture2D* texture, float value)
{
    if (material == NULL) {
        return;
    }

    MaterialMap* map = &material->maps[MATERIAL_MAP_ROUGHNESS];

    if (texture != NULL) {
        map->texture = *texture;
    }
    else if (map->texture.id == 0) {
        map->texture = R3D_GetWhiteTexture();
    }

    map->value = value;
}

void R3D_SetMaterialMetalness(Material* material, Texture2D* texture, float value)
{
    if (material == NULL) {
        return;
    }

    MaterialMap* map = &material->maps[MATERIAL_MAP_METALNESS];

    if (texture != NULL) {
        map->texture = *texture;
    }
    else if (map->texture.id == 0) {
        map->texture = R3D_GetWhiteTexture();
    }

    map->value = value;
}

void R3D_SetMaterialEmission(Material* material, Texture2D* texture, Color color, float value)
{
    if (material == NULL) {
        return;
    }

    MaterialMap* map = &material->maps[MATERIAL_MAP_EMISSION];

    if (texture != NULL) {
        map->texture = *texture;
    }
    else if (map->texture.id == 0) {
        map->texture = R3D_GetWhiteTexture();
    }

    map->color = color;
    map->value = value;
}

Texture2D R3D_GetWhiteTexture(void)
{
    Texture2D texture = { 0 };
    texture.id = R3D.texture.white;
    texture.width = 1;
    texture.height = 1;
    texture.mipmaps = 1;
    texture.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    return texture;
}

Texture2D R3D_GetBlackTexture(void)
{
    Texture2D texture = { 0 };
    texture.id = R3D.texture.black;
    texture.width = 1;
    texture.height = 1;
    texture.mipmaps = 1;
    texture.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    return texture;
}

Texture2D R3D_GetNormalTexture(void)
{
    Texture2D texture = { 0 };
    texture.id = R3D.texture.normal;
    texture.width = 1;
    texture.height = 1;
    texture.mipmaps = 1;
    texture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32;
    return texture;
}

Texture2D R3D_GetBufferColor(void)
{
    Texture2D texture = { 0 };
    texture.id = R3D.framebuffer.post.target;
    texture.width = R3D.state.resolution.width;
    texture.height = R3D.state.resolution.height;
    texture.mipmaps = 1;
    texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    return texture;
}

Texture2D R3D_GetBufferNormal(void)
{
    Texture2D texture = { 0 };
    texture.id = R3D.framebuffer.gBuffer.normal;
    texture.width = R3D.state.resolution.width;
    texture.height = R3D.state.resolution.height;
    texture.mipmaps = 1;
    texture.format = PIXELFORMAT_UNCOMPRESSED_R32;
    return texture;
}

Texture2D R3D_GetBufferDepth(void)
{
    Texture2D texture = { 0 };
    texture.id = R3D.framebuffer.gBuffer.depth;
    texture.width = R3D.state.resolution.width;
    texture.height = R3D.state.resolution.height;
    texture.mipmaps = 1;
    texture.format = PIXELFORMAT_UNCOMPRESSED_R32;
    return texture;
}

Matrix R3D_GetMatrixView(void)
{
    return R3D.state.transform.view;
}

Matrix R3D_GetMatrixInvView(void)
{
    return R3D.state.transform.invView;
}

Matrix R3D_GetMatrixProjection(void)
{
    return R3D.state.transform.proj;
}

Matrix R3D_GetMatrixInvProjection(void)
{
    return R3D.state.transform.invProj;
}

void R3D_DrawBufferAlbedo(float x, float y, float w, float h)
{
    Texture2D tex = {
        .id = R3D.framebuffer.gBuffer.albedo,
        .width = R3D.state.resolution.width,
        .height = R3D.state.resolution.width
    };

    DrawTexturePro(
        tex, (Rectangle) { 0, 0, (float)tex.width, (float)tex.height },
        (Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
    );

    DrawRectangleLines(
        (int)(x + 0.5f), (int)(y + 0.5f),
        (int)(w + 0.5f), (int)(h + 0.5f),
        (Color) { 255, 0, 0, 255 }
    );
}

void R3D_DrawBufferEmission(float x, float y, float w, float h)
{
    Texture2D tex = {
        .id = R3D.framebuffer.gBuffer.emission,
        .width = R3D.state.resolution.width,
        .height = R3D.state.resolution.height
    };

    DrawTexturePro(
        tex, (Rectangle) { 0, 0, (float)tex.width, (float)tex.height },
        (Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
    );

    DrawRectangleLines(
        (int)(x + 0.5f), (int)(y + 0.5f),
        (int)(w + 0.5f), (int)(h + 0.5f),
        (Color) { 255, 0, 0, 255 }
    );
}

void R3D_DrawBufferNormal(float x, float y, float w, float h)
{
    Texture2D tex = {
        .id = R3D.framebuffer.gBuffer.normal,
        .width = R3D.state.resolution.width,
        .height = R3D.state.resolution.height
    };

    DrawTexturePro(
        tex, (Rectangle) { 0, 0, (float)tex.width, (float)tex.height },
        (Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
    );

    DrawRectangleLines(
        (int)(x + 0.5f), (int)(y + 0.5f),
        (int)(w + 0.5f), (int)(h + 0.5f),
        (Color) { 255, 0, 0, 255 }
    );
}

void R3D_DrawBufferORM(float x, float y, float w, float h)
{
    Texture2D tex = {
        .id = R3D.framebuffer.gBuffer.orm,
        .width = R3D.state.resolution.width,
        .height = R3D.state.resolution.height
    };

    DrawTexturePro(
        tex, (Rectangle) { 0, 0, (float)tex.width, (float)tex.height },
        (Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
    );

    DrawRectangleLines(
        (int)(x + 0.5f), (int)(y + 0.5f),
        (int)(w + 0.5f), (int)(h + 0.5f),
        (Color) { 255, 0, 0, 255 }
    );
}

void R3D_DrawBufferSSAO(float x, float y, float w, float h)
{
    Texture2D tex = {
        .id = R3D.framebuffer.pingPongSSAO.target,
        .width = R3D.state.resolution.width / 2,
        .height = R3D.state.resolution.height / 2
    };

    DrawTexturePro(
        tex, (Rectangle) { 0, 0, (float)tex.width, (float)tex.height },
        (Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
    );

    DrawRectangleLines(
        (int)(x + 0.5f), (int)(y + 0.5f),
        (int)(w + 0.5f), (int)(h + 0.5f),
        (Color) { 255, 0, 0, 255 }
    );
}

void R3D_DrawBufferBrightColors(float x, float y, float w, float h)
{
    Texture2D tex = {
        .id = R3D.framebuffer.scene.bright,
        .width = R3D.state.resolution.width,
        .height = R3D.state.resolution.height
    };

    DrawTexturePro(
        tex, (Rectangle) { 0, 0, (float)tex.width, (float)tex.height },
        (Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
    );

    DrawRectangleLines(
        (int)(x + 0.5f), (int)(y + 0.5f),
        (int)(w + 0.5f), (int)(h + 0.5f),
        (Color) { 255, 0, 0, 255 }
    );
}

void R3D_DrawBufferBloom(float x, float y, float w, float h)
{
    Texture2D tex = {
        .id = R3D.framebuffer.pingPongBloom.target,
        .width = R3D.state.resolution.width / 2,
        .height = R3D.state.resolution.height / 2
    };

    DrawTexturePro(
        tex, (Rectangle) { 0, 0, (float)tex.width, (float)tex.height },
        (Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
    );

    DrawRectangleLines(
        (int)(x + 0.5f), (int)(y + 0.5f),
        (int)(w + 0.5f), (int)(h + 0.5f),
        (Color) { 255, 0, 0, 255 }
    );
}
