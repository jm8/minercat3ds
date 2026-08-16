#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>
#include <3ds/gfx.h>

#include "render.h"
#include "3ds/allocator/linear.h"
#include "c3d/framebuffer.h"
#include "c3d/renderqueue.h"
#include "game.h"
#include "play.h"
#include "texture_t3x.h"
#include "vshader_shbin.h"

#define CLEAR_COLOR 0x68B0D8FF

#define DISPLAY_TRANSFER_FLAGS (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define UV_X (1.0f / 64.0f)
#define UV_Y (1.0f / 16.0f)
#define TEXTURE_HEIGHT 16
#define UV_EDGE 0

#define CHUNK_MAX_FACE_COUNT (WORLD_WIDTH * WORLD_WIDTH * CHUNK_HEIGHT)
#define CHUNK_MAX_VERTEX_COUNT (6 * CHUNK_MAX_FACE_COUNT)
#define MAX_VISIBLE_CHUNKS 8
#define CAT_VERTEX_COUNT 3

static DVLB_s *vshader_dvlb;
static shaderProgram_s program;
int uLoc_projection, uLoc_modelView;
int uLoc_lightVec, uLoc_lightHalfVec, uLoc_lightClr, uLoc_material;
static C3D_Mtx projection;
static C3D_Mtx material = {{
    {{0.0f, 0.2f, 0.2f, 0.2f}}, // Ambient
    {{0.0f, 0.4f, 0.4f, 0.4f}}, // Diffuse
    {{0.0f, 0.8f, 0.8f, 0.8f}}, // Specular
    {{1.0f, 0.0f, 0.0f, 0.0f}}, // Emission
}};
static C3D_Tex texture_tex;
C3D_Tex cat_tex;

static const vertex cube_faces[] = {
    // First face (PZ)
    // First triangle
    {{.5 + -0.5f, .5 + -0.5f, .5 + +0.5f}, {0.0f + UV_EDGE, 0.0f + UV_EDGE}, {0.0f, 0.0f, +1.0f}},
    {{.5 + +0.5f, .5 + -0.5f, .5 + +0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {0.0f, 0.0f, +1.0f}},
    {{.5 + +0.5f, .5 + +0.5f, .5 + +0.5f}, {UV_X - UV_EDGE, UV_Y - UV_EDGE}, {0.0f, 0.0f, +1.0f}},
    // Second triangle
    {{.5 + +0.5f, .5 + +0.5f, .5 + +0.5f}, {UV_X - UV_EDGE, UV_Y - UV_EDGE}, {0.0f, 0.0f, +1.0f}},
    {{.5 + -0.5f, .5 + +0.5f, .5 + +0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {0.0f, 0.0f, +1.0f}},
    {{.5 + -0.5f, .5 + -0.5f, .5 + +0.5f}, {0.0f + UV_EDGE, 0.0f + UV_EDGE}, {0.0f, 0.0f, +1.0f}},

    // Second face (MZ)
    // First triangle
    {{.5 + -0.5f, .5 + -0.5f, .5 + -0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {0, 0, -1}},
    {{.5 + -0.5f, .5 + +0.5f, .5 + -0.5f}, {UV_X - UV_EDGE, UV_Y - UV_EDGE}, {0, 0, -1}},
    {{.5 + +0.5f, .5 + +0.5f, .5 + -0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {0, 0, -1}},
    // Second triangle
    {{.5 + +0.5f, .5 + +0.5f, .5 + -0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {0, 0, -1}},
    {{.5 + +0.5f, .5 + -0.5f, .5 + -0.5f}, {0.0f + UV_EDGE, 0.0f + UV_EDGE}, {0, 0, -1}},
    {{.5 + -0.5f, .5 + -0.5f, .5 + -0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {0, 0, -1}},

    // Third face (PX)
    // First triangle
    // +X face
    {{.5 + +0.5f, .5 + -0.5f, .5 + -0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {+1, 0, 0}},
    {{.5 + +0.5f, .5 + +0.5f, .5 + -0.5f}, {UV_X - UV_EDGE, UV_Y - UV_EDGE}, {+1, 0, 0}},
    {{.5 + +0.5f, .5 + +0.5f, .5 + +0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {+1, 0, 0}},

    {{.5 + +0.5f, .5 + +0.5f, .5 + +0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {+1, 0, 0}},
    {{.5 + +0.5f, .5 + -0.5f, .5 + +0.5f}, {0.0f + UV_EDGE, 0.0f + UV_EDGE}, {+1, 0, 0}},
    {{.5 + +0.5f, .5 + -0.5f, .5 + -0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {+1, 0, 0}},

    // Fourth face (MX)
    // First triangle
    {{.5 + -0.5f, .5 + -0.5f, .5 + -0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {-1, 0, 0}},
    {{.5 + -0.5f, .5 + -0.5f, .5 + +0.5f}, {0.0f + UV_EDGE, 0.0f + UV_EDGE}, {-1, 0, 0}},
    {{.5 + -0.5f, .5 + +0.5f, .5 + +0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {-1, 0, 0}},
    // Second triangle
    {{.5 + -0.5f, .5 + +0.5f, .5 + +0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {-1, 0, 0}},
    {{.5 + -0.5f, .5 + +0.5f, .5 + -0.5f}, {UV_X - UV_EDGE, UV_Y - UV_EDGE}, {-1, 0, 0}},
    {{.5 + -0.5f, .5 + -0.5f, .5 + -0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {-1, 0, 0}},

    // Fifth face (PY)
    // First triangle
    {{.5 + -0.5f, .5 + +0.5f, .5 + -0.5f}, {0.0f + UV_EDGE, 0.0f + UV_EDGE}, {0.0f, +1.0f, 0.0f}},
    {{.5 + -0.5f, .5 + +0.5f, .5 + +0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {0.0f, +1.0f, 0.0f}},
    {{.5 + +0.5f, .5 + +0.5f, .5 + +0.5f}, {UV_X - UV_EDGE, UV_Y - UV_EDGE}, {0.0f, +1.0f, 0.0f}},
    // Second triangle
    {{.5 + +0.5f, .5 + +0.5f, .5 + +0.5f}, {UV_X - UV_EDGE, UV_Y - UV_EDGE}, {0.0f, +1.0f, 0.0f}},
    {{.5 + +0.5f, .5 + +0.5f, .5 + -0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {0.0f, +1.0f, 0.0f}},
    {{.5 + -0.5f, .5 + +0.5f, .5 + -0.5f}, {0.0f + UV_EDGE, 0.0f + UV_EDGE}, {0.0f, +1.0f, 0.0f}},

    // Sixth face (MY)
    // First triangle
    {{.5 + -0.5f, .5 + -0.5f, .5 + -0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {0.0f, -1.0f, 0.0f}},
    {{.5 + +0.5f, .5 + -0.5f, .5 + -0.5f}, {0.0f + UV_EDGE, 0.0f + UV_EDGE}, {0.0f, -1.0f, 0.0f}},
    {{.5 + +0.5f, .5 + -0.5f, .5 + +0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {0.0f, -1.0f, 0.0f}},

    // Second triangle
    {{.5 + +0.5f, .5 + -0.5f, .5 + +0.5f}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {0.0f, -1.0f, 0.0f}},
    {{.5 + -0.5f, .5 + -0.5f, .5 + +0.5f}, {UV_X - UV_EDGE, UV_Y - UV_EDGE}, {0.0f, -1.0f, 0.0f}},
    {{.5 + -0.5f, .5 + -0.5f, .5 + -0.5f}, {0.0f + UV_EDGE, UV_Y - UV_EDGE}, {0.0f, -1.0f, 0.0f}},
};

static bool loadTextureFromMem(C3D_Tex *tex, C3D_TexCube *cube, const void *data, size_t size) {
    Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, false);
    if (!t3x)
        return false;

    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);
    return true;
}

#define OVERLAY_OFFSET 0.001
void meshAddFace(Mesh *m, int face, int x, int y, int z, int texX, int texY, int overlay) {
    if (m->nVertex >= CHUNK_MAX_VERTEX_COUNT)
        return;
    memcpy(m->vertices + m->nVertex, cube_faces + 6 * face, 6 * sizeof(vertex));
    for (int i = 0; i < 6; i++) {
        m->vertices[m->nVertex + i].position[0] += x + OVERLAY_OFFSET * overlay * m->vertices[m->nVertex + i].normal[0];
        m->vertices[m->nVertex + i].position[1] += y + OVERLAY_OFFSET * overlay * m->vertices[m->nVertex + i].normal[1];
        m->vertices[m->nVertex + i].position[2] += z + OVERLAY_OFFSET * overlay * m->vertices[m->nVertex + i].normal[2];
        m->vertices[m->nVertex + i].texcoord[1] += (TEXTURE_HEIGHT - 1 - texY) * UV_Y;
        m->vertices[m->nVertex + i].texcoord[0] += texX * UV_X;
    }
    m->nVertex += 6;
}

void meshInit(Mesh *m) {
    m->vertices = linearAlloc((CHUNK_MAX_VERTEX_COUNT * MAX_VISIBLE_CHUNKS + CAT_VERTEX_COUNT) * sizeof(vertex));
    m->nVertex  = 0;
}

#define ADD_FACES(texX, topTexX, bottomTexX, texY, overlay) \
    { \
        if (yy == 0 || !chunk[W(x, yy - 1, z)]) \
            meshAddFace(m, FACE_MY, x, yy, z, (bottomTexX), (texY), overlay); \
        if (yy == CHUNK_HEIGHT - 1 || !chunk[W(x, yy + 1, z)]) \
            meshAddFace(m, FACE_PY, x, yy, z, (topTexX), (texY), overlay); \
        if (x == 0 || !chunk[W(x - 1, yy, z)]) \
            meshAddFace(m, FACE_MX, x, yy, z, (texX), (texY), overlay); \
        if (x == WORLD_WIDTH - 1 || !chunk[W(x + 1, yy, z)]) \
            meshAddFace(m, FACE_PX, x, yy, z, (texX), (texY), overlay); \
        if (z == 0 || !chunk[W(x, yy, z - 1)]) \
            meshAddFace(m, FACE_MZ, x, yy, z, (texX), (texY), overlay); \
        if (z == WORLD_WIDTH - 1 || !chunk[W(x, yy, z + 1)]) \
            meshAddFace(m, FACE_PZ, x, yy, z, (texX), (texY), overlay); \
    }

int blockIdOffset(int y) {
    if (y == WORLD_HEIGHT - 1) {
        return 1;
    }
    if (y > 8) {
        return 2;
    }
    return 3;
}

void meshBuild(Mesh *m, u32 *chunk) {
    m->nVertex = 0;

    for (int yy = 0; yy < CHUNK_HEIGHT; yy++) {
        int o       = blockIdOffset(yy);
        int topO    = (o == 1) ? 0 : o;
        int bottomO = (o == 1) ? 2 : o;
        for (int x = 0; x < WORLD_WIDTH; x++) {
            for (int z = 0; z < WORLD_WIDTH; z++) {
                int b = chunk[W(x, yy, z)] & MASK_BLOCK;
                if (b)
                    ADD_FACES(b + o - 1, b + topO - 1, b + bottomO - 1, 0, 0);
            }
        }
    }
    m->nSolidVertex = m->nVertex;

    for (int yy = 0; yy < CHUNK_HEIGHT; yy++) {
        int o    = blockIdOffset(yy);
        int topO = (o == 1) ? 0 : o;
        for (int x = 0; x < WORLD_WIDTH; x++) {
            for (int z = 0; z < WORLD_WIDTH; z++) {
                int damage = visibleDamage(chunk[W(x, yy, z)]);
                if (damage) {
                    ADD_FACES(damage, damage, damage, 1, 1);
                }
            }
        }
    }
    m->nDamageVertex = m->nVertex - m->nSolidVertex;
}

void meshUpdateSelected(Mesh *m, u32 *chunk, int selected, int x, int yy, int z) {
    m->nVertex = m->nSolidVertex + m->nDamageVertex;
    if (!selected) {
        return;
    }

    ADD_FACES(0, 0, 0, 1, 2);
}

void meshUpdateDamage(Mesh *m, u32 *chunk, int x, int yy, int z, int oldDamage, int damage) {
    if (!damage) {
        return;
    }
    if (oldDamage >= damage)
        return;
    int difference = damage - oldDamage;
    if (oldDamage) {
        for (int i = m->nSolidVertex; i < m->nSolidVertex + m->nDamageVertex; i += 6) {
            float avgX = (m->vertices[i].position[0] + m->vertices[i + 1].position[0] + m->vertices[i + 2].position[0]) / 3.0;
            float avgY = (m->vertices[i].position[1] + m->vertices[i + 1].position[1] + m->vertices[i + 2].position[1]) / 3.0;
            float avgZ = (m->vertices[i].position[2] + m->vertices[i + 1].position[2] + m->vertices[i + 2].position[2]) / 3.0;
            float cX   = avgX - 0.5 * m->vertices[i].normal[0];
            float cY   = avgY - 0.5 * m->vertices[i].normal[1];
            float cZ   = avgZ - 0.5 * m->vertices[i].normal[2];
            if ((int)cX == x && (int)cY == yy && (int)cZ == z) {
                for (int j = 0; j < 6; j++) {
                    m->vertices[i + j].texcoord[0] += UV_X * difference;
                }
            }
        }
    } else {
        m->nVertex = m->nSolidVertex + m->nDamageVertex;
        ADD_FACES(damage, damage, damage, 1, 1);
        m->nDamageVertex = m->nVertex - m->nSolidVertex;
    }
}

void renderInit(Game *g) {
    gfxInitDefault();
    gfxSet3D(true);
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    g->leftTarget = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(g->leftTarget, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    g->rightTarget = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(g->rightTarget, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

    // Load the vertex shader, create a shader program and bind it
    vshader_dvlb = DVLB_ParseFile((u32 *)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    C3D_BindProgram(&program);

    // Get the location of the uniforms
    uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
    uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
    uLoc_lightVec     = shaderInstanceGetUniformLocation(program.vertexShader, "lightVec");
    uLoc_lightHalfVec = shaderInstanceGetUniformLocation(program.vertexShader, "lightHalfVec");
    uLoc_lightClr     = shaderInstanceGetUniformLocation(program.vertexShader, "lightClr");
    uLoc_material     = shaderInstanceGetUniformLocation(program.vertexShader, "material");

    // Configure attributes for use with the vertex shader
    C3D_AttrInfo *attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2=normal

    meshInit(&g->mesh);

    // Configure buffers
    C3D_BufInfo *bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, g->mesh.vertices, sizeof(vertex), 3, 0x210);

    if (!loadTextureFromMem(&texture_tex, NULL, texture_t3x, texture_t3x_size))
        svcBreak(USERBREAK_PANIC);
    C3D_TexBind(0, &texture_tex);
    C3D_TexSetFilter(&texture_tex, GPU_NEAREST, GPU_NEAREST);

    // Configure the first fragment shading substage to blend the texture color with
    // the vertex color (calculated by the vertex shader using a lighting algorithm)
    // See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
    C3D_TexEnv *env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
}

void renderScreen(Game *g, C3D_RenderTarget *target, float iod) {
    Mtx_PerspStereoTilt(&projection, C3D_AngleFromDegrees(85.0f), C3D_AspectRatioTop, 0.01f, 1000.0f, iod, 2.0f, false);

    C3D_RenderTargetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
    C3D_FrameDrawOn(target);

    C3D_Mtx modelView;

    Mtx_Identity(&modelView);
    Mtx_RotateX(&modelView, -g->camera.pitch, true);
    Mtx_RotateY(&modelView, -g->camera.yaw, true);
    Mtx_Translate(&modelView, -g->camera.x, -g->camera.y, -g->camera.z, true);

    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_material, &material);
    C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightVec, 0.0f, 0.0f, -1.0f, 0.0f);
    C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightHalfVec, 0.0f, 0.0f, -1.0f, 0.0f);
    C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightClr, 1.0f, 1.0f, 1.0f, 1.0f);

    // Draw the VBO
    C3D_DrawArrays(GPU_TRIANGLES, 0, g->mesh.nVertex);

    g->mesh.vertices[MAX_VISIBLE_CHUNKS * CHUNK_MAX_VERTEX_COUNT]     = (vertex){{0, 16, 0}, {0.0f + UV_EDGE, 0.0f + UV_EDGE}, {0.0f, 0.0f, +1.0f}};
    g->mesh.vertices[MAX_VISIBLE_CHUNKS * CHUNK_MAX_VERTEX_COUNT + 1] = (vertex){{16, 16, 0}, {UV_X - UV_EDGE, 0.0f + UV_EDGE}, {0.0f, 0.0f, +1.0f}};
    g->mesh.vertices[MAX_VISIBLE_CHUNKS * CHUNK_MAX_VERTEX_COUNT + 2] = (vertex){{16, 32, 0}, {UV_X - UV_EDGE, UV_Y - UV_EDGE}, {0.0f, 0.0f, +1.0f}};
    C3D_DrawArrays(GPU_TRIANGLES, MAX_VISIBLE_CHUNKS * CHUNK_MAX_VERTEX_COUNT, CAT_VERTEX_COUNT);
}

void renderFrame(Game *g) {
    float slider = osGet3DSliderState();
    float iod    = slider / 3;
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    renderScreen(g, g->leftTarget, -iod);
    if (iod > 0.0f) {
        renderScreen(g, g->rightTarget, iod);
    }
    C3D_FrameEnd(0);
}

void renderExit(Game *g) {
    // Free the texture
    C3D_TexDelete(&texture_tex);
    C3D_TexDelete(&cat_tex);

    // Free the VBO
    linearFree(g->mesh.vertices);

    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);

    C3D_Fini();
    gfxExit();
}
